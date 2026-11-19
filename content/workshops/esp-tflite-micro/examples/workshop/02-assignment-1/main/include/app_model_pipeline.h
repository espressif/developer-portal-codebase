/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "app_bmi270.h"
#include "app_btn.h"

typedef struct {
    app_btn_handle_t *btn_handle;
    app_bmi270_handle_t *imu_handle;
    QueueHandle_t inference_queue;
    SemaphoreHandle_t inference_done_sem;
} app_model_pipeline_ctx_t;

/**
 * @brief Start the model pipeline
 *
 * @param ctx Pointer to the model pipeline context
 * @return esp_err_t ESP_OK on success, otherwise an error code
 */
esp_err_t app_model_pipeline_start(app_model_pipeline_ctx_t *ctx);
