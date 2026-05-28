"""LED test: drive each LED on each primary colour, watch the lux sensors."""
from __future__ import annotations

import time
from typing import Any

from .base import JigContext, TestStep

# Hue values (0-360) that map to roughly red / green / blue in the firmware's
# HSV-style colour helpers.
_COLOURS = [("red", 0), ("green", 120), ("blue", 240)]


class LedTest(TestStep):
    name = "leds"

    def run(self, ctx: JigContext) -> dict[str, Any]:
        if not (ctx.lux1 and ctx.lux2):
            raise RuntimeError("lux sensors not configured")

        thr = ctx.settings.get("thresholds", default={}) or {}
        ambient_max = float(thr.get("ambient_lux_max", 30))
        delta_min = float(thr.get("lit_lux_min_delta", 150))

        ctx.rpc.led_test_off()
        time.sleep(0.2)
        ambient1 = ctx.lux1.read().lux
        ambient2 = ctx.lux2.read().lux
        if ambient1 > ambient_max or ambient2 > ambient_max:
            raise RuntimeError(
                f"ambient too bright: lux1={ambient1:.1f} lux2={ambient2:.1f}"
            )

        per_colour: dict[str, dict[str, float]] = {}
        for name, hue in _COLOURS:
            ctx.rpc.led_test(1, hue)  # LED1 only
            time.sleep(0.2)
            lit1 = ctx.lux1.read().lux
            ctx.rpc.led_test(2, hue)  # LED2 only
            time.sleep(0.2)
            lit2 = ctx.lux2.read().lux
            ctx.rpc.led_test_off()

            d1 = lit1 - ambient1
            d2 = lit2 - ambient2
            per_colour[name] = {"led1_delta": d1, "led2_delta": d2}
            if d1 < delta_min or d2 < delta_min:
                raise RuntimeError(
                    f"{name}: insufficient lux delta (led1={d1:.1f} led2={d2:.1f})"
                )

        return {
            "ambient": {"lux1": ambient1, "lux2": ambient2},
            "per_colour": per_colour,
        }
