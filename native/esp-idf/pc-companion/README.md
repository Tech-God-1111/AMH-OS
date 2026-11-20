# AMH-OS Windows Companion

The ESP32-S3 hosts the AMH-OS desktop website. A browser page cannot directly launch Windows applications, so this optional companion runs on the laptop and accepts a small allowlist of authenticated requests from the AMH-OS page.

## Requirements

- Windows 10/11
- Python 3.10 or newer
- Laptop connected to the AMH-OS Wi-Fi access point
- AMH-OS desktop open at `http://192.168.4.1`

## Start

Open PowerShell in this directory and run:

```powershell
py .\amh_companion.py
```

The companion prints a one-time random pairing token. Keep the terminal open. In AMH-OS, open **Start → Settings**, paste the token, and select **Connect**.

## Supported actions in this prototype

- Web Browser: opens a page in the Windows registered default browser.
- File Explorer: launches Windows File Explorer.
- Task Manager: launches Windows Task Manager.

The companion binds only to `127.0.0.1`, checks the exact AMH-OS origin, and requires a bearer token. It does not accept arbitrary shell commands or arbitrary file paths.

## Important limitations

- This is a development prototype, not a hardened security product.
- The current AMH-OS setup access point uses development credentials. Use it only in a trusted development environment.
- Browser private-network permission prompts may appear depending on browser version and policy.
- The AMH-OS Terminal currently implements a small device-side command set; it is not Windows CMD or PowerShell.
- PIN login, file browsing APIs, process enumeration, and persistent user storage are not implemented in this milestone.
