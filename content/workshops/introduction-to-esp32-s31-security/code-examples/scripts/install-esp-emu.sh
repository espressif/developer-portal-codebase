#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
tools_dir="${repo_dir}/.tools"
bin_dir="${tools_dir}/bin"
esp_emu_version="0.51.0"
esp_emu_source_revision="61d71cef2b1f13a936173c6d282a52c093137d6a"

mkdir -p "${bin_dir}"

curl -fsSL \
    "https://raw.githubusercontent.com/espressif/esp-emulator/${esp_emu_source_revision}/install.sh" |
    sh -s -- --version "${esp_emu_version}" --bin-dir "${bin_dir}"

curl -fsSL \
    "https://raw.githubusercontent.com/espressif/esp-emulator/${esp_emu_source_revision}/tools/make-efuse.py" \
    -o "${tools_dir}/make-efuse.py"

echo "Installed esp-emu ${esp_emu_version} in ${bin_dir}"
echo "Downloaded make-efuse.py at ${esp_emu_source_revision} to ${tools_dir}"

