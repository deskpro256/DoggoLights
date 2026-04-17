#include "web_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_config.h"
#include "app_state.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "freertos/event_groups.h"
#include "leds.h"
#include "ota_pull.h"
#include "storage.h"

static const char *TAG = "web";
static const char *HOST_HINT = "doggylights.local";
static const int AP_RESTART_GRACE_MS = 1000;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAILED_BIT BIT1

static httpd_handle_t s_server;
static bool s_wifi_inited;
static bool s_ap_running;
static bool s_ota_sequence_running;
static esp_netif_t *s_ap_netif;
static esp_netif_t *s_sta_netif;
static EventGroupHandle_t s_wifi_events;
static char s_ap_runtime_ssid[33];

typedef struct {
    char ssid[33];
    char password[65];
    char url[256];
    char current_version[32];
} ota_sequence_request_t;

typedef struct {
    bool on;
} ap_toggle_request_t;

static void ap_restart_task(void *arg);

static size_t copy_bounded(char *dst, size_t dst_size, const char *src) {
    size_t n;

    if (dst_size == 0) {
        return 0;
    }

    n = strnlen(src, dst_size - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
    return n;
}

static int from_hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static void url_decode_inplace(char *text) {
    char *src = text;
    char *dst = text;

    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            int hi = from_hex(src[1]);
            int lo = from_hex(src[2]);
            if (hi >= 0 && lo >= 0) {
                *dst++ = (char)((hi << 4) | lo);
                src += 3;
                continue;
            }
        }
        if (*src == '+') {
            *dst++ = ' ';
            ++src;
            continue;
        }
        *dst++ = *src++;
    }

    *dst = '\0';
}

static bool query_value_decoded(const char *query, const char *key, char *out, size_t out_size) {
    if (httpd_query_key_value(query, key, out, out_size) != ESP_OK) {
        return false;
    }
    url_decode_inplace(out);
    return true;
}

