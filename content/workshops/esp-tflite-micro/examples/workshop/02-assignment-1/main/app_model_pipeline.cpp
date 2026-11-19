/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "app_bmi270.h"
#include "app_btn.h"
#include "app_model.h"
#include "app_model_pipeline.h"

static const char *TAG = "Model Pipeline";

typedef struct {
    float (*data)[3];
    size_t num_samples;
} inference_msg_t;

static void collect_once(app_model_pipeline_ctx_t *ctx, app_bmi270_handle_t *imu_handle,
                         const float trigger_sample[3], bool send_to_inference)
{
    const float sumabs = fabsf(trigger_sample[0]) + fabsf(trigger_sample[1]) + fabsf(trigger_sample[2]);
    ESP_LOGI(TAG, "Trigger met (|g|sum=%.2f > %d), collecting %d samples",
             sumabs, CONFIG_TRIGGER_THRESHOLD, CONFIG_NUM_SAMPLES);

    imu_handle->gyr_data[0][0] = trigger_sample[0];
    imu_handle->gyr_data[0][1] = trigger_sample[1];
    imu_handle->gyr_data[0][2] = trigger_sample[2];

    for (int i = 1; i < CONFIG_NUM_SAMPLES; i++) {
        vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_INTERVAL_MS));
        float t[3] = {0};
        esp_err_t e = app_bmi270_collect_once(imu_handle, t);
        if (e != ESP_OK) {
            ESP_LOGW(TAG, "Sample %d not ready (%s), retry", i, esp_err_to_name(e));
            i--;
            continue;
        }
        imu_handle->gyr_data[i][0] = t[0];
        imu_handle->gyr_data[i][1] = t[1];
        imu_handle->gyr_data[i][2] = t[2];
    }

    if (send_to_inference) {
        inference_msg_t msg = {
            .data = imu_handle->gyr_data,
            .num_samples = CONFIG_NUM_SAMPLES,
        };
        if (xQueueSend(ctx->inference_queue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
            xSemaphoreTake(ctx->inference_done_sem, portMAX_DELAY);
        } else {
            ESP_LOGW(TAG, "Inference queue full, drop data");
        }
    }
}

static void collect_task(void *arg)
{
    app_model_pipeline_ctx_t *ctx = (app_model_pipeline_ctx_t *)arg;
    app_btn_handle_t *btn_handle = ctx->btn_handle;
    app_bmi270_handle_t *imu_handle = ctx->imu_handle;

    while (1) {
        if (app_btn_get_mode(btn_handle) == APP_BTN_MODE_INFERENCE) {
            /* Sliding window inference mode: continuously collect and infer */
            static int sample_idx = 0;

            /* Collect one sample */
            float g[3] = {0};
            esp_err_t once_err = app_bmi270_collect_once(imu_handle, g);
            if (once_err != ESP_OK) {
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            /* Store sample in buffer */
            imu_handle->gyr_data[sample_idx][0] = g[0];
            imu_handle->gyr_data[sample_idx][1] = g[1];
            imu_handle->gyr_data[sample_idx][2] = g[2];
            sample_idx++;

            /* Once buffer is full, do inference and slide window */
            if (sample_idx >= CONFIG_NUM_SAMPLES) {
                /* Perform inference on current full window */
                inference_msg_t msg = {
                    .data = imu_handle->gyr_data,
                    .num_samples = CONFIG_NUM_SAMPLES,
                };

                if (xQueueSend(ctx->inference_queue, &msg, 0) == pdTRUE) {
                    xSemaphoreTake(ctx->inference_done_sem, portMAX_DELAY);
                }

                /* Slide window: shift left by STEP samples */
                const int step = CONFIG_SLIDING_WINDOW_STEP;
                memmove(imu_handle->gyr_data,
                        &imu_handle->gyr_data[step],
                        (CONFIG_NUM_SAMPLES - step) * sizeof(float[3]));
                sample_idx = CONFIG_NUM_SAMPLES - step;
            }

            vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_INTERVAL_MS));
            continue;
        }

        /* Manual collection mode (button triggered) */
        if (xSemaphoreTake(btn_handle->btn_sem, pdMS_TO_TICKS(1000)) != pdTRUE) {
            continue;
        }

        while (1) {
            if (app_btn_get_mode(btn_handle) == APP_BTN_MODE_INFERENCE) {
                break;
            }
            float g[3] = {0};
            esp_err_t once_err = app_bmi270_collect_once(imu_handle, g);
            if (once_err != ESP_OK) {
                ESP_LOGW(TAG, "Failed to read trigger sample (%s)", esp_err_to_name(once_err));
                continue;
            }

            const float sumabs = fabsf(g[0]) + fabsf(g[1]) + fabsf(g[2]);
            if (sumabs <= (float)CONFIG_TRIGGER_THRESHOLD) {
                ESP_LOGI(TAG, "Trigger not met (|g|sum=%.2f <= %d), skip capture", sumabs, CONFIG_TRIGGER_THRESHOLD);
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            ESP_LOGI(TAG, "Trigger met (|g|sum=%.2f > %d), collecting %d samples (include trigger sample)",
                     sumabs, CONFIG_TRIGGER_THRESHOLD, CONFIG_NUM_SAMPLES);

            collect_once(ctx, imu_handle, g, false);

            printf("\n=== Data Collection Results ===\n");
            for (int i = 0; i < CONFIG_NUM_SAMPLES; i++) {
                if (i == CONFIG_NUM_SAMPLES - 1) {
                    printf("%.5f,%.5f,%.5f",
                           imu_handle->gyr_data[i][0], imu_handle->gyr_data[i][1], imu_handle->gyr_data[i][2]);
                } else {
                    printf("%.5f,%.5f,%.5f,",
                           imu_handle->gyr_data[i][0], imu_handle->gyr_data[i][1], imu_handle->gyr_data[i][2]);
                }
            }
            printf("\n=== End of Data Collection ===\n\n");
            break;
        }
    }
}

