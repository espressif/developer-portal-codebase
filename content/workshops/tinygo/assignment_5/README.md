# Assignment 5: Wi-Fi Client

## Overview

This assignment demonstrates Wi-Fi connectivity on ESP32-C3/ESP32-S3 using the espradio package. Examples include:
- Wi-Fi network scanning
- Connecting to Wi-Fi networks
- HTTP client requests

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

Each example is a complete program. Build by specifying the source file.

**Wi-Fi credentials for this workshop:**
- SSID: `tinygo`
- Password: `gophercamp`

### 1. Wi-Fi Network Scanner (scan.go)

Scans for available Wi-Fi networks and displays SSID and signal strength. No credentials required.

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic scan.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic scan.go
```

### 2. Wi-Fi Connection (connect.go)

Connects to Wi-Fi network and displays assigned IP address.

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  connect.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  connect.go
```

### 3. HTTP Client (http_client.go)

Fetches a webpage via HTTP and displays the response.

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  http_client.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic \
  -ldflags="-X main.ssid=tinygo -X main.password=gophercamp" \
  http_client.go
```

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32-C3 or ESP32-S3 board
- Wi-Fi network (2.4GHz)
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-5/
