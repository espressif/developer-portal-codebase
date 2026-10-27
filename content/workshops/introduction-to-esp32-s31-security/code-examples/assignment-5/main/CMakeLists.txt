# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(
    SRCS
        "https_mbedtls_example_main.c"
        "wifi_creds.c"
        "workshop_wifi_connect.c"
        "workshop_ota.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        app_update
        bootloader_support
        efuse
        esp_app_format
        esp_event
        esp_http_client
        esp_https_ota
        esp_netif
        esp_partition
        esp_wifi
        mbedtls
        nvs_flash
        spi_flash
    EMBED_TXTFILES "${project_dir}/server_certs/ca_cert.pem"
)
