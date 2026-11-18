# Getting started on Windows

## Confirm the hardware
Identify the exact ESP32-S3 carrier-board model and module marking. Boards differ in flash, PSRAM, USB wiring, and reserved pins. Do not guess these settings.

## Install MicroPython
Official ESP32-S3 downloads:
https://micropython.org/download/ESP32_GENERIC_S3/

Official ESP32 installation guide:
https://docs.micropython.org/en/latest/esp32/tutorial/intro.html

Select firmware and flash settings for the exact board. This repository does not include a firmware binary and does not automatically erase the device.

## Prepare Windows
Install Python 3, then run:

    py -m pip install --upgrade mpremote

Clone this repository and open it in PyCharm.

## Deploy application files
Find the COM port in Device Manager, then from the repository root run:

    .\tools\deploy.ps1 -Port COM16

Replace COM16 with the actual port. This copies application files only.

## First connection
If no Wi-Fi credentials are saved, the board starts:
- SSID: AMH-OS-SETUP
- Password: amhsetup1234

Connect to that access point and open http://192.168.4.1. Enter your Wi-Fi network name and password in the dashboard. Check serial output for the station IP.

Change the setup AP password in /config.json before shared use. The initial password is public and is only a first-boot convenience.

## Serial diagnostics
Use a serial terminal at 115200 baud. At the MicroPython prompt:

    from amh.commands import run
    run("help")
    run("sys")
    run("wifi")

## Troubleshooting
- No COM port: check the USB cable, connector, and Device Manager.
- Dashboard unavailable: check serial output for IP and server startup.
- Setup AP missing: confirm the runtime supports ESP32-S3 Wi-Fi.
- Import error: confirm amh/ and web/ were copied to the device.
