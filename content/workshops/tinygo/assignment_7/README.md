# Assignment 7: AI Edge Models

## Overview

This assignment explores edge AI concepts with pure Go implementations. Examples include:
- Threshold-based gesture classification
- Pattern recognition with moving average
- Decision tree activity classification
- k-Nearest neighbors (k-NN) concepts

## Board Support

- **M5Stack Core2** (ESP32 with built-in BMI260 accelerometer)
- Any ESP32 board with external accelerometer

## Build Instructions

```bash
tinygo flash -target m5stack-core2 .
```

## Examples

### threshold.go
Threshold-based gesture detection. Classifies shake, wave, and idle states.

### pattern.go
Pattern recognition with moving average smoothing and peak detection.

### decision_tree.go
Decision tree for activity classification (running, walking, sitting, stationary).

### knn.go
Simple k-NN implementation for small datasets.

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32 board with accelerometer
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-7/
