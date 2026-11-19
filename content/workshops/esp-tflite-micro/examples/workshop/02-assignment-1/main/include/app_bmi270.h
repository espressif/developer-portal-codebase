/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include <stddef.h>
#include "esp_err.h"
#include "bmi270_api.h"
#include "i2c_bus.h"

typedef struct {
    bmi270_handle_t bmi_handle;
    i2c_bus_handle_t i2c_bus;
    float (*gyr_data)[3];
} app_bmi270_handle_t;

/**
 * @brief Initialize the BMI270 sensor
 *
 * @return app_bmi270_handle_t* Pointer to the BMI270 handle
 */
app_bmi270_handle_t* app_bmi270_init(void);

/**
 * @brief Collect IMU data from the BMI270 sensor
 *
 * @param handle Pointer to the BMI270 handle
 * @param imu_data Pointer to the IMU data
 * @param num_samples Number of samples to collect
 * @return esp_err_t ESP_OK on success, otherwise an error code
 */
esp_err_t app_bmi270_collect(app_bmi270_handle_t* handle, float (*imu_data)[3], size_t num_samples);

/**
 * @brief Collect a single sample from the BMI270 sensor
 *
 * @param handle Pointer to the BMI270 handle
 * @param imu_data Pointer to the IMU data
 * @return esp_err_t ESP_OK on success, otherwise an error code
 */
esp_err_t app_bmi270_collect_once(app_bmi270_handle_t* handle, float imu_data[3]);
