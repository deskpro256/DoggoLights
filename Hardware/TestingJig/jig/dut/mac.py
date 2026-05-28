"""MAC helpers."""
from __future__ import annotations


def normalize(mac: str) -> str:
    s = mac.strip().upper().replace("-", ":")
    parts = s.split(":")
    if len(parts) != 6:
        raise ValueError(f"bad MAC: {mac!r}")
    return ":".join(p.zfill(2) for p in parts)


def short(mac: str) -> str:
    """Last 3 bytes, no separators - matches firmware's AP SSID suffix."""
    n = normalize(mac).replace(":", "")
    return n[-6:]


def ap_ssid(mac: str, prefix: str = "DoggoLights") -> str:
    return f"{prefix}{short(mac)}"
