"""Shared mutable runtime state for the jig.

The test runner thread writes here; the Flask app reads from here for the UI
and for SSE updates.
"""
from __future__ import annotations

import threading
import time
from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass
class StepResult:
    name: str
    status: str = "pending"   # pending | running | pass | fail | skip
    message: str = ""
    details: dict[str, Any] = field(default_factory=dict)
    started_at: float | None = None
    finished_at: float | None = None


@dataclass
class RunState:
    dut_port: str | None = None
    dut_mac: str | None = None
    dut_serial_short: str | None = None   # last 6 hex of MAC
    firmware_version: str | None = None
    overall: str = "idle"                 # idle | running | pass | fail
    steps: list[StepResult] = field(default_factory=list)
    started_at: float | None = None
    finished_at: float | None = None
    log_tail: list[str] = field(default_factory=list)


class StateStore:
    """Thread-safe wrapper. The runner mutates inside `with store:` blocks."""

    def __init__(self) -> None:
        self._lock = threading.RLock()
        self._state = RunState()
        self._rev = 0

    def __enter__(self) -> RunState:
        self._lock.acquire()
        return self._state

    def __exit__(self, *exc: Any) -> None:
        self._rev += 1
        self._lock.release()

    def snapshot(self) -> dict[str, Any]:
        with self._lock:
            return {"rev": self._rev, "state": asdict(self._state)}

    def reset(self) -> None:
        with self:
            self._state.__init__()  # type: ignore[misc]

    def push_log(self, line: str, cap: int = 500) -> None:
        with self:
            self._state.log_tail.append(f"{time.strftime('%H:%M:%S')}  {line}")
            if len(self._state.log_tail) > cap:
                del self._state.log_tail[: len(self._state.log_tail) - cap]


STORE = StateStore()
