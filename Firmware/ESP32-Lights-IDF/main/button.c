#include "button.h"

#include <stdbool.h>

#include "app_config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static button_event_cb_t s_cb;

bool button_is_pressed(void) {
    return gpio_get_level(DL_BUTTON_GPIO) == 0;
}

bool button_wait_for_press(uint32_t timeout_ms) {
    int64_t start_ms = esp_timer_get_time() / 1000;

    while (((esp_timer_get_time() / 1000) - start_ms) < (int64_t)timeout_ms) {
        if (button_is_pressed()) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return button_is_pressed();
}

static void button_task(void *arg) {
    (void)arg;

    bool prev_pressed = false;
    int64_t press_start_ms = 0;
    int64_t last_release_ms = 0;
    int click_count = 0;
    bool long_fired = false;

    while (1) {
        bool pressed = button_is_pressed();
        int64_t now_ms = esp_timer_get_time() / 1000;

        if (pressed && !prev_pressed) {
            press_start_ms = now_ms;
            long_fired = false;
        }

        // Fire LONG while still held after DL_SLEEP_PRESS_MS (no release needed)
        if (pressed && !long_fired) {
            int64_t held_ms = now_ms - press_start_ms;
            if (held_ms >= DL_SLEEP_PRESS_MS) {
                long_fired = true;
                click_count = 0;  // cancel any pending click
                if (s_cb) {
                    s_cb(BUTTON_EVENT_LONG);
                }
            }
        }

        if (!pressed && prev_pressed) {
            if (!long_fired) {
                // Normal short release — count as click
                click_count++;
                last_release_ms = now_ms;
            }
            // If long already fired, ignore the release
        }

        if (click_count > 0 && (now_ms - last_release_ms) > DL_DOUBLE_CLICK_MS) {
            if (s_cb) {
                s_cb(click_count >= 2 ? BUTTON_EVENT_DOUBLE : BUTTON_EVENT_SINGLE);
            }
            click_count = 0;
        }

        prev_pressed = pressed;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void button_init(button_event_cb_t cb) {
    s_cb = cb;

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << DL_BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    xTaskCreate(button_task, "button_task", 3072, NULL, 6, NULL);
}
