#include "ota_pull.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "leds.h"

static const char *TAG = "ota_pull";
static bool s_running;
static char s_url[256];

static bool ends_with(const char *s, const char *suffix) {
    size_t ls;
    size_t le;

    if (!s || !suffix) {
        return false;
    }

    ls = strlen(s);
    le = strlen(suffix);
    if (ls < le) {
        return false;
    }
    return strcmp(s + (ls - le), suffix) == 0;
}

static void trim_inplace(char *s) {
    char *start;
    char *end;

    if (!s || !s[0]) {
        return;
    }

    start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *end = '\0';

    // Drop UTF-8 BOM if present.
    if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) {
        memmove(s, s + 3, strlen(s + 3) + 1);
    }
}

static int parse_version_parts(const char *ver, int parts[4], int *count) {
    char tmp[32];
    char *saveptr = NULL;
    char *tok;
    int i = 0;

    if (!ver || !ver[0] || !parts || !count) {
        return -1;
    }

    snprintf(tmp, sizeof(tmp), "%s", ver);
    trim_inplace(tmp);
    if (tmp[0] == 'v' || tmp[0] == 'V') {
        memmove(tmp, tmp + 1, strlen(tmp + 1) + 1);
    }
    tok = strtok_r(tmp, ".", &saveptr);
    while (tok && i < 4) {
        int j;
        for (j = 0; tok[j]; ++j) {
            if (!isdigit((unsigned char)tok[j])) {
                return -1;
            }
        }
        parts[i++] = atoi(tok);
        tok = strtok_r(NULL, ".", &saveptr);
    }

    if (i == 0) {
        return -1;
    }
    while (i < 4) {
        parts[i++] = 0;
    }
    *count = 4;
    return 0;
}

static int compare_versions(const char *a, const char *b) {
    int pa[4] = {0};
    int pb[4] = {0};
    int ca = 0;
    int cb = 0;

    if (parse_version_parts(a, pa, &ca) == 0 && parse_version_parts(b, pb, &cb) == 0) {
        for (int i = 0; i < 4; ++i) {
            if (pa[i] < pb[i]) return -1;
            if (pa[i] > pb[i]) return 1;
        }
        return 0;
    }

    return strcmp(a ? a : "", b ? b : "");
}