static esp_err_t send_json(httpd_req_t *req, const char *json) {
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static void touch_activity(void) {
    app_state_touch_web_activity();
}

static void build_runtime_ap_ssid(const char *base_ssid, char *out, size_t out_size) {
    static const char *k_default_ssid = "DoggoLights";
    uint8_t mac[6] = {0};
    char suffix[7] = {0};
    size_t suffix_len;
    size_t base_max;
    bool append_suffix;

    if (!out || out_size == 0) {
        return;
    }

    out[0] = '\0';

    if (!base_ssid || base_ssid[0] == '\0') {
        base_ssid = k_default_ssid;
    }

    // Only append MAC suffix for the default SSID.
    append_suffix = (strcmp(base_ssid, k_default_ssid) == 0);
    if (!append_suffix) {
        copy_bounded(out, out_size, base_ssid);
        return;
    }

    if (esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP) != ESP_OK) {
        copy_bounded(out, out_size, base_ssid);
        return;
    }

    snprintf(suffix, sizeof(suffix), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    suffix_len = strlen(suffix);
    base_max = (out_size - 1 > suffix_len) ? (out_size - 1 - suffix_len) : 0;

    if (base_max == 0) {
        copy_bounded(out, out_size, suffix);
        return;
    }

    copy_bounded(out, base_max + 1, base_ssid);
    strncat(out, suffix, out_size - strlen(out) - 1);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)arg;
    (void)event_data;

    if (!s_wifi_events) {
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupSetBits(s_wifi_events, WIFI_FAILED_BIT);
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

static bool connect_to_home_wifi(const char *ssid, const char *password, TickType_t timeout_ticks) {
    EventBits_t bits;
    wifi_config_t sta_cfg = {0};

    if (!ssid || !ssid[0]) {
        ESP_LOGW(TAG, "Home Wi-Fi SSID is not configured");
        return false;
    }

    xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT);

    copy_bounded((char *)sta_cfg.sta.ssid, sizeof(sta_cfg.sta.ssid), ssid);
    copy_bounded((char *)sta_cfg.sta.password, sizeof(sta_cfg.sta.password), password ? password : "");
    sta_cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    bits = xEventGroupWaitBits(
        s_wifi_events,
        WIFI_CONNECTED_BIT | WIFI_FAILED_BIT,
        pdTRUE,
        pdFALSE,
        timeout_ticks
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to home Wi-Fi for OTA");
        return true;
    }

    ESP_LOGW(TAG, "Failed to connect to home Wi-Fi for OTA");
    esp_wifi_disconnect();
    return false;
}

static void ota_sequence_task(void *arg) {
    ota_sequence_request_t *request = (ota_sequence_request_t *)arg;
    char resolved_url[256] = {0};
    char latest_version[32] = {0};
    bool has_update = true;

    web_server_stop_ap();

    if (!connect_to_home_wifi(request->ssid, request->password, pdMS_TO_TICKS(15000))) {
        s_ota_sequence_running = false;
        web_server_start_ap();
        free(request);
        vTaskDelete(NULL);
        return;
    }

    if (ota_pull_resolve_target(
            request->url,
            request->current_version,
            resolved_url,
            sizeof(resolved_url),
            latest_version,
            sizeof(latest_version),
            &has_update) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to resolve OTA target from %s", request->url);
        s_ota_sequence_running = false;
        web_server_start_ap();
        free(request);
        vTaskDelete(NULL);
        return;
    }

    if (!has_update) {
        ESP_LOGI(TAG, "OTA skipped: device already up to date (current=%s latest=%s)", request->current_version, latest_version);
        s_ota_sequence_running = false;
        web_server_start_ap();
        free(request);
        vTaskDelete(NULL);
        return;
    }

    if (ota_pull_start(resolved_url) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start OTA task");
        s_ota_sequence_running = false;
        web_server_start_ap();
        free(request);
        vTaskDelete(NULL);
        return;
    }

    free(request);
    vTaskDelete(NULL);
}

static esp_err_t root_get(httpd_req_t *req) {
    touch_activity();

    FILE *f = fopen("/spiffs/index.html", "r");
    if (!f) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "index.html not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");
    char buf[512];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
            fclose(f);
            httpd_resp_sendstr_chunk(req, NULL);
            return ESP_FAIL;
        }
    }
    fclose(f);
    return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t status_get(httpd_req_t *req) {
    touch_activity();

    runtime_state_t st;
    device_config_t cfg;
    char presets[192];
    app_state_get_runtime(&st);
    app_state_get_config(&cfg);

    snprintf(
        presets,
        sizeof(presets),
        "[{\"effect\":%d,\"hue1\":%d,\"hue2\":%d},{\"effect\":%d,\"hue1\":%d,\"hue2\":%d},{\"effect\":%d,\"hue1\":%d,\"hue2\":%d}]",
        cfg.presets[0].effect,
        cfg.presets[0].hue1,
        cfg.presets[0].hue2,
        cfg.presets[1].effect,
        cfg.presets[1].hue1,
        cfg.presets[1].hue2,
        cfg.presets[2].effect,
        cfg.presets[2].hue1,
        cfg.presets[2].hue2
    );

    char json[960];
    snprintf(
        json,
        sizeof(json),
        "{\"battery_percent\":%d,\"active_preset\":%d,\"wifi_ap_running\":%s,\"firmware_version\":\"%s\",\"ota_url\":\"%s\",\"ap_ssid\":\"%s\",\"ap_runtime_ssid\":\"%s\",\"home_ssid\":\"%s\",\"home_wifi_set\":%s,\"hostname_hint\":\"%s\",\"presets\":%s}",
        st.battery_percent,
        st.active_preset,
        st.wifi_ap_running ? "true" : "false",
        cfg.firmware_version,
        cfg.ota_url,
        cfg.ap_ssid,
        s_ap_runtime_ssid,
        cfg.home_ssid,
        cfg.home_wifi_set ? "true" : "false",
        HOST_HINT,
        presets
    );
    return send_json(req, json);
}

