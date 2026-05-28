"""Current draw sanity-check via INA228, exercised concurrently with LED test."""
from __future__ import annotations

import time
from typing import Any

from .base import JigContext, TestStep


class CurrentTest(TestStep):
    name = "current"

    def run(self, ctx: JigContext) -> dict[str, Any]:
        if not ctx.current_sensor:
            raise RuntimeError("INA228 not configured")
        thr = ctx.settings.get("thresholds", default={}) or {}
        idle_max_ma = float(thr.get("idle_current_ma_max", 25))
        static_max_ma = float(thr.get("static_current_ma_max", 120))

        # All LEDs off -> idle baseline.
        ctx.rpc.led_test_off()
        time.sleep(0.3)
        idle_ma = ctx.current_sensor.read_current_a() * 1000
        if idle_ma > idle_max_ma:
            raise RuntimeError(f"idle current too high: {idle_ma:.1f} mA")

        # SET_PRESET 0 = static colour in firmware (see rpc.c).
        ctx.rpc.set_preset(0)
        time.sleep(0.3)
        static_ma = ctx.current_sensor.read_current_a() * 1000
        if static_ma > static_max_ma:
            raise RuntimeError(f"static current too high: {static_ma:.1f} mA")

        ctx.rpc.led_test_off()
        return {"idle_ma": idle_ma, "static_ma": static_ma}
