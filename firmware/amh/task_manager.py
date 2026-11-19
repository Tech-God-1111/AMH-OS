"""Resource monitor for the MicroPython AMH-OS runtime."""
import gc
import time

try:
    import machine
except ImportError:
    machine = None

START_TICKS = time.ticks_ms()


def snapshot():
    gc.collect()
    try:
        from amh import bluetooth
        ble_state = "available" if bluetooth.status()["supported"] else "unsupported"
    except Exception:
        ble_state = "unavailable"
    services = [
        {"name": "usb-serial", "state": "running", "kind": "system"},
        {"name": "filesystem", "state": "ready", "kind": "system"},
        {"name": "shell", "state": "ready", "kind": "system"},
        {"name": "bluetooth-le", "state": ble_state, "kind": "hardware"},
    ]
    result = {
        "runtime": "MicroPython",
        "uptime_ms": time.ticks_diff(time.ticks_ms(), START_TICKS),
        "free_heap": gc.mem_free(),
        "allocated_heap": gc.mem_alloc() if hasattr(gc, "mem_alloc") else None,
        "services": services,
        "note": "Shows AMH-OS services, not every internal FreeRTOS task."
    }
    try:
        import os
        fs = os.statvfs("/")
        result["storage_total"] = fs[0] * fs[2]
        result["storage_free"] = fs[0] * fs[3]
    except Exception:
        pass
    if machine is not None:
        try:
            result["cpu_mhz"] = machine.freq() // 1000000
        except Exception:
            pass
    return result
