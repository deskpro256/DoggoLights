#include "leds.h"

#include <math.h>

#include "app_config.h"
#include "app_state.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "leds";

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

static bool s_power_on = true;
static bool s_manual_test_enabled;
static bool s_manual_led1_enabled;
static bool s_manual_led2_enabled;
static int s_manual_led1_hue;
static int s_manual_led2_hue;
static volatile int s_ap_signal;
static volatile int s_error_signal;
static volatile leds_ota_state_t s_ota_state = LEDS_OTA_STATE_IDLE;

static int invert_pwm(int value) {
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    return 255 - value;
}

static rgb_t hsl_to_rgb(int hue) {
    float h = fmodf((float)hue + 360.0f, 360.0f);
    float s = 1.0f;
    float l = 0.5f;

    float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }

    rgb_t out = {
        .r = (uint8_t)((r + m) * 255.0f),
        .g = (uint8_t)((g + m) * 255.0f),
        .b = (uint8_t)((b + m) * 255.0f),
    };
    return out;
}

// When an inverted duty reaches 255 on an 8-bit timer the pin still pulses
// LOW once per PWM cycle, causing a faint glow on common-anode LEDs.
// Using ledc_stop() with idle_level=1 holds the pin truly HIGH (= off).
static void set_channel(ledc_channel_t ch, int inv_duty) {
    if (inv_duty >= 255) {
        ledc_stop(LEDC_LOW_SPEED_MODE, ch, 1);
    } else {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, inv_duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
    }
}

static void set_led_pair(rgb_t c1, rgb_t c2) {
    set_channel(LEDC_CHANNEL_0, invert_pwm(c1.r));
    set_channel(LEDC_CHANNEL_1, invert_pwm(c1.g));
    set_channel(LEDC_CHANNEL_2, invert_pwm(c1.b));
    set_channel(LEDC_CHANNEL_3, invert_pwm(c2.r));
    set_channel(LEDC_CHANNEL_4, invert_pwm(c2.g));
    set_channel(LEDC_CHANNEL_5, invert_pwm(c2.b));
}

static void all_off(void) {
    for (int i = 0; i < 6; ++i) {
        ledc_stop(LEDC_LOW_SPEED_MODE, (ledc_channel_t)i, 1);
    }
}

