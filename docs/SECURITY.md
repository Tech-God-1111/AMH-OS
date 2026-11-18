# Security notes

This is an early development platform for a trusted local network.
- The HTTP dashboard has no authentication. Never expose it to the public internet.
- The initial setup AP password is public; change it before shared use.
- Wi-Fi credentials are stored in plaintext in the device filesystem.
- The filesystem endpoint only lists root entries; it cannot read, edit, or delete files.
- No arbitrary code execution or remote shell endpoint is provided.
- GPIO output is disabled until the exact carrier-board schematic and pin map are confirmed.
- Do not use this release to control locks, motors, heaters, mains equipment, or safety-critical systems.
- No firmware flashing, flash erasing, or eFuse operations are performed by the application or deploy script.
