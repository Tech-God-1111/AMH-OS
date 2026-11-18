# AMH-OS Lite

A lightweight ESP32-S3 device environment built on the official MicroPython runtime. The board runs the Python system layer and can expose a local dashboard or a USB-connected desktop demonstration.

## Current features
- MicroPython-based ESP32-S3 system layer.
- USB serial JSON protocol for the AMH-OS Desktop demo.
- Browser desktop with lock screen, application launcher, dock and demo windows.
- Read-only live hardware status over USB: device ID, runtime, free heap, uptime and CPU frequency.
- Safe demonstration commands: help, version, sys, wifi.
- Existing Wi-Fi dashboard and JSON API retained in the firmware package.
- GPIO output remains disabled until the exact carrier-board pinout is verified.

## USB desktop demonstration
1. Pull the latest repository.
2. Deploy application files: `.\tools\deploy.ps1 -Port COM16` (replace COM16 with the actual port).
3. Start the local desktop server: `.\tools\launch-desktop.ps1`.
4. Open `http://localhost:8765` in current Chrome or Edge.
5. Click **Connect ESP32 over USB**, select the board's COM port and approve access.
6. Unlock with demo PIN `1234`.

See [USB Desktop Demo](docs/USB_DESKTOP_DEMO.md) for details.

## Foundation setup
- Official MicroPython firmware: https://micropython.org/download/ESP32_GENERIC_S3/
- Install tools: `py -m pip install --upgrade mpremote`
- Run project checks: `.\tools\check.ps1`

## Important
This is an embedded device environment and a desktop demonstration, not a general-purpose PC operating system. The desktop runs in the host browser; it does not replace or automatically launch over the host OS. The serial interface exposes a small read-only/allowlisted command set, not arbitrary remote execution. Deployment only copies application files; it does not erase flash or install firmware.

## License
MIT.
