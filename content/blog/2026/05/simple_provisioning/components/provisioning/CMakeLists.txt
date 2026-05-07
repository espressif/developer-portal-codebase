idf_component_register(SRCS "provisioning.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_netif esp_event nvs_flash network_provisioning)
