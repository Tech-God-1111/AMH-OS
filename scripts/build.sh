#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NUTTX="$ROOT/third_party/nuttx"
if [[ ! -f "$NUTTX/.config" ]]; then
  echo "No configuration. Run ./scripts/configure.sh first." >&2
  exit 1
fi
for tool in xtensa-esp-elf-gcc xtensa-esp-elf-ld; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "Missing $tool. Install the Espressif Xtensa toolchain and update PATH." >&2
    exit 1
  }
done
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
cd "$NUTTX"
make -j"$JOBS"
echo "Build finished. Inspect generated images before flashing."
