#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
required = [
    "README.md", "Makefile", "apps/amh/amh_main.c", "apps/amh/Kconfig",
    "apps/amh/Make.defs", "apps/amh/Makefile", "scripts/bootstrap.sh",
    "scripts/configure.sh", "scripts/build.sh", "scripts/flash.sh",
    "scripts/serial.sh", "scripts/amh.ps1", "docs/ARCHITECTURE.md", "docs/GETTING_STARTED.md",
    "docs/COMMANDS.md", "boards/esp32s3/README.md",
]
missing = [path for path in required if not (root / path).is_file()]
if missing:
    raise SystemExit("Missing files: " + ", ".join(missing))

source = (root / "apps/amh/amh_main.c").read_text()
for command in ('"help"', '"sys"', '"wifi"', '"fs"', '"apps"', '"run"'):
    if command not in source:
        raise SystemExit("Missing command " + command)

flash = (root / "scripts/flash.sh").read_text()
if "erase_flash" in flash:
    raise SystemExit("Full-chip erase must not be in the flash script.")
if "AMH_BOARD_CONFIRMED" not in flash or '== "FLASH"' not in flash:
    raise SystemExit("Flash script must have board and typed-confirmation gates.")

config = (root / "scripts/configure.sh").read_text()
for option in ("CONFIG_BUILTIN", "CONFIG_NSH_BUILTIN_APPS", "CONFIG_SCHED_WAITPID"):
    if option not in config:
        raise SystemExit("Missing NuttX built-in application option " + option)

app_make = (root / "apps/amh/Makefile").read_text()
for setting in ("PROGNAME", "PRIORITY", "STACKSIZE", "MODULE"):
    if setting not in app_make:
        raise SystemExit("Missing NuttX app build setting " + setting)

windows_entry = (root / "scripts/amh.ps1").read_text(encoding="utf-8")
for token in ("ValidateSet", "xtensa-esp-elf-gcc.exe", "ConfirmBoard"):
    if token not in windows_entry:
        raise SystemExit("Windows workflow is missing " + token)

hardware = (root / "docs/HARDWARE.md").read_text(encoding="utf-8")
if "2.4 GHz only" not in hardware:
    raise SystemExit("ESP32-S3 2.4 GHz Wi-Fi limitation is not documented.")

getting_started = (root / "docs/GETTING_STARTED.md").read_text(encoding="utf-8")
if "Windows + PyCharm" not in getting_started or "WSL2" not in getting_started:
    raise SystemExit("Windows/PyCharm getting-started instructions are missing.")

print("AMH-OS project structure sanity check passed.")
