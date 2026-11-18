# AMH-OS USB Desktop Demo

Version 0.2.0 is a demonstration desktop, not a replacement for the host computer's operating system.

## Architecture
- ESP32-S3 runs MicroPython and the AMH-OS JSON-line USB serial service.
- The school/host computer renders the desktop in Chrome or Edge.
- Web Serial connects to the ESP32's USB-UART bridge at 115200 baud.
- The desktop requests read-only status and a small allowlist of demonstration commands. It does not provide a remote shell or arbitrary code execution.

## Windows setup
1. Pull the latest repository.
2. Install the CH343 USB-UART driver if Windows does not detect the board.
3. Deploy application files to the ESP32-S3 using the existing script:
   `.\tools\deploy.ps1 -Port COM16` (replace COM16 if needed).
4. Start the desktop server:
   `.\tools\launch-desktop.ps1`
5. Open `http://localhost:8765` in current Chrome or Edge.
6. Click **Connect ESP32 over USB**, select the board's COM port, and allow access.
7. Unlock the visual demo with PIN **1234**.

## Demo features
- Lock screen and desktop shell.
- Applications menu and dock.
- Computer, File Manager, Terminal, Settings, About and System Monitor windows.
- Live device identity, MicroPython version, free heap, uptime, CPU frequency and Wi-Fi mode when connected over USB.
- Safe serial commands: `help`, `version`, `sys`, `wifi`.

## Important
- This does not auto-launch on a school computer. Open the page and approve the serial connection manually, in accordance with school rules.
- The lock PIN is a visual demo gate, not secure authentication.
- The file manager is a presentation mock-up; it does not browse or modify the ESP32 filesystem.
- The USB serial service takes over UART0, so the normal interactive REPL is not available while this demo firmware is running. To recover the REPL, use the board's BOOT/RESET procedure and mpremote/esptool as appropriate.
- Deployment copies application files only; it does not erase flash or replace MicroPython firmware.
- Use only on a computer where you are permitted to connect USB devices.
