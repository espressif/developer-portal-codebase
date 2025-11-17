# Assignment 1.3 -- Solution

This repository contains the solution code for Assignment 1.3. The goal of this assignment is to practice using multiple `sdkconfig` configurations to manage different build variants (debug and production) of the same ESP-IDF project.

## Objective

The assignment demonstrates how to:

* Create separate configuration files for production and debugging
* Control logging behavior using `sdkconfig`
* Use profile files to streamline the build process with `idf.py`

While the functional behavior of the application remains unchanged, this task introduces the concept of configuration-based project versioning using standard ESP-IDF tooling.

---

## Project Structure

The folder layout after completing this assignment is as follows:

```
.
|-- main
|   |-- CMakeLists.txt
|   |-- app_main.c
|   `-- idf_component.yml
|-- profiles
|   |-- debug
|   `-- prod
|-- sdkconfig
|-- sdkconfig.debug
|-- sdkconfig.defaults
|-- sdkconfig.old
`-- sdkconfig.prod
```

### Key Files

* `sdkconfig.defaults`: Base configuration, setting the ESP32-C3 target
* `sdkconfig.prod`: Suppresses both bootloader and application logs
* `sdkconfig.debug`: Enables default logging (e.g. `INFO` level)
* `profiles/prod`: Preconfigured build arguments for the production profile
* `profiles/debug`: Preconfigured build arguments for the debug profile

---

## How to Build

You can build and flash either version of the project using the corresponding profile:

### Build the Production Version

```bash
idf.py @profiles/prod build
idf.py @profiles/prod -p <YOUR_PORT> flash monitor
```

### Build the Debug Version

```bash
idf.py @profiles/debug build
idf.py @profiles/debug -p <YOUR_PORT> flash monitor
```

Make sure the correct port is selected in place of `<YOUR_PORT>`.

---

## Notes

* The `save-defconfig` tool was used to generate minimal config diffs for `sdkconfig.prod` and `sdkconfig.debug`.
* This assignment focuses only on modifying configuration files. No application logic changes were made.
* These configuration files can be extended with additional settings as needed in future assignments or projects.

