/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <string.h>
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "model.h"
#include "app_model.h"

static const char *TAG = "APP_MODEL";
static const char *kLabels[] = { "Counterclockwise circle", "V", "Unknown" };

// TensorFlow Lite model related
constexpr int kExpectedSamples = 200;
constexpr int kExpectedAxes = 3;
constexpr int kTensorArenaSize = 50000;
constexpr size_t kLabelCount = sizeof(kLabels) / sizeof(kLabels[0]);
static_assert(CONFIG_NUM_SAMPLES == kExpectedSamples,
              "The model requires CONFIG_NUM_SAMPLES=200");
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

void app_model_init()
{
    const tflite::Model *model = tflite::GetModel(model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Model provided is schema version %d not equal to supported "
                    "version %d.",
                    model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::MicroMutableOpResolver<8> resolver;

    if (resolver.AddSoftmax() != kTfLiteOk) {
        MicroPrintf("Failed to add softmax operator.");
        return;
    }

    if (resolver.AddFullyConnected() != kTfLiteOk) {
        MicroPrintf("Failed to add fully connected operator.");
        return;
    }

    if (resolver.AddMaxPool2D() != kTfLiteOk) {
        MicroPrintf("Failed to add max pool 2D operator.");
        return;
    }

    if (resolver.AddConv2D() != kTfLiteOk) {
        MicroPrintf("Failed to add conv 2D operator.");
        return;
    }

    if (resolver.AddAveragePool2D() != kTfLiteOk) {
        MicroPrintf("Failed to add average pool 2D operator.");
        return;
    }

    if (resolver.AddExpandDims() != kTfLiteOk) {
        MicroPrintf("Failed to add expand dims operator.");
        return;
    }

    if (resolver.AddReshape() != kTfLiteOk) {
        MicroPrintf("Failed to add reshape operator.");
        return;
    }

    if (resolver.AddMean() != kTfLiteOk) {
        MicroPrintf("Failed to add mean operator.");
        return;
    }

    // Create interpreter
    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory for model tensor
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        MicroPrintf("AllocateTensors() failed");
        return;
    }

    TfLiteTensor *model_input = interpreter->input(0);
    TfLiteTensor *model_output = interpreter->output(0);
    if (model_input == nullptr || model_input->type != kTfLiteFloat32 ||
        model_input->dims == nullptr || model_input->dims->size != 3 ||
        model_input->dims->data[0] != 1 ||
        model_input->dims->data[1] != kExpectedSamples ||
        model_input->dims->data[2] != kExpectedAxes) {
        ESP_LOGE(TAG, "Expected float32 input tensor [1,%d,%d]",
                 kExpectedSamples, kExpectedAxes);
        return;
    }

    if (model_output == nullptr || model_output->type != kTfLiteFloat32 ||
        model_output->dims == nullptr || model_output->dims->size != 2 ||
        model_output->dims->data[0] != 1 ||
        model_output->dims->data[1] != static_cast<int>(kLabelCount)) {
        ESP_LOGE(TAG, "Model output count must match %u labels",
                 static_cast<unsigned>(kLabelCount));
        return;
    }

    input = model_input;
    output = model_output;
    ESP_LOGI(TAG, "Model ready: input [1,%d,%d], %u classes",
             kExpectedSamples, kExpectedAxes,
             static_cast<unsigned>(kLabelCount));
}

bool app_model_predict(float (*imu_data)[3], size_t num_samples, app_model_result_t *result)
{
    if (interpreter == nullptr || input == nullptr || output == nullptr) {
        ESP_LOGE(TAG, "Model not initialized");
        return false;
    }

    if (imu_data == nullptr || num_samples != kExpectedSamples) {
        ESP_LOGE(TAG, "Expected %d IMU samples, received %u",
                 kExpectedSamples, static_cast<unsigned>(num_samples));
        return false;
    }

    const size_t input_bytes = num_samples * kExpectedAxes * sizeof(float);
    if (input_bytes != input->bytes) {
        ESP_LOGE(TAG, "Input byte count mismatch: expected %u, received %u",
                 static_cast<unsigned>(input->bytes),
                 static_cast<unsigned>(input_bytes));
        return false;
    }

    // Copy IMU data to input tensor
    memcpy(input->data.f, imu_data, input_bytes);

    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke failed");
        return false;
    }

    // Get output tensor
    float *output_data = output->data.f;
    int num_classes = output->dims->data[1];
    if (output_data == nullptr || num_classes != static_cast<int>(kLabelCount)) {
        ESP_LOGE(TAG, "Invalid output tensor");
        return false;
    }

    // Find class with maximum probability
    int pred_idx = 0;
    float max_prob = output_data[0];
    for (int i = 1; i < num_classes; i++) {
        if (output_data[i] > max_prob) {
            max_prob = output_data[i];
            pred_idx = i;
        }
    }

    // Store result if requested
    if (result != nullptr) {
        result->class_id = pred_idx;
        result->confidence = max_prob * 100.0f;
        result->class_name = kLabels[pred_idx];
    } else {
        /* Only print if result is not requested (manual mode) */
        printf("Predicted class: %s, confidence: %.1f%%\n", kLabels[pred_idx], max_prob * 100.0f);
        fflush(stdout);
    }

    return true;
}
