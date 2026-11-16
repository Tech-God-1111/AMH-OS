#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NUTTX="$ROOT/third_party/nuttx"
PORT="$1"
if [[ -z "$PORT" || ! -f "$NUTTX/nuttx" ]]; then
  echo "Usage: ./scripts/flash.sh /dev/ttyUSB0 (build firmware first)" >&2
  exit 2
fi
echo "WARNING: this replaces the firmware currently on $PORT."
echo "No full-chip erase is requested. Confirm board, port and flash settings."
read -r -p "Type FLASH to continue: " CONFIRM
[[ "$CONFIRM" == "FLASH" ]] || { echo "Cancelled."; exit 1; }
cd "$NUTTX"
make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)" flash ESPTOOL_PORT="$PORT" ESPTOOL_BINDIR=./
