"""TI INA228 current/voltage/power monitor (high-side, low Rsense).

Driver is intentionally minimal - we only need bus voltage and shunt current
for the jig's sleep/idle/preset current checks.

Datasheet registers (16-bit unless noted):
    0x00 CONFIG       (RW)
    0x01 ADC_CONFIG   (RW)
    0x02 SHUNT_CAL    (RW)
    0x04 VSHUNT       (24-bit, signed, 312.5 nV/LSB at ADCRANGE=0)
    0x05 VBUS         (24-bit, unsigned, 195.3125 uV/LSB)
    0x07 CURRENT      (24-bit, signed, current_LSB)

TODO: implement actual register reads once the HAT exists.
"""
from __future__ import annotations

from dataclasses import dataclass

from .i2c_bus import I2CBus


@dataclass
class INA228Config:
    address: int = 0x40
    rshunt_ohm: float = 0.010
    max_current_a: float = 1.0


class INA228:
    REG_CONFIG     = 0x00
    REG_ADC_CONFIG = 0x01
    REG_SHUNT_CAL  = 0x02
    REG_VSHUNT     = 0x04
    REG_VBUS       = 0x05
    REG_CURRENT    = 0x07

    def __init__(self, bus: I2CBus, cfg: INA228Config | None = None) -> None:
        self.bus = bus
        self.cfg = cfg or INA228Config()
        self._current_lsb = self.cfg.max_current_a / (1 << 19)

    def configure(self) -> None:
        # TODO: write CONFIG / ADC_CONFIG / SHUNT_CAL based on cfg.
        raise NotImplementedError

    def read_bus_voltage_v(self) -> float:
        raise NotImplementedError

    def read_current_a(self) -> float:
        raise NotImplementedError

    def read_current_ua(self) -> float:
        return self.read_current_a() * 1_000_000.0
