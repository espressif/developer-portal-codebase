#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"

if [[ $# -ne 1 || ! "$1" =~ ^[2-5]$ ]]; then
    echo "Usage: $0 ASSIGNMENT_NUMBER (2-5)" >&2
    exit 2
fi

assignment_number="$1"
esp_emu="${repo_dir}/.tools/bin/esp-emu"
make_efuse="${repo_dir}/.tools/make-efuse.py"
state_dir="${repo_dir}/.state/assignment-${assignment_number}"
efuse_file="${state_dir}/device.efuse"
flash_file="${state_dir}/device-flash.bin"
merged_file="${repo_dir}/assignment-${assignment_number}/build/merged-assignment-${assignment_number}.bin"

if [[ ! -x "${esp_emu}" || ! -f "${make_efuse}" ]]; then
    echo "Error: native emulator tools are missing." >&2
    echo "Run scripts/install-esp-emu.sh first." >&2
    exit 1
fi

"${script_dir}/merge-emulator-image.sh" "${assignment_number}"
mkdir -p "${state_dir}"

if [[ ! -f "${efuse_file}" ]]; then
    python3 "${make_efuse}" \
        --chip esp32s31 \
        --chip-rev 0.0 \
        -o "${efuse_file}"
fi

if [[ ! -f "${flash_file}" ]]; then
    cp "${merged_file}" "${flash_file}"
fi

echo "Using persistent emulator state in ${state_dir}"
echo "Delete that directory only when you intentionally want a blank emulated chip."

exec "${esp_emu}" \
    --chip esp32s31 \
    --firmware "${flash_file}" \
    --efuse "${efuse_file}" \
    --save-state \
    --net user

