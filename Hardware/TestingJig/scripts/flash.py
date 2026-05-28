"""Dev shim. Same behaviour as the installed `doggojig-flash` console script."""
import sys

from jig.cli import flash_main

if __name__ == "__main__":
    flash_main()
    sys.exit(0)
