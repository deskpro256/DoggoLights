#include "rpc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_config.h"
#include "app_state.h"
#include "battery.h"
#include "button.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "leds.h"
#include "storage.h"
#include "web_server.h"

static const char *TAG = "rpc";
static bool s_mfg_passed;
static bool s_mfg_unlocked;

static void rpc_send(const char *line) {
    usb_serial_jtag_write_bytes(line, strlen(line), pdMS_TO_TICKS(50));
    usb_serial_jtag_write_bytes("\r\n", 2, pdMS_TO_TICKS(50));
}

static void rpc_status(void) {
    runtime_state_t st;
    device_config_t cfg;
    int battery_mv = battery_read_millivolts();
    app_state_get_runtime(&st);
    app_state_get_config(&cfg);

    char out[512];
    snprintf(
        out,
        sizeof(out),
        "{\"ok\":true,\"battery_percent\":%d,\"battery_mv\":%d,\"active_preset\":%d,\"wifi_ap_running\":%s,\"button_pressed\":%s,\"fw\":\"%s\"}",
        st.battery_percent,
        battery_mv,
        st.active_preset,
        st.wifi_ap_running ? "true" : "false",
        button_is_pressed() ? "true" : "false",
        cfg.firmware_version
    );
    rpc_send(out);
}

