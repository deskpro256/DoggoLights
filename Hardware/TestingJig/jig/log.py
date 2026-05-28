"""Project-wide logging setup."""
import logging
import sys


def setup(level: int = logging.INFO) -> None:
    logging.basicConfig(
        level=level,
        format="%(asctime)s %(levelname)-7s %(name)-22s %(message)s",
        stream=sys.stdout,
    )


def get(name: str) -> logging.Logger:
    return logging.getLogger(name)
