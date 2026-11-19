import argparse
import re
import sys
from pathlib import Path

import serial
from serial.tools import list_ports

START = "=== Data Collection Results ==="
END = "=== End of Data Collection ==="
ANSI = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
ESPRESSIF_VID = 0x303A


def board_ports():
    skipped = ("bluetooth", "debug-console")
    candidates = []
    for info in list_ports.comports():
        device = info.device or ""
        label = f"{device} {info.description or ''}".lower()
        if any(word in label for word in skipped):
            continue
        candidates.append(info)

    espressif = [info.device for info in candidates if info.vid == ESPRESSIF_VID]
    if espressif:
        return espressif

    usb_marks = ("usbmodem", "usbserial", "ttyACM", "ttyUSB", "COM", "wchusb", "SLAB")
    return [info.device for info in candidates if any(mark in info.device for mark in usb_marks)]


def open_board():
    names = board_ports()
    if not names:
        print("No board serial port found.", file=sys.stderr)
        return None
    for name in names:
        try:
            port = serial.Serial(name, 115200, timeout=0.2)
        except serial.SerialException as exc:
            print(f"Skipping {name}: {exc}")
            continue
        print(f"Using {name}")
        return port
    print("Could not open a board serial port.", file=sys.stderr)
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--count", type=int, default=25)
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path("shuttle_dataset/new_gesture.csv"),
    )
    args = parser.parse_args()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    rows = []
    capturing = False
    pending = ""

    print("Long-press Boot for about 2 seconds, then release.")
    print("Short-press Boot and perform the gesture once for each sample.")
    print(f"Need {args.count} rows. Press Ctrl+C to stop early.")

    port = open_board()
    if port is None:
        return 1

    try:
        while len(rows) < args.count:
            chunk = port.read(4096)
            if not chunk:
                continue
            pending += ANSI.sub("", chunk.decode("utf-8", "replace"))
            while "\n" in pending:
                line, pending = pending.split("\n", 1)
                line = line.strip()
                if "Mode switched to" in line:
                    print(line)
                if START in line:
                    capturing = True
                    continue
                if END in line:
                    capturing = False
                    continue
                if not capturing or "," not in line:
                    continue
                values = [item for item in line.split(",") if item]
                if len(values) != 600:
                    print(f"Skipped a row with {len(values)} numbers.")
                    continue
                rows.append(",".join(values))
                print(f"Saved {len(rows)}/{args.count}")
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        port.close()

    text = "\n".join(rows)
    if text:
        text += "\n"
    args.output.write_text(text)
    print(f"Wrote {len(rows)} rows to {args.output}")
    return 0 if len(rows) >= args.count else 2


if __name__ == "__main__":
    raise SystemExit(main())
