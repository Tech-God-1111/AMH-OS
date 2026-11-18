# Architecture

AMH-OS Lite is an application environment on top of official MicroPython ESP32-S3 firmware. It is not a custom kernel, bootloader, scheduler, or driver stack.

Layers:
1. MicroPython runtime supplied separately for the exact board.
2. boot.py and main.py startup.
3. Python modules for configuration, Wi-Fi, device information, filesystem overview, and restricted hardware functions.
4. Small single-client HTTP server serving static files and JSON API routes.
5. HTML/CSS/JavaScript dashboard served by the ESP32.

Request flow: browser -> local HTTP server -> API router -> system module -> JSON response.

The board can serve the dashboard without cloud access. The browser and board must share a local network or the browser must join the setup AP.

Initial constraints: single-client server, read-only filesystem overview, no arbitrary Python execution, no remote shell, GPIO output disabled, no OTA, and no automatic firmware flashing.