static esp_err_t preset_post(httpd_req_t *req) {
    touch_activity();

    char q[128] = {0};
    if (httpd_req_get_url_query_str(req, q, sizeof(q)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query");
        return ESP_FAIL;
    }

    char v[16];
    int idx = 0, effect = 1, h1 = 0, h2 = 0;
    if (query_value_decoded(q, "idx", v, sizeof(v))) idx = atoi(v);
    if (query_value_decoded(q, "effect", v, sizeof(v))) effect = atoi(v);
    if (query_value_decoded(q, "h1", v, sizeof(v))) h1 = atoi(v);
    if (query_value_decoded(q, "h2", v, sizeof(v))) h2 = atoi(v);

    if (idx < 0 || idx > 2) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "idx must be 0..2");
        return ESP_FAIL;
    }

    if (h1 < 0) h1 = 0;
    if (h1 > 360) h1 = 360;
    if (h2 < 0) h2 = 0;
    if (h2 > 360) h2 = 360;

    device_config_t cfg;
    app_state_get_config(&cfg);
    cfg.presets[idx].effect = effect;
    cfg.presets[idx].hue1 = h1;
    cfg.presets[idx].hue2 = h2;
    app_state_set_config(&cfg);
    storage_save_config(&cfg);
    leds_set_active_preset(idx);

    return send_json(req, "{\"ok\":true}");
}

static esp_err_t device_post(httpd_req_t *req) {
    char q[768] = {0};
    char value[256];
    bool changed = false;
    bool ap_settings_changed = false;
    bool ap_ssid_changed = false;
    bool ap_pass_changed = false;
    bool home_ssid_changed = false;
    bool home_pass_changed = false;
    bool ota_url_changed = false;
    device_config_t cfg;

    touch_activity();

    if (httpd_req_get_url_query_str(req, q, sizeof(q)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query");
        return ESP_FAIL;
    }

    app_state_get_config(&cfg);

    if (query_value_decoded(q, "ap_ssid", value, sizeof(value))) {
        if (strcmp(cfg.ap_ssid, value) != 0) {
            ap_settings_changed = true;
            ap_ssid_changed = true;
        }
        copy_bounded(cfg.ap_ssid, sizeof(cfg.ap_ssid), value);
        changed = true;
    }
    if (query_value_decoded(q, "ap_pass", value, sizeof(value))) {
        size_t ap_pass_len = strlen(value);
        if (ap_pass_len > 0 && ap_pass_len < 8) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "AP password must be empty (open AP) or at least 8 characters");
            return ESP_FAIL;
        }
        if (strcmp(cfg.ap_pass, value) != 0) {
            ap_settings_changed = true;
            ap_pass_changed = true;
        }
        copy_bounded(cfg.ap_pass, sizeof(cfg.ap_pass), value);
        changed = true;
    }
    if (query_value_decoded(q, "home_ssid", value, sizeof(value))) {
        if (strcmp(cfg.home_ssid, value) != 0) {
            home_ssid_changed = true;
        }
        copy_bounded(cfg.home_ssid, sizeof(cfg.home_ssid), value);
        cfg.home_wifi_set = cfg.home_ssid[0] != '\0';
        changed = true;
    }
    if (query_value_decoded(q, "home_pass", value, sizeof(value))) {
        if (strcmp(cfg.home_pass, value) != 0) {
            home_pass_changed = true;
        }
        copy_bounded(cfg.home_pass, sizeof(cfg.home_pass), value);
        changed = true;
    }
    if (query_value_decoded(q, "ota_url", value, sizeof(value))) {
        if (strcmp(cfg.ota_url, value) != 0) {
            ota_url_changed = true;
        }
        copy_bounded(cfg.ota_url, sizeof(cfg.ota_url), value);
        changed = true;
    }

    if (!changed) {
        return send_json(req, "{\"ok\":true,\"message\":\"no_changes\"}");
    }

    app_state_set_config(&cfg);
    storage_save_config(&cfg);
    ESP_LOGI(
        TAG,
        "API /api/device saved (ap_ssid=%s, ap_pass=%s, home_ssid=%s, home_pass=%s, ota_url=%s)",
        ap_ssid_changed ? "changed" : "same",
        ap_pass_changed ? "changed" : "same",
        home_ssid_changed ? "changed" : "same",
        home_pass_changed ? "changed" : "same",
        ota_url_changed ? "changed" : "same"
    );

    if (ap_settings_changed && s_ap_running) {
        if (xTaskCreate(ap_restart_task, "ap_restart", 4096, NULL, 4, NULL) != pdPASS) {
            ESP_LOGW(TAG, "Could not start AP restart task after AP settings change");
        } else {
            ESP_LOGI(TAG, "AP settings changed. Restarting AP to apply new SSID/password.");
        }
    }

    esp_err_t err = send_json(req, "{\"ok\":true,\"message\":\"saved\"}");
    if (err != ESP_OK) {
        // Settings are already persisted at this point. If the browser drops
        // connection (e.g. AP/client transition), treat it as non-fatal.
        ESP_LOGW(TAG, "Client disconnected before save response was delivered");
        return ESP_OK;
    }
    return ESP_OK;
}

