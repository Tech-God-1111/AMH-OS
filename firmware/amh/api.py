import json
from amh import config, filesystem, hardware, system, wifi


def response(status, payload):
    return status, "application/json; charset=utf-8", json.dumps(payload)


def handle(method, path, body):
    if method == "GET" and path == "/api/status":
        return response(200, {"system": system.info(), "wifi": wifi.status()})
    if method == "GET" and path == "/api/files":
        return response(200, {"items": filesystem.list_root(), "read_only": True})
    if method == "POST" and path == "/api/wifi":
        try:
            data = json.loads(body or "{}")
            settings = config.save(data)
            state = wifi.start(settings)
            return response(200, {"saved": True, "wifi": state})
        except Exception as exc:
            return response(400, {"error": str(exc)})
    if method == "POST" and path == "/api/gpio":
        try:
            data = json.loads(body or "{}")
            return response(200, hardware.set_output(data.get("pin"), data.get("value", 0)))
        except Exception as exc:
            return response(400, {"error": str(exc)})
    return response(404, {"error": "Not found"})
