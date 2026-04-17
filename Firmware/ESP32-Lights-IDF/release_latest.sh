#!/usr/bin/env bash
set -euo pipefail

# Release helper:
# 1) Update version.txt (firmware image version, with v-prefix)
# 2) Run idf.py build (default)
# 3) Copy build/doggo_lights.bin -> ../latest/firmware.bin
# 4) Update ../latest/latest.txt with version + OTA URL

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <version> [--no-build]"
  echo "Example: $0 2.1.1"
  echo "Example: $0 v2.1.1"
  echo "Example: $0 v2.1.1 --no-build"
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIRMWARE_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SRC_BIN="$SCRIPT_DIR/build/doggo_lights.bin"
DST_BIN="$FIRMWARE_ROOT/latest/firmware.bin"
MANIFEST="$FIRMWARE_ROOT/latest/latest.txt"
VERSION_TXT="$SCRIPT_DIR/version.txt"
OTA_URL="https://raw.githubusercontent.com/deskpro256/DoggoLights/main/Firmware/latest/firmware.bin"

VERSION="$1"
VERSION="${VERSION#v}"
FW_VERSION="v$VERSION"

DO_BUILD=1
if [[ $# -eq 2 ]]; then
  if [[ "$2" == "--no-build" ]]; then
    DO_BUILD=0
  else
    echo "Unknown option: $2"
    echo "Only supported option is: --no-build"
    exit 1
  fi
fi

printf "%s\n" "$FW_VERSION" > "$VERSION_TXT"

if [[ "$DO_BUILD" -eq 1 ]]; then
  echo "Building firmware with version $FW_VERSION ..."
  (cd "$SCRIPT_DIR" && idf.py reconfigure build)
fi

if [[ ! -f "$SRC_BIN" ]]; then
  echo "Error: firmware binary not found at: $SRC_BIN"
  echo "Build may have failed."
  exit 1
fi

BIN_VERSIONS="$(strings "$SRC_BIN" | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$' || true)"
if ! grep -Fqx "$FW_VERSION" <<< "$BIN_VERSIONS"; then
  echo "Error: built firmware binary does not contain expected version '$FW_VERSION'"
  echo "Detected version strings in binary:"
  echo "$BIN_VERSIONS" | head -n 5
  echo "Refusing to publish stale firmware.bin"
  exit 1
fi

cp -f "$SRC_BIN" "$DST_BIN"

cat > "$MANIFEST" <<EOF
version=$VERSION
url=$OTA_URL
EOF

echo "Updated OTA artifacts:"
echo "  Binary   : $DST_BIN"
echo "  Firmware : $VERSION_TXT -> $FW_VERSION"
echo "  Manifest : $MANIFEST"
echo "  Version  : $VERSION"
echo "  Built    : $([[ "$DO_BUILD" -eq 1 ]] && echo yes || echo no)"
