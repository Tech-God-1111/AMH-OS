import time

try:
    import network
except ImportError:
    network = None


def _ip(interface):
    try:
        return interface.ifconfig()[0]
    except Exception:
        return ""


def start(settings):
    if network is None:
        return {"mode": "unavailable", "connected": False, "ip": ""}
    try:
        sta = network.WLAN(network.STA_IF)
        sta.active(True)
        ssid = settings.get("wifi_ssid", "")
        if ssid:
            print("[AMH] Connecting to Wi-Fi:", ssid)
            sta.connect(ssid, settings.get("wifi_password", ""))
            for unused in range(20):
                if sta.isconnected():
                    try:
                        network.WLAN(network.AP_IF).active(False)
                    except Exception:
                        pass
                    return {"mode": "station", "connected": True,
                            "ip": _ip(sta), "ssid": ssid}
                time.sleep(0.5)
        ap = network.WLAN(network.AP_IF)
        ap.active(True)
        ap.config(essid=settings.get("setup_ap_ssid", "AMH-OS-SETUP"),
                  password=settings.get("setup_ap_password", "amhsetup1234"))
        return {"mode": "setup_ap", "connected": False,
                "ip": _ip(ap), "ssid": settings.get("setup_ap_ssid")}
    except Exception as exc:
        print("[AMH] Wi-Fi error:", repr(exc))
        return {"mode": "error", "connected": False, "ip": ""}


def status():
    if network is None:
        return {"connected": False, "mode": "unavailable", "ip": ""}
    try:
        sta = network.WLAN(network.STA_IF)
        if sta.active() and sta.isconnected():
            return {"connected": True, "mode": "station", "ip": _ip(sta)}
        ap = network.WLAN(network.AP_IF)
        if ap.active():
            return {"connected": False, "mode": "setup_ap", "ip": _ip(ap)}
    except Exception:
        pass
    return {"connected": False, "mode": "offline", "ip": ""}
