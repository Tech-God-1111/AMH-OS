# Hardware compatibility and flash precautions

## Known test-board facts
- ESP32-S3 (QFN56), dual-core Xtensa LX7, up to 240 MHz.
- ESP32-S3-WROOM module family.
- 16 MB flash detected by Arduino diagnostic.
- Approximately 8 MB usable PSRAM detected by Arduino diagnostic.
- Two USB-C connectors labelled COM/UART and USB; COM/UART used for serial upload/monitor.
- Wi-Fi is **2.4 GHz only**, IEEE 802.11 b/g/n. 5 GHz Wi-Fi is not supported by ESP32-S3.

The exact carrier-board manufacturer/model and USB bridge are not confirmed.

## Initial upstream profile
Apache NuttX `esp32s3-devkit:nsh`. This is a baseline, not proof that every third-party ESP32-S3-WROOM board is electrically identical.

## Flash size
Do not blindly copy the NuttX documentation's example `-fs 4MB` setting onto a board detected with 16 MB flash. Review the generated image settings and selected board config before flashing.

## Non-destructive policy
- No automatic full-chip erase.
- No secure-boot or flash-encryption eFuse commands.
- No hardcoded Wi-Fi credentials.
- Flashing is a separate explicit action.
- Preserve the Wi-Fi Radar sketch source before replacing its firmware.
