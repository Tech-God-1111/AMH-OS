# Getting started (Windows + PyCharm)

AMH-OS is intended to be managed from Windows PowerShell inside PyCharm. **You do not need to install Linux, Ubuntu, or WSL2.**

NuttX itself still depends on GNU make, Bash and common Unix build utilities. The supported Windows-oriented route documented here uses **Cygwin as a Windows compatibility/tooling environment**, not a Linux operating system. Apache NuttX documents Cygwin as a workable Windows build environment. Its native CMD build mode is marked as less mature and not verified for all targets.

## 1. Install host prerequisites

Install:
- Git for Windows
- Python 3.10 or newer, with the Python launcher or `python` on PATH
- Cygwin 64-bit

In the Cygwin installer, include these packages (package names may vary slightly by Cygwin mirror/version):
`bash, make, gcc-core, gcc-g++, git, bison, flex, gperf, gettext, libncurses-devel, automake, autoconf, libtool, cmake, ninja, python3, python3-pip, curl, wget, unzip, xz`.

Keep the default Cygwin location `C:\cygwin64`, or set `CYGWIN_BASH` to the full path of your `bash.exe`.

## 2. Open the repository in PyCharm

Open the AMH-OS folder. Use the PyCharm Terminal with the PowerShell shell. From the repository root:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\amh.ps1 help
```

## 3. Prepare the Windows toolchain

```powershell
.\scripts\amh.ps1 setup
```

This downloads the Espressif Xtensa ESP32 toolchain Windows archive, verifies its published SHA-256 checksum, extracts it under `tools\xtensa-esp-elf`, and creates a project-local Python environment with esptool and pyserial.

The selected compiler package is pinned to the toolchain version referenced by the NuttX ESP32-S3 guide. If upstream NuttX CI changes its expected compiler, update the pin and checksum together after checking the official documentation.

## 4. Download upstream sources

```powershell
.\scripts\amh.ps1 bootstrap
```

This clones Apache NuttX and NuttX Apps into `third_party`. The folder is generated locally and is not committed.

## 5. Configure and build

```powershell
.\scripts\amh.ps1 configure
.\scripts\amh.ps1 build
.\scripts\amh.ps1 test
```

The PowerShell dispatcher invokes the NuttX Bash scripts through Cygwin. The Windows Xtensa compiler is added to PATH for the build.

**Important:** Apache NuttX documents Windows/Cygwin builds, but combining Cygwin build tools with a Windows-native cross-compiler can involve path and symbolic-link compatibility issues. This exact ESP32-S3 combination has not yet been validated in this repository. If the build fails, capture the complete first error; do not flash anything.

## 6. Configure Wi-Fi

The ESP32-S3 radio is **2.4 GHz only** and supports IEEE 802.11 b/g/n. Use a 2.4 GHz router SSID. A 5 GHz-only SSID is incompatible.

The NuttX Wi-Fi driver, network initialization, and WAPI options must be enabled in `third_party/nuttx` for the checked-out revision. Do not guess Kconfig names from another NuttX release. Save the configuration and rebuild after enabling the supported options.

## 7. Serial console

Connect the COM/UART USB connector. In Windows Device Manager, identify its COM port (for example, COM16).

```powershell
.\scripts\amh.ps1 serial -Port COM16
```

The terminal uses 115200 baud. Expected prompt: `nsh>`. Try `help`, `uname -a`, `amh help`, `amh sys`, `amh fs`, and `amh apps`.

## 8. Flash only after hardware review

Confirm the carrier-board model, flash size, PSRAM support and COM port. The profile is provisional until the exact board is identified. Preserve the Wi-Fi Radar sketch source before replacing its firmware.

```powershell
.\scripts\amh.ps1 flash -Port COM16 -ConfirmBoard
```

The script asks you to type `FLASH` before proceeding. It does not issue a full-chip erase command. Do not enable secure-boot or flash-encryption eFuses in this development project.