static void handle_line(char *line) {
    char token[80];

    if (strcmp(line, "PING") == 0) {
        rpc_send("{\"ok\":true,\"pong\":1}");
        return;
    }

    if (strcmp(line, "MFG_STATUS") == 0) {
        char out[160];
        snprintf(
            out,
            sizeof(out),
            "{\"ok\":true,\"mfg_passed\":%s,\"mfg_unlocked\":%s}",
            s_mfg_passed ? "true" : "false",
            s_mfg_unlocked ? "true" : "false"
        );
        rpc_send(out);
        return;
    }

    if (sscanf(line, "MFG_UNLOCK %79s", token) == 1) {
        if (strcmp(token, DL_RPC_GLOBAL_TOKEN) != 0) {
            rpc_send("{\"ok\":false,\"err\":\"bad_token\"}");
            return;
        }

        s_mfg_unlocked = true;
        if (s_mfg_passed) {
            s_mfg_passed = false;
            storage_save_mfg_passed(false);
        }
        rpc_send("{\"ok\":true,\"mfg_unlocked\":true}");
        return;
    }

    if (strcmp(line, "MFG_PASS") == 0) {
        if (!s_mfg_unlocked) {
            rpc_send("{\"ok\":false,\"err\":\"locked\"}");
            return;
        }
        s_mfg_passed = true;
        s_mfg_unlocked = false;
        storage_save_mfg_passed(true);
        rpc_send("{\"ok\":true,\"mfg_passed\":true,\"mfg_unlocked\":false}");
        return;
    }

    if (s_mfg_passed && !s_mfg_unlocked) {
        rpc_send("{\"ok\":false,\"err\":\"locked\",\"hint\":\"MFG_UNLOCK <token>\"}");
        return;
    }

    if (strcmp(line, "GET_STATUS") == 0) {
        rpc_status();
        return;
    }

    if (strcmp(line, "GET_BATTERY") == 0) {
        int adc_mv = battery_read_adc_mv();
        // Compensate for divider inline so we only sample once.
        int bat_mv  = adc_mv * (BAT_R_TOP_KOHM + BAT_R_BOT_KOHM) / BAT_R_BOT_KOHM;
        int percent = (bat_mv - BAT_MV_EMPTY) * 100 / (BAT_MV_FULL - BAT_MV_EMPTY);
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        char out[200];
        snprintf(
            out,
            sizeof(out),
            "{\"ok\":true,\"adc_mv\":%d,\"bat_mv\":%d,\"battery_percent\":%d}",
            adc_mv,
            bat_mv,
            percent
        );
        rpc_send(out);
        return;
    }

    if (strcmp(line, "GET_BUTTON") == 0) {
        rpc_send(button_is_pressed() ? "{\"ok\":true,\"pressed\":true}" : "{\"ok\":true,\"pressed\":false}");
        return;
    }

    int timeout_ms = 0;
    if (sscanf(line, "WAIT_BUTTON %d", &timeout_ms) == 1) {
        if (timeout_ms < 0) {
            timeout_ms = 0;
        }
        rpc_send(button_wait_for_press((uint32_t)timeout_ms)
            ? "{\"ok\":true,\"pressed\":true}"
            : "{\"ok\":false,\"pressed\":false,\"err\":\"timeout\"}");
        return;
    }

    int preset = 0;
    if (sscanf(line, "SET_PRESET %d", &preset) == 1) {
        if (preset >= 0 && preset <= 2) {
            leds_clear_manual_test();
            leds_set_active_preset(preset);
            rpc_send("{\"ok\":true}");
        } else {
            rpc_send("{\"ok\":false,\"err\":\"preset_range\"}");
        }
        return;
    }

    int idx = 0, effect = 0, h1 = 0, h2 = 0;
    if (sscanf(line, "SET_EFFECT %d %d %d %d", &idx, &effect, &h1, &h2) == 4) {
        if (idx < 0 || idx > 2) {
            rpc_send("{\"ok\":false,\"err\":\"idx_range\"}");
            return;
        }
        device_config_t cfg;
        app_state_get_config(&cfg);
        cfg.presets[idx].effect = effect;
        cfg.presets[idx].hue1 = h1;
        cfg.presets[idx].hue2 = h2;
        app_state_set_config(&cfg);
        storage_save_config(&cfg);
        leds_clear_manual_test();
        rpc_send("{\"ok\":true}");
        return;
    }

    int target = 0;
    int hue = 0;
    if (sscanf(line, "LED_TEST %d %d", &target, &hue) == 2) {
        if (hue < 0) hue = 0;
        if (hue > 360) hue = 360;

        if (target == 1) {
            leds_set_manual_test(1, hue, 0, 0);
        } else if (target == 2) {
            leds_set_manual_test(0, 0, 1, hue);
        } else if (target == 3) {
            leds_set_manual_test(1, hue, 1, hue);
        } else {
            rpc_send("{\"ok\":false,\"err\":\"target_range\"}");
            return;
        }

        rpc_send("{\"ok\":true}");
        return;
    }

    int hue1 = 0;
    int hue2 = 0;
    if (sscanf(line, "LED_TEST_DUAL %d %d", &hue1, &hue2) == 2) {
        if (hue1 < 0) hue1 = 0;
        if (hue1 > 360) hue1 = 360;
        if (hue2 < 0) hue2 = 0;
        if (hue2 > 360) hue2 = 360;
        leds_set_manual_test(1, hue1, 1, hue2);
        rpc_send("{\"ok\":true}");
        return;
    }

    if (strcmp(line, "LED_TEST_OFF") == 0) {
        leds_set_manual_test(0, 0, 0, 0);
        rpc_send("{\"ok\":true}");
        return;
    }

    if (strcmp(line, "LED_TEST_CLEAR") == 0) {
        leds_clear_manual_test();
        rpc_send("{\"ok\":true}");
        return;
    }

    if (strcmp(line, "WIFI_AP ON") == 0) {
        web_server_start_ap();
        rpc_send("{\"ok\":true}");
        return;
    }
    if (strcmp(line, "WIFI_AP OFF") == 0) {
        web_server_stop_ap();
        rpc_send("{\"ok\":true}");
        return;
    }

    rpc_send("{\"ok\":false,\"err\":\"unknown_command\"}");
}

static void rpc_task(void *arg) {
    (void)arg;

    uint8_t ch = 0;
    char line[320];
    int pos = 0;

    while (1) {
        int n = usb_serial_jtag_read_bytes(&ch, 1, pdMS_TO_TICKS(50));
        if (n <= 0) {
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            if (pos > 0) {
                line[pos] = '\0';
                handle_line(line);
                pos = 0;
            }
            continue;
        }

        if (pos < (int)sizeof(line) - 1) {
            line[pos++] = (char)ch;
        }
    }
}

void rpc_init(void) {
    usb_serial_jtag_driver_config_t usb_cfg = {
        .tx_buffer_size = 1024,
        .rx_buffer_size = 1024,
    };
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_cfg));

    s_mfg_passed = storage_load_mfg_passed();
    s_mfg_unlocked = !s_mfg_passed;

    ESP_LOGI(TAG, "RPC ready on USB CDC console (locked=%s)", s_mfg_unlocked ? "false" : "true");
    xTaskCreate(rpc_task, "rpc_task", 6144, NULL, 4, NULL);
}
