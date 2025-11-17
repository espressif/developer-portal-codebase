/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include "esp_log.h"
#include "temperature_sensor.h"


//#define USE_EXTERNAL_SENSOR

#ifdef USE_EXTERNAL_SENSOR
// === External (SHTC3) sensor implementation ===
#include "driver/i2c_master.h"
#include "shtc3.h"  // assumes this is available in your include path

typedef struct {
    i2c_master_dev_handle_t dev_handle;
} shtc3_ctx_t;

static esp_err_t shtc3_read_temperature(void *ctx, float *temperature)
{
    if (!ctx || !temperature) {
        return ESP_ERR_INVALID_ARG;
    }

    shtc3_ctx_t *sctx = (shtc3_ctx_t *)ctx;
    float temp = 0.0f, hum = 0.0f;
    esp_err_t err = shtc3_get_th(sctx->dev_handle, SHTC3_REG_T_CSD_NM, &temp, &hum);
    if (err == ESP_OK) {
        *temperature = temp;
    }
    return err;
}

static void shtc3_destroy(void *ctx)
{
    if (!ctx) return;
    shtc3_ctx_t *sctx = (shtc3_ctx_t *)ctx;
    shtc3_device_delete(sctx->dev_handle);
    free(sctx);
}

#else
// === Internal temperature sensor implementation ===
#include "driver/temperature_sensor.h"

typedef struct {
    temperature_sensor_handle_t handle;
} internal_sensor_ctx_t;

static esp_err_t internal_read_temperature(void *ctx, float *temperature)
{
    if (!ctx || !temperature) {
        return ESP_ERR_INVALID_ARG;
    }

    internal_sensor_ctx_t *sctx = (internal_sensor_ctx_t *)ctx;
    return temperature_sensor_get_celsius(sctx->handle, temperature);
}

static void internal_destroy(void *ctx)
{
    if (!ctx) return;
    internal_sensor_ctx_t *sctx = (internal_sensor_ctx_t *)ctx;
    temperature_sensor_disable(sctx->handle);
    temperature_sensor_uninstall(sctx->handle);
    free(sctx);
}
#endif

// === Common interface ===
struct temperature_sensor {
    void *ctx;
    esp_err_t (*read_temperature)(void *ctx, float *temperature);
    void (*destroy)(void *ctx);
};

temperature_sensor_t *temperature_sensor_create(void)
{
    void *ctx = NULL;
    temperature_sensor_t *sensor = calloc(1, sizeof(temperature_sensor_t));
    if (!sensor) return NULL;

#ifdef USE_EXTERNAL_SENSOR
    shtc3_ctx_t *sctx = calloc(1, sizeof(shtc3_ctx_t));
    if (!sctx) {
        free(sensor);
        return NULL;
    }

    // Assumes I2C bus handle is already created elsewhere
    extern i2c_master_bus_handle_t i2c_bus_handle;  // Define this in your main code
    sctx->dev_handle = shtc3_device_create(i2c_bus_handle, SHTC3_I2C_ADDR, 400000);
    if (!sctx->dev_handle) {
        free(sctx);
        free(sensor);
        return NULL;
    }

    ctx = sctx;
    sensor->read_temperature = shtc3_read_temperature;
    sensor->destroy = shtc3_destroy;
#else
    internal_sensor_ctx_t *sctx = calloc(1, sizeof(internal_sensor_ctx_t));
    if (!sctx) {
        free(sensor);
        return NULL;
    }

    temperature_sensor_config_t config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    esp_err_t err = temperature_sensor_install(&config, &sctx->handle);
    if (err != ESP_OK) {
        free(sctx);
        free(sensor);
        return NULL;
    }

    err = temperature_sensor_enable(sctx->handle);
    if (err != ESP_OK) {
        temperature_sensor_uninstall(sctx->handle);
        free(sctx);
        free(sensor);
        return NULL;
    }

    ctx = sctx;
    sensor->read_temperature = internal_read_temperature;
    sensor->destroy = internal_destroy;
#endif

    sensor->ctx = ctx;
    return sensor;
}

void temperature_sensor_delete(temperature_sensor_t *sensor)
{
    if (!sensor) return;
    if (sensor->destroy) {
        sensor->destroy(sensor->ctx);
    }
    free(sensor);
}

esp_err_t temperature_sensor_read_celsius(temperature_sensor_t *sensor, float *temperature)
{
    if (!sensor || !temperature || !sensor->read_temperature) {
        return ESP_ERR_INVALID_ARG;
    }
    return sensor->read_temperature(sensor->ctx, temperature);
}
