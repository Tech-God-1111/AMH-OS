"""BLE discovery helper. Does not implement Realme's proprietary watch protocol."""
import time

try:
    import bluetooth
except ImportError:
    bluetooth = None

_IRQ_SCAN_RESULT = 5
_IRQ_SCAN_DONE = 6
_ble = None
_results = []
_scanning = False
_error = ""


def _name_from_adv(data):
    index = 0
    while index + 1 < len(data):
        length = data[index]
        if length == 0:
            break
        end = index + 1 + length
        if end > len(data):
            break
        kind = data[index + 1]
        if kind in (0x08, 0x09):
            try:
                return bytes(data[index + 2:end]).decode("utf-8")
            except Exception:
                return ""
        index = end
    return ""


def _irq(event, data):
    global _scanning
    if event == _IRQ_SCAN_RESULT:
        addr_type, addr, adv_type, rssi, adv_data = data
        address = ":".join("%02X" % value for value in bytes(addr))
        item = {"address": address, "rssi": rssi,
                "name": _name_from_adv(adv_data), "advertisement_type": adv_type}
        for old in _results:
            if old["address"] == address:
                old.update(item)
                return
        if len(_results) < 30:
            _results.append(item)
    elif event == _IRQ_SCAN_DONE:
        _scanning = False


def status():
    return {"supported": bluetooth is not None, "scanning": _scanning,
            "devices_found": len(_results), "error": _error}


def start_scan(duration_ms=8000):
    global _ble, _scanning, _error, _results
    if bluetooth is None:
        return {"ok": False, "error": "Bluetooth LE module is not available in this firmware"}
    if _scanning:
        return {"ok": True, "message": "Scan already running"}
    try:
        if _ble is None:
            _ble = bluetooth.BLE()
            _ble.active(True)
            _ble.irq(_irq)
        _results = []
        _error = ""
        _ble.gap_scan(int(duration_ms), 30000, 30000, True)
        _scanning = True
        return {"ok": True, "message": "BLE scan started"}
    except Exception as exc:
        _error = str(exc)
        _scanning = False
        return {"ok": False, "error": _error}


def stop_scan():
    global _scanning, _error
    if _ble is not None:
        try:
            _ble.gap_scan(None)
        except Exception as exc:
            _error = str(exc)
    _scanning = False
    return status()


def results():
    return {"status": status(), "devices": list(_results)}
