#!/usr/bin/env bash
set -euo pipefail
PORT="$1"
if [[ -z "$PORT" ]]; then echo "Usage: ./scripts/serial.sh /dev/ttyUSB0"; exit 2; fi
command -v picocom >/dev/null 2>&1 || { echo "Install picocom first."; exit 1; }
exec picocom -b 115200 "$PORT"
