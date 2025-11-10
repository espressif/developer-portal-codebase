# Assignment 2.2 -- Solution

This repository contains the solution for **Assignment 2.2** of the ESB25 workshop series. Building on the previous assignment, this exercise demonstrates how to integrate a GPIO interrupt as a new event source in an event-driven system.

## Objective

Enhance the existing event loop by triggering an **alarm event** when **GPIO9** receives a falling edge (i.e., button press). This adds asynchronous input handling to the system using the ESP-IDF interrupt and event loop APIs.

## Key Concepts

* GPIO input configuration with falling edge interrupt (`GPIO_INTR_NEGEDGE`)
* ISR handler that posts events to the loop using `esp_event_isr_post`
* Differentiating alarm event sources with custom event IDs:

  * `ALARM_EVENT_CHECK`: periodic check from a timer
  * `ALARM_EVENT_BUTTON`: triggered via GPIO interrupt
* Seamless integration into the existing event loop structure

## Features

* Posts temperature readings every 5 seconds
* Periodically checks the alarm condition every 200 ms
* Triggers an alarm immediately when a button on GPIO9 is pressed
* Unified event-driven architecture using ESP-IDF’s default event loop

## Implementation Overview

* **GPIO9** is configured as a falling-edge interrupt input with pull-up enabled.
* When the button is pressed, the ISR posts `ALARM_EVENT_BUTTON` to the loop.
* A dedicated handler responds to this event and publishes an alarm message.
* Other logic (temperature and periodic alarm checks) continues to run on timers as before.

## Notes

* `esp_event_isr_post` is specifically designed for posting events from ISR context.
* This assignment reinforces the advantage of decoupling input sources using a unified event loop mechanism.

