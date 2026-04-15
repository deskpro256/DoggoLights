#pragma once

#include <stdbool.h>

void leds_init(void);
void leds_set_power(bool on);
void leds_next_preset(void);
void leds_set_active_preset(int idx);
void leds_show_battery_level(int percent);
void leds_signal_ap_status(bool enabled);
void leds_signal_error(void);
void leds_set_manual_test(int led1_enabled, int led1_hue, int led2_enabled, int led2_hue);
void leds_clear_manual_test(void);