static void inference_task(void *arg)
{
    app_model_pipeline_ctx_t *ctx = (app_model_pipeline_ctx_t *)arg;
    inference_msg_t msg;
    static float inference_buf[CONFIG_NUM_SAMPLES][3];
    app_model_result_t result;

    static int last_class_id = -1;
    static int same_class_count = 0;
    static bool motion_reported = false;  /*!< Has this motion been reported */
    const int MIN_COUNT_TO_CONFIRM = 3;   /*!< Need 3 consistent detections to print */
    const int MIN_COUNT_TO_RESET = 5;     /*!< After 5 detections, ready for next motion */

    while (1) {
        if (xQueueReceive(ctx->inference_queue, &msg, portMAX_DELAY) == pdTRUE) {
            memcpy(inference_buf, msg.data, msg.num_samples * 3 * sizeof(float));
            xSemaphoreGive(ctx->inference_done_sem);

            if (app_model_predict(inference_buf, msg.num_samples, &result)) {
                if (result.confidence >= CONFIG_INFERENCE_CONFIDENCE_THRESHOLD) {
                    if (result.class_id != last_class_id) {
                        // New class detected, reset counter
                        last_class_id = result.class_id;
                        same_class_count = 1;
                        motion_reported = false;
                    } else {
                        same_class_count++;

                        if (same_class_count == MIN_COUNT_TO_CONFIRM && !motion_reported) {
                            ESP_LOGI(TAG, "Motion detected: %s (confidence: %.1f%%)",
                                     result.class_name, result.confidence);
                            fflush(stdout);
                            motion_reported = true;
                        }

                        if (same_class_count >= MIN_COUNT_TO_RESET) {
                            last_class_id = -1;
                            same_class_count = 0;
                            motion_reported = false;
                        }
                    }
                } else {
                    if (motion_reported && same_class_count >= MIN_COUNT_TO_CONFIRM) {
                        last_class_id = -1;
                        same_class_count = 0;
                        motion_reported = false;
                    }
                }
            }
        }
    }
}

esp_err_t app_model_pipeline_start(app_model_pipeline_ctx_t *ctx)
{
    if (ctx == nullptr || ctx->btn_handle == nullptr || ctx->imu_handle == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    ctx->inference_queue = xQueueCreate(2, sizeof(inference_msg_t));
    if (ctx->inference_queue == nullptr) {
        ESP_LOGE(TAG, "Failed to create inference queue");
        return ESP_ERR_NO_MEM;
    }
    ctx->inference_done_sem = xSemaphoreCreateBinary();
    if (ctx->inference_done_sem == nullptr) {
        ESP_LOGE(TAG, "Failed to create inference done semaphore");
        vQueueDelete(ctx->inference_queue);
        return ESP_ERR_NO_MEM;
    }

    BaseType_t ret = xTaskCreate(collect_task, "collect", 5 * 1024, ctx, 4, nullptr);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create collect task");
        vSemaphoreDelete(ctx->inference_done_sem);
        vQueueDelete(ctx->inference_queue);
        return ESP_FAIL;
    }

    ret = xTaskCreate(inference_task, "inference", 6 * 1024, ctx, 5, nullptr);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create inference task");
        vSemaphoreDelete(ctx->inference_done_sem);
        vQueueDelete(ctx->inference_queue);
        return ESP_FAIL;
    }
    return ESP_OK;
}
