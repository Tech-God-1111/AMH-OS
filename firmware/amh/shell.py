"""Small allowlisted AMH-OS shell. This is not a Python REPL or remote system shell."""
from amh import fs_service

_cwd = "/"


def _resolve(path):
    if not path:
        return _cwd
    if path.startswith("/"):
        return path
    return (_cwd.rstrip("/") + "/" + path) if _cwd != "/" else "/" + path


def _format_entries(path):
    data = fs_service.list_dir(path)
    lines = ["Directory: " + data["path"]]
    for item in data["entries"]:
        marker = "[DIR] " if item["type"] == "directory" else "      "
        lines.append(marker + item["name"] + ("" if item["type"] == "directory" else "  " + str(item["size"]) + " B"))
    return "\n".join(lines)


def run(command):
    global _cwd
    raw = str(command or "").strip()
    if not raw:
        return ""
    if len(raw) > 256:
        return "Error: command exceeds 256 characters"
    parts = raw.split()
    name = parts[0].lower()
    args = parts[1:]
    try:
        if name in ("help", "?"):
            return ("AMH-OS shell commands:\n"
                    "help | clear | version | sys | uptime | free | ps | df\n"
                    "pwd | ls [path] | cd [path] | cat <file>\n"
                    "mkdir <dir> | touch <file> | write <file> <text>\n"
                    "rm <file> | rmdir <empty-dir> | mv <src> <dst> | cp <src> <dst>\n"
                    "wifi | ble status | ble scan | ble stop\n"
                    "No arbitrary Python execution or host-PC shell access.")
        if name == "clear":
            return "__CLEAR__"
        if name == "version":
            return "AMH-OS 0.3.0 | MicroPython"
        if name == "pwd":
            return _cwd
        if name == "ls":
            return _format_entries(_resolve(args[0] if args else _cwd))
        if name == "cd":
            target = _resolve(args[0] if args else "/")
            fs_service.list_dir(target)
            _cwd = target.rstrip("/") or "/"
            return _cwd
        if name == "cat" and len(args) == 1:
            return fs_service.read_file(_resolve(args[0]))["content"]
        if name == "mkdir" and len(args) == 1:
            fs_service.make_dir(_resolve(args[0]))
            return "Directory created."
        if name == "touch" and len(args) == 1:
            result = fs_service.touch_file(_resolve(args[0]))
            return "File ready: " + result["path"]
        if name == "write" and len(args) >= 2:
            path = _resolve(args[0])
            fs_service.write_file(path, " ".join(args[1:]))
            return "Wrote file: " + path
        if name == "rm" and len(args) == 1:
            fs_service.remove_file(_resolve(args[0]))
            return "File removed."
        if name == "rmdir" and len(args) == 1:
            fs_service.remove_empty_dir(_resolve(args[0]))
            return "Empty directory removed."
        if name == "mv" and len(args) == 2:
            fs_service.rename_path(_resolve(args[0]), _resolve(args[1]))
            return "Path moved."
        if name == "cp" and len(args) == 2:
            fs_service.copy_file(_resolve(args[0]), _resolve(args[1]))
            return "File copied."
        if name == "sys":
            from amh import system
            return "\n".join("%s: %s" % item for item in system.info().items())
        if name == "uptime":
            from amh import task_manager
            return "Uptime: %s ms" % task_manager.snapshot()["uptime_ms"]
        if name == "free":
            import gc
            gc.collect()
            return "Free heap: %s bytes" % gc.mem_free()
        if name == "ps":
            from amh import task_manager
            return "\n".join("%s  %s  %s" % (x["name"], x["state"], x["kind"]) for x in task_manager.snapshot()["services"])
        if name == "df":
            from amh import task_manager
            data = task_manager.snapshot()
            return "Storage total: %s bytes\nStorage free: %s bytes" % (data.get("storage_total", "?"), data.get("storage_free", "?"))
        if name == "wifi":
            from amh import wifi
            return "\n".join("%s: %s" % item for item in wifi.status().items())
        if name == "ble":
            from amh import bluetooth
            action = args[0].lower() if args else "status"
            if action == "scan":
                return str(bluetooth.start_scan())
            if action == "stop":
                return str(bluetooth.stop_scan())
            return str(bluetooth.results())
        return "Unknown command: " + name + " (type help)"
    except Exception as exc:
        return "Error: " + str(exc)
