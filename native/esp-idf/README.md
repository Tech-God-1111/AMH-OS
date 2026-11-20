# AMH-OS Native ESP32-S3

This is the native ESP-IDF implementation. It is separate from the existing MicroPython firmware in `firmware/`.

## Current milestone

The firmware starts:
- The native FreeRTOS-based AMH-OS kernel monitor task
- A WPA2 setup access point named `AMH-OS-SETUP`
- An HTTP server hosted by the ESP32-S3
- A graphical system dashboard at `http://192.168.4.1`
- A live status endpoint at `/api/status`

The browser interface is served by the board itself. The browser is only the display and input surface; no separate desktop server is required for this milestone.

## Connect

1. Flash the native firmware to the ESP32-S3.
2. Connect the computer to Wi-Fi `AMH-OS-SETUP`.
3. Enter password `amhsetup`.
4. Open `http://192.168.4.1`.

The setup network is intended for development. Change the default password before using this outside a trusted environment.

## Build

Use ESP-IDF 5.5.x with the ESP32-S3 target:

```powershell
idf.py set-target esp32s3
idf.py build
```

Do not run `erase_flash` as part of normal development. The native app uses the existing NVS and factory-app partition layout unless a later documented partition change is introduced.
