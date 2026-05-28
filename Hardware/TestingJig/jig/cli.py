"""Console entry points exposed by pyproject.toml.

We re-use the existing module-level mains from `run.py` / `scripts/` so the
"installed" experience and the "git clone + python run.py" experience stay
identical.
"""
from __future__ import annotations

import sys


def main() -> None:
    # Lazy import so `--help` is fast and so missing optional deps (RPi.GPIO
    # off-Pi) don't blow up at import time.
    from .runtime import run_server
    run_server()


def flash_main() -> None:
    from .runtime import run_flash
    sys.exit(run_flash())


def fetch_main() -> None:
    from .runtime import run_fetch
    sys.exit(run_fetch())
