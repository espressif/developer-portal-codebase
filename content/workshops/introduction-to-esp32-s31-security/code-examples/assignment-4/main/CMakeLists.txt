# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

idf_component_register(
    SRCS
        "https_mbedtls_example_main.c"
        "wifi_creds.c"
        "workshop_wifi_connect.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        bootloader_support
        efuse
        esp_event
        esp_netif
        esp_partition
        esp_wifi
        mbedtls
        nvs_flash
        spi_flash
)
