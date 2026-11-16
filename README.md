# AMH-OS

**AMH-OS** is an ESP32-S3 operating-system distribution built on Apache NuttX.

The repository contains an AMH-OS NSH command application, a provisional ESP32-S3 board profile, Windows/PyCharm workflow scripts, and documentation.

> **Hardware target:** ESP32-S3-WROOM development board. The board previously reported 16 MB flash and about 8 MB usable PSRAM. Its exact carrier-board model is not confirmed, so the upstream profile must be checked before flashing.

## Milestones

| Version | Scope | Implementation status |
| --- | --- | --- |
| 0.1 | NuttX boot, kernel startup, serial NSH | Upstream board configuration integrated; physical boot not yet tested |
| 0.2 | AMH-OS command system | AMH command dispatcher and subcommands added |
| 0.3 | CPU/memory monitoring | Uptime and optional heap statistics; live CPU utilization/PSRAM telemetry remains board-specific |
| 0.4 | Wi-Fi/network management | NuttX WAPI/network workflow and allow-listed native utility launcher added; driver configuration and hardware test pending |
| 0.5 | Filesystem/application management | VFS directory/capacity inspection and allow-listed utility launcher added |

## Architecture

- **Apache NuttX** provides the RTOS kernel, scheduler, drivers, networking stack, VFS and NSH.
- **AMH-OS** provides the branded command layer and device-management helpers.
- **No independent kernel or custom bootloader is claimed.** This is an NuttX-based distribution.

## Windows + PyCharm workflow

Windows is the primary host workflow. No Linux installation, Ubuntu installation, or WSL2 is required.

1. Install Git for Windows, Python 3, and Cygwin with the packages listed in `docs/GETTING_STARTED.md`.
2. Open this repository in PyCharm.
3. Run `scripts/amh.ps1` from the PyCharm PowerShell terminal:
   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass
   .\scripts\amh.ps1 setup
   .\scripts\amh.ps1 bootstrap
   .\scripts\amh.ps1 configure
   .\scripts\amh.ps1 build
   .\scripts\amh.ps1 test
   ```

The PowerShell entry point handles the workflow, while Cygwin supplies the Bash/GNU build utilities required by NuttX's configuration and make system. This is a Windows development setup, not a Linux distribution. NuttX documents Cygwin as a Windows build environment; the native CMD build path is described upstream as less mature and not verified for every target.

The ESP32-S3 Xtensa Windows toolchain is downloaded and checksum-verified by the setup command. The exact Windows/Cygwin integration still requires a real build test; do not treat the build as validated until it succeeds locally.

## Wi-Fi hardware constraint

The ESP32-S3 supports **2.4 GHz Wi-Fi only** (IEEE 802.11 b/g/n). AMH-OS Wi-Fi testing must use a 2.4 GHz access point/network. It cannot join a 5 GHz-only network.

## NuttX build and board notes

The baseline profile is `esp32s3-devkit:nsh`. Wi-Fi driver and WAPI options must be enabled for the exact upstream NuttX revision. The exact carrier-board model is still unconfirmed.

## Safety and compatibility

- Building does not alter the board. Flashing replaces the firmware.
- Preserve the Wi-Fi Radar sketch source before flashing.
- The flash helper requires explicit board confirmation and typing `FLASH`; it does not request a full-chip erase.
- Do not enable secure-boot or flash-encryption eFuses in this development project.
- Do not assume all ESP32-S3-WROOM carrier boards have identical USB, UART, flash, PSRAM or pin configurations.
- Never commit Wi-Fi credentials.

## Validation

This repository contains source and build integration, not a verified prebuilt firmware image. No successful NuttX compile or physical-board test is claimed yet. See `docs/VALIDATION.md`.
