# simple-ota

Minimal ESP-IDF example that connects to Wi-Fi and upgrades its own firmware
over HTTP using the simplified `esp_https_ota()` interface.

## What it does

1. Connects to the Wi-Fi network configured via `menuconfig` (SSID/password).
2. Prints the running firmware version (e.g. `Hello world v1`).
3. Downloads `firmware.bin` from the configured URL and logs `OTA started`.
4. On success, logs `OTA ended successfully` and reboots into the new firmware,
   which then prints its own version string (e.g. `Hello world v2`).

## Firmware setup

1. Configure Wi-Fi credentials and the firmware URL:
   ```
   idf.py menuconfig
   ```
   Under **Simple OTA Configuration**, set `WIFI_SSID`, `WIFI_PASSWORD`, and
   `FIRMWARE_UPGRADE_URL` (e.g. `http://192.168.1.100:8070/firmware.bin`,
   using the IP address of the machine running the server below).

2. Build and flash the first version (with `FIRMWARE_VERSION_MESSAGE` in
   [main/main.c](main/main.c) left as `"Hello world v1"`):
   ```
   idf.py -p <PORT> flash monitor
   ```

3. Change `FIRMWARE_VERSION_MESSAGE` in [main/main.c](main/main.c) to
   `"Hello world v2"` and rebuild (without flashing over serial):
   ```
   idf.py build
   ```
   This produces `build/simple-ota.bin`, the new firmware to serve.

## Serving the firmware

[serve_firmware.py](serve_firmware.py) serves `build/simple-ota.bin` at
`/firmware.bin` over plain HTTP, matching `FIRMWARE_UPGRADE_URL`.

It only uses the Python standard library and is runnable directly with
[uv](https://docs.astral.sh/uv/):

```
uv run serve_firmware.py
```

This serves `build/simple-ota.bin` on port `8070`. On start it prints the
exact URL to use for `FIRMWARE_UPGRADE_URL`, based on your machine's LAN IP.

### Custom file or port

```
uv run serve_firmware.py path/to/app.bin
uv run serve_firmware.py path/to/app.bin 9000
```

### Without uv

The script has no third-party dependencies, so plain Python also works:
```
python3 serve_firmware.py
```

## Running the upgrade

1. With the v1 firmware running and connected to Wi-Fi, start the server on
   your computer:
   ```
   uv run serve_firmware.py
   ```
2. Reset the device (or trigger the upgrade path in your code). It will log
   `OTA started`, download the new binary, log `OTA ended successfully`, and
   reboot.
3. Watch the serial monitor: it should now print `Hello world v2`, confirming
   the new firmware is running.

## Notes

- The server only responds to `GET /firmware.bin`; any other path returns 404.
- Make sure the device and the machine running the server are on the same
  network, and that `FIRMWARE_UPGRADE_URL` uses the server machine's IP
  (not `localhost`).
- OTA over plain HTTP requires `CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP=y`, already
  set in [sdkconfig.defaults](sdkconfig.defaults).
