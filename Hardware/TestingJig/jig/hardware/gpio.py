"""RPi GPIO helpers for the jig's boot/rst pulldowns and button solenoid."""
from __future__ import annotations

import time
from contextlib import contextmanager
from typing import Iterator

try:
    import RPi.GPIO as GPIO  # type: ignore
except ImportError:  # dev machine
    GPIO = None  # type: ignore


class JigGPIO:
    def __init__(self, *, boot_pin: int, reset_pin: int, button_solenoid: int,
                 dut_power: int | None = None) -> None:
        if GPIO is None:
            raise RuntimeError("RPi.GPIO not available on this host")
        self.boot = boot_pin
        self.rst = reset_pin
        self.btn = button_solenoid
        self.pwr = dut_power
        GPIO.setmode(GPIO.BCM)
        for pin in (self.boot, self.rst, self.btn, self.pwr):
            if pin is not None:
                GPIO.setup(pin, GPIO.OUT, initial=GPIO.HIGH)

    # --- DUT power -----------------------------------------------------
    def power_on(self) -> None:
        if self.pwr is not None:
            GPIO.output(self.pwr, GPIO.HIGH)

    def power_off(self) -> None:
        if self.pwr is not None:
            GPIO.output(self.pwr, GPIO.LOW)

    def power_cycle(self, off_ms: int = 500) -> None:
        self.power_off()
        time.sleep(off_ms / 1000)
        self.power_on()

    # --- Boot / reset --------------------------------------------------
    @contextmanager
    def enter_download_mode(self) -> Iterator[None]:
        """Hold BOOT low, pulse RST. After yield, releases everything."""
        GPIO.output(self.boot, GPIO.LOW)
        GPIO.output(self.rst, GPIO.LOW)
        time.sleep(0.05)
        GPIO.output(self.rst, GPIO.HIGH)
        time.sleep(0.10)
        try:
            yield
        finally:
            GPIO.output(self.boot, GPIO.HIGH)

    def reset_dut(self) -> None:
        GPIO.output(self.rst, GPIO.LOW)
        time.sleep(0.05)
        GPIO.output(self.rst, GPIO.HIGH)

    # --- Button solenoid -----------------------------------------------
    def press_button(self, hold_ms: int = 150) -> None:
        GPIO.output(self.btn, GPIO.LOW)   # active-low solenoid driver
        time.sleep(hold_ms / 1000)
        GPIO.output(self.btn, GPIO.HIGH)

    def cleanup(self) -> None:
        GPIO.cleanup()
