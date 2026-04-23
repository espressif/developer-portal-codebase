# Assignment 2: Blinky

## Overview

This assignment demonstrates GPIO control by blinking LEDs. Multiple examples included:
- Basic blinky for different boards
- Morse code SOS signal
- Serial output debugging
- RGB LED (NeoPixel) control

## Hardware Setup

### Connecting an LED to ESP32

When connecting an external LED to your ESP32 board, you need:

1. **LED** (any color)
2. **Resistor** (see values below)
3. **Breadboard and jumper wires**

#### LED Pin Identification

- **Anode (+)**: Longer leg, connects to ESP32 GPIO pin through resistor
- **Cathode (-)**: Shorter leg, flat side on LED casing, connects to GND

#### Resistor Values for ESP32 (3.3V)

| LED Color | Forward Voltage | Resistor Range | Common Value |
|-----------|-----------------|----------------|--------------|
| Red | 1.8-2.2V | 330Ω - 1kΩ | 470Ω |
| Green | 2.0-3.0V | 220Ω - 680Ω | 330Ω |
| Blue | 3.0-3.3V | 100Ω - 330Ω | 220Ω |
| Orange/Yellow | 2.0-2.2V | 330Ω - 680Ω | 470Ω |

**Wiring**: ESP32 GPIO → Resistor → LED Anode → LED Cathode → GND

## Board Support

- **ESP32**: esp32-generic target (LED on GPIO2)
- **ESP32-S3**: esp32s3-generic target (LED on GPIO2)
- **ESP32-C3**: esp32c3-generic target (LED on GPIO8)

## Prerequisites

Before flashing RGB LED examples, download required dependencies:

```bash
go mod download tinygo.org/x/drivers
```

This ensures the `ws2812` driver package is available for TinyGo.

## Examples

Each example is a complete program. Build by specifying the source file:

### 1. Basic Blinky (blinky.go)

Simple LED blink with 500ms intervals.

**ESP32:**
```bash
tinygo flash -target esp32-generic blinky.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic blinky.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic blinky.go
```

### 2. Morse Code SOS (morse.go)

Blinks SOS Morse code pattern (··· --- ···) repeatedly.

**ESP32:**
```bash
tinygo flash -target esp32-generic morse.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic morse.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic morse.go
```

### 3. Serial Debug Output (serial.go)

Blinks LED and prints status messages to USB serial (115200 baud). Use `tinygo monitor` or `screen` to view output.

**ESP32:**
```bash
tinygo flash -target esp32-generic serial.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic serial.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic serial.go
```

### 4. RGB LED NeoPixel - ESP32 (rgb_led_esp32.go)

Cycles through colors on WS2812/NeoPixel RGB LED connected to GPIO2.

**ESP32:**
```bash
tinygo flash -target esp32-generic rgb_led_esp32.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic rgb_led_esp32.go
```

### 5. RGB LED NeoPixel - ESP32-C3 (rgb_led_esp32c3.go)

Cycles through colors on WS2812/NeoPixel RGB LED connected to GPIO8.

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic rgb_led_esp32c3.go
```

## Requirements

- TinyGo 0.41+
- Go 1.26+
- Supported hardware board
- USB cable for flashing

## Monitoring Serial Output

```bash
# Using tinygo monitor
tinygo monitor

# Using screen (Linux/macOS)
screen /dev/ttyUSB0 115200

# Using picocom
picocom -b 115200 /dev/ttyUSB0
```

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-2/
