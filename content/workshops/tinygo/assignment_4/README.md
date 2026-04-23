# Assignment 4: Sensors

## Overview

This assignment demonstrates I2C sensor reading and ADC input on ESP32 microcontrollers. Examples include:
- I2C communication with IMU sensors using raw I2C commands
- Reading accelerometer data (X, Y, Z)
- ADC joystick input with oversampling and normalization
- Displaying sensor readings on screen
- Motion detection and orientation

## Board Support

- **ESP32**: esp32-generic target
- **ESP32-C3**: esp32c3-generic target
- **ESP32-S3**: esp32s3-generic target

## Hardware Configuration

### I2C Bus Connection (ESP32-C3 with MPU-6050)

| Signal | GPIO  |
|--------|-------|
| SDA    | GPIO4 |
| SCL    | GPIO5 |

### I2C Bus Connection (esp-rust-board standard)

| Signal | GPIO  |
|--------|-------|
| SDA    | GPIO10 |
| SCL    | GPIO8  |

### ADC Joystick Connection (ESP32-S3)

| Axis   | GPIO       | ADC Pin       | ADC Channel |
|--------|------------|---------------|-------------|
| X-axis | GPIO4      | machine.ADC4  | ADC1_CH0    |
| Y-axis | GPIO6      | machine.ADC6  | ADC1_CH2    |

**Hardware Reference:** Dual-axis joystick using GPIO4 (X) and GPIO6 (Y).
- GPIO4 and GPIO6 use ADC1 which has no XTAL constraints (unlike GPIO15/16)
- ESP32-S3 ADC returns scaled values (0-65520), not raw 12-bit
- Center position: ~32760, Deadzone: ±5000
- Joystick potentiometers connect between ground and VCC (3.3V)
- Wiper outputs connect to GPIO4 (X) and GPIO6 (Y)

**Note:** ESP32-S3 ADC Get() returns scaled values (0-65520) per TinyGo implementation. GPIO15/16 are used by XTAL_32P/XTAL_32N and return stuck values ~30400.

### Supported I2C Sensors

| Peripheral          | Part Number  | I2C Address | Implementation |
|---------------------|--------------|-------------|-----------------|
| IMU/Accelerometer   | MPU-6050     | 0x68        | Raw I2C commands |
| IMU/Accelerometer   | ICM-42670-P  | 0x68        | Raw I2C commands |
| IMU/Accelerometer   | BMI160       | 0x68        | Raw I2C commands |
| Temp & Humidity     | SHTC3        | 0x70        | Raw I2C commands (see comments) |

**Note:** Examples use raw I2C commands via machine.I2C for maximum compatibility. No external driver packages required. This approach works with any I2C sensor.

## Examples

Each example is a complete program. Build by specifying the source file.

### 1. MPU-6050 Accelerometer (mpu6050.go)

Reads MPU-6050 accelerometer data via raw I2C on ESP32-C3. Demonstrates I2C initialization, sensor identification via WHO_AM_I register, sensor wake-up, and reading accelerometer data. Includes validation to verify sensor communication.

**Hardware:**
- MPU-6050 sensor
- SDA connected to GPIO4
- SCL connected to GPIO5
- I2C address: 0x68

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic mpu6050.go
```

**Expected Output:**
- WHO_AM_I: 0x68 (sensor validation)
- Accelerometer X, Y, Z values in g-force
- 1 reading per second

### 2. Basic Sensor Reading (main.go)

Reads ICM-42670-P accelerometer data via raw I2C and outputs to serial monitor. Demonstrates I2C initialization, sensor wake-up, and reading accelerometer registers. Works with any I2C IMU sensor.

**ESP32:**
```bash
tinygo flash -target esp32-generic main.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic main.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic main.go
```

### 3. Motion Detection (motion.go)

Simple motion detection with threshold-based triggering using raw I2C. Detects significant movement changes by comparing accelerometer readings.

**ESP32:**
```bash
tinygo flash -target esp32-generic motion.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic motion.go
```

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic motion.go
```

