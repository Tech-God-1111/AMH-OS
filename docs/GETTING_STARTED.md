# Getting started (Windows + WSL2 Ubuntu)

## 1. Preserve existing firmware
Keep the Wi-Fi Radar sketch source in a safe folder outside this repository before flashing. This preserves the application source for rebuilding later; it is not a complete flash backup.

**Building does not modify the ESP32-S3. Flashing replaces the firmware currently on it.** The flash script requires typing FLASH and does not run `erase_flash`.

## 2. Ubuntu prerequisites
```bash
sudo apt update
sudo apt install -y git build-essential bison flex gperf gettext libncurses-dev \
  libtool autoconf automake texinfo genromfs gcc g++ make cmake ninja-build \
  python3 python3-pip python3-venv curl wget xz-utils unzip picocom
git clone https://github.com/Tech-God-1111/AMH-OS.git
cd AMH-OS
```

Install the Espressif Xtensa ESP32-S3 toolchain required by the NuttX revision. Ensure `xtensa-esp-elf-gcc` and `xtensa-esp-elf-ld` are on PATH. Follow the official NuttX guide: https://nuttx.apache.org/docs/latest/platforms/xtensa/esp32s3/index.html

## 3. Download upstream sources
```bash
bash scripts/bootstrap.sh
```

## 4. Configure and build
```bash
bash scripts/configure.sh
bash scripts/build.sh
```

The baseline is `esp32s3-devkit:nsh`. The script registers and enables the AMH app. For Wi-Fi, run `make menuconfig` inside `third_party/nuttx` and enable the ESP32-S3 Wi-Fi driver, network initialization and WAPI options supported by that exact NuttX revision. Save and rebuild. Unsupported Kconfig symbols are not guessed.

## 5. Serial console
Connect the COM/UART USB connector. Under WSL2, USB serial devices may need USBIPD forwarding; alternatively build in WSL2 and flash from Windows. Linux serial example:
```bash
bash scripts/serial.sh /dev/ttyUSB0
```
Use 115200 baud. Expected prompt: `nsh>`. Try `help`, `uname -a`, `amh help`, `amh sys`, `amh fs`, and `amh apps`.

## 6. Flash only after hardware review
Confirm the carrier-board model, flash size, PSRAM support and serial port. The profile is provisional until the exact board is identified.
```bash
bash scripts/flash.sh /dev/ttyUSB0
```
The script asks for explicit confirmation. Do not enable secure boot or flash-encryption eFuses in this development project.

If Windows shows COM16, WSL2 may not expose it as `/dev/ttyUSB0`. The Linux port number need not match the Windows COM number.
