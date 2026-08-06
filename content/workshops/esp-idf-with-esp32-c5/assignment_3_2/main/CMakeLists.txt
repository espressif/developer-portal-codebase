idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_https_ota esp_http_client nvs_flash bootloader_support app_update)