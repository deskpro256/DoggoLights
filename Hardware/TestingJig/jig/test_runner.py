"""Main test orchestration: fetch fw -> flash -> connect RPC -> run tests -> save."""
from __future__ import annotations

import threading
import time
from typing import Any

from .database import ResultsDB
from .dut.rpc_client import RpcClient, RpcConfig
from .firmware.esptool_wrapper import Esptool, EsptoolConfig
from .firmware.updater import FirmwareUpdater
from .log import get
from .settings import Settings
from .state import STORE
from .tests.base import JigContext, TestStep
from .tests.button_test import ButtonTest
from .tests.current_test import CurrentTest
from .tests.led_test import LedTest
from .tests.lock_test import LockTest
from .tests.wifi_test import WifiTest

LOG = get(__name__)

STEPS_BUILDERS = [LedTest, CurrentTest, WifiTest, ButtonTest, LockTest]


class TestRunner:
    """Runs the full sequence in a worker thread.

    Hardware drivers (GPIO/I2C) are injected so we can stub them out on a
    dev machine; the Flask app builds them once at startup.
    """

    def __init__(
        self,
        settings: Settings,
        db: ResultsDB,
        *,
        gpio=None,
        current_sensor=None,
        lux1=None,
        lux2=None,
    ) -> None:
        self.settings = settings
        self.db = db
        self.gpio = gpio
        self.current_sensor = current_sensor
        self.lux1 = lux1
        self.lux2 = lux2
        self._thread: threading.Thread | None = None
        self._busy = threading.Lock()

    # --- public --------------------------------------------------------
    def is_busy(self) -> bool:
        return self._busy.locked()

    def start(self, port: str, *, flash_only: bool = False) -> None:
        if not self._busy.acquire(blocking=False):
            raise RuntimeError("runner already busy")
        self._thread = threading.Thread(
            target=self._safe_run, args=(port, flash_only), daemon=True
        )
        self._thread.start()

    # --- internals -----------------------------------------------------
    def _safe_run(self, port: str, flash_only: bool) -> None:
        try:
            self._run(port, flash_only=flash_only)
        except Exception as e:
            LOG.exception("runner crashed")
            STORE.push_log(f"FATAL: {e}")
            with STORE as st:
                st.overall = "fail"
                st.finished_at = time.time()
        finally:
            self._busy.release()
            self.db.save_run(STORE.snapshot())

    def _run(self, port: str, *, flash_only: bool = False) -> None:
        STORE.reset()
        with STORE as st:
            st.dut_port = port
            st.overall = "running"
            st.started_at = time.time()

        # 1. firmware
        STORE.push_log("fetching latest firmware")
        fw_cfg = self.settings.get("firmware", default={}) or {}
        updater = FirmwareUpdater(
            fw_cfg.get("latest_manifest_url", ""),
            fw_cfg.get("binary_url", ""),
            fw_cfg.get("cache_dir", "./.cache/firmware"),
            factory_url=fw_cfg.get("factory_url", ""),
        )
        fw = updater.ensure_latest(factory=True)
        with STORE as st:
            st.firmware_version = fw.version

        # 2. boot into download mode, read MAC, then flash + hard-reset.
        esp_cfg_raw = self.settings.get("esptool", default={}) or {}
        esp = Esptool(port, EsptoolConfig(**{k: v for k, v in esp_cfg_raw.items()
                                              if k in EsptoolConfig.__dataclass_fields__}))

        try:
            mac = esp.read_mac()
            STORE.push_log(f"DUT MAC: {mac}")
        except Exception as e:
            LOG.warning("read_mac failed: %s", e)
            mac = ""

        if self.gpio:
            with self.gpio.enter_download_mode():
                STORE.push_log("flashing firmware")
                esp.flash(fw.path)
            self.gpio.reset_dut()
        else:
            STORE.push_log("flashing firmware")
            esp.flash(fw.path)

        if flash_only:
            STORE.push_log("flash-only: skipping RPC + tests")
            with STORE as st:
                st.dut_mac = mac or None
                from .dut import mac as macmod
                st.dut_serial_short = macmod.short(mac) if mac else None
                st.overall = "pass"
                st.finished_at = time.time()
            return

        # 3. After flash + hard_reset the USB-CDC device re-enumerates on
        # Windows; the old COM handle is invalid for a moment. Give the OS
        # a few seconds to bring it back before we open RPC.
        boot_delay = float(self.settings.get("rpc", "post_flash_delay_s", default=5.0))
        STORE.push_log(f"waiting {boot_delay:.1f}s for DUT to reboot")
        time.sleep(boot_delay)

        # 4. open RPC and do a single GET_STATUS (no retries, no pinging).
        rpc_cfg_raw = self.settings.get("rpc", default={}) or {}
        rpc = RpcClient(port, RpcConfig(**{k: v for k, v in rpc_cfg_raw.items()
                                            if k in RpcConfig.__dataclass_fields__
                                            and k != "post_flash_delay_s"}))
        with rpc:
            status = rpc.wait_ready(timeout_s=15.0)
            STORE.push_log(f"DUT status: {status}")
            from .dut import mac as macmod
            short = macmod.short(mac) if mac else ""
            with STORE as st:
                st.dut_mac = mac or None
                st.dut_serial_short = short or None

            # Make sure we start unlocked so MFG_PASS at the end is meaningful.
            try:
                rpc.mfg_unlock()
            except Exception as e:
                LOG.warning("mfg_unlock failed: %s", e)

            ctx = JigContext(
                settings=self.settings,
                rpc=rpc,
                gpio=self.gpio,
                current_sensor=self.current_sensor,
                lux1=self.lux1,
                lux2=self.lux2,
            )
            ctx.scratch["dut_mac"] = mac

            overall_pass = True
            for step_cls in STEPS_BUILDERS:
                step: TestStep = step_cls()
                result = step.execute(ctx)
                if result.status != "pass":
                    overall_pass = False
                    break

        with STORE as st:
            st.overall = "pass" if overall_pass else "fail"
            st.finished_at = time.time()
