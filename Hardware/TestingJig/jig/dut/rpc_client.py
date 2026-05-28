"""Line-based JSON RPC client matching `Firmware/ESP32-Lights-IDF/main/rpc.c`.

The firmware writes responses to the USB-CDC console as a single JSON line
terminated with CRLF. Commands are also line-terminated. Every response
contains an `ok` field; we raise `RpcError` on `ok:false`.

Recognised commands (see rpc.c::handle_line):
    PING
    GET_STATUS / MFG_STATUS / GET_BATTERY / GET_BUTTON
    WAIT_BUTTON <ms>
    SET_PRESET <0..2>
    SET_EFFECT <idx> <effect> <h1> <h2>
    LED_TEST <target> <hue>     # target: 1=LED1 only, 2=LED2 only, 3=both
    LED_TEST_DUAL <h1> <h2>
    LED_TEST_OFF / LED_TEST_CLEAR
    WIFI_AP ON / WIFI_AP OFF
    MFG_UNLOCK <token> / MFG_PASS
"""
from __future__ import annotations

import json
import time
from dataclasses import dataclass
from typing import Any

import serial


class RpcError(RuntimeError):
    def __init__(self, payload: dict[str, Any]) -> None:
        super().__init__(payload.get("err", "rpc_error"))
        self.payload = payload


@dataclass
class RpcConfig:
    baud: int = 115200
    read_timeout_s: float = 2.0
    unlock_token: str = "doggo-mfg-2026"


class RpcClient:
    def __init__(self, port: str, cfg: RpcConfig | None = None) -> None:
        self.port = port
        self.cfg = cfg or RpcConfig()
        self._ser: serial.Serial | None = None

    # --- connection ----------------------------------------------------
    def open(self) -> None:
        # The DUT may have just re-enumerated after a reset; the COM device
        # can briefly be "access denied" or vanish. Retry for ~3 s.
        deadline = time.monotonic() + 3.0
        last: Exception | None = None
        while time.monotonic() < deadline:
            try:
                self._ser = serial.Serial(
                    self.port, self.cfg.baud, timeout=self.cfg.read_timeout_s
                )
                break
            except (serial.SerialException, PermissionError, OSError) as e:
                last = e
                time.sleep(0.25)
        else:
            raise RuntimeError(f"could not open {self.port}: {last}")
        time.sleep(0.1)
        self._ser.reset_input_buffer()

    def close(self) -> None:
        if self._ser:
            self._ser.close()
            self._ser = None

    def __enter__(self) -> "RpcClient":
        self.open()
        return self

    def __exit__(self, *exc: Any) -> None:
        self.close()

    # --- transport -----------------------------------------------------
    def send(self, line: str) -> dict[str, Any]:
        assert self._ser is not None, "open() first"
        # Drop anything the firmware printed before our command (boot logs,
        # late responses from a previous command, etc.).
        self._ser.reset_input_buffer()
        self._ser.write((line + "\r\n").encode("ascii"))
        self._ser.flush()

        # Read lines until we find one that parses as JSON. Non-JSON lines
        # (ESP-IDF boot log, ESP_LOGI, etc.) are silently skipped.
        deadline = time.monotonic() + self.cfg.read_timeout_s
        while time.monotonic() < deadline:
            raw = self._ser.readline()
            if not raw:
                continue
            text = raw.decode("utf-8", errors="replace").strip()
            if not text or not text.startswith("{"):
                continue
            try:
                obj = json.loads(text)
            except json.JSONDecodeError:
                continue
            if not obj.get("ok", False):
                raise RpcError(obj)
            return obj
        raise TimeoutError(f"no RPC response to {line!r}")

    def wait_ready(self, timeout_s: float = 10.0) -> dict[str, Any]:
        """Poll GET_STATUS until the firmware responds. Use after reset/flash.

        Logs any non-JSON noise (boot logs, ESP_LOGI) so a non-RPC firmware
        is easy to spot.
        """
        import logging
        log = logging.getLogger(__name__)
        deadline = time.monotonic() + timeout_s
        last_err: Exception | None = None
        attempts = 0
        while time.monotonic() < deadline:
            attempts += 1
            try:
                resp = self.send("GET_STATUS")
                log.info("DUT responded after %d attempts", attempts)
                return resp
            except (TimeoutError, RuntimeError) as e:
                last_err = e
                # Drain whatever junk arrived so the next send starts clean.
                if self._ser is not None and self._ser.in_waiting:
                    junk = self._ser.read(self._ser.in_waiting)
                    log.info("DUT junk while waiting: %r", junk[:200])
                time.sleep(0.5)
        raise TimeoutError(f"DUT not ready after {timeout_s}s ({last_err})")

    # --- convenience wrappers -----------------------------------------
    def ping(self) -> dict[str, Any]:
        return self.send("PING")

    def status(self) -> dict[str, Any]:
        return self.send("GET_STATUS")

    def battery(self) -> dict[str, Any]:
        return self.send("GET_BATTERY")

    def wait_button(self, timeout_ms: int) -> dict[str, Any]:
        return self.send(f"WAIT_BUTTON {timeout_ms}")

    def led_test(self, target: int, hue: int) -> dict[str, Any]:
        return self.send(f"LED_TEST {target} {hue}")

    def led_test_off(self) -> dict[str, Any]:
        return self.send("LED_TEST_OFF")

    def set_preset(self, idx: int) -> dict[str, Any]:
        return self.send(f"SET_PRESET {idx}")

    def wifi_ap(self, on: bool) -> dict[str, Any]:
        return self.send("WIFI_AP ON" if on else "WIFI_AP OFF")

    def mfg_unlock(self) -> dict[str, Any]:
        return self.send(f"MFG_UNLOCK {self.cfg.unlock_token}")

    def mfg_pass(self) -> dict[str, Any]:
        return self.send("MFG_PASS")

    def mfg_status(self) -> dict[str, Any]:
        return self.send("MFG_STATUS")
