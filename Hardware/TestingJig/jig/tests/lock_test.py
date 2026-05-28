"""Final step: mark the DUT manufactured + locked via MFG_PASS."""
from __future__ import annotations

from typing import Any

from .base import JigContext, TestStep


class LockTest(TestStep):
    name = "lock"

    def run(self, ctx: JigContext) -> dict[str, Any]:
        # Firmware boots unlocked on first run; MFG_PASS persists locked state.
        ctx.rpc.mfg_pass()
        st = ctx.rpc.mfg_status()
        if not st.get("mfg_passed") or st.get("mfg_unlocked"):
            raise RuntimeError(f"unexpected MFG_STATUS: {st}")
        return {"mfg_status": st}
