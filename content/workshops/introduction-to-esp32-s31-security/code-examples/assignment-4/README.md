<!--
SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
SPDX-License-Identifier: Apache-2.0
-->

# ESP32-S31 security workshop — Assignment 4

Standalone ESP-IDF v6.1 project demonstrating Secure Boot V2 (RSA-3072),
flash encryption in release mode, flash-encryption-backed NVS encryption,
Wi-Fi credentials loaded from encrypted NVS, raw-versus-decrypted partition
reads, a raw NVS dump, and an HTTPS client.

## Local prerequisite

Create a development-only signing key in this directory before configuring:

```sh
python -m espsecure generate-signing-key --version 2 --scheme rsa3072 secure_boot_signing_key.pem
```

The project references that local file but does not include it. Never commit or
reuse the key for production.

## Configure, build, and merge

Physically disconnect the development board by unplugging its USB cable before
continuing. From the repository root, use ESP-IDF v6.1:

```sh
scripts/merge-emulator-image.sh 4
```

Load `assignment-4/build/merged-assignment-4.bin` in the browser emulator and
select ESP32-S31.

Confirm that the bootloader ends below `0x10000`.

## Run and expected behavior

Run only on a fresh ESP32-S31 emulator. This firmware permanently enables
Secure Boot, release-mode flash encryption, disables ROM download mode, and
disables JTAG in the emulated eFuses. Do not flash it to workshop hardware.

The browser emulator uses the public test access point `myssid` /
`mypassword`; its lack of internet access makes the public HTTPS request fail,
which is expected. On first boot, expect Secure Boot and XTS key provisioning,
in-place flash encryption, NVS key generation, and a reset. After reset:

- `SECURE_BOOT_DIGEST0` and `XTS_AES_128_KEY` key-purpose logs appear.
- `esp_partition_read` matches the test string while `esp_flash_read` shows
  ciphertext.
- the raw NVS dump does not expose the SSID or password.
- Wi-Fi connects using credentials loaded from NVS.

Native `esp-emu --save-state` writes emulator state and encrypted flash files.
Keep those generated files outside source control and never substitute a new
plaintext merged image for an already encrypted saved flash image.
