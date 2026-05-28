"""Config loader.

Search order (first match wins):
    1. Explicit path passed to `load()`
    2. $DOGGOJIG_CONFIG (env var)
    3. ./config.yaml in the current working dir
    4. ~/.config/doggojig/config.yaml
    5. /etc/doggojig/config.yaml
    6. Packaged default (`jig/default_config.yaml`) - always succeeds
"""
from __future__ import annotations

import os
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml

try:
    from importlib.resources import files as _pkg_files
except ImportError:  # pragma: no cover
    _pkg_files = None  # type: ignore


@dataclass
class Settings:
    raw: dict[str, Any] = field(default_factory=dict)
    source: str = "default"

    def get(self, *keys: str, default: Any = None) -> Any:
        node: Any = self.raw
        for k in keys:
            if not isinstance(node, dict) or k not in node:
                return default
            node = node[k]
        return node


def _candidate_paths() -> list[Path]:
    paths: list[Path] = []
    env = os.environ.get("DOGGOJIG_CONFIG")
    if env:
        paths.append(Path(env))
    paths.append(Path.cwd() / "config.yaml")
    paths.append(Path.home() / ".config" / "doggojig" / "config.yaml")
    paths.append(Path("/etc/doggojig/config.yaml"))
    return paths


def _load_packaged_default() -> dict[str, Any]:
    if _pkg_files is None:
        return {}
    data = _pkg_files("jig").joinpath("default_config.yaml").read_text(encoding="utf-8")
    return yaml.safe_load(data) or {}


def _deep_merge(base: dict[str, Any], over: dict[str, Any]) -> dict[str, Any]:
    out = dict(base)
    for k, v in over.items():
        if isinstance(v, dict) and isinstance(out.get(k), dict):
            out[k] = _deep_merge(out[k], v)
        else:
            out[k] = v
    return out


def load(path: Path | str | None = None) -> Settings:
    base = _load_packaged_default()

    if path is not None:
        p = Path(path)
        with p.open("r", encoding="utf-8") as fh:
            user = yaml.safe_load(fh) or {}
        return Settings(raw=_deep_merge(base, user), source=str(p))

    for cand in _candidate_paths():
        if cand.is_file():
            with cand.open("r", encoding="utf-8") as fh:
                user = yaml.safe_load(fh) or {}
            return Settings(raw=_deep_merge(base, user), source=str(cand))

    return Settings(raw=base, source="packaged-default")
