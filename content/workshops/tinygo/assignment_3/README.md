# Assignment 3: Display

## Overview

This assignment demonstrates LCD display control on M5Stack Core2. Examples include:
- Display initialization and power management
- Drawing shapes (lines, rectangles, circles)
- Text rendering with fonts
- Basic animation

## Board Support

- **M5Stack Core2** (ESP32 with ILI9342C display)

## Build Instructions

```bash
tinygo flash -target m5stack-core2 .
```

## Examples

### main.go
Display initialization with basic screen fill. Demonstrates AXP192 power management for display backlight and ILI9342C driver configuration.

### shapes.go
Drawing various shapes on display: lines, rectangles, circles, triangles.

### text.go
Text rendering example using tinygl-font with Roboto font.

### animation.go
Simple bouncing ball animation example.

## Requirements

- TinyGo 0.41+
- Go 1.26+
- M5Stack Core2 board
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-3/
