# ESP32-S31 Security Workshop — Assignment 1

This standalone ESP-IDF v6.1 project runs the stock `https_mbedtls` flow with
the certificate bundle, 4 MB flash, and the built-in large single-factory-app
partition layout.

## Configure and build

Use ESP-IDF v6.1:

```bash
idf.py --preview set-target esp32s31
idf.py menuconfig
idf.py build
```

In **Example Connection Configuration**, select Wi-Fi and enter the SSID and
password for your access point. Real credentials are intentionally not stored
in this project.

To run on a development board:

```bash
idf.py -p PORT flash monitor
```

## Expected result

The device obtains an IP address, validates the certificate for
`www.howsmyssl.com`, reports the negotiated cipher suite, and prints the HTTPS
response. The build output should show that the app fits in the 1.5 MB factory
partition.
