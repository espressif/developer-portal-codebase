# Button Component Demo

This example demonstrates how to use the ESP-IDF Button component to handle various button events on ESP32 devices.

## Overview

The Button component provides a simple and efficient way to handle button inputs with support for multiple event types:
- Single click
- Double click
- Long press
- Press repeat

## Hardware Requirements

- ESP32 development board (ESP32, ESP32-C3, ESP32-S2, ESP32-S3, etc.)
- Push button connected to GPIO 9
- Pull-up resistor (internal pull-up is enabled by default)

## Hardware Configuration

The button should be connected as follows:
- **GPIO Pin**: GPIO 9
- **Active Level**: Low (0) - button press pulls the pin to ground
- **Internal Pull-up**: Enabled

## Dependencies

This project uses the following ESP-IDF component:
- **button** component (version ^4.0.0)
- **ESP-IDF** version 4.4 or higher

Dependencies are managed through the ESP Component Registry via `idf_component.yml`.

## Button Configuration

The application configures the button with the following parameters:
- **Long press time**: 5000 ms (5 seconds)
- **Short press time**: 200 ms

## Event Callbacks

The application registers callbacks for the following button events:

1. **Single Click** - Triggered when the button is pressed and released once
2. **Double Click** - Triggered when the button is pressed and released twice quickly
3. **Long Press** - Triggered when the button is held for the configured long press time
4. **Press Repeat** - Triggered repeatedly while the button is held down

## Building and Flashing

### Configure the project

```bash
idf.py set-target <target>
```

Replace `<target>` with your chip (e.g., `esp32`, `esp32c3`, `esp32s3`).

### Build the project

```bash
idf.py build
```

### Flash to device

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with your device's serial port (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).

## Expected Output

When you interact with the button, you should see log messages like:

```
I (12345) BUTTON: Button single click!
I (12678) BUTTON: Button double click!
I (15234) BUTTON: Button long press!
I (16000) BUTTON: Button press repeat!
```

## Customization

You can customize the button behavior by modifying:
- `BUTTON_IO_NUM` - Change the GPIO pin number
- `BUTTON_ACTIVE_LEVEL` - Change between active low (0) or active high (1)
- `btn_cfg.long_press_time` - Adjust long press duration
- `btn_cfg.short_press_time` - Adjust short press duration

## Troubleshooting

- If button presses are not detected, verify the GPIO pin connection
- If you experience button bouncing, adjust the `short_press_time` parameter
- Ensure your target chip supports the configured GPIO pin

## License

This example is provided under the Apache License 2.0.
