#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NUTTX="$ROOT/third_party/nuttx"
APPS="$ROOT/third_party/apps"
APP_DST="$APPS/examples/amh"
if [[ ! -x "$NUTTX/tools/configure.sh" || ! -d "$APPS/examples" ]]; then
  echo "Run bash scripts/bootstrap.sh first." >&2
  exit 1
fi
mkdir -p "$APP_DST"
cp "$ROOT/apps/amh/amh_main.c" "$APP_DST/amh_main.c"
cp "$ROOT/apps/amh/Kconfig" "$APP_DST/Kconfig"
cp "$ROOT/apps/amh/Make.defs" "$APP_DST/Make.defs"
cp "$ROOT/apps/amh/Makefile" "$APP_DST/Makefile"
grep -q 'examples/amh/Kconfig' "$APPS/examples/Kconfig" || printf '\nsource "examples/amh/Kconfig"\n' >> "$APPS/examples/Kconfig"
grep -q 'examples/amh/Make.defs' "$APPS/examples/Make.defs" || printf '\ninclude examples/amh/Make.defs\n' >> "$APPS/examples/Make.defs"
cd "$NUTTX"
./tools/configure.sh esp32s3-devkit:nsh
for option in CONFIG_EXAMPLES_AMH CONFIG_MM_MALLINFO CONFIG_LIBC_EXECFUNCS; do
  grep -q "^$option=y$" .config || printf '\n%s=y\n' "$option" >> .config
done
make olddefconfig
echo "Configured esp32s3-devkit:nsh with AMH-OS command."
echo "Wi-Fi driver and WAPI options must be enabled manually in make menuconfig."
echo "Review flash settings before building or flashing."
