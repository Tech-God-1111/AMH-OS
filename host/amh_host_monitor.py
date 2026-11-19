"""Optional, read-only Windows metrics service for AMH-OS Desktop.

Binds only to 127.0.0.1:8766. It exposes metrics over GET and never runs
commands, changes settings, or accepts remote network connections.
"""
import json
import os
import platform
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlsplit

try:
    import psutil
except ImportError:
    psutil = None

HOST = "127.0.0.1"
PORT = 8766
ALLOWED_ORIGINS = {
    "http://localhost:8765",
    "http://127.0.0.1:8765",
}


def _bytes(value):
    return int(value)


def collect_metrics():
    if psutil is None:
        return {
            "available": False,
            "error": "psutil is not installed. Run: py -m pip install -r host/requirements.txt",
        }

    vm = psutil.virtual_memory()
    drive = os.environ.get("SystemDrive", "C:") + "\\"
    try:
        disk = psutil.disk_usage(drive)
        disk_data = {
            "path": drive,
            "total": _bytes(disk.total),
            "used": _bytes(disk.used),
            "free": _bytes(disk.free),
            "percent": float(disk.percent),
        }
    except Exception as exc:
        disk_data = {"error": str(exc)}

    net = psutil.net_io_counters()
    boot = psutil.boot_time()
    processes = []
    for proc in psutil.process_iter(["pid", "name", "memory_info"]):
        try:
            info = proc.info
            memory = info.get("memory_info")
            processes.append({
                "pid": int(info["pid"]),
                "name": str(info.get("name") or "Unknown")[:80],
                "memory_bytes": _bytes(memory.rss) if memory else 0,
            })
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue
    processes.sort(key=lambda item: item["memory_bytes"], reverse=True)

    running_services = None
    if hasattr(psutil, "win_service_iter"):
        try:
            running_services = sum(
                1 for service in psutil.win_service_iter()
                if service.status() == "running"
            )
        except Exception:
            running_services = None

    return {
        "available": True,
        "platform": platform.platform(),
        "computer": platform.node(),
        "cpu_percent": float(psutil.cpu_percent(interval=0.15)),
        "cpu_logical_cores": psutil.cpu_count(logical=True),
        "cpu_physical_cores": psutil.cpu_count(logical=False),
        "memory": {
            "total": _bytes(vm.total),
            "used": _bytes(vm.used),
            "available": _bytes(vm.available),
            "percent": float(vm.percent),
        },
        "storage": disk_data,
        "uptime_seconds": max(0, int(time.time() - boot)),
        "network": {
            "bytes_sent": _bytes(net.bytes_sent),
            "bytes_received": _bytes(net.bytes_recv),
        },
        "running_windows_services": running_services,
        "process_count": len(processes),
        "top_processes_by_memory": processes[:8],
        "read_only": True,
    }


class Handler(BaseHTTPRequestHandler):
    server_version = "AMHHostMonitor/1.0"

    def _send(self, status, payload):
        body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        origin = self.headers.get("Origin", "")
        if origin in ALLOWED_ORIGINS:
            self.send_header("Access-Control-Allow-Origin", origin)
            self.send_header("Vary", "Origin")
        self.end_headers()
        self.wfile.write(body)

    def _valid_host(self):
        try:
            parsed = urlsplit("//" + self.headers.get("Host", ""))
            return parsed.hostname in ("127.0.0.1", "localhost") and parsed.port == PORT
        except Exception:
            return False

    def do_OPTIONS(self):
        if not self._valid_host():
            self._send(403, {"available": False, "error": "Local access only"})
            return
        origin = self.headers.get("Origin", "")
        if origin not in ALLOWED_ORIGINS:
            self._send(403, {"available": False, "error": "Origin not allowed"})
            return
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", origin)
        self.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Vary", "Origin")
        self.end_headers()

    def do_GET(self):
        if not self._valid_host():
            self._send(403, {"available": False, "error": "Local access only"})
            return
        origin = self.headers.get("Origin", "")
        if origin and origin not in ALLOWED_ORIGINS:
            self._send(403, {"available": False, "error": "Origin not allowed"})
            return
        if self.path == "/health":
            self._send(200, {"service": "AMH-OS Host Monitor", "available": psutil is not None})
            return
        if self.path != "/api/v1/status":
            self._send(404, {"available": False, "error": "Not found"})
            return
        try:
            data = collect_metrics()
            self._send(200 if data.get("available") else 503, data)
        except Exception as exc:
            self._send(500, {"available": False, "error": str(exc)})

    def log_message(self, fmt, *args):
        print("[AMH Host Monitor] " + (fmt % args))


def main():
    print("AMH-OS Windows Host Monitor")
    print("Read-only metrics endpoint: http://127.0.0.1:8766")
    print("Listening on loopback only. Press Ctrl+C to stop.")
    if psutil is None:
        print("Missing dependency. Install with: py -m pip install -r host/requirements.txt")
    server = HTTPServer((HOST, PORT), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping AMH-OS Host Monitor...")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
