"""SQLite results store. One row per completed run."""
from __future__ import annotations

import json
import sqlite3
import time
from pathlib import Path
from typing import Any

_SCHEMA = """
CREATE TABLE IF NOT EXISTS runs (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp       REAL NOT NULL,
    mac             TEXT,
    serial_short    TEXT,
    firmware        TEXT,
    overall         TEXT NOT NULL,
    duration_s      REAL,
    steps_json      TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_runs_mac ON runs(mac);
"""


class ResultsDB:
    def __init__(self, path: str | Path) -> None:
        self.path = Path(path)
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self._conn() as c:
            c.executescript(_SCHEMA)

    def _conn(self) -> sqlite3.Connection:
        c = sqlite3.connect(self.path)
        c.row_factory = sqlite3.Row
        return c

    def save_run(self, snapshot: dict[str, Any]) -> int:
        s = snapshot["state"]
        started = s.get("started_at") or time.time()
        finished = s.get("finished_at") or time.time()
        with self._conn() as c:
            cur = c.execute(
                "INSERT INTO runs(timestamp, mac, serial_short, firmware, overall, duration_s, steps_json)"
                " VALUES(?,?,?,?,?,?,?)",
                (
                    finished,
                    s.get("dut_mac"),
                    s.get("dut_serial_short"),
                    s.get("firmware_version"),
                    s.get("overall"),
                    finished - started,
                    json.dumps(s.get("steps", [])),
                ),
            )
            return int(cur.lastrowid or 0)

    def recent(self, limit: int = 50) -> list[dict[str, Any]]:
        with self._conn() as c:
            rows = c.execute(
                "SELECT * FROM runs ORDER BY id DESC LIMIT ?", (limit,)
            ).fetchall()
        return [dict(r) for r in rows]
