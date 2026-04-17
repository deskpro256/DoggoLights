#include <stdbool.h>
#include <string.h>

#include "app_config.h"
#include "app_state.h"
#include "battery.h"
#include "button.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_ota_ops.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "leds.h"
#include "rpc.h"
#include "storage.h"
#include "web_server.h"

static const char *TAG = "app";

static void sync_firmware_version(device_config_t *cfg) {
    const esp_app_desc_t *desc = esp_app_get_description();

    if (!cfg || !desc || !desc->version[0]) {
        return;
    }

    if (strcmp(cfg->firmware_version, desc->version) != 0) {
        size_t n = strnlen(desc->version, sizeof(cfg->firmware_version) - 1);
        ESP_LOGI(TAG, "Updating stored firmware version: %s -> %s", cfg->firmware_version, desc->version);
        memcpy(cfg->firmware_version, desc->version, n);
        cfg->firmware_version[n] = '\0';
        storage_save_config(cfg);
    }
}

static const char *effect_name(int e) {
    switch (e) {
        case 1: return "Static";
        case 2: return "Breathing";
        case 3: return "FlipFlop";
        case 4: return "Rainbow";
        default: return "Unknown";
    }
}

static void log_boot_info(void) {
    device_config_t cfg;
    app_state_get_config(&cfg);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);

    int adc_mv  = battery_read_adc_mv();
    int bat_mv  = battery_read_millivolts();
    int bat_pct = battery_read_percent();

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  DoggoLights IDF %s", cfg.firmware_version);
    ESP_LOGI(TAG, "  MAC  : %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "  Battery  : %d mV (ADC pin: %d mV) -> %d%%",
             bat_mv, adc_mv, bat_pct);
    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "  AP SSID  : %s", web_server_get_runtime_ap_ssid());
    ESP_LOGI(TAG, "  AP Pass  : %s", cfg.ap_pass[0] ? "(set)" : "(open)");
    ESP_LOGI(TAG, "  Primary WiFi: %s", cfg.home_ssid[0] ? cfg.home_ssid : "(not configured)");
    ESP_LOGI(TAG, "  Backup WiFi : %s", cfg.backup_ssid[0] ? cfg.backup_ssid : "(not configured)");
    ESP_LOGI(TAG, "----------------------------------------");
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "  Preset %d : effect=%-10s  hue1=%3d  hue2=%3d",
                 i + 1,
                 effect_name(cfg.presets[i].effect),
                 cfg.presets[i].hue1,
                 cfg.presets[i].hue2);
    }
    ESP_LOGI(TAG, "========================================");
}

static void ota_boot_confirm_if_pending(void) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;

    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK) {
        return;
    }

    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
        esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "OTA image confirmed valid; rollback canceled");
        } else {
            ESP_LOGE(TAG, "Failed to confirm OTA image: %s", esp_err_to_name(err));
        }
    }
}

static void enter_sleep(void) {
    // Turn LEDs off immediately so the user sees instant feedback at the 4s mark
    leds_set_power(false);

    web_server_stop_ap();

    runtime_state_t st;
    app_state_get_runtime(&st);
    st.powered_on = false;
    app_state_set_runtime(&st);

    // LONG fires while the button is still held — wait for release so the
    // GPIO wakeup trigger (LOW) doesn't fire the instant we enter deep sleep.
    while (gpio_get_level(DL_BUTTON_GPIO) == 0) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // debounce

    esp_deep_sleep_enable_gpio_wakeup(1ULL << DL_BUTTON_GPIO, ESP_GPIO_WAKEUP_GPIO_LOW);
    esp_deep_sleep_start();
}

static void on_button(button_event_t event) {
    switch (event) {
        case BUTTON_EVENT_SINGLE:
            leds_next_preset();
            {
                runtime_state_t st;
                app_state_get_runtime(&st);
                ESP_LOGI(TAG, "Button SINGLE: switched to preset %d", st.active_preset + 1);
            }
            break;
        case BUTTON_EVENT_DOUBLE:
            if (web_server_network_running()) {
                ESP_LOGI(TAG, "Button DOUBLE: stopping WiFi (AP/STA)");
                web_server_stop_ap();
                leds_signal_ap_status(false);
            } else {
                ESP_LOGI(TAG, "Button DOUBLE: starting preferred WiFi (saved networks first, AP fallback)");
                bool ap_started = web_server_start_preferred_network();
                leds_signal_ap_status(ap_started);
            }
            break;
        case BUTTON_EVENT_LONG:
            ESP_LOGI(TAG, "Button LONG: entering deep sleep");
            enter_sleep();
            break;
        default:
            ESP_LOGW(TAG, "Button event unknown: %d", (int)event);
            break;
    }
}

void app_main(void) {
    // If woken from deep sleep by the button, require it to be held for
    // DL_WAKE_PRESS_MS (1 s) before booting. Accidental touches go back to sleep.
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
        gpio_set_pull_mode(DL_BUTTON_GPIO, GPIO_PULLUP_ONLY);
        int64_t t0 = esp_timer_get_time() / 1000;
        while ((esp_timer_get_time() / 1000 - t0) < DL_WAKE_PRESS_MS) {
            if (gpio_get_level(DL_BUTTON_GPIO) != 0) {
                // Released early — go back to deep sleep
                esp_deep_sleep_enable_gpio_wakeup(1ULL << DL_BUTTON_GPIO, ESP_GPIO_WAKEUP_GPIO_LOW);
                esp_deep_sleep_start();
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    ota_boot_confirm_if_pending();

    app_state_init();

    device_config_t cfg;
    storage_load_or_default(&cfg);
    sync_firmware_version(&cfg);
    app_state_set_config(&cfg);

    runtime_state_t st = {
        .powered_on = true,
        .wifi_ap_running = false,
        .active_preset = 0,
        .battery_percent = 100,
        .last_web_activity_ms = 0,
    };
    app_state_set_runtime(&st);

    battery_init();
    int battery = battery_read_percent();
    app_state_set_battery(battery);

    leds_init();
    leds_show_battery_level(battery);

    web_server_init();
    button_init(on_button);
    rpc_init();

    log_boot_info();

    while (1) {
        int pct = battery_read_percent();
        app_state_set_battery(pct);

        if (pct <= 5) {
            ESP_LOGW(TAG, "Battery critical (%d%%) -> sleep", pct);
            enter_sleep();
        }

        runtime_state_t r;
        app_state_get_runtime(&r);
        if (r.wifi_ap_running) {
            uint64_t now_ms = esp_timer_get_time() / 1000ULL;
            if ((now_ms - r.last_web_activity_ms) > DL_AP_AUTO_OFF_MS) {
                ESP_LOGI(TAG, "Auto-stopping AP due to inactivity");
                web_server_stop_ap();
                leds_signal_ap_status(false);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
