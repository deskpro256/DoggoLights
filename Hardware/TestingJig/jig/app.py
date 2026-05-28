"""Flask app: serves the Vue UI, REST control, SSE state stream."""
from __future__ import annotations

import json
import time
from pathlib import Path
from typing import Any

from flask import Flask, Response, jsonify, request, send_from_directory

from .database import ResultsDB
from .settings import Settings
from .state import STORE
from .test_runner import TestRunner

WEB_DIR = Path(__file__).resolve().parent / "web"


def _autodetect_port(settings: Settings) -> str | None:
    """Pick the first device matching `usb.port_glob`.

    On the Pi this is `/dev/ttyACM*` and there's only ever one DUT in the jig,
    so first-match is fine. On Windows the glob is `COM*` which `glob.glob`
    can't expand, so we fall back to `pyserial`'s list_ports for dev testing.
    """
    import glob as _glob
    import fnmatch
    pattern = settings.get("usb", "port_glob", default="/dev/ttyACM*")
    matches = sorted(_glob.glob(pattern))
    if matches:
        return matches[0]

    try:
        from serial.tools import list_ports
    except ImportError:
        return None
    ports = [p.device for p in list_ports.comports()]
    # If the configured pattern is a COM*-style glob, honour it.
    filtered = [p for p in ports if fnmatch.fnmatch(p, pattern)]
    if filtered:
        return sorted(filtered)[0]
    return ports[0] if ports else None


def create_app(settings: Settings, runner: TestRunner, db: ResultsDB) -> Flask:
    app = Flask(__name__, static_folder=None)

    # --- static UI ----------------------------------------------------
    @app.get("/")
    def index() -> Response:
        return send_from_directory(WEB_DIR, "index.html")

    @app.get("/<path:filename>")
    def static_files(filename: str) -> Response:
        return send_from_directory(WEB_DIR, filename)

    # --- REST ---------------------------------------------------------
    @app.get("/api/state")
    def get_state() -> Any:
        return jsonify(STORE.snapshot())

    @app.get("/api/history")
    def get_history() -> Any:
        return jsonify(db.recent())

    @app.post("/api/run")
    def start_run() -> Any:
        body = request.get_json(silent=True) or {}
        port = body.get("port") or _autodetect_port(settings)
        flash_only = bool(body.get("flash_only", False))
        if not port:
            glob_pattern = settings.get("usb", "port_glob", default="/dev/ttyACM*")
            return jsonify({
                "ok": False,
                "err": f"no DUT detected (looked for {glob_pattern})",
            }), 404
        try:
            runner.start(port, flash_only=flash_only)
        except RuntimeError as e:
            return jsonify({"ok": False, "err": str(e)}), 409
        return jsonify({"ok": True, "port": port, "flash_only": flash_only})

    @app.get("/api/dut")
    def dut_info() -> Any:
        port = _autodetect_port(settings)
        return jsonify({"port": port, "present": bool(port)})

    # --- SSE state stream --------------------------------------------
    @app.get("/api/events")
    def events() -> Response:
        def gen():
            last_rev = -1
            while True:
                snap = STORE.snapshot()
                if snap["rev"] != last_rev:
                    last_rev = snap["rev"]
                    yield f"data: {json.dumps(snap)}\n\n"
                time.sleep(0.25)
        return Response(gen(), mimetype="text/event-stream")

    return app
