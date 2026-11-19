/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_err.h"
#include "esp_log.h"
#include "app_bmi270.h"
#include "app_btn.h"
#include "app_model.h"
#include "app_model_pipeline.h"

static const char *TAG = "Motion Estimation";

extern "C" void app_main(void)
{
    app_btn_handle_t *btn_handle = app_btn_init();
    if (btn_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize button");
        return;
    }

    app_bmi270_handle_t *imu_handle = app_bmi270_init();
    if (imu_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize BMI270 sensor");
        return;
    }

    app_model_init();

    static app_model_pipeline_ctx_t pipeline_ctx;
    pipeline_ctx.btn_handle = btn_handle;
    pipeline_ctx.imu_handle = imu_handle;

    esp_err_t ret = app_model_pipeline_start(&pipeline_ctx);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start model pipeline");
    }
}
