# AMH-OS Native 3.0

**Branch:** `native-v3.0`  
**Project:** `native/esp-idf-v3`  
**Target:** ESP32-S3, 16 MB flash, octal PSRAM configuration (verify against the exact module)

AMH-OS 3.0 combines the native desktop from `native-desktop-v1` with a modular HAL and staged boot kernel inspired by the supplied Phase 2 archive.

## Architecture

- `components/hal`: runtime chip, core, flash, PSRAM, heap, MAC, reset-reason and ESP-IDF detection.
- `components/kernel`: timed boot stages, RTC failure counter and safe-mode restart-loop protection.
- `main/wifi_service.c`: development AP service.
- `main/ui_service.c`: embedded desktop and live hardware status endpoint.
- `main/main.c`: staged startup and OTA image validation.
- `main/web/index.html`: desktop prototype carried forward from native-desktop-v1.
- `pc-companion/amh_companion.py`: allowlisted Windows companion.

## Build

Use the project's ESP-IDF 5.5.x environment:

```powershell
cd native/esp-idf-v3
idf.py set-target esp32s3
idf.py build
idf.py size
```

The project uses a custom 16 MB partition map with two 2 MB OTA slots. The `storage` partition is reserved for a future filesystem integration; it is not mounted by this milestone. OTA partition layout and rollback configuration are groundwork, not a complete network OTA updater.

## Hardware configuration warning

The defaults enable octal PSRAM at 80 MHz to match the reported 8 MB embedded PSRAM class of the target board. Confirm the exact module's PSRAM mode before building/flashing. The runtime HAL reports actual initialized PSRAM and flash capacity; it does not make incorrect electrical/configuration assumptions safe.

## Reliability behavior

- NVS initialization never automatically erases stored data.
- Critical boot failures trigger controlled retries only until the safe-mode threshold.
- In safe mode, another critical failure is logged and the device remains in a recovery wait instead of entering an endless reboot loop.
- OTA image validation occurs only after all declared boot stages complete.
- No bootloader, partition, or flash erase operation is performed by the build scripts.

## Desktop status and limitations

The desktop, Start menu, taskbar, windows, terminal prototype, and companion launch integration are inherited from the 0.1 desktop milestone. The terminal remains a small AMH-OS command interpreter, not a general shell. File Explorer, browser, and Task Manager actions run on the paired Windows laptop through the companion.

Not yet implemented: persistent filesystem mounting/formatting, full app manager, PIN/login security, router STA/AP+STA provisioning, complete OTA download/install flow, process enumeration, and a production security model. The setup AP credentials are development-only.

## Flashing

Build and inspect first. Flash only after explicitly reviewing the generated partition table and binary size. Do not use `erase_flash` as a routine step. Preserve the existing full-flash backup.
