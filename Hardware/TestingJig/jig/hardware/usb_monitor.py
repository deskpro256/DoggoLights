"""Watch for DUT plug/unplug events on `/dev/ttyACM*` using pyudev."""
from __future__ import annotations

import glob
import threading
from typing import Callable

try:
    import pyudev  # type: ignore
except ImportError:
    pyudev = None  # type: ignore


PortCallback = Callable[[str], None]


class UsbMonitor:
    def __init__(self, port_glob: str = "/dev/ttyACM*") -> None:
        self.port_glob = port_glob
        self._on_add: PortCallback | None = None
        self._on_remove: PortCallback | None = None
        self._thread: threading.Thread | None = None
        self._stop = threading.Event()

    def on_add(self, cb: PortCallback) -> None:
        self._on_add = cb

    def on_remove(self, cb: PortCallback) -> None:
        self._on_remove = cb

    def current_ports(self) -> list[str]:
        return sorted(glob.glob(self.port_glob))

    def start(self) -> None:
        if pyudev is None:
            raise RuntimeError("pyudev not available on this host")
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._stop.set()

    def _run(self) -> None:
        ctx = pyudev.Context()
        monitor = pyudev.Monitor.from_netlink(ctx)
        monitor.filter_by(subsystem="tty")
        for device in iter(monitor.poll, None):
            if self._stop.is_set():
                return
            node = device.device_node or ""
            if not node.startswith("/dev/ttyACM"):
                continue
            if device.action == "add" and self._on_add:
                self._on_add(node)
            elif device.action == "remove" and self._on_remove:
                self._on_remove(node)
