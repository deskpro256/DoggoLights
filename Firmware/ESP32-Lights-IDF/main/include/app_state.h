#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    EFFECT_STATIC = 1,
    EFFECT_BREATHING = 2,
    EFFECT_FLIPFLOP = 3,
    EFFECT_RAINBOW = 4,
} effect_t;

typedef struct {
    int effect;
    int hue1;
    int hue2;
} preset_t;

typedef struct {
    char ap_ssid[33];
    char ap_pass[65];
    char home_ssid[33];
    char home_pass[65];
    char backup_ssid[33];
    char backup_pass[65];
    bool prefer_backup_first;
    char firmware_version[16];
    char ota_url[256];
    bool home_wifi_set;
    preset_t presets[3];
} device_config_t;

typedef struct {
    bool powered_on;
    bool wifi_ap_running;
    int active_preset;
    int battery_percent;
    uint64_t last_web_activity_ms;
} runtime_state_t;

void app_state_init(void);
void app_state_get_config(device_config_t *out_cfg);
void app_state_set_config(const device_config_t *cfg);
void app_state_update_config_preset(int preset_idx, int effect, int hue1, int hue2);

void app_state_get_runtime(runtime_state_t *out_state);
void app_state_set_runtime(const runtime_state_t *state);
void app_state_set_battery(int percent);
void app_state_set_wifi_ap_running(bool running);
void app_state_touch_web_activity(void);
