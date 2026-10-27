<!--
SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
SPDX-License-Identifier: Apache-2.0
-->

# ESP32-S31 Security Workshop Examples

Controlled, standalone ESP-IDF v6.1 projects for the ESP32-S31 security
workshop. Each assignment is a complete snapshot: participants can open and
build any folder without first applying changes from an earlier assignment.

## Projects

| Folder | Security state | Runs on |
|--------|----------------|---------|
| `assignment-1` | TLS server authentication with the ESP certificate bundle | ESP32-S31 development board |
| `assignment-2` | Assignment 1 + HMAC-based NVS encryption | Browser or native emulator |
| `assignment-3` | Assignment 2 code + Secure Boot V2; NVS encryption intentionally off | Browser or native emulator |
| `assignment-4` | Secure Boot V2 + release flash encryption + flash-encryption-backed NVS | Browser or native emulator |
| `assignment-5` | Assignment 4 + signed HTTPS OTA and app rollback | Native emulator |

Assignments 2–5 use the public emulator Wi-Fi credentials
`myssid` / `mypassword`. They are not real secrets.

## Safety

Do not flash Assignments 2–5 to a workshop board. Their first boot programs
security eFuses. On real hardware, those changes are permanent.

Private keys, generated certificates, build output, merged images, and emulator
state are excluded by `.gitignore`. The scripts never generate production
keys; all generated material is for this lab only.

## Prerequisites

Load an ESP-IDF v6.1 environment, then verify it:

```bash
scripts/check-environment.sh
```

The scripts reject another ESP-IDF version. Assignment 5 also needs `openssl`.

## Prepare and build

Build any assignment by number:

```bash
scripts/build-assignment.sh 1
scripts/build-assignment.sh 2
scripts/build-assignment.sh 3
scripts/build-assignment.sh 4
scripts/build-assignment.sh 5
```

The build script:

1. checks the ESP-IDF environment;
2. generates a local development signing key for Assignments 3–5 if missing;
3. generates Assignment 5's matching OTA certificate and private key if
   missing;
4. sets the target to `esp32s31` on a clean project; and
5. runs `idf.py build`.

To prepare keys without building:

```bash
scripts/prepare-assignment.sh 5
```

## Create emulator images

Assignments 2–5 share a fixed 4 MB partition layout. Build and merge one into
a complete flash image with:

```bash
scripts/merge-emulator-image.sh 2
```

The output is `assignment-N/build/merged-assignment-N.bin`. For Assignments
2–4, select ESP32-S31 in the browser emulator and load that file.

## Native emulator

Install the native emulator into the ignored `.tools` directory:

```bash
scripts/install-esp-emu.sh
```

The installer pins `esp-emu` to version 0.51.0 and downloads its eFuse helper
from a fixed source revision.

Run an assignment with persistent flash and eFuse files:

```bash
scripts/run-native-emulator.sh 5
```

State is kept under `.state/assignment-N`. The script creates a plaintext flash
copy only when no saved flash exists, preventing an encrypted device state from
being accidentally replaced by a new plaintext image.

## Assignment 5 OTA server

After building an update, publish its signed application image:

```bash
cp assignment-5/build/s31_security_assignment_5.bin \
   assignment-5/ota_server/firmware.bin
scripts/serve-ota.sh
```

The HTTPS server binds only to `127.0.0.1:8070`. Native `esp-emu --net user`
maps the device's `192.168.4.1` to that loopback address.

## Source control policy

Commit source, defaults, partition tables, READMEs, and scripts. Never commit:

- `secure_boot_signing_key.pem`
- `wrong_key.pem`
- `server_certs/ca_key.pem`
- generated Assignment 5 certificate pairs
- `sdkconfig`, build output, binaries, or emulator state
- real Wi-Fi, cloud, or device credentials

No Git repository is initialized here. Review these files, choose the remote
and license policy, then initialize and commit them in the destination
repository.

