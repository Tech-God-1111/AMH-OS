"""Safe JSON-line USB serial service for the AMH-OS desktop demo."""
import gc
import json
import time

try:
    import machine
    import uos
except ImportError:
    machine = None
    uos = None

BAUDRATE = 115200
MAX_LINE = 512


def _reply(uart, payload):
    try:
        uart.write(json.dumps(payload) + "\n")
    except Exception:
        pass


def _handle(message):
    kind = message.get("type", "")
    if kind == "hello":
        return {"type": "hello", "name": "AMH-OS", "version": "0.2.0",
                "device": "ESP32-S3", "transport": "USB Serial"}
    if kind == "ping":
        return {"type": "pong"}
    if kind == "status":
        from amh import system, wifi
        data = system.info()
        data["wifi"] = wifi.status()
        data["type"] = "status"
        data["version"] = "0.2.0"
        return data
    if kind == "command":
        command = str(message.get("command", "")).strip().lower()
        if command in ("help", "version", "sys", "wifi"):
            if command == "help":
                output = "Available demo commands: help, version, sys, wifi"
            elif command == "version":
                output = "AMH-OS Desktop Demo 0.2.0 | MicroPython"
            elif command == "sys":
                from amh import system
                output = "\n".join("%s: %s" % item for item in system.info().items())
            else:
                from amh import wifi
                output = "\n".join("%s: %s" % item for item in wifi.status().items())
            return {"type": "command_result", "command": command, "output": output}
        return {"type": "command_result", "command": command,
                "output": "Command not available. Type help."}
    return {"type": "error", "message": "Unsupported request"}


def run():
    if machine is None:
        raise RuntimeError("ESP32 machine module unavailable")
    uart = machine.UART(0, baudrate=BAUDRATE, bits=8, parity=None, stop=1)
    # UART0 is the board's CH343 USB-UART bridge. Detach the interactive REPL
    # so the desktop can use a clean, newline-delimited JSON protocol.
    if uos is not None:
        try:
            uos.dupterm(None)
        except Exception:
            pass
    print("[AMH] USB desktop service ready at 115200 baud")
    buffer = b""
    while True:
        chunk = uart.read()
        if chunk:
            buffer += chunk
            if len(buffer) > MAX_LINE:
                buffer = b""
                _reply(uart, {"type": "error", "message": "Request too long"})
            while b"\n" in buffer:
                line, buffer = buffer.split(b"\n", 1)
                try:
                    request = json.loads(line.decode().strip())
                    _reply(uart, _handle(request))
                except Exception as exc:
                    _reply(uart, {"type": "error", "message": str(exc)})
        gc.collect()
        time.sleep_ms(20)
