Here's a clean and professional `README.md` for your repository:

# Assignment 2.1 -- Solution

This repository contains the solution for **Assignment 2.1** of the ESB25 Workshop series. The goal is to decouple temperature measurement and alarm publishing logic using the ESP-IDF default event loop.

## Objective

In this assignment, we refactor the existing application to use event-based architecture. Instead of directly invoking functions on timers, we post custom events to the default loop, which dispatches them to registered handlers. This improves modularity and prepares the codebase for handling multiple event sources.

## Key Concepts

* Use of `esp_event_loop_create_default` to manage application-level events.
* Definition of custom event bases and event IDs for temperature and alarm handling.
* Timer-driven event posting via `esp_timer_create` and `esp_timer_start_periodic`.
* Registration of handler functions with `esp_event_handler_register`.
* Modular separation between measurement, decision-making, and cloud publishing logic.

## Features

* Posts temperature readings every 5 seconds.
* Checks the alarm condition every 200 milliseconds.
* Publishes events to the cloud using an abstract `cloud_manager` module.
* Maintains system responsiveness through an event-driven main loop.

## File Structure

* `app_main.c`: Main application logic, including event and timer setup.
* `cloud_manager.*`: Abstracted interface for MQTT/cloud interactions.
* `temperature_sensor.*`: Temperature reading module.
* `alarm.*`: Alarm condition checker.

## How It Works

1. Two timers periodically trigger events:

   * Temperature event every 5 seconds
   * Alarm check every 200 milliseconds
2. These events are posted to the default event loop.
3. Registered handlers process the events:

   * Temperature handler reads the sensor and publishes the value.
   * Alarm handler checks for an active alarm and notifies the cloud if triggered.

## Notes

* The event loop abstraction is especially useful when more components (e.g., GPIO interrupts, network events) are added.
* This assignment lays the groundwork for more advanced event-driven designs, as explored in future exercises.

## Next Steps

For an extension of this project with GPIO-triggered events, continue with [Assignment 2.2](https://your-link-to-assignment-2-2).
