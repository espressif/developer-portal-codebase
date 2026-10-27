# SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

idf_component_register(SRCS "https_mbedtls_example_main.c"
                    PRIV_REQUIRES esp_event nvs_flash esp_netif mbedtls
                    INCLUDE_DIRS ".")
