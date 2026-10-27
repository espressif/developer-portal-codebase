<!--
SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
SPDX-License-Identifier: Apache-2.0
-->

# ESP32-S31 security workshop — Assignment 5

Standalone ESP-IDF v6.1 project extending Assignment 4 with signed HTTPS OTA,
app rollback, and application version `1.0.0`. Anti-rollback and virtual eFuse
secure-version emulation remain disabled.

## Local key and certificate prerequisites

Create the development signing key locally:

```sh
python -m espsecure generate-signing-key --version 2 --scheme rsa3072 secure_boot_signing_key.pem
```

Create the lab server certificate before configuring:

```sh
mkdir -p server_certs ota_server
openssl req -x509 -newkey rsa:2048 -nodes -days 365 \
  -keyout server_certs/ca_key.pem -out server_certs/ca_cert.pem \
  -subj "/CN=192.168.4.1" -addext "subjectAltName=IP:192.168.4.1"
```

The firmware embeds only `server_certs/ca_cert.pem`. Its IP subject alternative
name must be `192.168.4.1`, matching the OTA URL. `ca_key.pem` is private server
key material: it remains on the host, is ignored by source control, and must
never be embedded or committed. The signing key and certificate are referenced
by the project but are intentionally absent.

## Configure, build, and merge

Physically disconnect the development board by unplugging its USB cable before
continuing. From the repository root, use ESP-IDF v6.1:

```sh
scripts/merge-emulator-image.sh 5
```

Confirm that the bootloader ends below `0x10000` and the app fits a `0x1C0000`
OTA slot.

To serve a signed update, copy the signed application binary into the generated
server directory, then start the loopback-only HTTPS server:

```sh
cp build/s31_security_assignment_5.bin ota_server/firmware.bin
cd ota_server
python3 ../ota_server.py ../server_certs/ca_cert.pem ../server_certs/ca_key.pem
```

## Run and expected behavior

Run only with native `esp-emu`, using `--net user --save-state`. Inside the
emulator, `192.168.4.1` reaches host loopback `127.0.0.1`; `ota_server.py`
listens only on `127.0.0.1:8070`.

From the repository root, run:

```sh
scripts/run-native-emulator.sh 5
```

This firmware permanently enables Secure Boot, release-mode flash encryption,
disables ROM download mode, and disables JTAG in emulated eFuses. Do not flash
it to workshop hardware. Preserve the emulator's flash and eFuse files together
and never overwrite encrypted saved flash with a plaintext merged image.

Expected behavior:

- version 1.0.0 boots from `ota_0` and checks
  `https://192.168.4.1:8070/firmware.bin`;
- HTTPS verifies the embedded lab CA, while Secure Boot verifies the downloaded
  application signature;
- a newly booted update is confirmed only after Wi-Fi connects;
- an unconfirmed or deliberately failed update rolls back;
- the self-test failure option is off by default.

The browser emulator cannot preserve state across image loads or reach the host
OTA server, so it is unsuitable for this assignment.
