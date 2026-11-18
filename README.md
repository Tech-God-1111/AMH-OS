# AMH-OS Lite

A lightweight ESP32-S3 device environment built on the official MicroPython runtime. The board runs the Python system layer and hosts its own HTML/CSS/JavaScript dashboard. No custom kernel, NuttX, Cygwin, Bash, Make, or cross-compiler is needed for application development.

## Foundation features
- Wi-Fi station connection with first-boot setup access point.
- Local browser dashboard and JSON API.
- Device ID, runtime, free heap, uptime, CPU frequency, and network status.
- Read-only root filesystem listing.
- Wi-Fi configuration through the dashboard.
- GPIO API present, but output pins disabled until the exact carrier-board pinout is verified.
- REPL command helpers: help, sys, wifi, fs, version.

## Layout
- firmware/boot.py and firmware/main.py: startup.
- firmware/amh/: system modules and HTTP server.
- firmware/web/: browser dashboard.
- tools/: Windows PowerShell deployment and checks.
- docs/: architecture, setup, commands, security.

## Quick start on Windows
1. Identify the exact ESP32-S3 carrier-board model.
2. Install official MicroPython ESP32-S3 firmware using board-specific instructions: https://micropython.org/download/ESP32_GENERIC_S3/
3. Install Python 3 and run: py -m pip install --upgrade mpremote
4. Clone this repository and open it in PyCharm.
5. Deploy application files: .\tools\deploy.ps1 -Port COM16 (replace COM16 with the actual port).
6. If no saved Wi-Fi network is available, connect to the setup AP and open http://192.168.4.1.

Initial setup AP: SSID AMH-OS-SETUP, password amhsetup1234. Change it before shared use.

## Important
This repository contains application files, not a board-specific MicroPython firmware binary. The deploy script only copies application files. It does not erase flash, install firmware, or modify eFuses. Confirm exact board and flash settings before installing firmware.

Run checks with .\tools\check.ps1.

## Status
Early foundation. File editing, OTA, arbitrary remote commands, GPIO output, and automatic firmware flashing are not enabled in this release.

## License
MIT.
