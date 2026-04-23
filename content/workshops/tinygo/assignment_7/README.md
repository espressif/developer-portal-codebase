# Assignment 7: AI Edge Models

## Overview

This assignment explores edge AI concepts with pure Go implementations. Examples include:
- Threshold-based gesture classification
- Pattern recognition with moving average
- Decision tree activity classification
- k-Nearest neighbors (k-NN) concepts

## Board Support

- **M5Stack Core2** (ESP32 with built-in BMI260 accelerometer)
- Any ESP32 board with external I2C accelerometer

## Examples

Each example is a complete program. Build by specifying the source file.

### 1. Threshold Gesture Detection (threshold.go)

Simple gesture detection using acceleration thresholds. Classifies shake, wave, and idle states.

**M5Stack Core2:**
```bash
tinygo flash -target m5stack-core2 threshold.go
```

### 2. Pattern Recognition (pattern.go)

Pattern recognition with moving average smoothing and peak detection. Demonstrates sensor data filtering.

**M5Stack Core2:**
```bash
tinygo flash -target m5stack-core2 pattern.go
```

### 3. Decision Tree Classifier (decision_tree.go)

Decision tree for activity classification (running, walking, sitting, stationary).

**M5Stack Core2:**
```bash
tinygo flash -target m5stack-core2 decision_tree.go
```

### 4. k-Nearest Neighbors (knn.go)

Simple k-NN implementation for small datasets.

**M5Stack Core2:**
```bash
tinygo flash -target m5stack-core2 knn.go
```

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32 board with accelerometer (BMI260 or compatible I2C)
- USB-C cable

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-7/
