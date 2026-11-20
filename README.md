# AMH-OS — Native ESP32-S3

**Repository:** [Tech-God-1111/AMH-OS](https://github.com/Tech-God-1111/AMH-OS)  
**Active native branch:** `native-esp32s3`  
**Target:** ESP32-S3, 16 MB flash, 8 MB embedded PSRAM  
**Native SDK:** ESP-IDF 5.5.6

AMH-OS is an embedded operating environment under development for ESP32-S3. The native implementation is being developed separately from the existing MicroPython firmware so that the earlier implementation remains preserved.

## Native implementation

The native ESP-IDF project lives in [`native/esp-idf/`](native/esp-idf/).

Current native source includes:
- Native startup and a FreeRTOS-backed kernel monitor task.
- ESP32-S3 Wi-Fi setup access point.
- HTTP server running directly on the ESP32-S3.
- Embedded AMH-OS graphical dashboard served by the board.
- Live system status endpoint at `/api/status`.

### Open the AMH-OS interface

After the native firmware has been built and flashed:

1. Connect a computer to Wi-Fi **AMH-OS-SETUP**.
2. Enter password **amhsetup**.
3. Open **http://192.168.4.1** in a browser.

The web page is embedded in the firmware and served by the ESP32-S3. The browser is the display/input surface; it is not a separate host application. The setup AP credentials are development defaults and must be changed before wider deployment.

**Important:** The native UI code has been added to this branch, but this new network/UI milestone still needs to be built and tested on the physical board. Do not treat the UI as hardware-verified until that test is complete.

## Build

Open an ESP-IDF 5.5.6 PowerShell environment, then:

```powershell
Set-Location "native/esp-idf"
idf.py set-target esp32s3
idf.py build
```

Check the selected COM port before flashing. Normal application updates should not require a full-chip erase. Keep the existing full-flash backup safe.

## Display architecture

The ESP32-S3 development board has no built-in HDMI output. The current native graphical interface is therefore served over Wi-Fi by the board and rendered in a browser on the computer. This is a board-hosted interface, not a claim that the board has a physical HDMI desktop output.

## Existing implementation

The earlier MicroPython firmware, web dashboard, and desktop prototype remain in their existing directories. They are not silently treated as native ESP-IDF components and have not been replaced by this native implementation.

## Validation status

The initial native boot/kernel firmware was previously built, flashed, and observed running on the ESP32-S3. The new Wi-Fi and embedded UI source has now been added to the branch; it must pass an ESP-IDF build and a live browser test before the new milestone is called complete.

## Safety and recovery

- Preserve the existing full-flash backup.
- Verify board and COM port before flashing.
- Inspect build output before deployment.
- Do not use `erase_flash` as a routine step.
- Obtain explicit approval before any flash or erase operation.
- Keep native development on `native-esp32s3`; preserve all other branches.
