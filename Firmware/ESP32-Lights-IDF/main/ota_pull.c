#include "ota_pull.h"

#include <stdio.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ota_pull";
static bool s_running;
static char s_url[256];

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

    ESP_LOGI(TAG, "Starting OTA from %s", s_url);
    esp_err_t err = esp_https_ota(&ota_cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "OTA success, restarting");
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();
    }

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
