#!/usr/bin/env python3
"""AMH-OS Windows companion: allowlisted desktop actions only.

Binds to loopback, requires a random bearer token, and never accepts shell text.
"""
from __future__ import annotations

import secrets
import subprocess
import threading
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST = "127.0.0.1"
PORT = 8765
ALLOWED_ORIGIN = "http://192.168.4.1"
TOKEN = secrets.token_urlsafe(24)


class Handler(BaseHTTPRequestHandler):
    server_version = "AMH-OS-Companion/0.1"

    def log_message(self, fmt: str, *args: object) -> None:
        print("[AMH-COMPANION] " + (fmt % args))

    def _headers(self, status: int = 200) -> None:
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        origin = self.headers.get("Origin", "")
        if origin == ALLOWED_ORIGIN:
            self.send_header("Access-Control-Allow-Origin", ALLOWED_ORIGIN)
            self.send_header("Vary", "Origin")
            self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "Authorization, Content-Type")
            self.send_header("Access-Control-Allow-Private-Network", "true")
        self.end_headers()

    def _json(self, status: int, payload: str) -> None:
        self._headers(status)
        self.wfile.write(payload.encode("utf-8"))

    def do_OPTIONS(self) -> None:
        if self.headers.get("Origin") != ALLOWED_ORIGIN:
            self._headers(403)
            return
        self._headers(204)

    def do_GET(self) -> None:
        if self.path != "/api/health":
            self._json(404, '{"error":"Not found"}')
            return
        if not self._authorized():
            self._json(401, '{"error":"Unauthorized"}')
            return
        self._json(200, '{"status":"online","companion":"AMH-OS Windows Companion","version":"0.1.0"}')

    def do_POST(self) -> None:
        if self.headers.get("Origin") != ALLOWED_ORIGIN:
            self._json(403, '{"error":"Origin not allowed"}')
            return
        if not self._authorized():
            self._json(401, '{"error":"Unauthorized"}')
            return
        actions = {
            "/api/launch/browser": self._browser,
            "/api/launch/explorer": self._explorer,
            "/api/launch/taskmgr": self._taskmgr,
        }
        action = actions.get(self.path)
        if action is None:
            self._json(404, '{"error":"Action not allowed"}')
            return
        try:
            action()
            self._json(200, '{"status":"launched"}')
        except OSError as exc:
            self._json(500, '{"error":"Could not launch application: ' + str(exc).replace('"', "'") + '"}')

    def _authorized(self) -> bool:
        return secrets.compare_digest(
            self.headers.get("Authorization", ""),
            "Bearer " + TOKEN,
        )

    @staticmethod
    def _browser() -> None:
        # Uses Windows' registered default browser; no executable path is supplied.
        if not webbrowser.open("https://www.google.com", new=2):
            raise OSError("Default browser did not accept the launch request")

    @staticmethod
    def _explorer() -> None:
        subprocess.Popen(["explorer.exe"], close_fds=True)

    @staticmethod
    def _taskmgr() -> None:
        subprocess.Popen(["taskmgr.exe"], close_fds=True)


def main() -> None:
    server = ThreadingHTTPServer((HOST, PORT), Handler)
    print("=" * 58)
    print(" AMH-OS WINDOWS COMPANION 0.1.0")
    print("=" * 58)
    print("Listening only on this laptop: http://127.0.0.1:" + str(PORT))
    print("Pairing token (paste into AMH-OS Settings):")
    print(TOKEN)
    print("Allowed actions: default browser, File Explorer, Task Manager")
    print("No arbitrary CMD, PowerShell, or file-system command endpoint.")
    print("Keep this terminal running while using the AMH-OS desktop.")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping AMH-OS companion...")
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()
