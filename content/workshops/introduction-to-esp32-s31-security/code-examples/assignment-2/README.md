# ESP32-S31 Security Workshop — Assignment 2

This standalone ESP-IDF v6.1 project keeps the Assignment 1 HTTPS client and
adds HMAC-based encryption for the default NVS partition. On first boot it
stores the emulator's public `myssid` / `mypassword` credentials in the
`workshop_wifi` namespace, burns an HMAC key into key block 0, prints the HMAC
key purpose, and dumps the first 512 raw NVS bytes.

> **Warning:** Physically disconnect the development board by unplugging its
> USB cable before continuing. Run this project only in the ESP32 emulator;
> first boot permanently writes an eFuse key.

## Configure, build, and merge

From the repository root, use ESP-IDF v6.1:

```bash
scripts/merge-emulator-image.sh 2
```

Load `assignment-2/build/merged-assignment-2.bin` in the browser emulator and
select ESP32-S31.

## Expected result

First boot reports an `HMAC_UP` key in `BLOCK_KEY0`, stores and reloads the
credentials from NVS, prints an encrypted raw NVS prefix, and connects to the
emulator access point. The HTTPS connection then fails because the browser
emulator has no internet access. A reset reuses the existing eFuse key and NVS
contents.

The manifest path dependency supplies the workshop's
`CONFIG_EXAMPLE_WIFI_*` Kconfig inputs; the application uses its own Wi-Fi
connection helper at runtime.
