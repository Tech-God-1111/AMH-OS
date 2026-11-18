import socket
from amh import api

ASSETS = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/app.css": ("app.css", "text/css; charset=utf-8"),
    "/app.js": ("app.js", "application/javascript; charset=utf-8"),
}


def send(client, status, content_type, body):
    if isinstance(body, str):
        body = body.encode("utf-8")
    reason = "OK" if status == 200 else "Not Found" if status == 404 else "Bad Request"
    header = ("HTTP/1.1 %d %s\r\n" % (status, reason)
              + "Content-Type: %s\r\n" % content_type
              + "Content-Length: %d\r\n" % len(body)
              + "Connection: close\r\nCache-Control: no-store\r\n\r\n")
    client.send(header.encode("utf-8"))
    client.send(body)


def read_request(client):
    data = b""
    while b"\r\n\r\n" not in data and len(data) < 8192:
        chunk = client.recv(512)
        if not chunk:
            break
        data += chunk
    if b"\r\n\r\n" not in data:
        return None
    header, body = data.split(b"\r\n\r\n", 1)
    lines = header.decode("utf-8", "ignore").split("\r\n")
    parts = lines[0].split(" ")
    if len(parts) < 2:
        return None
    method, path = parts[0].upper(), parts[1].split("?", 1)[0]
    length = 0
    for line in lines[1:]:
        if line.lower().startswith("content-length:"):
            try:
                length = int(line.split(":", 1)[1].strip())
            except Exception:
                length = 0
    if length > 2048:
        return method, path, ""
    while len(body) < length:
        chunk = client.recv(min(512, length - len(body)))
        if not chunk:
            break
        body += chunk
    return method, path, body[:length].decode("utf-8", "ignore")


def serve_client(client):
    request = read_request(client)
    if request is None:
        send(client, 400, "application/json", '{"error":"Invalid request"}')
        return
    method, path, body = request
    if method == "GET" and path in ASSETS:
        filename, content_type = ASSETS[path]
        try:
            with open("/web/" + filename, "rb") as handle:
                send(client, 200, content_type, handle.read())
        except Exception:
            send(client, 404, "text/plain", "Dashboard asset missing")
        return
    status, content_type, payload = api.handle(method, path, body)
    send(client, status, content_type, payload)


def serve(host="0.0.0.0", port=80):
    address = socket.getaddrinfo(host, port)[0][-1]
    server = socket.socket()
    try:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except Exception:
        pass
    server.bind(address)
    server.listen(2)
    print("[AMH] Dashboard listening on port", port)
    while True:
        client, remote = server.accept()
        try:
            client.settimeout(3)
            serve_client(client)
        except Exception as exc:
            print("[AMH] HTTP client error:", repr(exc))
        finally:
            try:
                client.close()
            except Exception:
                pass
