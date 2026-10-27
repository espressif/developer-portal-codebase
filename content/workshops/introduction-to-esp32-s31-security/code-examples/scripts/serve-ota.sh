#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
project_dir="${repo_dir}/assignment-5"
server_dir="${project_dir}/ota_server"
cert_file="${project_dir}/server_certs/ca_cert.pem"
key_file="${project_dir}/server_certs/ca_key.pem"
firmware_file="${server_dir}/firmware.bin"

for required_file in "${cert_file}" "${key_file}" "${firmware_file}"; do
    if [[ ! -f "${required_file}" ]]; then
        echo "Error: missing ${required_file}" >&2
        echo "Prepare Assignment 5 and publish firmware.bin before starting the server." >&2
        exit 1
    fi
done

cd "${server_dir}"
exec python3 ../ota_server.py "${cert_file}" "${key_file}"

