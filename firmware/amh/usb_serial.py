"""Safe JSON-line USB serial service for AMH-OS 0.3."""
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
MAX_LINE = 10000


def _reply(uart, payload):
    try:
        uart.write((json.dumps(payload) + "\n").encode("utf-8"))
    except Exception:
        pass


def _handle(message):
    kind = message.get("type", "")
    if kind == "hello":
        return {"type": "hello", "name": "AMH-OS", "version": "0.3.0",
                "device": "ESP32-S3", "transport": "USB Serial"}
    if kind == "ping":
        return {"type": "pong"}
    if kind == "status":
        from amh import system, wifi, task_manager, bluetooth
        data = system.info()
        data["wifi"] = wifi.status()
        data["tasks"] = task_manager.snapshot()
        data["bluetooth"] = bluetooth.status()
        data["type"] = "status"
        data["version"] = "0.3.0"
        return data
    if kind == "command":
        from amh import shell
        command = str(message.get("command", ""))
        return {"type": "command_result", "command": command,
                "output": shell.run(command)}
    if kind == "fs":
        from amh import fs_service
        action = str(message.get("action", "list"))
        path = str(message.get("path", "/"))
        if action == "list":
            result = fs_service.list_dir(path)
        elif action == "read":
            result = fs_service.read_file(path)
        elif action == "write":
            result = fs_service.write_file(path, message.get("content", ""))
        elif action == "mkdir":
            result = fs_service.make_dir(path)
        elif action == "remove":
            result = fs_service.remove_file(path)
        elif action == "rmdir":
            result = fs_service.remove_empty_dir(path)
        elif action == "rename":
            result = fs_service.rename_path(path, str(message.get("destination", "")))
        elif action == "copy":
            result = fs_service.copy_file(path, str(message.get("destination", "")))
        else:
            raise ValueError("Unsupported filesystem action")
        return {"type": "fs_result", "action": action, "result": result}
    if kind == "tasks":
        from amh import task_manager
        return {"type": "tasks_result", "result": task_manager.snapshot()}
    if kind == "ble":
        from amh import bluetooth
        action = str(message.get("action", "status"))
        if action == "scan":
            result = bluetooth.start_scan(message.get("duration_ms", 8000))
        elif action == "stop":
            result = bluetooth.stop_scan()
        else:
            result = bluetooth.results()
        return {"type": "ble_result", "action": action, "result": result}
    return {"type": "error", "message": "Unsupported request"}


def run():
    if machine is None:
        raise RuntimeError("ESP32 machine module unavailable")
    uart = machine.UART(0, baudrate=BAUDRATE, bits=8, parity=None, stop=1)
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
