#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NUTTX="$ROOT/third_party/nuttx"
if [[ "${AMH_BOARD_CONFIRMED:-}" != "YES" ]]; then
  echo "REFUSED: confirm the exact carrier-board model first (AMH_BOARD_CONFIRMED=YES)." >&2
  exit 2
fi
if [[ $# -lt 1 ]]; then
  echo "Usage: AMH_BOARD_CONFIRMED=YES bash scripts/flash.sh /dev/ttyUSB0" >&2
  exit 2
fi
PORT="$1"
if [[ ! -f "$NUTTX/nuttx" ]]; then
  echo "Firmware ELF not found. Run the build first." >&2
  exit 1
fi
echo "WARNING: flashing replaces the firmware currently on $PORT."
echo "No full-chip erase is requested. Verify board, port and flash settings."
read -r -p "Type FLASH to continue: " CONFIRM
[[ "$CONFIRM" == "FLASH" ]] || { echo "Cancelled."; exit 1; }
cd "$NUTTX"
make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)" flash ESPTOOL_PORT="$PORT" ESPTOOL_BINDIR=./
