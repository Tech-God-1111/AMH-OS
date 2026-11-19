# AMH-OS

A MicroPython-powered ESP32-S3 device environment with a browser-rendered desktop connected over USB serial.

## Desktop applications

- Terminal with an allowlisted AMH-OS command shell.
- File Explorer for the ESP32 filesystem: list, edit, create, rename/move, copy and delete text files, plus create and remove empty folders.
- Dual-system Task Manager: ESP32-S3 runtime metrics plus optional Windows host metrics.
- Settings for PIN, password or no-login convenience lock.
- Web Browser launcher that opens pages in a new tab on the host computer.
- Bluetooth panel for Bluetooth Low Energy (BLE) discovery when the installed MicroPython firmware includes BLE support.

## ESP32-S3 services

- MicroPython runtime, bounded filesystem service, allowlisted shell, resource monitor and BLE scanner.
- USB serial uses newline-delimited JSON at 115200 baud.
- System files such as `boot.py`, `main.py`, `config.json` and the `amh` package are protected from File Explorer mutations.
- File preview/write/copy limits are 8 KB. Directory copy and recursive delete are not enabled.
- GPIO outputs remain disabled until the carrier-board pinout is verified.
- Device Task Manager reports MicroPython heap, MicroPython filesystem capacity, uptime, CPU frequency and registered AMH-OS services. It does not report per-core CPU utilization or every internal FreeRTOS task.

## Windows host monitor (optional)

The Windows companion is a separate, read-only Python service. It reports aggregate CPU usage, logical/physical core counts, RAM, system-drive usage, uptime, network counters, running Windows service count and the top processes by memory.

1. Install the optional dependency: `py -m pip install -r host/requirements.txt`.
2. In a separate PowerShell window, run: `.\tools\launch-host-monitor.ps1`.
3. Keep that window open while using AMH-OS. The desktop reads metrics from `127.0.0.1:8766`.

The service binds only to loopback (127.0.0.1), accepts read-only GET requests from the AMH-OS localhost desktop, and does not execute commands or change Windows settings. If it is not running, ESP32 monitoring continues normally.

## Setup on Windows

1. Download the repository's `main` branch.
2. Install mpremote: `py -m pip install --upgrade mpremote`.
3. Deploy the AMH-OS application files: `.\tools\deploy.ps1 -Port COM16` (change COM16 to the board's actual port).
4. Start the desktop server: `.\tools\launch-desktop.ps1`.
5. Open `http://localhost:8765` in current Chrome or Edge.
6. Click **Connect ESP32 over USB** and select the board's serial port.

The default desktop convenience PIN is `1234`. Change it in Settings before demonstrating the desktop. This browser-side lock is not device-level security.

## Bluetooth limitations

- ESP32-S3 supports Bluetooth Low Energy, not Bluetooth Classic.
- BLE discovery depends on whether the installed MicroPython firmware exposes the `bluetooth` module.
- The current Bluetooth panel discovers nearby BLE advertisements. It does not yet implement general GATT pairing or direct realme Watch S2 data access.

## Architecture and validation limits

- AMH-OS is an embedded ESP32 environment with a host-rendered desktop, not a replacement for Windows/macOS/Linux.
- The ESP32-S3 does not render modern web pages; the Browser app opens sites in the host computer's browser.
- The host monitor and ESP32 monitor are separate data sources.
- No physical ESP32-S3 validation is claimed by this repository update. Test USB serial, file operations and BLE scanning on the actual board before relying on them.
- Do not use on a computer where USB serial access is not permitted.

## Checks

Run `.\tools\check.ps1`.
