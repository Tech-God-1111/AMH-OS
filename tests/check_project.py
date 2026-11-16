#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[1]
required = [
    "README.md", "Makefile", "apps/amh/amh_main.c", "apps/amh/Kconfig",
    "apps/amh/Make.defs", "scripts/bootstrap.sh", "scripts/configure.sh",
    "scripts/build.sh", "scripts/flash.sh", "scripts/serial.sh",
    "docs/ARCHITECTURE.md", "docs/GETTING_STARTED.md", "boards/esp32s3/README.md",
]
missing = [p for p in required if not (root / p).is_file()]
if missing:
    raise SystemExit("Missing files: " + ", ".join(missing))
source = (root / "apps/amh/amh_main.c").read_text()
for name in ('"help"', '"sys"', '"wifi"', '"fs"', '"apps"', '"run"'):
    if name not in source:
        raise SystemExit("Missing command " + name)
flash = (root / "scripts/flash.sh").read_text()
if "erase_flash" in flash:
    raise SystemExit("Full-chip erase must not be in the flash script.")
print("AMH-OS project structure sanity check passed.")
