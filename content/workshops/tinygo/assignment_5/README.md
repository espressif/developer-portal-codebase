# Assignment 5: Wi-Fi Client

## Overview

This assignment demonstrates Wi-Fi connectivity on ESP32-C3/ESP32-S3 using the espradio package. Examples include:
- Wi-Fi network scanning
- Connecting to Wi-Fi networks
- HTTP client requests

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

### scan.go
Wi-Fi network scanner. Lists all available networks with SSID and signal strength.

### connect.go
Wi-Fi connection example. Connects to specified network and displays IP address.

### http_client.go
HTTP client example. Fetches webpage and displays response.

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32-C3 or ESP32-S3 board
- Wi-Fi network (2.4GHz)
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-5/
