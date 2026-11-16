# ESP32-S3 board profile

Initial NuttX profile: `esp32s3-devkit:nsh`. This is provisional because the exact carrier-board model is not confirmed.

## Console
- UART0 / COM-UART USB connector
- 115200 baud, 8 data bits, no parity, 1 stop bit
- Expected prompt: `nsh>`

Before flashing, confirm the module marking, carrier-board model, USB/UART wiring, flash/PSRAM part and NuttX flash configuration. Local Arduino diagnostics previously reported 16 MB flash and about 8 MB usable PSRAM; these values must be checked independently under NuttX.