static esp_err_t factory_reset_post(httpd_req_t *req) {
    touch_activity();

    device_config_t cfg;
    bool ap_was_running = s_ap_running;
    storage_factory_reset(&cfg);
    app_state_set_config(&cfg);
    leds_set_active_preset(0);

    build_runtime_ap_ssid(cfg.ap_ssid, s_ap_runtime_ssid, sizeof(s_ap_runtime_ssid));
    ESP_LOGW(TAG, "Factory reset applied. AP base restored and user credentials cleared.");

    if (ap_was_running) {
        if (xTaskCreate(ap_restart_task, "ap_restart", 4096, NULL, 4, NULL) != pdPASS) {
            ESP_LOGW(TAG, "Could not start AP restart task after factory reset");
        } else {
            ESP_LOGI(TAG, "Factory reset: restarting AP to apply default SSID/password");
        }
    }

    esp_err_t err = send_json(req, "{\"ok\":true,\"message\":\"factory_reset\"}");
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Client disconnected before factory reset response was delivered");
        return ESP_OK;
    }
    return ESP_OK;
}

static esp_err_t ota_post(httpd_req_t *req) {
    device_config_t cfg;
    ota_sequence_request_t *request;
    touch_activity();

    char q[512] = {0};
    char url[256] = {0};

    app_state_get_config(&cfg);
    if (httpd_req_get_url_query_str(req, q, sizeof(q)) == ESP_OK) {
        query_value_decoded(q, "url", url, sizeof(url));
    }

    if (url[0] != '\0') {
        copy_bounded(cfg.ota_url, sizeof(cfg.ota_url), url);
        app_state_set_config(&cfg);
        storage_save_config(&cfg);
    }

    if (!cfg.home_wifi_set || !cfg.home_ssid[0]) {
        leds_signal_error();
        ESP_LOGW(TAG, "OTA rejected: home Wi-Fi settings are required");
        httpd_resp_set_status(req, "400 Bad Request");
        return send_json(req, "{\"ok\":false,\"error\":\"home_wifi_required\",\"message\":\"Home Wi-Fi settings are required for OTA\"}");
    }

    if (s_ota_sequence_running) {
        leds_signal_error();
        httpd_resp_set_status(req, "409 Conflict");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"ok\":false,\"error\":\"ota_already_running\"}");
        return ESP_FAIL;
    }

    request = calloc(1, sizeof(*request));
    if (!request) {
        leds_signal_error();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Could not allocate OTA request");
        return ESP_FAIL;
    }

    copy_bounded(request->ssid, sizeof(request->ssid), cfg.home_ssid);
    copy_bounded(request->password, sizeof(request->password), cfg.home_pass);
    copy_bounded(request->url, sizeof(request->url), cfg.ota_url);
    copy_bounded(request->current_version, sizeof(request->current_version), cfg.firmware_version);

    s_ota_sequence_running = true;
    if (xTaskCreate(ota_sequence_task, "ota_sequence", 6144, request, 4, NULL) != pdPASS) {
        s_ota_sequence_running = false;
        free(request);
        leds_signal_error();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Could not start OTA sequence");
        return ESP_FAIL;
    }

    return send_json(req, "{\"ok\":true,\"message\":\"ota_sequence_started\"}");
}

