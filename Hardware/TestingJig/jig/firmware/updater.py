"""Download the latest firmware binary from GitHub and cache it on disk.

The repo publishes a small manifest at `Firmware/latest/latest.txt` plus the
`firmware.bin` next to it. We pull the manifest first so we can record the
version string we flashed, then download the binary if the version changed.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import requests

from ..log import get

LOG = get(__name__)


@dataclass
class FirmwareInfo:
    version: str
    path: Path


class FirmwareUpdater:
    def __init__(
        self,
        manifest_url: str,
        binary_url: str,
        cache_dir: str | Path,
        *,
        factory_url: str = "",
    ) -> None:
        self.manifest_url = manifest_url
        self.binary_url = binary_url
        self.factory_url = factory_url
        self.cache = Path(cache_dir)
        self.cache.mkdir(parents=True, exist_ok=True)

    def _read_version(self) -> str:
        r = requests.get(self.manifest_url, timeout=10)
        r.raise_for_status()
        # latest.txt is "version=...\nurl=...\n" - take the first version line.
        for line in r.text.splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith("version="):
                return line.split("=", 1)[1].strip()
            # Backwards compat: bare version string on first line.
            return line
        raise RuntimeError("empty manifest")

    def ensure_latest(self, *, allow_offline: bool = True,
                      factory: bool = True) -> FirmwareInfo:
        """Download the latest firmware if needed.

        If `factory=True` and `factory_url` is set, downloads the merged
        factory image (bootloader+partition+otadata+app) used by the testing
        jig. Otherwise downloads the app-only OTA binary.
        """
        version_file = self.cache / "version.txt"
        if factory and self.factory_url:
            bin_file = self.cache / "firmware-factory.bin"
            url = self.factory_url
        else:
            bin_file = self.cache / "firmware.bin"
            url = self.binary_url

        try:
            version = self._read_version()
        except Exception as e:
            if allow_offline and bin_file.exists() and version_file.exists():
                LOG.warning("offline, using cached firmware: %s", e)
                return FirmwareInfo(version_file.read_text().strip(), bin_file)
            raise

        cached = version_file.read_text().strip() if version_file.exists() else ""
        if cached != version or not bin_file.exists():
            LOG.info("downloading %s firmware %s", "factory" if factory else "app", version)
            r = requests.get(url, timeout=60)
            r.raise_for_status()
            bin_file.write_bytes(r.content)
            version_file.write_text(version)
        else:
            LOG.info("firmware %s already cached", version)

        return FirmwareInfo(version, bin_file)
