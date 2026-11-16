# AMH-OS command reference

Commands are entered at the NuttX NSH prompt.

| Command | Purpose |
| --- | --- |
| amh help | Show AMH-OS command list |
| amh sys | Show uptime, target details and optional heap statistics |
| amh wifi | Explain native NuttX Wi-Fi setup and WAPI workflow |
| amh fs [path] | List a directory and report filesystem capacity when available |
| amh apps | List utilities allowed by the AMH launcher |
| amh run <utility> [args] | Launch a utility from the fixed allow-list |

## Wi-Fi workflow

The ESP32-S3 Wi-Fi driver, network stack and WAPI tools must be enabled in the selected NuttX configuration. The AMH launcher passes arguments directly to an executable; it does not invoke a shell to parse user-provided strings.

Examples (interface naming depends on the board configuration):

```text
nsh> amh run ifconfig
nsh> amh run wapi scan wlan0
nsh> wapi psk wlan0 <passphrase> 3
nsh> wapi essid wlan0 <ssid> 1
nsh> amh run renew wlan0
nsh> amh run ifconfig wlan0
```

Enter credentials locally at the serial console. Never commit credentials or paste them into public logs.

## Application allow-list

The initial launcher allow-list includes uname, ps, free, ls, df, ifconfig, wapi, renew, ping, mount, umount, date and uptime. Individual commands may be unavailable unless enabled in the NuttX build.
