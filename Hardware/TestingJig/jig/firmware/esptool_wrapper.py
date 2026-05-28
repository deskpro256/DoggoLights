"""Thin wrapper around the `esptool` Python module.

We import esptool.py directly instead of shelling out so we can capture
errors and reuse the same process / port assumptions.
"""
from __future__ import annotations

import io
import sys
from contextlib import redirect_stdout
from dataclasses import dataclass
from pathlib import Path

import esptool

from ..log import get

LOG = get(__name__)


@dataclass
class EsptoolConfig:
    chip: str = "esp32c3"
    baud: int = 921600
    flash_addr: str = "0x10000"
    before: str = "default_reset"
    after: str = "hard_reset"


class Esptool:
    def __init__(self, port: str, cfg: EsptoolConfig) -> None:
        self.port = port
        self.cfg = cfg

    def _run(self, args: list[str]) -> str:
        base = [
            "--chip", self.cfg.chip,
            "--port", self.port,
            "--baud", str(self.cfg.baud),
            "--before", self.cfg.before,
            "--after", self.cfg.after,
        ]
        LOG.info("esptool %s", " ".join(base + args))

        # The COM port can be briefly held by a previous run's still-dying
        # handle, or by Windows itself during USB-CDC re-enumeration. Retry
        # the whole esptool invocation a couple of times on "access denied"
        # / port-busy style errors before giving up.
        import time as _time
        last_err: Exception | None = None
        for attempt in range(4):
            buf = io.StringIO()
            with redirect_stdout(buf):
                try:
                    esptool.main(base + args)
                    return buf.getvalue()
                except SystemExit as e:
                    if e.code in (None, 0):
                        return buf.getvalue()
                    out = buf.getvalue()
                    msg = f"esptool failed: rc={e.code}\n{out}"
                    if "Access is denied" in out or "could not open port" in out:
                        last_err = RuntimeError(msg)
                        LOG.warning("esptool port busy (attempt %d), retrying", attempt + 1)
                        _time.sleep(1.0)
                        continue
                    raise RuntimeError(msg) from e
        raise last_err or RuntimeError("esptool failed after retries")

    def read_mac(self) -> str:
        out = self._run(["read_mac"])
        for line in out.splitlines():
            if "MAC:" in line:
                return line.split("MAC:", 1)[1].strip()
        raise RuntimeError("could not parse MAC from esptool output")

    def erase(self) -> None:
        self._run(["erase_flash"])

    def flash(self, binary: Path) -> None:
        self._run(["write_flash", self.cfg.flash_addr, str(binary)])
