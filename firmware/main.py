import gc
from amh import usb_serial


def main():
    print("\n==============================")
    print(" AMH-OS Lite - USB Desktop Demo")
    print("==============================")
    gc.collect()
    print("[AMH] Free heap:", gc.mem_free())
    usb_serial.run()


try:
    main()
except Exception as exc:
    print("[AMH] Startup failed:", repr(exc))
    print("[AMH] Check firmware deployment and reset the board.")
