# Assignment 6: Wi-Fi Server

## Overview

This assignment demonstrates HTTP server on ESP32-C3/ESP32-S3. Examples include:
- HTTP server with multiple endpoints
- LED control via web interface
- Serving HTML pages
- JSON API for sensor data

## Board Support

- **ESP32-C3**: esp32c3-generic target
- **ESP32-S3**: esp32s3-generic target
- **NOT supported**: ESP32 (original)

## Prerequisites

Before flashing, download required dependencies:

```bash
go mod download tinygo.org/x/espradio
go mod download tinygo.org/x/drivers
go mod download tinygo.org/x/espradio/netlink
```

This ensures the Wi-Fi radio, network driver, and netlink packages are available for TinyGo.

## Examples

Each example is a complete program.

**Wi-Fi credentials for this workshop:**
- SSID: `tinygo`
- Password: `gophercamp`

### 1. HTTP LED Control Server (main.go)

HTTP server with web interface to control LED. Serves HTML page with buttons and JSON status endpoint.

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  main.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  main.go
```

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32-C3 or ESP32-S3 board
- Wi-Fi network (2.4GHz)
- USB-C cable

## Usage

1. Flash firmware
2. Watch serial output for IP address
3. Open browser: `http://YOUR_BOARD_IP:8080`
4. Control LED via web interface

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-6/
