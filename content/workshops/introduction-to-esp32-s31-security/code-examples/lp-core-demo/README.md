# ESP32-S31 LP-Core Demo

This demo uses the ESP32-S31 LP core while the HP cores are in deep sleep.
On cold boot, the HP core connects to Wi-Fi, holds the connection for 10
seconds for power measurement, disconnects, keeps the HP domain running for
another 10 seconds, and then starts the LP core. An LP timer event represents
a simulated button press. The LP core counts the events in retained LP memory
and wakes the HP core after 30 pulses.

The LP core halts between timer events instead of using a delay loop, keeping
the low-power interval suitable for current measurement.

## Demo scenario

This application represents a battery-powered device that needs network
connectivity only during startup and must monitor events while consuming as
little power as possible:

1. The application boots on the HP core.
2. It connects to Wi-Fi and remains connected for 10 seconds.
3. Wi-Fi is disconnected and powered down.
4. After the HP-only measurement interval, the LP core is started and the HP
   core enters deep sleep.
5. The LP core counts events while the HP core is off. In a real application,
   these events could be pulses from a sensor, flow meter, rotary encoder,
   push button, or another digital source. This demo generates them internally
   with the LP timer.
6. After 30 events, the LP core wakes the HP core.
7. The HP core reads and processes the retained pulse count, reports the
   result, and returns to deep sleep.
8. The LP core continues monitoring and repeats the wake-process-sleep cycle.

Wi-Fi runs only after a cold boot; LP-core wakeups do not reconnect to the
network in this demo.

## Build and flash

This project requires **ESP-IDF v6.1**. ESP32-S31 is a preview target in this
release, so the `--preview` option is required.

Before building, set `DEMO_WIFI_SSID` and `DEMO_WIFI_PASSWORD` near the top
of `main/ulp-demo.c`.

> **Warning:** Plain-text credentials are used only for this controlled test.
> Never commit real credentials or use source-defined passwords in production.

### 1. Load the ESP-IDF v6.1 environment

Adjust the path to match your ESP-IDF installation:

```sh
cd ~/esp/v6.1/esp-idf
. ./export.sh
idf.py --version
```

The version command should report `ESP-IDF v6.1`.

### 2. Configure and build

From this project's root directory:

```sh
idf.py --preview set-target esp32s31
idf.py --preview build
```

### 3. Flash and monitor

Connect the ESP32-S31-Function-CoreBoard-1 through its USB-to-UART port and
replace `PORT` with the board's serial port:

```sh
idf.py --preview -p PORT flash monitor
```

Example on macOS:

```sh
idf.py --preview -p /dev/cu.usbserial-11420 flash monitor
```

Linux ports usually appear as `/dev/ttyUSB0` or `/dev/ttyACM0`; Windows uses
a name such as `COM5`. Press `Ctrl-]` to exit the serial monitor.

Expected output:

```text
hp_core: [HP stage 1/4] HP core booted
hp_core: [HP stage 2/4] Cold boot: initializing LP core
hp_core: Pulse interval=1000 ms, wake threshold=30 pulses
hp_core: [Wi-Fi stage 1/5] Initializing station
hp_core: [Wi-Fi stage 3/5] Connected, IP address: 192.0.2.1
hp_core: [Wi-Fi stage 4/5] Holding connection for 10 seconds
hp_core: [Wi-Fi stage 5/5] Disconnecting and powering down Wi-Fi
hp_core: [HP measurement] Keeping the HP domain running for 10 seconds
hp_core: [HP measurement] Complete; transitioning to LP-core operation
hp_core: LP-core firmware loaded into retained LP memory
hp_core: LP core started successfully
hp_core: [HP stage 3/4] Enabling LP-core wakeup source
hp_core: [HP stage 4/4] Entering deep sleep; LP core is now in control

hp_core: [HP stage 2/4] LP-core wakeup detected
hp_core: [LP stage 4/4] HP wake request completed
hp_core: LP report: pulses in cycle=30, total pulses=30, HP wakeups=1
```

The sequence repeats, with the total pulse and HP-wakeup counters increasing.
The Wi-Fi phase runs only after a cold boot, not after LP-core wakeups.
LP stages are retained and reported when the HP core wakes. The LP core does
not print while the HP core sleeps, because enabling an LP UART for every
pulse would alter the power measurement.

## Power-profiler markers

Machine-readable serial markers use the `power_profile` log tag and the
format `STAGE=<name>`. They identify application start, Wi-Fi connection and
hold, Wi-Fi shutdown, HP-only hold, LP-core startup, deep-sleep entry, LP
wakeup, and the reported pulse totals.

## Configuration

Run `idf.py menuconfig` and open **LP-Core Pulse Counter Demo** to change:

- HP-only measurement interval
- Simulated pulse interval
- Number of pulses required to wake the HP core

## Current measurement

On the ESP32-S31-Function-CoreBoard-1, remove the J5 jumper and connect an
ammeter or power profiler in series across J5. J5 measures the current drawn
by the ESP32-S31-WROOM-3 module rather than all peripherals on the board.

The trace should show a low-current LP/deep-sleep interval and a short
high-current pulse whenever the HP core wakes. Disconnecting unnecessary
debug connections and avoiding LP UART logging gives a more representative
measurement.

After a cold boot, the power trace should show Wi-Fi association, a 10-second
connected interval, Wi-Fi shutdown, a 10-second HP-only interval, and then
the lower-power LP/deep-sleep interval.
