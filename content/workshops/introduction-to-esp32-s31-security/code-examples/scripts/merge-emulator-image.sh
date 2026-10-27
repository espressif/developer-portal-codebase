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
project_dir="${repo_dir}/assignment-${assignment_number}"
project_name="s31_security_assignment_${assignment_number}"
output_file="build/merged-assignment-${assignment_number}.bin"

"${script_dir}/build-assignment.sh" "${assignment_number}"

cd "${project_dir}"
python -m esptool --chip esp32s31 merge-bin \
    -o "${output_file}" \
    --flash-mode dio \
    --flash-size 4MB \
    0x2000 build/bootloader/bootloader.bin \
    0x10000 build/partition_table/partition-table.bin \
    0x17000 build/ota_data_initial.bin \
    0x20000 "build/${project_name}.bin"

echo "Created ${project_dir}/${output_file}"

