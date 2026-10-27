#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

required_commands=(idf.py python3)
for command_name in "${required_commands[@]}"; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Error: ${command_name} is not available in PATH." >&2
        echo "Load the ESP-IDF v6.1 environment before running this script." >&2
        exit 1
    fi
done

if [[ -z "${IDF_PATH:-}" ]]; then
    echo "Error: IDF_PATH is not set. Load the ESP-IDF v6.1 environment." >&2
    exit 1
fi

idf_version="$(idf.py --version)"
if [[ "${idf_version}" != *"v6.1"* ]]; then
    echo "Error: this workshop is pinned to ESP-IDF v6.1." >&2
    echo "Detected: ${idf_version}" >&2
    exit 1
fi

echo "Environment OK: ${idf_version}"
echo "IDF_PATH=${IDF_PATH}"

