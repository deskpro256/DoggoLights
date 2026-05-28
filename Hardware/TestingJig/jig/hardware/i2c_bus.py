"""Shared I2C bus handle (smbus2).

All I2C device drivers (INA228, lux sensors) accept an `I2CBus` so we don't
fight over `/dev/i2c-1`.
"""
from __future__ import annotations

import threading

try:
    from smbus2 import SMBus, i2c_msg  # type: ignore
except ImportError:  # dev machine without I2C
    SMBus = None  # type: ignore
    i2c_msg = None  # type: ignore


class I2CBus:
    def __init__(self, bus_number: int = 1) -> None:
        if SMBus is None:
            raise RuntimeError("smbus2 not available on this host")
        self._bus = SMBus(bus_number)
        self._lock = threading.Lock()

    def read_block(self, addr: int, reg: int, length: int) -> bytes:
        with self._lock:
            write = i2c_msg.write(addr, [reg])
            read = i2c_msg.read(addr, length)
            self._bus.i2c_rdwr(write, read)
            return bytes(read)

    def write_block(self, addr: int, reg: int, data: bytes) -> None:
        with self._lock:
            self._bus.write_i2c_block_data(addr, reg, list(data))

    def close(self) -> None:
        self._bus.close()
