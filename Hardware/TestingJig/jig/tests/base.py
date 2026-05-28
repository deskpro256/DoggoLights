"""Base class and shared context for jig test steps."""
from __future__ import annotations

import time
from dataclasses import dataclass, field
from typing import Any

from ..dut.rpc_client import RpcClient
from ..hardware.gpio import JigGPIO
from ..hardware.ina228 import INA228
from ..hardware.lux_sensor import LuxSensor
from ..settings import Settings
from ..state import STORE, StepResult


@dataclass
class JigContext:
    settings: Settings
    rpc: RpcClient
    gpio: JigGPIO | None = None
    current_sensor: INA228 | None = None
    lux1: LuxSensor | None = None
    lux2: LuxSensor | None = None
    scratch: dict[str, Any] = field(default_factory=dict)


class TestStep:
    name: str = "step"

    def run(self, ctx: JigContext) -> dict[str, Any]:  # pragma: no cover - interface
        raise NotImplementedError

    # --- runner glue --------------------------------------------------
    def execute(self, ctx: JigContext) -> StepResult:
        result = StepResult(name=self.name, status="running", started_at=time.time())
        with STORE as st:
            st.steps.append(result)
        STORE.push_log(f"[{self.name}] start")
        try:
            details = self.run(ctx) or {}
            result.status = "pass"
            result.details = details
            STORE.push_log(f"[{self.name}] PASS")
        except Exception as e:
            result.status = "fail"
            result.message = str(e)
            STORE.push_log(f"[{self.name}] FAIL: {e}")
        finally:
            result.finished_at = time.time()
            with STORE:
                pass  # bump revision
        return result
