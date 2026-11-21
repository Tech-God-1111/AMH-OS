# AMH-OS Native 3.0

**Branch:** `native-v3.0`  
**Project:** `native/esp-idf-v3`  
**Target:** ESP32-S3, 16 MB flash, octal PSRAM configuration (verify against the exact module)

AMH-OS 3.0 combines the native desktop from `native-desktop-v1` with a modular HAL and staged boot kernel inspired by the supplied Phase 2 archive.

## Architecture

- `components/hal`: runtime chip, core, flash, PSRAM, heap, MAC, reset-reason and ESP-IDF detection.
- `components/kernel`: timed boot stages, RTC failure counter and safe-mode restart-loop protection.
- `main/wifi_service.c`: development AP service.
- `main/storage_service.c`: non-destructive FAT mount and restricted file APIs.
- `main/ui_service.c`: embedded desktop and live hardware status endpoint.
- `main/main.c`: staged startup and OTA image validation.
- `main/web/index.html`: desktop prototype carried forward from native-desktop-v1.
- `pc-companion/amh_companion.py`: allowlisted Windows companion.

## Network and storage

- Setup Wi-Fi: `AMH-OS-SETUP`
- Development password: `AMHOS-Setup-2026` (change before sharing the device)
- Desktop: `http://192.168.4.1`
- Status: `GET /api/status`
- Files: `GET /api/files?path=/`, `GET /api/file?path=/name`, `PUT /api/file`, `POST /api/mkdir`, `DELETE /api/file?path=/name`, `POST /api/storage/format`, `POST /api/storage/format`
- First-time storage provisioning: use Device File Explorer and explicitly confirm `FORMAT AMH-OS STORAGE`. This formats only the `storage` partition; it does not erase NVS or application slots.

## Build

Use the project's ESP-IDF 5.5.x environment:

```powershell
cd native/esp-idf-v3
idf.py set-target esp32s3
idf.py build
idf.py size
```

The project uses a custom 16 MB partition map with two 2 MB OTA slots and a FAT data partition. The storage service mounts an existing FAT volume without formatting it, and exposes bounded directory listing, text-file read/write, directory creation, and delete APIs to the desktop. A blank or invalid volume remains unavailable rather than being erased automatically; first-time provisioning is performed only after an explicit typed confirmation in Device File Explorer. OTA partition layout and rollback configuration are groundwork, not a complete network OTA updater.

## Hardware configuration warning

The defaults enable octal PSRAM at 80 MHz to match the reported 8 MB embedded PSRAM class of the target board. Confirm the exact module's PSRAM mode before building/flashing. The runtime HAL reports actual initialized PSRAM and flash capacity; it does not make incorrect electrical/configuration assumptions safe.

## Reliability behavior

- NVS initialization never automatically erases stored data.
- Critical boot failures trigger controlled retries only until the safe-mode threshold.
- In safe mode, optional storage, Wi-Fi and desktop services are skipped; the device remains in a recovery wait instead of entering an endless reboot loop. A successful safe-mode session clears the failure counter so a later manual reboot can retry normal startup.
- OTA image validation occurs only after all declared boot stages complete in normal mode; a safe-mode boot does not mark a pending OTA image valid.
- No bootloader, partition, or flash erase operation is performed by the build scripts.

## Desktop status and limitations

The desktop, Start menu, taskbar, movable/minimizable/maximizable windows, terminal prototype, and companion launch integration are inherited from the desktop milestone. The terminal is an allowlisted AMH-OS command interpreter with `help`, `version`, `status`, `uptime`, `apps`, `clear`, `ls`, `cat`, `mkdir`, `rm`, and `write`; it does not expose an arbitrary shell. Device File Explorer uses the ESP32 storage APIs when a valid FAT volume is mounted and also offers a companion action to open the laptop's File Explorer. Browser and Task Manager actions run on the paired Windows laptop through the companion.

Not yet implemented: full app manager, PIN/login security, router STA/AP+STA provisioning, complete OTA download/install flow, process enumeration, and a production security model. The setup AP credentials are development-only.

## Known limitations

- The desktop runs in the laptop browser; the ESP32-S3 serves the UI and system APIs. It does not run a local Chromium browser.
- The terminal is a restricted built-in command interpreter, not a general Windows/Linux shell.
- The companion exposes only allowlisted Windows launch actions and is bound to localhost.
- Desktop PIN authentication, multi-user accounts, a complete app package manager, router client mode, and a network OTA download/install client are not included in this V3.0 milestone.
- The setup AP password is for development only. Change it before sharing or deploying the device.

## Flashing

Build and inspect first. Flash only after explicitly reviewing the generated partition table and binary size. Do not use `erase_flash` as a routine step. Preserve the existing full-flash backup.
