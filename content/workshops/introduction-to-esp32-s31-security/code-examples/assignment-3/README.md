# ESP32-S31 Security Workshop — Assignment 3

This standalone ESP-IDF v6.1 project keeps the Assignment 2 partition layout,
NVS credential helpers, raw NVS dump, and HTTPS task. NVS encryption is
disabled, while Secure Boot V2 uses RSA-3072 build signing, permanently
disables ROM download mode, and logs both the Secure Boot state and the
`SECURE_BOOT_DIGEST0` key block.

> **Warning:** Physically disconnect the development board by unplugging its
> USB cable before continuing. Run this project only in the ESP32 emulator;
> first boot permanently enables Secure Boot, writes eFuses, and disables
> recovery and debug interfaces.

## Create the local development key

The required key is deliberately absent. Generate it locally in this directory:

```bash
python -m espsecure generate-signing-key --version 2 --scheme rsa3072 secure_boot_signing_key.pem
```

Never commit this private key, and never use it as a production root of trust.
The filename is covered by `.gitignore`.

## Configure, build, and merge

From the repository root, use ESP-IDF v6.1:

```bash
scripts/merge-emulator-image.sh 3
```

Load `assignment-3/build/merged-assignment-3.bin` in the browser emulator and
select ESP32-S31.

## Expected result

On first boot, the bootloader verifies the signatures, burns the public-key
digest into `BLOCK_KEY0`, enables Secure Boot V2, and permanently disables ROM
download mode. The app reports `Secure Boot enabled: yes` and
`SECURE_BOOT_DIGEST0 in BLOCK_KEY0`. The raw NVS dump exposes the public test
password because NVS encryption is intentionally disabled in this assignment.
The HTTPS connection fails because the browser emulator has no internet access.
