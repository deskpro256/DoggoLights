"""Physical button test using the solenoid plus DUT WAIT_BUTTON RPC."""
from __future__ import annotations

import threading
import time
from typing import Any

from .base import JigContext, TestStep


class ButtonTest(TestStep):
    name = "button"

    def run(self, ctx: JigContext) -> dict[str, Any]:
        if not ctx.gpio:
            raise RuntimeError("GPIO not configured")

        # Fire the solenoid shortly after we arm WAIT_BUTTON.
        def _press_later() -> None:
            time.sleep(0.5)
            ctx.gpio.press_button(hold_ms=120)

        threading.Thread(target=_press_later, daemon=True).start()
        resp = ctx.rpc.wait_button(timeout_ms=3000)
        return {"rpc": resp}
