## Assignment 1.1 -- Solution

This repository contains the solution to an ESP-IDF assignment focused on implementing an `alarm` component. The aim is to encapsulate the logic for periodic alarm triggering in a reusable and configurable module.

---

## Overview

The assignment requires the creation of an `alarm` component, refactoring the original application code to use this new module, and making certain internal thresholds configurable via `menuconfig`.

The alarm system simulates an event that has a configurable probability of being triggered at a given interval.

---

## Assignment Objectives

The assignment was completed in the following steps:

1. Verified that the base example runs correctly on the target hardware.
2. Created a new `alarm` component.
3. Moved existing alarm logic into the component.
4. Replaced hardcoded configuration values with user-configurable `menuconfig` options.

---

## alarm Component API

The `alarm` component exposes the following functions:

```c
alarm_t *alarm_create(void);
bool is_alarm_set(alarm_t *alarm);
void alarm_delete(alarm_t *alarm);
```

* `alarm_create()`: Initializes internal state.
* `is_alarm_set()`: Returns `true` if the alarm is active based on random evaluation and time interval.
* `alarm_delete()`: Frees allocated memory.

---

## Configuration Parameters

The component includes a `Kconfig` file, allowing configuration via `menuconfig`:

| Parameter                   | Default | Description                                        |
| --------------------------- | ------- | -------------------------------------------------- |
| `ALARM_THRESHOLD_PERCENT`   | 2       | Percentage chance the alarm will be triggered      |
| `ALARM_REFRESH_INTERVAL_MS` | 1000    | Time interval (ms) between alarm state evaluations |

These parameters replace previously hardcoded `#define` constants.

---

## Component Implementation

The component is structured as follows:

```
components/alarm/
├── alarm.c                   // Component implementation
├── include/
│   └── alarm.h               // Public header file
├── Kconfig                   // Configuration options
├── CMakeLists.txt            // Component build file
```

### `CMakeLists.txt` Configuration

The component registers a dependency on `esp_timer`, which is required for time tracking:

```cmake
idf_component_register(SRCS "alarm.c"
                       REQUIRES esp_timer
                       INCLUDE_DIRS "include")
```

---

## Example Behavior

The `is_alarm_set()` function checks if a time interval (configurable via `CONFIG_ALARM_REFRESH_INTERVAL_MS`) has passed since the last evaluation. If so, a new random value is generated and compared to `CONFIG_ALARM_THRESHOLD_PERCENT` to determine whether the alarm should be set.

The state is updated only once per interval, reducing redundant random evaluations and ensuring consistent behavior.

---

## Usage Example

```c
#include "alarm.h"

void app_main(void) {
    alarm_t *alarm = alarm_create();
    if (!alarm) {
        // Handle error
    }

    while (true) {
        if (is_alarm_set(alarm)) {
            // Take action
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Polling interval
    }

    alarm_delete(alarm);
}
```

---

## Purpose

This repository serves as a reference implementation for modularizing simple logic into an ESP-IDF component. It demonstrates how to encapsulate functionality, expose a clean API, and provide configuration flexibility via `menuconfig`.
