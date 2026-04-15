#pragma once

#include "driver/gpio.h"

#define DL_BUTTON_GPIO GPIO_NUM_0

#define DL_LED1_R_GPIO GPIO_NUM_5
#define DL_LED1_G_GPIO GPIO_NUM_4
#define DL_LED1_B_GPIO GPIO_NUM_6
#define DL_LED2_R_GPIO GPIO_NUM_2
#define DL_LED2_G_GPIO GPIO_NUM_21
#define DL_LED2_B_GPIO GPIO_NUM_1

#define DL_BAT_ADC_GPIO GPIO_NUM_3

#define DL_AP_AUTO_OFF_MS (5 * 60 * 1000)
#define DL_LONG_PRESS_MS 1000
#define DL_DOUBLE_CLICK_MS 500

#define DL_LED_EFFECT_TICK_MS 10
#define DL_BREATHING_INHALE_MS 500
#define DL_BREATHING_EXHALE_MS 500
#define DL_FLIPFLOP_INTERVAL_MS 300

#define DL_RPC_UART_NUM 1
#define DL_RPC_TX_GPIO GPIO_NUM_7
#define DL_RPC_RX_GPIO GPIO_NUM_20
#define DL_RPC_BAUD 115200
#define DL_RPC_GLOBAL_TOKEN "doggo-mfg-2026"

#define DL_DEFAULT_OTA_URL "https://raw.githubusercontent.com/deskpro256/DoggoLights/main/Firmware/latest/firmware.bin"
