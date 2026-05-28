"""WiFi test: ask DUT to start softAP, associate with nmcli, verify RPC over USB still works."""
from __future__ import annotations

import time
from typing import Any

from ..dut import mac as macmod
from ..wifi import nmcli
from .base import JigContext, TestStep


class WifiTest(TestStep):
    name = "wifi"

    def run(self, ctx: JigContext) -> dict[str, Any]:
        dut_mac = ctx.scratch.get("dut_mac")
        if not dut_mac:
            raise RuntimeError("DUT MAC not known")

        prefix = ctx.settings.get("wifi", "ap_ssid_prefix", default="DoggoLights")
        ssid = macmod.ap_ssid(dut_mac, prefix=prefix)
        conn_name = ctx.settings.get("wifi", "nm_connection", default="doggolights-test")

        ctx.rpc.wifi_ap(True)
        time.sleep(2.0)

        # Wait for SSID to appear in scan.
        found = None
        for _ in range(10):
            for ap in nmcli.scan():
                if ap.ssid == ssid:
                    found = ap
                    break
            if found:
                break
            time.sleep(1.0)
        if not found:
            raise RuntimeError(f"AP {ssid!r} not visible in scan")

        nmcli.connect_open(ssid, connection_name=conn_name)
        try:
            rssi = nmcli.rssi() or 0
            # Sanity-check the DUT is still alive over USB while AP is up.
            ctx.rpc.ping()
        finally:
            nmcli.disconnect(conn_name)
            ctx.rpc.wifi_ap(False)

        return {"ssid": ssid, "bssid": found.bssid, "signal": found.signal, "rssi": rssi}
