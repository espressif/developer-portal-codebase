# Assignment 6: Wi-Fi Server

## Overview

This assignment demonstrates HTTP server on ESP32-C3/ESP32-S3. Examples include:
- HTTP server with multiple endpoints
- LED control via web interface
- Serving HTML pages
- JSON API for sensor data

## Board Support

- **ESP32-C3**: m5stack-stampc3 target
- **ESP32-S3**: esp32s3-generic target
- **NOT supported**: ESP32 (original)

## Build Instructions

### ESP32-C3
```bash
tinygo flash -target m5stack-stampc3 \
  -ldflags="-X main.ssid=YourSSID -X main.password=YourPassword" .
```

### ESP32-S3
```bash
tinygo flash -target esp32s3-generic \
  -ldflags="-X main.ssid=YourSSID -X main.password=YourPassword" .
```

## Examples

### main.go
Basic HTTP server with LED control endpoints. Serves HTML interface with buttons.

### sensor_server.go
Advanced server with accelerometer data. Real-time sensor readings over HTTP.

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
