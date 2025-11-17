# ESP-IDF MQTT Example with Temperature Sensors

This example project demonstrates using MQTT over TCP to publish and subscribe to topics with the ESP32 platform. It supports reading temperature from either the ESP32’s internal temperature sensor or an external SHTC3 sensor, based on a configuration flag.

## Features

- Wi-Fi or Ethernet connectivity (selectable via `menuconfig`)
- MQTT client for publishing/subscribing to topics
- Configurable sensor mode: internal or external (SHTC3 via I2C)
- Periodic temperature logging
- Full MQTT event handling with diagnostic logging

---

## Sensor Support

You can choose between two sensor sources at compile time:

- **Internal sensor**: Uses the ESP32’s onboard temperature sensor
- **SHTC3 external sensor**: Connects via I2C and provides more accurate readings

Selection is done through the `menuconfig` system. The default is the internal sensor.

---

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/your-username/esp-idf-mqtt-example.git
cd esp-idf-mqtt-example
````

### 2. Open with Visual Studio Code

Ensure you have the [Espressif VSCode extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) installed and set up correctly.

* Open the folder in VSCode
* Use the **ESP-IDF: Set Espressif Device Target** command to select your target (e.g., `esp32`)
* Run **ESP-IDF: Configure project** to open the menuconfig interface

### 3. Configure the project

In the menuconfig interface:

* **Example Configuration**

  * Set **Wi-Fi SSID and Password**
  * Set **MQTT Broker URL** (e.g., `mqtt://broker.hivemq.com`)
  * Select **Sensor Type** (internal or SHTC3)
* Optionally, adjust logging verbosity or MQTT settings under related sections

### 4. Build, Flash and Monitor

Use the VSCode Command Palette:

* **ESP-IDF: Build Project**
* **ESP-IDF: Flash (UART)**
* **ESP-IDF: Monitor**

---

## MQTT Topics

| Topic         | Direction | QoS | Description                   |
| ------------- | --------- | --- | ----------------------------- |
| `/topic/qos1` | Publish   | 1   | Sends test data on connection |
| `/topic/qos0` | Publish   | 0   | Sends data after subscription |
| `/topic/qos0` | Subscribe | 0   | Listens for incoming messages |
| `/topic/qos1` | Subscribe | 1   | Subscribes then unsubscribes  |

---

## Example Output

```
[APP] Free memory: 320000 bytes
Temperature: 27.13 °C
MQTT_EVENT_CONNECTED
sent publish successful, msg_id=1234
MQTT_EVENT_DATA
TOPIC=/topic/qos0
DATA=hello from broker
```

---

## Notes

* The SHTC3 sensor should be connected to the default I2C pins (customizable via menuconfig).
* Temperature readings are taken periodically at 1-second intervals during startup.
* MQTT broker URI must use the format `mqtt://<hostname>` or `mqtt://<ip>`.

---

## License

This example is released into the public domain or under the CC0 license, at your option.
