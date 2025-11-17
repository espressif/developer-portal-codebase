#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct temperature_sensor temperature_sensor_t;

temperature_sensor_t *temperature_sensor_create(void);
void temperature_sensor_delete(temperature_sensor_t *sensor);
esp_err_t temperature_sensor_read_celsius(temperature_sensor_t *sensor, float *temperature);

#ifdef __cplusplus
}
#endif
