idf_component_register(SRCS "cloud_manager.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif protocol_examples_common
                    INCLUDE_DIRS "include")
