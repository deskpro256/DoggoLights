"""Wrappers around the `nmcli` CLI.

Used to associate the RPi's wlan0 with the DUT's softAP, then disconnect
when the wifi test step is done.
"""
from __future__ import annotations

import subprocess
from dataclasses import dataclass


class NmcliError(RuntimeError):
    pass


def _run(args: list[str], *, check: bool = True, timeout: float = 15.0) -> str:
    res = subprocess.run(
        ["nmcli", *args], capture_output=True, text=True, timeout=timeout
    )
    if check and res.returncode != 0:
        raise NmcliError(res.stderr.strip() or f"nmcli {args} failed")
    return res.stdout


@dataclass
class ApInfo:
    ssid: str
    bssid: str
    signal: int   # dBm? nmcli reports percent; keep raw int
    rssi_dbm: int | None = None


def scan(iface: str = "wlan0") -> list[ApInfo]:
    _run(["device", "wifi", "rescan", "ifname", iface], check=False)
    out = _run([
        "-t", "-f", "SSID,BSSID,SIGNAL", "device", "wifi", "list", "ifname", iface,
    ])
    results: list[ApInfo] = []
    for line in out.splitlines():
        # nmcli -t escapes ':' inside BSSID as '\:'. Split on un-escaped ':'.
        parts: list[str] = []
        cur = ""
        i = 0
        while i < len(line):
            c = line[i]
            if c == "\\" and i + 1 < len(line):
                cur += line[i + 1]
                i += 2
                continue
            if c == ":":
                parts.append(cur)
                cur = ""
                i += 1
                continue
            cur += c
            i += 1
        parts.append(cur)
        if len(parts) >= 3 and parts[0]:
            try:
                signal = int(parts[2])
            except ValueError:
                signal = 0
            results.append(ApInfo(ssid=parts[0], bssid=parts[1], signal=signal))
    return results


def connect_open(ssid: str, *, connection_name: str | None = None,
                 iface: str = "wlan0") -> None:
    name = connection_name or ssid
    # Delete any stale connection profile first so we always get a fresh assoc.
    subprocess.run(["nmcli", "connection", "delete", name],
                   capture_output=True, text=True)
    _run([
        "device", "wifi", "connect", ssid,
        "ifname", iface, "name", name,
    ], timeout=30.0)


def disconnect(connection_name: str) -> None:
    subprocess.run(["nmcli", "connection", "down", connection_name],
                   capture_output=True, text=True)
    subprocess.run(["nmcli", "connection", "delete", connection_name],
                   capture_output=True, text=True)


def rssi(iface: str = "wlan0") -> int | None:
    out = _run(["-t", "-f", "IN-USE,SIGNAL,SSID", "device", "wifi"], check=False)
    for line in out.splitlines():
        if line.startswith("*"):
            parts = line.split(":")
            if len(parts) >= 2:
                try:
                    return int(parts[1])
                except ValueError:
                    return None
    return None
