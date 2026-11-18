import gc
from amh import config, wifi, webserver


def main():
    print("\n==============================")
    print(" AMH-OS Lite")
    print("==============================")
    settings = config.load()
    state = wifi.start(settings)
    gc.collect()
    print("[AMH] Wi-Fi mode:", state.get("mode"))
    print("[AMH] Address:", state.get("ip", "unavailable"))
    print("[AMH] Free heap:", gc.mem_free())
    webserver.serve()


try:
    main()
except Exception as exc:
    print("[AMH] Startup failed:", repr(exc))
    print("[AMH] MicroPython REPL remains available.")
