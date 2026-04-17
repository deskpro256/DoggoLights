# DoggoLights ESP-IDF Firmware (ESP32-C3)

Fresh-start firmware for ESP32-C3-WROOM-02 with:

- Low-power lighting controller with three presets
- Button gestures: single click (next preset), double click (Wifi AP on/off), long press (power off)
- Captive configuration web server from SPIFFS (no inline HTML in C code)
- OTA pull update endpoint (GitHub/raw URL or your own server)
- UART RPC endpoint for production test board integration

## Build

1. Install ESP-IDF (v5.1+ recommended).
2. In this folder:

```bash
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Filesystem Web UI

The UI is in `main/web/index.html` and gets packed into the `storage` SPIFFS partition on build.
Device variables are injected through JSON endpoints (`/api/status`) instead of inline C strings.

## OTA Pull

Configure the URL in the web UI, then trigger OTA via:

- Web API: `POST /api/ota?url=https://.../firmware.bin`

The firmware uses `esp_https_ota` and reboots on success.

Rollback safety is enabled. If a new OTA image fails to boot reliably, ESP-IDF bootloader rollback keeps the previous firmware as recovery image.

## RPC Quick Commands

UART default: 115200 baud (USB serial/JTAG console channel can also be used).

- `PING`
- `GET_STATUS`
- `GET_BATTERY`
- `GET_BUTTON`
- `WAIT_BUTTON <timeout_ms>`
- `SET_PRESET <0|1|2>`
- `SET_EFFECT <preset> <effect> <hue1> <hue2>`
- `LED_TEST <1|2|3> <hue>`
- `LED_TEST_DUAL <hue1> <hue2>`
- `LED_TEST_OFF`
- `LED_TEST_CLEAR`
- `WIFI_AP ON|OFF`

Responses are line-delimited JSON to simplify test-fixture parsing.
