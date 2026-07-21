#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "paho-mqtt>=2.0",
# ]
# ///
"""Publish duty-cycle values to the MQTT broker.

By default, publishes a random value (0.1 – 0.9) every 5 s.
Use --set to publish a single specific duty-cycle value and exit.
"""

import argparse
import random
import time

import paho.mqtt.client as mqtt

BROKER = "broker.freemqtt.com"
PORT = 1883
USERNAME = "freemqtt"
PASSWORD = "public"
TOPIC = "c845041f/light"
INTERVAL = 5  # seconds


def on_connect(client: mqtt.Client, _userdata, _flags, rc, _properties=None):
    if rc == 0:
        print(f"Connected to {BROKER}:{PORT}")
    else:
        print(f"Connection failed with code {rc}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--set",
        type=float,
        metavar="VALUE",
        help="Publish a single duty-cycle value (0.0 – 1.0) and exit.",
    )
    args = parser.parse_args()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect

    print(f"Connecting to {BROKER}:{PORT} ...")
    client.connect(BROKER, PORT, keepalive=60)
    client.loop_start()

    try:
        if args.set is not None:
            value = round(args.set, 2)
            client.publish(TOPIC, str(value), qos=1)
            print(f"Published {value} to {TOPIC}")
        else:
            while True:
                value = round(random.uniform(0.1, 0.9), 2)
                client.publish(TOPIC, str(value), qos=1)
                print(f"Published {value} to {TOPIC}")
                time.sleep(INTERVAL)
    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
