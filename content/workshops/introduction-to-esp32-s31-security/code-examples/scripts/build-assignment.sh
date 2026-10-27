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

"${script_dir}/prepare-assignment.sh" "${assignment_number}"

cd "${project_dir}"
if [[ ! -f sdkconfig ]] || ! grep -q '^CONFIG_IDF_TARGET="esp32s31"$' sdkconfig; then
    idf.py --preview set-target esp32s31
fi
idf.py build

echo "Assignment ${assignment_number} built successfully."