### 4. Joystick ADC Reader (joystick.go)

Reads joystick position using ADC pins. Demonstrates ADC configuration, oversampling for stable readings, deadzone handling, and value normalization. Outputs raw values (0-65520), normalized values (0.0-1.0), and directional vectors (-1.0 to 1.0). Uses GPIO4/GPIO6 to avoid XTAL constraints on GPIO15/16.

**How Joystick ADC Works:**

Joysticks contain two potentiometers (variable resistors), one for each axis:
- Each potentiometer acts as a voltage divider
- Wiper output voltage varies from 0V to 3.3V based on position
- ESP32 ADC converts voltage to digital value (0-65520)

**ADC Value Mapping:**
```
LEFT/UP position:    0 (0V)     → Direction -1.0
Center position:     ~32760 (1.65V)  → Direction 0.0 (in deadzone)
RIGHT/DOWN position: 65520 (3.3V)    → Direction 1.0
```

**Features:**
- GPIO4/GPIO6 for X/Y axes (ADC1, no XTAL constraints)
- ESP32-S3 scaled ADC range (0-65520), center at ~32760
- Deadzone ±10000 around center for stable readings
- Oversampling (5-10 samples) for noise reduction
- Dual-axis joystick support
- Direction normalization (-1.0 to 1.0)

**Pin Configuration (ESP32-S3):**
- X-axis: GPIO4 (machine.ADC4)
- Y-axis: GPIO6 (machine.ADC6)

**Hardware Wiring:**
```
Joystick Module          ESP32-S3
┌─────────────┐         ┌──────────┐
│ VCC         ├─────────┤ 3.3V     │
│ GND         ├─────────┤ GND      │
│ VRX (X-axis)├─────────┤ GPIO4    │
│ VRY (Y-axis)├─────────┤ GPIO6    │
└─────────────┘         └──────────┘
```

**Output Format:**
```
X=[raw] Y=[raw] | X=[0.0-1.0] Y=[0.0-1.0] | Dir: [x,y]
```

Example output:
```
X=0 Y=161 | X=0.00 Y=0.00 | Dir: [-1.00,-0.99]
X=26505 Y=726 | X=0.40 Y=0.01 | Dir: [-0.19,-0.97]
X=62718 Y=6721 | X=0.95 Y=0.10 | Dir: [0.91,-0.79]
```

**ESP32-S3 (Recommended for esp-rust-board):**
```bash
tinygo flash -target esp32s3-generic joystick.go
```

**ESP32-C3:**
```bash
tinygo flash -target esp32c3-generic joystick.go
```

**ESP32:**
```bash
tinygo flash -target esp32-generic joystick.go
```

### 4. Joystick Game (game.go)

Interactive ASCII art game controlled by joystick. Navigate player (@) to collect stars (*). Demonstrates game loops, real-time input processing, boundary checking, and state management.

**Game Mechanics:**

The game runs a continuous loop with these steps:
1. **Read ADC values** from joystick (5-sample oversampling)
2. **Convert to direction** using deadzone detection
3. **Update player position** at fixed time intervals (150ms)
4. **Check collision** with goal (spawn new goal on collect)
5. **Render game board** using ANSI escape codes

**Direction Detection:**
```
ADC Value < 22760 (center - 10000) → Direction -1 (UP/LEFT)
ADC Value > 42760 (center + 10000) → Direction +1 (DOWN/RIGHT)
ADC Value in range                  → Direction 0 (CENTER/NO MOVE)
```

**Features:**
- 20x10 game board rendered to serial console
- Real-time joystick control with 150ms movement speed
- Score tracking and random goal spawning
- ANSI escape codes for non-destructive screen refresh
- Deadzone filtering (±10000) for precise control
- X-axis direction inverted (hardware-specific)
- Boundary checking to keep player in board

