import os


def list_root():
    items = []
    try:
        names = os.listdir("/")
    except Exception:
        names = []
    for name in names:
        items.append({"name": name, "type": "entry"})
    items.sort(key=lambda item: item["name"])
    return items