static esp_err_t captive_redirect_get(httpd_req_t *req) {
    touch_activity();
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
    return httpd_resp_send(req, NULL, 0);
}

static void ap_toggle_task(void *arg) {
    ap_toggle_request_t *req = (ap_toggle_request_t *)arg;
    bool want_on = req && req->on;

    if (req) {
        free(req);
    }

    // Give HTTP response a moment to leave before tearing down the server.
    vTaskDelay(pdMS_TO_TICKS(60));
    ESP_LOGI(TAG, "AP toggle task executing: target=%s", want_on ? "ON" : "OFF");

    if (want_on) {
        web_server_start_ap();
        leds_signal_ap_status(true);
    } else {
        web_server_stop_ap();
        leds_signal_ap_status(false);
    }

    vTaskDelete(NULL);
}

static void ap_restart_task(void *arg) {
    (void)arg;

    // Give HTTP response/poll requests time to settle before disconnecting clients.
    vTaskDelay(pdMS_TO_TICKS(AP_RESTART_GRACE_MS));
    ESP_LOGI(TAG, "AP restart task executing");
    web_server_stop_ap();
    vTaskDelay(pdMS_TO_TICKS(250));
    web_server_start_ap();
    leds_signal_ap_status(true);

    vTaskDelete(NULL);
}

void web_server_stop_ap(void) {
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }

    if (s_ap_running) {
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_NULL);
        s_ap_running = false;
        app_state_set_wifi_ap_running(false);
        ESP_LOGI(TAG, "Config AP stopped");
    }
}

static esp_err_t wifi_ap_post(httpd_req_t *req) {
    touch_activity();

    char q[32] = {0};
    char v[8] = {0};
    bool want_on = false;
    if (httpd_req_get_url_query_str(req, q, sizeof(q)) == ESP_OK &&
        httpd_query_key_value(q, "on", v, sizeof(v)) == ESP_OK) {
        want_on = atoi(v) != 0;
        ESP_LOGI(TAG, "API /api/wifi_ap request: on=%d", want_on ? 1 : 0);

        ap_toggle_request_t *toggle_req = calloc(1, sizeof(*toggle_req));
        if (!toggle_req) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Could not allocate AP toggle request");
            return ESP_FAIL;
        }
        toggle_req->on = want_on;

        if (xTaskCreate(ap_toggle_task, "ap_toggle", 4096, toggle_req, 4, NULL) != pdPASS) {
            free(toggle_req);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Could not start AP toggle task");
            return ESP_FAIL;
        }
    } else {
        ESP_LOGW(TAG, "API /api/wifi_ap called without valid on=<0|1> query");
    }

    return send_json(req, "{\"ok\":true}");
}

