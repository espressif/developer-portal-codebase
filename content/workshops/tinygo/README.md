# TinyGo Embedded Workshop - Source Code

This repository contains the source code examples for the TinyGo Embedded Workshop.

## Workshop Overview

The TinyGo Embedded Workshop teaches embedded development using Go on ESP32 microcontrollers. Through 7 assignments, you'll learn GPIO control, display programming, sensor reading, Wi-Fi connectivity, and edge AI concepts.

## Assignments

### Assignment 1: Install TinyGo
Test installation with a simple "Hello World" program.

### Assignment 2: Blinky
LED control examples including:
- Basic blinky for ESP32/ESP32-S3/ESP32-C3
- Morse code SOS signal
- Serial output debugging
- RGB LED (NeoPixel) control

### Assignment 3: Display
LCD display programming for M5Stack Core2:
- Display initialization and power management
- Drawing shapes (lines, rectangles, circles)
- Text rendering with fonts
- Basic animation

### Assignment 4: Sensors
I2C sensor reading with BMI260 accelerometer:
- Reading accelerometer data
- Displaying sensor readings
- Motion detection
- Orientation detection

### Assignment 5: Wi-Fi Client
Wi-Fi connectivity on ESP32-C3/ESP32-S3:
- Wi-Fi network scanning
- Connecting to Wi-Fi networks
- HTTP client requests

### Assignment 6: Wi-Fi Server
HTTP server on ESP32-C3/ESP32-S3:
- Web server with multiple endpoints
- LED control via web interface
- JSON API for sensor data

### Assignment 7: AI Edge Models
Edge AI concepts in pure Go:
- Threshold-based gesture classification
- Pattern recognition
- Decision tree classification
- k-Nearest neighbors (k-NN)

## Requirements

- Go 1.26+
- TinyGo 0.41+
- ESP32 development board:
  - M5Stack Core2 (recommended for assignments 1-4, 7)
  - M5Stack StampC3 (ESP32-C3, for assignments 5-6)
  - XIAO ESP32-C3 / ESP32-S3
- USB-C cable (data + power)

## Build Instructions

Each assignment directory contains a complete working example:

```bash
# Navigate to assignment directory
cd assignment_2

# Build for your target board
tinygo flash -target esp32s3-generic .        # ESP32-S3
tinygo flash -target m5stack-core2 .          # ESP32 (M5Stack Core2)
tinygo flash -target m5stack-stampc3 .        # ESP32-C3 (M5Stack StampC3)
```

## Wi-Fi Credentials

For Wi-Fi assignments (5-6), pass credentials at compile time:

```bash
tinygo flash -target m5stack-stampc3 \
  -ldflags="-X main.ssid=YourSSID -X main.password=YourPassword" .
```

## Board Support Matrix

| Assignment | ESP32 | ESP32-S3 | ESP32-C3 |
|------------|-------|----------|----------|
| 1. Install  | Host | Host | Host |
| 2. Blinky   | Yes | Yes | Yes |
| 3. Display  | Yes (Core2) | No | No |
| 4. Sensors  | Yes (Core2) | No | No |
| 5. Wi-Fi Client | No | Yes | Yes |
| 6. Wi-Fi Server | No | Yes | Yes |
| 7. AI Edge  | Yes (Core2) | No | No |

## Related Resources

- [Workshop Homepage](https://developer.espressif.com/workshops/tinygo/)
- [TinyGo Documentation](https://tinygo.org/)
- [TinyGo Drivers](https://github.com/tinygo-org/drivers)
- [M5Stack Core2 Docs](https://docs.m5stack.com/en/core/core2)

## License

Copyright (c) 2026 Espressif Systems (Shanghai) Co. Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
