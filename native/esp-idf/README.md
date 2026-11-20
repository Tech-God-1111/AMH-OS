# AMH-OS Native ESP32-S3

This is the native ESP-IDF implementation, separate from the existing MicroPython firmware.

## Desktop milestone: native-desktop-v1

The ESP32-S3 serves the AMH-OS desktop directly from its firmware image at `http://192.168.4.1`. The desktop assets are embedded into the native firmware during the ESP-IDF build; no separate PC web server is used.

The current desktop prototype includes:
- Desktop wallpaper, icons, Start menu, taskbar, clock, and movable application windows
- System Monitor backed by the ESP32 `/api/status` endpoint
- AMH-OS Terminal with a small device-side command interpreter (`help`, `version`, `status`, `uptime`, `apps`, `clear`)
- Settings panel for pairing the optional Windows companion
- Launch actions for the laptop's default browser, File Explorer, and Task Manager through the companion

## Windows companion

A browser page cannot directly start Windows programs. Run the optional local companion on the laptop:

```powershell
py .\pc-companion\amh_companion.py
```

It prints a random pairing token. Open **Start → Settings** in AMH-OS, paste the token, and select **Connect**. Keep the companion terminal open.

The companion binds only to loopback and exposes a fixed allowlist: default browser, Explorer, and Task Manager. It does not expose arbitrary CMD/PowerShell execution or arbitrary file access. See [pc-companion/README.md](pc-companion/README.md).

## Connect

1. Flash the native firmware to the ESP32-S3.
2. Connect the computer to Wi-Fi `AMH-OS-SETUP`.
3. Enter password `amhsetup`.
4. Open `http://192.168.4.1`.

The setup access point is a development network. Its credentials are not suitable for deployment. In this AP-only prototype, internet access on the laptop may be unavailable while connected to the board unless the laptop has another internet connection (for example Ethernet). Router client mode / AP+STA provisioning is a planned networking milestone.

## Build

Use ESP-IDF 5.5.x with the ESP32-S3 target:

```powershell
idf.py set-target esp32s3
idf.py build
```

The HTML desktop is embedded using ESP-IDF `EMBED_TXTFILES`. The app also uses the existing NVS partition for Wi-Fi initialization.

## Not implemented yet

This is an early desktop foundation, not a complete general-purpose OS. Persistent file management, real laptop file browsing, process enumeration, PIN login/lock screen, multi-user accounts, router Wi-Fi configuration, and full desktop security are not implemented yet. The terminal is an AMH-OS command interpreter, not Windows CMD or PowerShell.

Do not run `erase_flash` as part of normal development. Preserve the existing NVS and backup image.
