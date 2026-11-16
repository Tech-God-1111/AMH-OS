# AMH-OS

**AMH-OS** is a lightweight ESP32-S3 operating-system distribution built on Apache NuttX.

This repository contains the AMH-OS board configuration, startup integration, custom NSH command applications, network utilities, filesystem/application management helpers, and reproducible build/flash scripts.

> **Hardware target:** ESP32-S3-WROOM development board. The current development board has been observed with 16 MB flash and 8 MB PSRAM. The exact carrier-board model is not yet confirmed, so the initial board profile is based on Espressif's ESP32-S3-DevKitC configuration and must be checked against the board's pinout and flash/PSRAM wiring before flashing.

## Milestones

| Version | Scope |
| --- | --- |
| 0.1 | NuttX boot, kernel startup, serial console / NSH |
| 0.2 | AMH-OS command namespace and help |
| 0.3 | CPU frequency, uptime, heap and PSRAM diagnostics |
| 0.4 | Wi-Fi/network status and management commands |
| 0.5 | Mounted filesystem and application inventory/launch helpers |

## Architecture

- **Apache NuttX** provides the RTOS kernel, scheduler, drivers, networking stack, VFS and NSH.
- **AMH-OS applications** provide the branded shell commands and device-management layer.
- **Board configuration** is kept separate from AMH-OS application code.
- **No custom bootloader or kernel fork is claimed:** NuttX's ESP32-S3 boot flow and kernel are used and configured for this distribution.

## Repository layout

```
AMH-OS/
├── apps/amh/                 # AMH-OS NSH commands
├── boards/                   # Board profile and configuration notes
├── config/                   # NuttX configuration fragments
├── docs/                     # Architecture, setup and hardware notes
├── scripts/                  # WSL/Linux setup, configure, build and flash
├── tests/                    # Host-side checks for project metadata/scripts
├── third_party/              # Upstream NuttX checkout location (not vendored)
├── .gitmodules               # Upstream NuttX and apps references
├── Makefile                  # Convenience targets
└── README.md
```

## Build environment

The supported development path is **Ubuntu 22.04/24.04 or WSL2 Ubuntu** on Windows. The scripts use the upstream NuttX build system and ESP32-S3 toolchain. They do not erase flash automatically.

1. Read [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md).
2. Confirm the exact board model and flash/PSRAM configuration in [boards/esp32s3/README.md](boards/esp32s3/README.md).
3. Run `./scripts/bootstrap.sh` in Ubuntu/WSL2.
4. Run `./scripts/configure.sh`, then `./scripts/build.sh`.
5. Inspect the generated image and port before running `./scripts/flash.sh`.

The first boot is expected on the serial console at **115200 baud**. The shell prompt is NuttX NSH; AMH-OS commands are registered as additional commands.

## Safety and hardware notes

- Do not run `esptool erase_flash` as a routine setup step. Flashing can replace the existing Wi-Fi Radar firmware.
- Keep a copy of the Wi-Fi Radar sketch before the first flash.
- Do not enable flash encryption, secure boot eFuses, or irreversible security settings in this development project.
- Do not assume every ESP32-S3-WROOM carrier board has the same USB, UART, flash, PSRAM or pin configuration.
- Wi-Fi credentials should be entered locally during setup; never commit them to this repository.

## Current validation status

This repository is the source project, not a prebuilt firmware download. Build success and physical-board boot/network tests must be recorded separately. A successful compile does not prove that the unconfirmed carrier board is fully compatible.
