import gc
import time

try:
    import machine
except ImportError:
    machine = None

START_TICKS = time.ticks_ms()


def info():
    gc.collect()
    result = {
        "name": "AMH-OS",
        "version": "0.3.0",
        "runtime": "MicroPython",
        "free_heap": gc.mem_free(),
        "allocated_heap": gc.mem_alloc() if hasattr(gc, "mem_alloc") else None,
        "uptime_ms": time.ticks_diff(time.ticks_ms(), START_TICKS),
    }
    if machine is not None:
        try:
            result["device_id"] = "".join(["%02x" % b for b in machine.unique_id()])
        except Exception:
            result["device_id"] = "unknown"
        try:
            result["frequency_mhz"] = machine.freq() // 1000000
        except Exception:
            pass
    return result
