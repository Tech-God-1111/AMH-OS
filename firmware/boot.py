# Minimal startup hook. Keep the serial REPL recoverable.
try:
    import machine
    print("\n[AMH-OS Lite] Booting")
except Exception as exc:
    print("[AMH-OS Lite] Boot hook:", exc)
