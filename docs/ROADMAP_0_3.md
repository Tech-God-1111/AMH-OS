# AMH-OS 0.3 development plan

## Runtime architecture
- ESP32-S3 + MicroPython: device filesystem, allowlisted shell, resource monitor and BLE discovery.
- USB serial: newline-delimited JSON requests; no arbitrary Python execution and no host shell access.
- Host desktop: HTML/CSS/JavaScript UI served on localhost. Browser launch opens a URL in the host browser; the ESP32 does not run a modern browser engine.

## 0.3 device services
- Bounded file operations under the ESP32 filesystem root.
- Shell commands for navigation, file operations, diagnostics, Wi-Fi and BLE.
- Resource snapshot for heap, storage, uptime, CPU frequency and registered AMH services.
- BLE scanner using MicroPython's bluetooth module when included in the installed firmware.

## Important limits
- The resource monitor lists AMH-OS registered services, not every FreeRTOS task. MicroPython does not provide a general desktop process manager API.
- BLE scanning is discovery only. Pairing and watch data access require the watch's supported GATT services and any vendor-specific authentication/protocol.
- The realme Watch S2 (RMW2401) advertises Bluetooth and BLE support. ESP32-S3 supports BLE, not Bluetooth Classic. This does not guarantee direct watch pairing or access to watch data.
- File reads and writes are capped at 8 KB in this first implementation. Directory copy and recursive delete are intentionally not exposed.
- The login screen in the browser desktop is a convenience lock, not a security boundary for the ESP32 filesystem or the host computer.