static esp_err_t fetch_text(const char *url, char *out, size_t out_len) {
    esp_http_client_config_t cfg = {
        .url = url,
        .timeout_ms = 15000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client;
    int total = 0;
    int status = 0;

    if (!url || !out || out_len < 2) {
        return ESP_ERR_INVALID_ARG;
    }

    out[0] = '\0';
    client = esp_http_client_init(&cfg);
    if (!client) {
        return ESP_FAIL;
    }

    if (esp_http_client_open(client, 0) != ESP_OK) {
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    // Pull response headers so status code is available before body parsing.
    (void)esp_http_client_fetch_headers(client);
    status = esp_http_client_get_status_code(client);
    if (status < 200 || status >= 300) {
        ESP_LOGW(TAG, "Manifest fetch HTTP status %d from %s", status, url);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    while (1) {
        int n = esp_http_client_read(client, out + total, (int)out_len - 1 - total);
        if (n < 0) {
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        }
        if (n == 0) {
            break;
        }
        total += n;
        if ((size_t)total >= out_len - 1) {
            break;
        }
    }

    out[total] = '\0';
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ESP_OK;
}

static void split_manifest_url(const char *input_url, char *manifest_url, size_t manifest_url_len, char *fallback_bin_url, size_t fallback_bin_url_len) {
    const char *last_slash;

    manifest_url[0] = '\0';
    fallback_bin_url[0] = '\0';

    if (ends_with(input_url, ".txt")) {
        snprintf(manifest_url, manifest_url_len, "%s", input_url);
        last_slash = strrchr(input_url, '/');
        if (last_slash) {
            int base_len = (int)(last_slash - input_url + 1);
            if (base_len > 0 && (size_t)base_len < fallback_bin_url_len) {
                memcpy(fallback_bin_url, input_url, (size_t)base_len);
                fallback_bin_url[base_len] = '\0';
                strncat(fallback_bin_url, "firmware.bin", fallback_bin_url_len - strlen(fallback_bin_url) - 1);
            }
        }
        return;
    }

    if (ends_with(input_url, ".bin")) {
        snprintf(fallback_bin_url, fallback_bin_url_len, "%s", input_url);
        last_slash = strrchr(input_url, '/');
        if (last_slash) {
            int base_len = (int)(last_slash - input_url + 1);
            if (base_len > 0 && (size_t)base_len < manifest_url_len) {
                memcpy(manifest_url, input_url, (size_t)base_len);
                manifest_url[base_len] = '\0';
                strncat(manifest_url, "latest.txt", manifest_url_len - strlen(manifest_url) - 1);
            }
        }
        return;
    }

    snprintf(fallback_bin_url, fallback_bin_url_len, "%s", input_url);
}

static void parse_manifest(const char *text, char *out_version, size_t out_version_len, char *out_url, size_t out_url_len) {
    char buf[512];
    char *saveptr = NULL;
    char *line;

    out_version[0] = '\0';
    out_url[0] = '\0';

    snprintf(buf, sizeof(buf), "%s", text ? text : "");
    line = strtok_r(buf, "\r\n", &saveptr);
    while (line) {
        char *eq;
        trim_inplace(line);
        if (line[0] && line[0] != '#') {
            eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char *key = line;
                char *val = eq + 1;
                trim_inplace(key);
                trim_inplace(val);
                if (strcasecmp(key, "version") == 0) {
                    snprintf(out_version, out_version_len, "%s", val);
                } else if (strcasecmp(key, "url") == 0) {
                    snprintf(out_url, out_url_len, "%s", val);
                }
            } else if (!out_version[0]) {
                // Backward-compatible one-line manifest: "2.1.0"
                snprintf(out_version, out_version_len, "%s", line);
            } else if (!out_url[0]) {
                // Backward-compatible second line as URL
                snprintf(out_url, out_url_len, "%s", line);
            }
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    trim_inplace(out_version);
    trim_inplace(out_url);
}

esp_err_t ota_pull_resolve_target(
    const char *input_url,
    const char *current_version,
    char *out_bin_url,
    size_t out_bin_url_len,
    char *out_new_version,
    size_t out_new_version_len,
    bool *out_has_update
) {
    char manifest_url[256] = {0};
    char fallback_bin_url[256] = {0};
    char manifest_text[512] = {0};
    char manifest_version[32] = {0};
    char manifest_bin_url[256] = {0};
    esp_err_t err;

    if (!input_url || !current_version || !out_bin_url || !out_new_version || !out_has_update) {
        return ESP_ERR_INVALID_ARG;
    }

    *out_has_update = true;
    snprintf(out_bin_url, out_bin_url_len, "%s", input_url);
    snprintf(out_new_version, out_new_version_len, "%s", current_version);

    split_manifest_url(input_url, manifest_url, sizeof(manifest_url), fallback_bin_url, sizeof(fallback_bin_url));

    if (!manifest_url[0]) {
        ESP_LOGW(TAG, "No manifest URL pattern for OTA URL, proceeding directly");
        return ESP_OK;
    }

    err = fetch_text(manifest_url, manifest_text, sizeof(manifest_text));
    if (err != ESP_OK) {
        if (ends_with(input_url, ".txt")) {
            ESP_LOGE(TAG, "Failed to fetch required manifest %s", manifest_url);
            return err;
        }
        ESP_LOGW(TAG, "Manifest fetch failed (%s), proceeding with direct OTA URL", esp_err_to_name(err));
        return ESP_OK;
    }

    parse_manifest(manifest_text, manifest_version, sizeof(manifest_version), manifest_bin_url, sizeof(manifest_bin_url));
    if (!manifest_version[0]) {
        ESP_LOGW(
            TAG,
            "Manifest %s has no version (content preview: '%.80s'), proceeding with direct OTA URL",
            manifest_url,
            manifest_text
        );
        return ESP_OK;
    }

    snprintf(out_new_version, out_new_version_len, "%s", manifest_version);
    if (compare_versions(manifest_version, current_version) <= 0) {
        *out_has_update = false;
        ESP_LOGI(TAG, "OTA skipped: current=%s latest=%s", current_version, manifest_version);
        return ESP_OK;
    }

    if (manifest_bin_url[0]) {
        snprintf(out_bin_url, out_bin_url_len, "%s", manifest_bin_url);
    } else if (fallback_bin_url[0]) {
        snprintf(out_bin_url, out_bin_url_len, "%s", fallback_bin_url);
    }

    ESP_LOGI(TAG, "OTA update available: current=%s latest=%s", current_version, manifest_version);
    ESP_LOGI(TAG, "OTA binary URL: %s", out_bin_url);
    return ESP_OK;
}

static void ota_task(void *arg) {
    (void)arg;

    esp_http_client_config_t http_cfg = {
        .url = s_url,
        .timeout_ms = 20000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_https_ota_config_t ota_cfg = {
        .http_config = &http_cfg,
    };

    leds_set_ota_state(LEDS_OTA_STATE_IN_PROGRESS);
    ESP_LOGI(TAG, "Starting OTA from %s", s_url);
    esp_err_t err = esp_https_ota(&ota_cfg);
    if (err == ESP_OK) {
        leds_set_ota_state(LEDS_OTA_STATE_SUCCESS);
        ESP_LOGI(TAG, "OTA success, restarting");
        vTaskDelay(pdMS_TO_TICKS(800));
        esp_restart();
    }

    leds_set_ota_state(LEDS_OTA_STATE_FAILED);
    ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(err));
    s_running = false;
    vTaskDelete(NULL);
}

esp_err_t ota_pull_start(const char *url) {
    if (!url || strlen(url) < 8) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_running) {
        return ESP_ERR_INVALID_STATE;
    }

    s_running = true;
    snprintf(s_url, sizeof(s_url), "%s", url);
    BaseType_t ok = xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
    if (ok != pdPASS) {
        s_running = false;
        return ESP_FAIL;
    }
    return ESP_OK;
}
