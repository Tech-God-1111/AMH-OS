#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 1 ]]; then
  echo "Usage: bash scripts/serial.sh /dev/ttyUSB0" >&2
  exit 2
fi
PORT="$1"
if ! command -v picocom >/dev/null 2>&1; then
  echo "Install picocom first: sudo apt install picocom" >&2
  exit 1
fi
exec picocom -b 115200 "$PORT"
