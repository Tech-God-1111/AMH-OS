# AMH-OS

**AMH-OS** is an ESP32-S3 operating-system distribution built on Apache NuttX.

The repository contains an AMH-OS NSH command application, a provisional ESP32-S3 board profile, setup/build/flash helpers, and documentation.

> **Hardware target:** ESP32-S3-WROOM development board. The board previously reported 16 MB flash and about 8 MB usable PSRAM. Its exact carrier-board model is not confirmed, so the upstream profile must be checked before flashing.

## Milestones

| Version | Scope | Implementation status |
| --- | --- | --- |
| 0.1 | NuttX boot, kernel startup, serial NSH | Upstream board configuration integrated; physical boot not yet tested |
| 0.2 | AMH-OS command system | `amh` dispatcher and subcommands added |
| 0.3 | CPU/memory monitoring | Uptime and optional heap reporting; CPU utilization/PSRAM telemetry still board-specific |
| 0.4 | Wi-Fi/network management | NuttX WAPI/ifconfig workflow documented and exposed through allow-listed launcher; driver setup/testing pending |
| 0.5 | Filesystem/application management | VFS directory/capacity inspection and allow-listed utility launcher added |

## Architecture

- **Apache NuttX** provides the RTOS kernel, scheduler, drivers, networking stack, VFS and NSH.
- **AMH-OS** provides the branded command layer and device-management helpers.
- **No independent kernel or custom bootloader is claimed.** This is an NuttX-based distribution.

## Repository layout

```
AMH-OS/
├── apps/amh/                 # AMH-OS NSH command app
├── boards/esp32s3/           # Provisional board profile notes
├── docs/                     # Architecture, setup, hardware and validation
├── scripts/                  # Upstream checkout, configure, build, flash, serial
├── tests/                    # Offline project sanity check
├── third_party/              # Downloaded NuttX + NuttX apps (not committed)
├── Makefile
└── README.md
```

## Build

Use Ubuntu 22.04/24.04 or WSL2 Ubuntu. Read [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md), then run:

```bash
bash scripts/bootstrap.sh
bash scripts/configure.sh
bash scripts/build.sh
```

The baseline profile is `esp32s3-devkit:nsh`. Install the Espressif Xtensa toolchain first. Wi-Fi driver and WAPI options must be enabled in NuttX menuconfig for the exact upstream revision.

The first boot should expose NuttX NSH on UART0 at 115200 baud. AMH commands are invoked as `amh help`, `amh sys`, `amh wifi`, `amh fs`, and `amh apps`.

## Safety and compatibility

- Building does not alter the board. Flashing replaces the current firmware.
- Preserve the Wi-Fi Radar sketch source before flashing.
- The flash helper requires typing `FLASH` and does not request a full-chip erase.
- Do not enable secure-boot or flash-encryption eFuses in this development project.
- Do not assume all ESP32-S3-WROOM carrier boards have identical USB, UART, flash, PSRAM or pin configurations.
- Never commit Wi-Fi credentials.

## Validation

This repository contains source and build integration, not a verified prebuilt firmware image. No successful NuttX compile or physical-board test is claimed yet. See [docs/VALIDATION.md](docs/VALIDATION.md).
