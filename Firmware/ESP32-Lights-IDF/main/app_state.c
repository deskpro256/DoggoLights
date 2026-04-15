#include "app_state.h"

#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static device_config_t s_cfg;
static runtime_state_t s_runtime;
static SemaphoreHandle_t s_lock;

void app_state_init(void) {
    s_lock = xSemaphoreCreateMutex();
    memset(&s_cfg, 0, sizeof(s_cfg));
    memset(&s_runtime, 0, sizeof(s_runtime));
    s_runtime.active_preset = 0;
    s_runtime.powered_on = true;
}

void app_state_get_config(device_config_t *out_cfg) {
    if (!out_cfg || !s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out_cfg = s_cfg;
    xSemaphoreGive(s_lock);
}

void app_state_set_config(const device_config_t *cfg) {
    if (!cfg || !s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_cfg = *cfg;
    xSemaphoreGive(s_lock);
}

void app_state_update_config_preset(int preset_idx, int effect, int hue1, int hue2) {
    if (!s_lock || preset_idx < 0 || preset_idx > 2) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_cfg.presets[preset_idx].effect = effect;
    s_cfg.presets[preset_idx].hue1 = hue1;
    s_cfg.presets[preset_idx].hue2 = hue2;
    xSemaphoreGive(s_lock);
}

void app_state_get_runtime(runtime_state_t *out_state) {
    if (!out_state || !s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out_state = s_runtime;
    xSemaphoreGive(s_lock);
}

void app_state_set_runtime(const runtime_state_t *state) {
    if (!state || !s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_runtime = *state;
    xSemaphoreGive(s_lock);
}

void app_state_set_battery(int percent) {
    if (!s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_runtime.battery_percent = percent;
    xSemaphoreGive(s_lock);
}

void app_state_set_wifi_ap_running(bool running) {
    if (!s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_runtime.wifi_ap_running = running;
    xSemaphoreGive(s_lock);
}

void app_state_touch_web_activity(void) {
    if (!s_lock) {
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_runtime.last_web_activity_ms = esp_timer_get_time() / 1000ULL;
    xSemaphoreGive(s_lock);
}
