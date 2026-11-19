/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

/**
 * @brief Initialize the model
 *
 */
void app_model_init(void);

/**
 * @brief Prediction result structure
 */
typedef struct {
    int class_id;
    float confidence;
    const char *class_name;
} app_model_result_t;

/**
 * @brief Predict the motion of the model
 *
 * @param imu_data Pointer to the IMU data
 * @param num_samples Number of samples to predict
 * @param result Pointer to store prediction result (optional, can be NULL to just print)
 * @return true if prediction successful, false otherwise
 */
bool app_model_predict(float (*imu_data)[3], size_t num_samples, app_model_result_t *result);
