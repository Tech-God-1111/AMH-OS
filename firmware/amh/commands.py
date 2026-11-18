def run(command="help"):
    parts = str(command).strip().lower().split()
    name = parts[0] if parts else "help"
    if name in ("help", "amh"):
        print("AMH-OS Lite: help, version, sys, wifi, fs")
    elif name == "version":
        from amh import VERSION
        print("AMH-OS Lite", VERSION)
    elif name == "sys":
        from amh.system import info
        for key, value in info().items():
            print("%s: %s" % (key, value))
    elif name == "wifi":
        from amh.wifi import status
        for key, value in status().items():
            print("%s: %s" % (key, value))
    elif name == "fs":
        from amh.filesystem import list_root
        for item in list_root():
            print("[%s] %s" % (item["type"], item["name"]))
    else:
        print("Unknown command:", name, "- run help")
