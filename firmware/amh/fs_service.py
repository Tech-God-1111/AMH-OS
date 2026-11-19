"""Bounded filesystem API for AMH-OS. All paths are confined to the device filesystem root."""
import os

MAX_READ_BYTES = 8192
MAX_WRITE_BYTES = 8192


def _path(path):
    value = str(path or "/").replace("\\", "/")
    if not value.startswith("/"):
        value = "/" + value
    parts = []
    for part in value.split("/"):
        if not part or part == ".":
            continue
        if part == "..":
            raise ValueError("Parent-directory traversal is not allowed")
        parts.append(part)
    return "/" + "/".join(parts) if parts else "/"


def _is_dir(stat_result):
    return bool(stat_result[0] & 0x4000)


def list_dir(path="/"):
    target = _path(path)
    names = os.listdir(target)
    entries = []
    for name in names:
        full = target.rstrip("/") + "/" + name if target != "/" else "/" + name
        try:
            st = os.stat(full)
            entries.append({"name": name, "path": full,
                            "type": "directory" if _is_dir(st) else "file",
                            "size": st[6] if not _is_dir(st) else 0})
        except Exception:
            entries.append({"name": name, "path": full, "type": "unknown", "size": 0})
    entries.sort(key=lambda item: (item["type"] != "directory", item["name"].lower()))
    return {"path": target, "entries": entries}


def read_file(path):
    target = _path(path)
    st = os.stat(target)
    if _is_dir(st):
        raise ValueError("Path is a directory")
    if st[6] > MAX_READ_BYTES:
        raise ValueError("File is too large to preview (8 KB limit)")
    with open(target, "r") as handle:
        return {"path": target, "content": handle.read()}


def write_file(path, content):
    target = _path(path)
    text = str(content)
    if len(text) > MAX_WRITE_BYTES:
        raise ValueError("Write limit is 8 KB")
    with open(target, "w") as handle:
        handle.write(text)
    return {"path": target, "bytes": len(text)}


def make_dir(path):
    target = _path(path)
    os.mkdir(target)
    return {"path": target, "created": True}


def remove_file(path):
    target = _path(path)
    if target == "/":
        raise ValueError("Cannot remove filesystem root")
    st = os.stat(target)
    if _is_dir(st):
        raise ValueError("Use remove_empty_dir for directories")
    os.remove(target)
    return {"path": target, "removed": True}


def remove_empty_dir(path):
    target = _path(path)
    if target == "/":
        raise ValueError("Cannot remove filesystem root")
    os.rmdir(target)
    return {"path": target, "removed": True}


def rename_path(source, destination):
    src, dst = _path(source), _path(destination)
    if src == "/" or dst == "/":
        raise ValueError("Cannot rename filesystem root")
    os.rename(src, dst)
    return {"source": src, "destination": dst, "renamed": True}


def copy_file(source, destination):
    src, dst = _path(source), _path(destination)
    st = os.stat(src)
    if _is_dir(st):
        raise ValueError("Directory copy is not supported")
    if st[6] > MAX_READ_BYTES:
        raise ValueError("File is too large to copy (8 KB limit)")
    with open(src, "r") as reader:
        data = reader.read()
    with open(dst, "w") as writer:
        writer.write(data)
    return {"source": src, "destination": dst, "bytes": len(data)}
