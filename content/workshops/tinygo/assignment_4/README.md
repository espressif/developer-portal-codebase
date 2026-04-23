# Assignment 4: Sensors

## Overview

This assignment demonstrates I2C sensor reading on M5Stack Core2. Examples include:
- I2C communication with BMI260 accelerometer
- Reading accelerometer data (X, Y, Z)
- Displaying sensor readings on screen
- Motion detection and orientation

## Board Support

- **M5Stack Core2** (ESP32 with built-in BMI260 accelerometer)

## Build Instructions

```bash
tinygo flash -target m5stack-core2 .
```

## Examples

### main.go
Basic accelerometer reading with serial output. Demonstrates I2C initialization and BMI260 driver usage.

### display.go
Accelerometer readings displayed on LCD screen with orientation detection.

### motion.go
Simple motion detection with threshold-based triggering.

## Requirements

- TinyGo 0.41+
- Go 1.26+
- M5Stack Core2 board
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-4/
