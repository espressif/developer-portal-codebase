# Assignment 1.2 -- Solution

This repository contains the solution to an ESP-IDF assignment focused on creating a `cloud_manager` component. The goal of the assignment is to modularize the cloud connection logic—specifically MQTT connectivity and data publishing—into a separate component that can be reused and extended independently of the main application logic.

---

## Overview

The assignment involves refactoring Wi-Fi and MQTT-related functionality into a new component called `cloud_manager`. This component is designed to abstract the cloud connection and publishing logic, providing a clean interface to the main application.

A key benefit of this approach is that the connection method (e.g., switching from MQTT to HTTP or MQTTS) can be changed with minimal impact on the application.

---

## Assignment Requirements

The following functions were implemented in the `cloud_manager` component:

```c
cloud_manager_t *cloud_manager_create(void);
esp_err_t cloud_manager_connect(cloud_manager_t *manager);
esp_err_t cloud_manager_disconnect(cloud_manager_t *manager);
esp_err_t cloud_manager_send_temperature(cloud_manager_t *manager, float temp);
esp_err_t cloud_manager_send_alarm(cloud_manager_t *manager);
void cloud_manager_delete(cloud_manager_t *manager);
```

Additionally, the component supports runtime configuration of connection parameters using `menuconfig`.

---

## Configuration Parameters

The following parameters are configurable via `menuconfig` under the **Cloud MQTT Configuration** menu:

| Parameter             | Default Value                | Description                         |
| --------------------- | ---------------------------- | ----------------------------------- |
| `BROKER_URL`          | `mqtt://test.mosquitto.org/` | MQTT broker URL                     |
| `TEMPERATURE_CHANNEL` | `/sensor/temperature`        | MQTT topic for temperature readings |
| `ALARM_CHANNEL`       | `/sensor/alarm`              | MQTT topic for alarm messages       |

---

## Usage Example

In `app_main.c`, the component is used as follows:

```c
#include "cloud_manager.h"

void app_main(void) {
    cloud_manager_t *cloud = cloud_manager_create();
    ESP_ERROR_CHECK(cloud_manager_connect(cloud));

    // Publish a temperature reading
    cloud_manager_send_temperature(cloud, 23.5f);

    // Send an alarm message
    cloud_manager_send_alarm(cloud);

    cloud_manager_disconnect(cloud);
    cloud_manager_delete(cloud);
}
```

---

## Component Structure

The component consists of the following files:

```
cloud_manager/
├── cloud_manager.h       // Public API declarations
├── cloud_manager.c       // Component implementation
├── Kconfig               // Configuration options for menuconfig
├── CMakeLists.txt        // Component build file
```

---

## Dependencies

This component relies on the following ESP-IDF components, which are specified in `CMakeLists.txt` using `PRIV_REQUIRES`:

* `mqtt`
* `nvs_flash`
* `esp_netif`
* `protocol_examples_common`

These dependencies handle MQTT communication, networking, and example Wi-Fi initialization routines.

---

## Implementation Notes

* `cloud_manager_create()` initializes the component structure and configures the MQTT client using values from `menuconfig`.
* `cloud_manager_connect()` initializes the network stack, connects to Wi-Fi, and starts the MQTT client.
* Data publishing functions construct and send payloads to the specified topics.
* All methods perform basic validation on input pointers and return appropriate error codes.

---

## Purpose

This repository is intended as a solution to the described assignment. It demonstrates correct use of ESP-IDF components to encapsulate cloud connectivity within a reusable and configurable module.


