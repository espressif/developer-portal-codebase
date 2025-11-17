# Assignment 3.2 -- Solution 

This repository contains the solution code for **Assignment 3.2** of the ESP-IDF Advanced Workshop. The objective of this exercise is to simulate a runtime crash, collect a core dump, analyze it, and fix the issue causing the crash.

>[!NOTE]
> After fixing the bugs, the code becomes identical to the assignment 2.2 code. 

## Overview

In this assignment, you neede to:

1. Enable core dump support in your ESP-IDF project.
2. Build and flash the application to your ESP32-C3 device.
3. Trigger a runtime fault and capture a core dump.
4. Analyze the resulting crash data to understand the root cause.
5. Fix the underlying bug(s) and verify the solution.

This hands-on activity is designed to give you practical experience with post-mortem debugging using ESP-IDF's core dump tools.

## Prerequisites

* ESP-IDF environment properly configured (version 5.4.2 or similar)
* ESP32-C3 hardware
* Base project: [assignment\_3\_2\_base](https://github.com/FBEZ-docs-and-templates/devrel-advanced-workshop-code/tree/main/assignment_3_2_base)

## Steps

### 1. Enable Core Dump to Flash

Launch the menuconfig tool:

```sh
idf.py menuconfig
```

Navigate to:

```
Component config  --->
  ESP System Settings  --->
    Core dump destination (UART / Flash / None)  ---> Flash
```

This setting ensures that the crash data will be written to flash memory when a fault occurs.

### 2. Build, Flash, and Monitor

Build and flash the application:

```sh
idf.py build flash monitor
```

After the device starts, wait for the crash to occur. When it does, halt the monitor using:

```sh
Ctrl + ]
```

### 3. Extract Core Dump

Open a new ESP-IDF terminal and run:

```sh
idf.py coredump-info > coredump.txt
```

This command extracts and decodes the core dump from the flash memory and saves it to `coredump.txt`.

### 4. Analyze the Crash

Open the generated `coredump.txt` file. Look for:

* The crashing function and line (e.g., `is_alarm_set (alarm=0x0)`).
* Stack trace and thread information.
* Suspect NULL pointer dereference or out-of-bounds access.

Use the call stack and register dump to identify where the crash occurred and why.

### 5. Fix the Bug(s)

Based on your analysis, modify the faulty code to:

* Validate pointers before dereferencing
* Ensure event data or handler arguments are not NULL
* Apply appropriate error checks and fail-safes

After making the changes, recompile, flash, and re-run the program. Confirm that the crash no longer occurs.

---

## Additional Notes

* Core dumps are an invaluable tool for diagnosing faults in real-world embedded applications.
* Always validate inputs to system components and external handlers.
* Familiarity with GDB-like analysis of stack traces will be helpful in more advanced debugging scenarios.

