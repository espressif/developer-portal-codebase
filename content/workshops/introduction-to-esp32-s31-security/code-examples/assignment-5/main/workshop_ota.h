/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "esp_err.h"

void workshop_ota_confirm_running_app(void);
esp_err_t workshop_ota_check_and_update(void);
