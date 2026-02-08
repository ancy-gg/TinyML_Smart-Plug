# type: ignore
Import("env")

from serial.tools import list_ports

def pick_usb_port():
    ports = list(list_ports.comports())
    for p in ports:
        dev = (p.device or "").lower()

        # Windows
        if dev.startswith("com"):
            return p.device

        # Linux
        if "ttyusb" in dev or "ttyacm" in dev:
            return p.device

        # macOS
        if "/dev/cu." in dev:
            return p.device

    return None

def force_usb_or_fail(source, target, env):
    usb = pick_usb_port()

    if usb:
        print(f"\n[AutoUpload] USB detected: {usb} -> using USB (esptool)\n")
        env.Replace(UPLOAD_PROTOCOL="esptool")
        env.Replace(UPLOAD_PORT=usb)
        env.Replace(UPLOAD_FLAGS=[])  # remove OTA-only flags
        return

    print("\n[AutoUpload] ERROR: No USB serial port detected.")
    print("[AutoUpload] This project uses HTTPS Pull-OTA for field updates.")
    print("[AutoUpload] Plug the board via USB to upload firmware.\n")
    env.Exit(1)

env.AddPreAction("upload", force_usb_or_fail)