#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.9"
# dependencies = []
# ///
"""Simple HTTP server that serves an ESP-IDF firmware binary as /firmware.bin.

By default it serves build/simple-ota.bin on port 8070, matching the
FIRMWARE_UPGRADE_URL default (http://<your-ip>:8070/firmware.bin).

It also serves /version.json, describing the version of the firmware being
served. The version is read from FIRMWARE_VERSION_MESSAGE in main/main.c so it
always matches the source the firmware.bin was built from. The device queries
this route first and only performs the OTA when the versions differ.

Usage:
    uv run serve_firmware.py                 # serve build/simple-ota.bin on :8070
    uv run serve_firmware.py path/to/app.bin  # serve a specific file
    uv run serve_firmware.py app.bin 9000     # custom file and port
"""

import http.server
import json
import os
import re
import shutil
import socket
import sys

DEFAULT_FIRMWARE = os.path.join("build", "simple-ota.bin")
DEFAULT_PORT = 8070
VERSION_DEFINE = "FIRMWARE_VERSION_MESSAGE"
MAIN_SOURCE = os.path.join("main", "main.c")


def read_firmware_version():
    """Extract FIRMWARE_VERSION_MESSAGE from main/main.c next to this script."""
    source = os.path.join(os.path.dirname(os.path.abspath(__file__)), MAIN_SOURCE)
    try:
        with open(source, "r", encoding="utf-8") as f:
            text = f.read()
    except OSError:
        return None
    match = re.search(r'#define\s+' + VERSION_DEFINE + r'\s+"([^"]*)"', text)
    return match.group(1) if match else None


def get_local_ip():
    """Best-effort detection of the LAN IP the ESP32 should connect to."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except OSError:
        return "127.0.0.1"
    finally:
        s.close()


def main():
    firmware = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_FIRMWARE
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT

    if not os.path.isfile(firmware):
        sys.exit(f"Firmware file not found: {firmware}\n"
                 "Build it first with 'idf.py build'.")

    firmware = os.path.abspath(firmware)

    version = read_firmware_version()
    if version is None:
        version = "unknown"
        sys.stderr.write(
            f"Warning: could not read {VERSION_DEFINE} from {MAIN_SOURCE}; "
            "serving version \"unknown\".\n")

    class FirmwareHandler(http.server.BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path == "/version.json":
                body = json.dumps({"version": version}).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Connection", "close")
                self.end_headers()
                self.wfile.write(body)
                return
            if self.path == "/firmware.bin":
                size = os.path.getsize(firmware)
                self.send_response(200)
                self.send_header("Content-Type", "application/octet-stream")
                self.send_header("Content-Length", str(size))
                self.send_header("Connection", "close")
                self.end_headers()
                with open(firmware, "rb") as f:
                    shutil.copyfileobj(f, self.wfile)
                return
            self.send_error(404, "Only /firmware.bin and /version.json are served")

        def log_message(self, fmt, *args):
            sys.stderr.write("%s - %s\n" % (self.address_string(), fmt % args))

    server = http.server.HTTPServer(("0.0.0.0", port), FirmwareHandler)
    url = f"http://{get_local_ip()}:{port}/firmware.bin"
    version_url = f"http://{get_local_ip()}:{port}/version.json"
    print(f"Serving {firmware} ({os.path.getsize(firmware)} bytes)")
    print(f"Advertising firmware version: {version}")
    print(f"Set FIRMWARE_UPGRADE_URL to: {url}")
    print(f"Set FIRMWARE_VERSION_URL to: {version_url}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")
        server.server_close()


if __name__ == "__main__":
    main()
