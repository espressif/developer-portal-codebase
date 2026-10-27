/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "esp_err.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_creds_provision_from_menuconfig_if_missing(void);
esp_err_t wifi_creds_load(wifi_config_t *cfg);

#ifdef __cplusplus
}
#endif
