# Assignment 2: Blinky

## Overview

This assignment demonstrates GPIO control by blinking LEDs. Multiple examples included:
- Basic blinky for different boards
- Morse code SOS signal
- Serial output debugging
- RGB LED (NeoPixel) control

## Board Support

- **ESP32**: m5stack-core2 target (LED on GPIO2)
- **ESP32-S3**: esp32s3-generic target (LED on GPIO2)
- **ESP32-C3**: m5stack-stampc3 target (LED on GPIO8)

## Build Instructions

### ESP32 (M5Stack Core2)
```bash
tinygo flash -target m5stack-core2 .
```

### ESP32-S3
```bash
tinygo flash -target esp32s3-generic .
```

### ESP32-C3 (M5Stack StampC3)
```bash
tinygo flash -target m5stack-stampc3 .
```

## Examples

### main.go
Basic LED blink example. Select the appropriate board by uncommenting the GPIO pin for your board.

### morse.go
Morse code SOS signal example. Blinks "SOS" repeatedly.

### serial.go
Serial output example for debugging. Prints status messages to USB serial.

### rgb_led_esp32.go
RGB LED (NeoPixel) example for ESP32 boards. Cycles through colors.

### rgb_led_esp32c3.go
RGB LED (NeoPixel) example for ESP32-C3 boards.

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
