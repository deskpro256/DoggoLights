"""Lux sensor placed directly above each DoggoLights RGB LED.

Two identical sensors on the same I2C bus at different addresses.
Final chip TBD (candidates: VEML7700 lumen-only, or TCS34725 / OPT4001 RGB).

For now we model only `read_lux()` plus optional per-colour channels so test
code can compare ambient vs lit and detect which LED is on.
"""
from __future__ import annotations

from dataclasses import dataclass

from .i2c_bus import I2CBus


@dataclass
class LuxReading:
    lux: float
    r: float = 0.0
    g: float = 0.0
    b: float = 0.0


class LuxSensor:
    def __init__(self, bus: I2CBus, address: int) -> None:
        self.bus = bus
        self.address = address

    def configure(self) -> None:
        raise NotImplementedError

    def read(self) -> LuxReading:
        raise NotImplementedError
