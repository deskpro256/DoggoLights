"""Dev shim. Same behaviour as the installed `doggojig-fetch` console script."""
import sys

from jig.cli import fetch_main

if __name__ == "__main__":
    fetch_main()
    sys.exit(0)
