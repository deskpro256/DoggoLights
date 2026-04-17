#include "storage.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "storage";

static void set_defaults(device_config_t *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    snprintf(cfg->ap_ssid, sizeof(cfg->ap_ssid), "DoggoLights");
    cfg->ap_pass[0] = '\0';
    cfg->prefer_backup_first = false;
    snprintf(cfg->firmware_version, sizeof(cfg->firmware_version), "v2.0.0");
    snprintf(cfg->ota_url, sizeof(cfg->ota_url), "%s", DL_DEFAULT_OTA_URL);
    cfg->presets[0] = (preset_t){ .effect = EFFECT_STATIC, .hue1 = 0, .hue2 = 0 };
    cfg->presets[1] = (preset_t){ .effect = EFFECT_BREATHING, .hue1 = 120, .hue2 = 120 };
    cfg->presets[2] = (preset_t){ .effect = EFFECT_RAINBOW, .hue1 = 0, .hue2 = 0 };
}

static void read_str(nvs_handle_t nvs, const char *key, char *out, size_t out_len, const char *fallback) {
    size_t len = out_len;
    esp_err_t err = nvs_get_str(nvs, key, out, &len);
    if (err != ESP_OK) {
        snprintf(out, out_len, "%s", fallback);
    }
}

void storage_load_or_default(device_config_t *cfg) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    set_defaults(cfg);

    nvs_handle_t nvs;
    err = nvs_open("doglights", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No config in NVS yet, using defaults");
        return;
    }

    read_str(nvs, "ap_ssid", cfg->ap_ssid, sizeof(cfg->ap_ssid), cfg->ap_ssid);
    read_str(nvs, "ap_pass", cfg->ap_pass, sizeof(cfg->ap_pass), cfg->ap_pass);
    read_str(nvs, "home_ssid", cfg->home_ssid, sizeof(cfg->home_ssid), "");
    read_str(nvs, "home_pass", cfg->home_pass, sizeof(cfg->home_pass), "");
    read_str(nvs, "backup_ssid", cfg->backup_ssid, sizeof(cfg->backup_ssid), "");
    read_str(nvs, "backup_pass", cfg->backup_pass, sizeof(cfg->backup_pass), "");
    read_str(nvs, "fw_ver", cfg->firmware_version, sizeof(cfg->firmware_version), cfg->firmware_version);
    read_str(nvs, "ota_url", cfg->ota_url, sizeof(cfg->ota_url), cfg->ota_url);

    uint8_t home_set = 0;
    if (nvs_get_u8(nvs, "home_set", &home_set) == ESP_OK) {
        cfg->home_wifi_set = home_set != 0;
    }
    uint8_t prefer_backup_first = 0;
    if (nvs_get_u8(nvs, "pref_bak_first", &prefer_backup_first) == ESP_OK) {
        cfg->prefer_backup_first = (prefer_backup_first != 0);
    }
    // Keep behavior robust across schema changes: infer from configured SSIDs.
    if (cfg->home_ssid[0] || cfg->backup_ssid[0]) {
        cfg->home_wifi_set = true;
    }

    for (int i = 0; i < 3; ++i) {
        char key_eff[8];
        char key_h1[8];
        char key_h2[8];
        snprintf(key_eff, sizeof(key_eff), "p%d_eff", i + 1);
        snprintf(key_h1, sizeof(key_h1), "p%d_h1", i + 1);
        snprintf(key_h2, sizeof(key_h2), "p%d_h2", i + 1);

        int32_t v = 0;
        if (nvs_get_i32(nvs, key_eff, &v) == ESP_OK) {
            cfg->presets[i].effect = (int)v;
        }
        if (nvs_get_i32(nvs, key_h1, &v) == ESP_OK) {
            cfg->presets[i].hue1 = (int)v;
        }
        if (nvs_get_i32(nvs, key_h2, &v) == ESP_OK) {
            cfg->presets[i].hue2 = (int)v;
        }
    }

    nvs_close(nvs);
}

void storage_save_config(const device_config_t *cfg) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("doglights", NVS_READWRITE, &nvs));

    ESP_ERROR_CHECK(nvs_set_str(nvs, "ap_ssid", cfg->ap_ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ap_pass", cfg->ap_pass));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "home_ssid", cfg->home_ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "home_pass", cfg->home_pass));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "backup_ssid", cfg->backup_ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "backup_pass", cfg->backup_pass));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "fw_ver", cfg->firmware_version));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ota_url", cfg->ota_url));
    ESP_ERROR_CHECK(nvs_set_u8(nvs, "home_set", cfg->home_wifi_set ? 1 : 0));
    ESP_ERROR_CHECK(nvs_set_u8(nvs, "pref_bak_first", cfg->prefer_backup_first ? 1 : 0));

    for (int i = 0; i < 3; ++i) {
        char key_eff[8];
        char key_h1[8];
        char key_h2[8];
        snprintf(key_eff, sizeof(key_eff), "p%d_eff", i + 1);
        snprintf(key_h1, sizeof(key_h1), "p%d_h1", i + 1);
        snprintf(key_h2, sizeof(key_h2), "p%d_h2", i + 1);
        ESP_ERROR_CHECK(nvs_set_i32(nvs, key_eff, cfg->presets[i].effect));
        ESP_ERROR_CHECK(nvs_set_i32(nvs, key_h1, cfg->presets[i].hue1));
        ESP_ERROR_CHECK(nvs_set_i32(nvs, key_h2, cfg->presets[i].hue2));
    }

    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
}

void storage_factory_reset(device_config_t *cfg_out) {
    device_config_t cfg;
    set_defaults(&cfg);
    storage_save_config(&cfg);

    if (cfg_out) {
        *cfg_out = cfg;
    }
}

bool storage_load_mfg_passed(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    nvs_handle_t nvs;
    err = nvs_open("doglights", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        return false;
    }

    uint8_t passed = 0;
    if (nvs_get_u8(nvs, "mfg_passed", &passed) != ESP_OK) {
        nvs_close(nvs);
        return false;
    }

    nvs_close(nvs);
    return passed != 0;
}

void storage_save_mfg_passed(bool passed) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("doglights", NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_u8(nvs, "mfg_passed", passed ? 1 : 0));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
}