**Pin Configuration (ESP32-S3):**
- X-axis: GPIO4 (machine.ADC4) - **direction inverted**
- Y-axis: GPIO6 (machine.ADC6) - normal direction

**ESP32-S3:**
```bash
tinygo flash -target esp32s3-generic game.go
```

**Game Controls:**
- Push joystick UP/DOWN/LEFT/RIGHT to move player (@)
- Center position = no movement
- Collect stars (*) to increase score
- Player position constrained to game board

**Display Format:**
```
+--------------------+
|....................|
|.........@...........|
|....................|
|...........*........|
|....................|
+--------------------+

Score: 5 | Move: RIGHT
X=12345 Y=32760
```

**Monitoring:**
Use `screen` or `picocom` for proper ANSI terminal support:
```bash
screen /dev/ttyUSB0 115200
picocom -b 115200 /dev/ttyUSB0
```

**Note:** ANSI escape codes may not display correctly in `tinygo monitor` or basic serial terminals. The game requires a VT100-compatible terminal for proper screen refresh.

### 5. Display Readings (display.go)

**Requires M5Stack Core2** - Displays accelerometer readings on LCD screen with orientation detection. This example is board-specific due to display hardware requirements.

```bash
tinygo flash -target m5stack-core2 display.go
```

## Requirements

- TinyGo 0.41+
- Go 1.26+
- ESP32 board with I2C sensor (MPU-6050, ICM-42670-P, BMI160, or compatible)
- USB-C cable
- For display.go: M5Stack Core2 with built-in LCD
- For joystick.go: ESP32-S3 with joystick module (optional)

## Customizing for Your Board

### I2C Sensors

To use different GPIO pins, edit the constants at the top of I2C example files:

```go
const (
    I2C_SDA = machine.GPIO10  // Change to your SDA pin
    I2C_SCL = machine.GPIO8   // Change to your SCL pin
    IMU_I2C_ADDR = 0x68       // Change to your sensor's I2C address
)
```

For MPU-6050 on ESP32-C3 (mpu6050.go):
```go
const (
    I2C_SDA = machine.GPIO4   // SDA pin
    I2C_SCL = machine.GPIO5   // SCL pin
    MPU6050_ADDR = 0x68       // I2C address
)
```

For different I2C sensors:
1. Update the I2C address constant
2. Modify register addresses if your sensor uses different registers
3. Adjust the scaling factor (currently 2048 LSB/g for ICM-42670-P, 16384 LSB/g for MPU-6050)
4. Refer to your sensor's datasheet for register map and configuration

### ADC Sensors

To use different ADC pins in joystick.go, edit the pin configuration:

```go
const (
    JOYSTICK_X_PIN = machine.ADC4 // Change to your X-axis pin
    JOYSTICK_Y_PIN = machine.ADC6 // Change to your Y-axis pin
)
```

Available ADC pins vary by board:
- **ESP32-S3**: ADC1-ADC20 (GPIO1-20) - widest selection
  - GPIO15/16 used by XTAL_32P/XTAL_32N, return stuck values ~30400
  - GPIO4 (ADC1_CH0) and GPIO6 (ADC1_CH2) recommended for joystick
- **ESP32-C3**: ADC0-ADC4 (GPIO0-4), ADC5 (GPIO5, avoid with WiFi)
- **ESP32**: ADC1_CH0-ADC1_CH7, ADC2_CH0-ADC2_CH11

For ESP32-S3 dual-axis joystick:
- X-axis: machine.ADC4 (GPIO4)
- Y-axis: machine.ADC6 (GPIO6)

Common ADC sensors:
- **Potentiometers**: 0-3.3V output
- **TMP36 temperature**: (mV - 500) / 10 = Celsius
- **Photoresistor**: Use voltage divider circuit
- **Joystick**: Two potentiometers (X and Y axes)

## Related Article

https://developer.espressif.com/workshops/tinygo/assignment-4/
