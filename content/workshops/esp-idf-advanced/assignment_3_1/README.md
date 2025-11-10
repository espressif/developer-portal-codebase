# Assignment 3.1 -- Solution

This repository contains the solution for **Assignment 3.1**. The goal of this exercise is to analyze the memory footprint of your firmware and reduce its size by disabling unused features.

>[!NOTE]
> The only difference from the assignment 3.1 and the assignment 2.2 code is in the configuration. You can check the difference in `sdkconfig.defaults`

## Objective

You will evaluate the size of your compiled binary and identify space-consuming components. Through a series of configuration changes, you'll optimize the build by removing unnecessary logging, SSL certificates, and MQTT features, gaining valuable insight into embedded memory management.

## Tasks Overview

1. **Build and inspect** the original project to identify large sections.
2. **Disable unused features** via `menuconfig` to reduce binary size:

   * Logging output
   * Root certificate bundle
   * MQTT SSL/WebSocket support
3. **Rebuild** and compare the output to measure improvements.

## Results

Throughout the steps, you progressively reduced the binary footprint by:

* Disabling runtime logging
* Removing unused TLS certificates
* Stripping optional MQTT features

These optimizations not only free up flash and RAM but also improve load times and stability on memory-constrained devices.

## Summary

This assignment demonstrated how to:

* Inspect the binary size and memory sections
* Use `menuconfig` to fine-tune build configurations
* Optimize firmware by disabling unnecessary components

Understanding and controlling your binary size is crucial in embedded development, especially for resource-constrained environments.

