idf_component_register(SRCS "temperature_sensor.c"
                    PRIV_REQUIRES "esp_driver_tsens" "esp_driver_i2c"
                    INCLUDE_DIRS "include")
