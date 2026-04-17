#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_EVENT_SINGLE,
    BUTTON_EVENT_DOUBLE,
    BUTTON_EVENT_LONG,
} button_event_t;

typedef void (*button_event_cb_t)(button_event_t event);

void button_init(button_event_cb_t cb);
bool button_is_pressed(void);
bool button_wait_for_press(uint32_t timeout_ms);
