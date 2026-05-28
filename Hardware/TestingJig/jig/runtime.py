"""Shared entry-point implementations used by both the installed `doggojig`
console scripts and the in-repo `run.py` / `scripts/*.py` shims.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from . import log, settings
from .app import create_app
from .database import ResultsDB
from .firmware.esptool_wrapper import Esptool, EsptoolConfig
from .firmware.updater import FirmwareUpdater
from .test_runner import TestRunner


def _build_hardware(cfg):
    """Construct I2C/GPIO drivers on a real Pi; degrade to None stubs elsewhere."""
    gpio = current = lux1 = lux2 = None
    try:
        from .hardware.gpio import JigGPIO
        from .hardware.i2c_bus import I2CBus
        from .hardware.ina228 import INA228, INA228Config
        from .hardware.lux_sensor import LuxSensor

        gpio = JigGPIO(
            boot_pin=cfg.get("gpio", "boot_pin"),
            reset_pin=cfg.get("gpio", "reset_pin"),
            button_solenoid=cfg.get("gpio", "button_solenoid"),
            dut_power=cfg.get("gpio", "dut_power"),
        )
        bus = I2CBus(cfg.get("i2c", "bus", default=1))
        current = INA228(bus, INA228Config(address=cfg.get("i2c", "ina228_addr")))
        lux1 = LuxSensor(bus, cfg.get("i2c", "lux_sensor_1"))
        lux2 = LuxSensor(bus, cfg.get("i2c", "lux_sensor_2"))
    except Exception as e:
        log.get(__name__).warning("hardware drivers unavailable (%s) - UI only", e)
    return gpio, current, lux1, lux2


def run_server() -> None:
    log.setup()
    cfg = settings.load()
    log.get(__name__).info("config source: %s", cfg.source)

    db_path = cfg.get("database", "path", default="./results.sqlite3")
    Path(db_path).parent.mkdir(parents=True, exist_ok=True)
    db = ResultsDB(db_path)

    gpio, current, lux1, lux2 = _build_hardware(cfg)
    runner = TestRunner(cfg, db, gpio=gpio, current_sensor=current, lux1=lux1, lux2=lux2)

    app = create_app(cfg, runner, db)
    app.run(
        host=cfg.get("server", "host", default="0.0.0.0"),
        port=cfg.get("server", "port", default=8080),
        threaded=True,
        use_reloader=False,
    )


def run_flash() -> int:
    ap = argparse.ArgumentParser(prog="doggojig-flash")
    ap.add_argument("--port", required=True)
    ap.add_argument("--erase", action="store_true")
    ap.add_argument("--no-fetch", action="store_true",
                    help="skip downloading firmware, use cached")
    args = ap.parse_args()

    log.setup()
    cfg = settings.load()

    esp_raw = cfg.get("esptool", default={}) or {}
    esp = Esptool(args.port, EsptoolConfig(**{
        k: v for k, v in esp_raw.items() if k in EsptoolConfig.__dataclass_fields__
    }))

    if args.erase:
        esp.erase()
        return 0

    fw_cfg = cfg.get("firmware", default={}) or {}
    cache_dir = Path(fw_cfg.get("cache_dir", "./.cache/firmware"))
    cache_dir.mkdir(parents=True, exist_ok=True)

    if args.no_fetch:
        bin_path = cache_dir / "firmware-factory.bin"
        if not bin_path.exists():
            bin_path = cache_dir / "firmware.bin"
        if not bin_path.exists():
            print("no cached firmware", file=sys.stderr)
            return 2
    else:
        updater = FirmwareUpdater(
            fw_cfg.get("latest_manifest_url", ""),
            fw_cfg.get("binary_url", ""),
            cache_dir,
            factory_url=fw_cfg.get("factory_url", ""),
        )
        bin_path = updater.ensure_latest(factory=True).path

    esp.flash(bin_path)
    return 0


def run_fetch() -> int:
    log.setup()
    cfg = settings.load()
    fw_cfg = cfg.get("firmware", default={}) or {}
    cache_dir = Path(fw_cfg.get("cache_dir", "./.cache/firmware"))
    cache_dir.mkdir(parents=True, exist_ok=True)
    updater = FirmwareUpdater(
        fw_cfg.get("latest_manifest_url", ""),
        fw_cfg.get("binary_url", ""),
        cache_dir,
        factory_url=fw_cfg.get("factory_url", ""),
    )
    fw = updater.ensure_latest(allow_offline=False, factory=True)
    print(f"firmware {fw.version} -> {fw.path}")
    return 0
