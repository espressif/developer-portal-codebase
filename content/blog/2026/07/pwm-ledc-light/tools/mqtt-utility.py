#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "paho-mqtt>=2.0",
# ]
# ///
"""Publish a random duty-cycle value (0.1 – 0.9) to the MQTT broker every 5 s."""

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
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect

    print(f"Connecting to {BROKER}:{PORT} ...")
    client.connect(BROKER, PORT, keepalive=60)
    client.loop_start()

    try:
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