void web_server_start_ap(void) {
    if (s_ap_running) {
        touch_activity();
        return;
    }

    device_config_t app_cfg;
    app_state_get_config(&app_cfg);

    size_t ap_pass_len = strlen(app_cfg.ap_pass);
    if (ap_pass_len > 0 && ap_pass_len < 8) {
        // WPA2 AP passwords must be >=8 chars. Recover gracefully from any
        // previously saved invalid value instead of crashing.
        ESP_LOGW(TAG, "Invalid AP password length (%u). Falling back to open AP.", (unsigned)ap_pass_len);
        app_cfg.ap_pass[0] = '\0';
        app_state_set_config(&app_cfg);
        storage_save_config(&app_cfg);
    }

    build_runtime_ap_ssid(app_cfg.ap_ssid, s_ap_runtime_ssid, sizeof(s_ap_runtime_ssid));

    wifi_config_t wifi_cfg = {0};
    wifi_cfg.ap.ssid_len = copy_bounded((char *)wifi_cfg.ap.ssid, sizeof(wifi_cfg.ap.ssid), s_ap_runtime_ssid);
    copy_bounded((char *)wifi_cfg.ap.password, sizeof(wifi_cfg.ap.password), app_cfg.ap_pass);
    wifi_cfg.ap.channel = 1;
    wifi_cfg.ap.max_connection = 4;
    wifi_cfg.ap.authmode = strlen((char *)wifi_cfg.ap.password) ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    httpd_config_t server_cfg = HTTPD_DEFAULT_CONFIG();
    server_cfg.max_uri_handlers = 16;
    server_cfg.uri_match_fn = httpd_uri_match_wildcard;
    server_cfg.stack_size = 8192;
    ESP_ERROR_CHECK(httpd_start(&s_server, &server_cfg));

    httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_get, .user_ctx = NULL};
    httpd_uri_t status = {.uri = "/api/status", .method = HTTP_GET, .handler = status_get, .user_ctx = NULL};
    httpd_uri_t preset = {.uri = "/api/preset", .method = HTTP_POST, .handler = preset_post, .user_ctx = NULL};
    httpd_uri_t device = {.uri = "/api/device", .method = HTTP_POST, .handler = device_post, .user_ctx = NULL};
    httpd_uri_t factory_reset = {.uri = "/api/factory_reset", .method = HTTP_POST, .handler = factory_reset_post, .user_ctx = NULL};
    httpd_uri_t ota = {.uri = "/api/ota", .method = HTTP_POST, .handler = ota_post, .user_ctx = NULL};
    httpd_uri_t wifi_ap = {.uri = "/api/wifi_ap", .method = HTTP_POST, .handler = wifi_ap_post, .user_ctx = NULL};
    httpd_uri_t captive = {.uri = "/*", .method = HTTP_GET, .handler = captive_redirect_get, .user_ctx = NULL};

    httpd_register_uri_handler(s_server, &root);
    httpd_register_uri_handler(s_server, &status);
    httpd_register_uri_handler(s_server, &preset);
    httpd_register_uri_handler(s_server, &device);
    httpd_register_uri_handler(s_server, &factory_reset);
    httpd_register_uri_handler(s_server, &ota);
    httpd_register_uri_handler(s_server, &wifi_ap);
    httpd_register_uri_handler(s_server, &captive);

    s_ap_running = true;
    app_state_set_wifi_ap_running(true);
    touch_activity();
    // Extra touch to ensure the activity timestamp is in the future before
    // the main loop's first timeout check after AP starts.
    vTaskDelay(pdMS_TO_TICKS(50));
    touch_activity();
    ESP_LOGI(TAG, "Config AP started: %s", s_ap_runtime_ssid);
}

bool web_server_ap_running(void) {
    return s_ap_running;
}

void web_server_init(void) {
    if (!s_wifi_inited) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        s_ap_netif = esp_netif_create_default_wifi_ap();
        s_sta_netif = esp_netif_create_default_wifi_sta();
        (void)s_ap_netif;
        (void)s_sta_netif;

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        s_wifi_events = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
        s_wifi_inited = true;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 8,
        .format_if_mount_failed = true,
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    // Pre-build runtime SSID so it is available before the AP is started.
    device_config_t app_cfg;
    app_state_get_config(&app_cfg);
    build_runtime_ap_ssid(app_cfg.ap_ssid, s_ap_runtime_ssid, sizeof(s_ap_runtime_ssid));
}

const char *web_server_get_runtime_ap_ssid(void) {
    return s_ap_runtime_ssid;
}
