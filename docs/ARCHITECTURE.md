# AMH-OS architecture

AMH-OS is an ESP32-S3 operating-system distribution built on Apache NuttX. It is not a new kernel written from scratch. NuttX supplies hardware startup/boot integration, kernel, scheduler, drivers, VFS, networking and NuttShell (NSH). AMH-OS supplies its identity, command application, board profile, setup scripts and device-management conventions.

## Layers
1. ESP32-S3 ROM and Espressif startup flow used by the NuttX port.
2. NuttX kernel: scheduling, synchronization, memory management and drivers.
3. NuttX services: NSH, VFS, network stack and configured utilities.
4. AMH command app: the `amh` NSH command and subcommands.
5. Future NuttX applications.

## Milestones
- 0.1: upstream `esp32s3-devkit:nsh`, UART0 console at 115200 baud.
- 0.2: `amh` command dispatcher with help and subcommands.
- 0.3: uptime and heap statistics where exposed by the selected NuttX configuration; chip/core/nominal clock identification. Runtime CPU utilization and external PSRAM accounting require board-specific telemetry and are not fabricated.
- 0.4: NuttX network/Wi-Fi stack and WAPI utilities. Driver options must match the checked-out NuttX revision.
- 0.5: filesystem inspection using VFS APIs and an allow-listed application launcher.

The first release registers one stable NSH builtin, `amh`, with subcommands. This avoids fragile patches to the upstream NSH parser.

Upstream NuttX and NuttX apps are separate checkouts under `third_party/`. Credentials and generated firmware binaries must never be committed.
