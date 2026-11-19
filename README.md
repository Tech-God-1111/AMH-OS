# AMH-OS

A MicroPython-powered ESP32-S3 device environment with a browser-rendered desktop connected over USB serial.

## Current development branch: `amh-os-v0.3`

### Desktop applications
- Terminal with an allowlisted AMH-OS command shell.
- File Explorer for the ESP32 filesystem: list, preview, create, rename/move, copy and delete files, plus create and remove empty folders.
- Task Manager with heap, storage, uptime, CPU frequency and registered AMH-OS services.
- Settings for PIN, password or no-login convenience lock.
- Web Browser launcher that opens pages in a new tab on the host computer.
- Bluetooth panel for BLE discovery when the installed MicroPython firmware includes BLE support.

### Device services
- ESP32-S3 runs MicroPython, the filesystem service, shell, resource monitor and BLE scanner.
- USB serial uses newline-delimited JSON at 115200 baud.
- System files such as `boot.py`, `main.py`, `config.json` and the `amh` package are protected from File Explorer mutations.
- File preview/write/copy limits are 8 KB. Directory copy and recursive delete are not enabled.
- GPIO outputs remain disabled until the carrier-board pinout is verified.

## Run on Windows
1. Pull the `amh-os-v0.3` branch.
2. Install mpremote: `py -m pip install --upgrade mpremote`.
3. Deploy to the board: `.\tools\deploy.ps1 -Port COM16` (change the COM port if required).
4. Start the desktop server: `.\tools\launch-desktop.ps1`.
5. Open `http://localhost:8765` in current Chrome or Edge.
6. Click **Connect ESP32 over USB** and select the board's serial port.

The default convenience PIN is `1234`. Change it in Settings before showing the desktop.

## Important architecture limits
- This is an embedded device environment, not a replacement for Windows/macOS/Linux.
- The ESP32-S3 cannot run a modern graphical web browser. AMH-OS opens websites in the host computer's browser.
- The Task Manager reports AMH-OS services and device resource metrics, not every internal FreeRTOS task.
- The browser login is stored on the host and is a convenience lock, not secure authentication. It does not protect the ESP32 filesystem from someone with physical USB access.
- BLE scanning is discovery only. Direct realme Watch S2 pairing/data access is not yet implemented.
- Do not use on a computer where USB serial access is not permitted.

## Checks
Run `.\tools\check.ps1`.
