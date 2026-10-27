#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"

if [[ $# -ne 1 || ! "$1" =~ ^[1-5]$ ]]; then
    echo "Usage: $0 ASSIGNMENT_NUMBER" >&2
    exit 2
fi

assignment_number="$1"
project_dir="${repo_dir}/assignment-${assignment_number}"

"${script_dir}/check-environment.sh"

if (( assignment_number >= 3 )); then
    signing_key="${project_dir}/secure_boot_signing_key.pem"
    if [[ -e "${signing_key}" ]]; then
        echo "Keeping existing signing key: ${signing_key}"
    else
        (
            cd "${project_dir}"
            python -m espsecure generate-signing-key \
                --version 2 \
                --scheme rsa3072 \
                secure_boot_signing_key.pem
        )
        chmod 600 "${signing_key}"
        echo "Generated development-only signing key: ${signing_key}"
    fi
fi

if (( assignment_number == 5 )); then
    command -v openssl >/dev/null 2>&1 || {
        echo "Error: openssl is required for Assignment 5." >&2
        exit 1
    }

    cert_dir="${project_dir}/server_certs"
    cert_file="${cert_dir}/ca_cert.pem"
    key_file="${cert_dir}/ca_key.pem"

    if [[ -e "${cert_file}" && -e "${key_file}" ]]; then
        echo "Keeping existing OTA certificate and private key."
    elif [[ -e "${cert_file}" || -e "${key_file}" ]]; then
        echo "Error: only one OTA certificate file exists." >&2
        echo "Move the existing file aside, then rerun this script to create a matching pair." >&2
        exit 1
    else
        mkdir -p "${cert_dir}" "${project_dir}/ota_server"
        openssl req -x509 -newkey rsa:2048 -nodes -days 365 \
            -keyout "${key_file}" \
            -out "${cert_file}" \
            -subj "/CN=192.168.4.1" \
            -addext "subjectAltName=IP:192.168.4.1"
        chmod 600 "${key_file}"
        echo "Generated lab OTA certificate with SAN IP:192.168.4.1."
    fi
fi

echo "Assignment ${assignment_number} is prepared."

