#pragma once

#include <stdbool.h>

typedef enum {
	LEDS_OTA_STATE_IDLE = 0,
	LEDS_OTA_STATE_IN_PROGRESS,
	LEDS_OTA_STATE_SUCCESS,
	LEDS_OTA_STATE_FAILED,
} leds_ota_state_t;

void leds_init(void);
void leds_set_power(bool on);
void leds_next_preset(void);
void leds_set_active_preset(int idx);
void leds_show_battery_level(int percent);
void leds_signal_ap_status(bool enabled);
void leds_signal_error(void);
void leds_set_ota_state(leds_ota_state_t state);
void leds_set_manual_test(int led1_enabled, int led1_hue, int led2_enabled, int led2_hue);
void leds_clear_manual_test(void);
