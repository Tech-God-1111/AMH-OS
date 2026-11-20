# AMH-OS — Native ESP32-S3 Development

**Branch:** `native-esp32s3`  
**Repository:** [Tech-God-1111/AMH-OS](https://github.com/Tech-God-1111/AMH-OS)  
**Target hardware:** ESP32-S3 (16 MB flash, 8 MB embedded PSRAM)

AMH-OS is an embedded operating environment being developed for the ESP32-S3. This branch is reserved for the **native ESP-IDF implementation** and its evolution toward a graphical desktop environment.

> **Development status:** Early native firmware bring-up. The first native image has been built, flashed to the ESP32-S3, and observed booting successfully. A graphical desktop/boot screen is not implemented in this branch yet.

---

## Branch purpose

The repository's existing MicroPython-based AMH-OS is preserved on its existing branches. The `native-esp32s3` branch is the development line for a native C firmware architecture based on ESP-IDF.

The intention is to keep the two implementations distinct while native functionality is developed and validated.

- Existing MicroPython implementation: preserved on `amh-os-0.4.1-kernel` and other existing branches.
- Native implementation: developed on `native-esp32s3`.
- Do not assume features from the MicroPython desktop are already available in native firmware.

## Current native milestone

The first native firmware milestone is **AMH-OS 0.1.0**.

Verified on the physical ESP32-S3:

- ESP32-S3 detected on UART `COM16`.
- Flash identified as 16 MB.
- ESP-IDF 5.5.6 build completed successfully.
- Native application flashed successfully.
- ESP-IDF bootloader loaded the application.
- AMH-OS boot banner and `AMH-OS BOOTED` message appeared in the serial monitor.
- Kernel foundation initialized.
- A FreeRTOS-backed monitor task reported a periodic heap heartbeat.

This is an early standalone firmware foundation, **not yet a complete independent operating system**. ESP-IDF supplies the platform support and FreeRTOS currently supplies task scheduling.

## Development roadmap

Work is planned in incremental, testable stages:

1. **Boot manager and diagnostics** — explicit boot states, initialization results, chip/flash/memory diagnostics, and startup reporting.
2. **Kernel foundation** — AMH-OS kernel services, lifecycle management, and structured task monitoring.
3. **Hardware abstraction** — controlled interfaces for supported ESP32-S3 peripherals.
4. **Storage and shell** — filesystem integration and a restricted AMH-OS command interface.
5. **USB host communication** — a documented protocol between native firmware and the PC companion.
6. **Graphical boot experience** — boot progress and status presented by the PC-side interface.
7. **Desktop environment** — native-device services connected to the AMH-OS desktop UI.
8. **Reliability and release** — recovery, validation, versioning, and deployment documentation.

The roadmap is a plan, not a claim that these features are already implemented.

## Graphical interface architecture

The target experience is a graphical AMH-OS boot screen followed by a desktop interface on the connected PC.

The ESP32-S3 does not provide a built-in HDMI display output. The planned arrangement is:

- **ESP32-S3:** runs native AMH-OS firmware and hardware/kernel services.
- **USB serial:** carries structured status and control messages between the board and PC.
- **PC companion:** renders the graphical boot screen and desktop in a browser.

The existing browser desktop is part of the repository's earlier MicroPython-oriented implementation. It must be adapted and tested against the native firmware protocol before it can be considered compatible.

## Native build environment

The initial native firmware was developed with:

- ESP-IDF **5.5.6**
- ESP32-S3 target
- Xtensa ESP32-S3 toolchain supplied by ESP-IDF
- CMake and Ninja supplied by the ESP-IDF installation

For a local ESP-IDF PowerShell environment, activate the installed ESP-IDF version using its `export.ps1` script, then change into the native project directory and build with:

```powershell
idf.py set-target esp32s3
idf.py build
```

The exact native project location and source layout may evolve as the native implementation is integrated into this repository.

## Flashing and recovery precautions

Flashing replaces firmware in the selected flash regions. Before deploying a new image:

1. Confirm the connected board and serial port.
2. Confirm the configured flash size matches the physical device.
3. Build and inspect the firmware before deployment.
4. Keep a verified full-flash backup in a safe location.
5. Obtain explicit approval before any flash or erase operation.

Do **not** run `erase_flash` as a routine deployment step. A full-chip erase is not required for ordinary application updates.

## Current limitations

- No native graphical boot screen is implemented yet.
- No native desktop protocol has been finalized.
- No native filesystem, shell, app launcher, or complete service manager is claimed yet.
- The current kernel monitor is a minimal diagnostic task, not a finished scheduler or kernel subsystem.
- Hardware support beyond the verified boot/diagnostic path must be implemented and tested separately.

## Contributing and development approach

- Keep native implementation changes on `native-esp32s3` until they are ready for review.
- Preserve existing MicroPython branches.
- Prefer small, reviewable commits with clear validation notes.
- Do not claim hardware validation unless the relevant test was actually run on the physical board.
- Document build steps, flash layout changes, and recovery implications alongside implementation changes.

---

**Project status:** Native ESP32-S3 bring-up successful; Stage 2 boot manager and system diagnostics are the next development milestone.
