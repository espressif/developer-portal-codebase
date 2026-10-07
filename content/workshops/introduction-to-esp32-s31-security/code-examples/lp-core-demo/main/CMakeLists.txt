set(app_sources "ulp-demo.c")

idf_component_register(SRCS ${app_sources}
                       REQUIRES ulp esp_wifi esp_event esp_netif nvs_flash
                       INCLUDE_DIRS ".")

set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_sources "ulp/main.c")
set(ulp_exp_dep_srcs ${app_sources})

ulp_embed_binary(${ulp_app_name} "${ulp_sources}" "${ulp_exp_dep_srcs}")