static void blink_color(rgb_t c1, rgb_t c2, int count, int ms) {
    for (int i = 0; i < count; ++i) {
        set_led_pair(c1, c2);
        vTaskDelay(pdMS_TO_TICKS(ms));
        all_off();
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}

void leds_show_battery_level(int percent) {
    rgb_t a, b;
    if (percent >= 75) {
        a = (rgb_t){0, 255, 0};
        b = (rgb_t){0, 255, 0};
    } else if (percent >= 50) {
        a = (rgb_t){255, 255, 0};
        b = (rgb_t){255, 255, 0};
    } else if (percent >= 25) {
        a = (rgb_t){255, 255, 0};
        b = (rgb_t){255, 0, 255};
    } else {
        a = (rgb_t){255, 0, 0};
        b = (rgb_t){255, 0, 0};
    }
    blink_color(a, b, 3, 120);
}

void leds_signal_ap_status(bool enabled) {
    s_ap_signal = enabled ? 1 : 2;
}

void leds_signal_error(void) {
    s_error_signal = 1;
}

void leds_set_ota_state(leds_ota_state_t state) {
    s_ota_state = state;
}

void leds_set_power(bool on) {
    s_power_on = on;
    if (!on) {
        all_off();
    }
}

void leds_next_preset(void) {
    runtime_state_t st;
    app_state_get_runtime(&st);
    st.active_preset = (st.active_preset + 1) % 3;
    app_state_set_runtime(&st);
}

void leds_set_active_preset(int idx) {
    if (idx < 0 || idx > 2) {
        return;
    }
    runtime_state_t st;
    app_state_get_runtime(&st);
    st.active_preset = idx;
    app_state_set_runtime(&st);
}

void leds_set_manual_test(int led1_enabled, int led1_hue, int led2_enabled, int led2_hue) {
    s_manual_test_enabled = true;
    s_manual_led1_enabled = led1_enabled != 0;
    s_manual_led2_enabled = led2_enabled != 0;
    s_manual_led1_hue = led1_hue;
    s_manual_led2_hue = led2_hue;
}

void leds_clear_manual_test(void) {
    s_manual_test_enabled = false;
}

static void led_task(void *arg) {
    (void)arg;
    int rainbow = 0;
    int breath = 0;
    int breath_dir = 1;
    int flip = 0;
    int ota_anim_phase = 0;
    leds_ota_state_t last_ota_state = LEDS_OTA_STATE_IDLE;
    const int breathing_cycle_ms = DL_BREATHING_INHALE_MS + DL_BREATHING_EXHALE_MS;
    const int breath_step = (190 * DL_LED_EFFECT_TICK_MS + (breathing_cycle_ms / 2)) / breathing_cycle_ms;

    while (1) {
        runtime_state_t st;
        device_config_t cfg;
        leds_ota_state_t ota_state = s_ota_state;
        app_state_get_runtime(&st);
        app_state_get_config(&cfg);

        if (ota_state != last_ota_state) {
            ota_anim_phase = 0;
            last_ota_state = ota_state;
        }

        if (ota_state == LEDS_OTA_STATE_IN_PROGRESS) {
            // OTA animation: double blink fade (two soft cyan pulses, then pause).
            int cycle = ota_anim_phase % 60;
            int intensity = 0;
            if (cycle < 10) {
                intensity = (cycle * 255) / 10;
            } else if (cycle < 20) {
                intensity = ((20 - cycle) * 255) / 10;
            } else if (cycle < 30) {
                intensity = ((cycle - 20) * 255) / 10;
            } else if (cycle < 40) {
                intensity = ((40 - cycle) * 255) / 10;
            }

            rgb_t ota = {
                .r = 0,
                .g = (uint8_t)((intensity * 170) / 255),
                .b = (uint8_t)intensity,
            };
            set_led_pair(ota, ota);
            ota_anim_phase = (ota_anim_phase + 1) % 60;
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        if (ota_state == LEDS_OTA_STATE_SUCCESS) {
            s_ota_state = LEDS_OTA_STATE_IDLE;
            blink_color((rgb_t){0, 255, 0}, (rgb_t){0, 255, 0}, 3, 90);
        } else if (ota_state == LEDS_OTA_STATE_FAILED) {
            s_ota_state = LEDS_OTA_STATE_IDLE;
            blink_color((rgb_t){255, 0, 0}, (rgb_t){255, 0, 0}, 3, 90);
        }

        if (s_ap_signal != 0) {
            int signal = s_ap_signal;
            s_ap_signal = 0;

            if (signal == 1) {
                // AP on: LED1 blue, LED2 green, double flash.
                blink_color((rgb_t){0, 0, 255}, (rgb_t){0, 255, 0}, 2, 120);
            } else if (signal == 2) {
                // AP off: LED1 blue, LED2 red, double flash.
                blink_color((rgb_t){0, 0, 255}, (rgb_t){255, 0, 0}, 2, 120);
            }
        }

        if (s_error_signal != 0) {
            s_error_signal = 0;
            // Error indication: both LEDs red, double flash.
            blink_color((rgb_t){255, 0, 0}, (rgb_t){255, 0, 0}, 2, 120);
        }

        if (!s_power_on || !st.powered_on) {
            all_off();
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (s_manual_test_enabled) {
            rgb_t off = {0, 0, 0};
            rgb_t c1 = s_manual_led1_enabled ? hsl_to_rgb(s_manual_led1_hue) : off;
            rgb_t c2 = s_manual_led2_enabled ? hsl_to_rgb(s_manual_led2_hue) : off;
            set_led_pair(c1, c2);
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        preset_t p = cfg.presets[st.active_preset];
        rgb_t c1 = hsl_to_rgb(p.hue1);
        rgb_t c2 = hsl_to_rgb(p.hue2);

        switch (p.effect) {
            case EFFECT_STATIC:
                set_led_pair(c1, c2);
                vTaskDelay(pdMS_TO_TICKS(20));
                break;
            case EFFECT_BREATHING: {
                float scale = (float)breath / 100.0f;
                rgb_t b1 = {(uint8_t)(c1.r * scale), (uint8_t)(c1.g * scale), (uint8_t)(c1.b * scale)};
                rgb_t b2 = {(uint8_t)(c2.r * scale), (uint8_t)(c2.g * scale), (uint8_t)(c2.b * scale)};
                set_led_pair(b1, b2);
                breath += breath_dir * (breath_step > 0 ? breath_step : 1);
                if (breath >= 100) {
                    breath = 100;
                    breath_dir = -1;
                }
                if (breath <= 5) {
                    breath = 5;
                    breath_dir = 1;
                }
                vTaskDelay(pdMS_TO_TICKS(DL_LED_EFFECT_TICK_MS));
                break;
            }
            case EFFECT_FLIPFLOP:
                if (flip == 0) {
                    rgb_t o = {0, 0, 0};
                    set_led_pair(c1, o);
                } else {
                    rgb_t o = {0, 0, 0};
                    set_led_pair(o, c2);
                }
                flip ^= 1;
                vTaskDelay(pdMS_TO_TICKS(DL_FLIPFLOP_INTERVAL_MS));
                break;
            case EFFECT_RAINBOW: {
                rgb_t r = hsl_to_rgb(rainbow);
                set_led_pair(r, r);
                rainbow = (rainbow + 3) % 360;
                vTaskDelay(pdMS_TO_TICKS(15));
                break;
            }
            default:
                set_led_pair(c1, c2);
                vTaskDelay(pdMS_TO_TICKS(30));
                break;
        }
    }
}

void leds_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    const gpio_num_t gpios[6] = {
        DL_LED1_R_GPIO, DL_LED1_G_GPIO, DL_LED1_B_GPIO,
        DL_LED2_R_GPIO, DL_LED2_G_GPIO, DL_LED2_B_GPIO,
    };

    for (int i = 0; i < 6; ++i) {
        ledc_channel_config_t ch = {
            .gpio_num = gpios[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = (ledc_channel_t)i,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 255,
            .hpoint = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }

    ESP_LOGI(TAG, "LED engine ready");
    xTaskCreate(led_task, "led_task", 4096, NULL, 5, NULL);
}
