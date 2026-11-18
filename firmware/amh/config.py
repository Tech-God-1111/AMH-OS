import json

CONFIG_PATH = "/config.json"
DEFAULTS = {
    "wifi_ssid": "",
    "wifi_password": "",
    "setup_ap_ssid": "AMH-OS-SETUP",
    "setup_ap_password": "amhsetup1234",
    "hostname": "amh-os-lite",
}


def load():
    try:
        with open(CONFIG_PATH, "r") as handle:
            saved = json.loads(handle.read())
    except Exception:
        saved = {}
    result = dict(DEFAULTS)
    if isinstance(saved, dict):
        for key in DEFAULTS:
            if key in saved:
                result[key] = saved[key]
    return result


def save(data):
    if not isinstance(data, dict):
        raise ValueError("Expected a JSON object")
    result = load()
    for key in DEFAULTS:
        if key in data:
            result[key] = str(data[key])
    if result["setup_ap_password"] and len(result["setup_ap_password"]) < 8:
        raise ValueError("Setup AP password must contain at least 8 characters")
    with open(CONFIG_PATH, "w") as handle:
        handle.write(json.dumps(result))
    return result
