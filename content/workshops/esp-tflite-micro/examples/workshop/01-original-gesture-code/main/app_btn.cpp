/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_check.h"
#include "esp_log.h"
#include "app_btn.h"

static const char *TAG = "APP_BTN";

void app_btn_short_press_cb(void* arg, void *data)
{
    app_btn_handle_t *handle = (app_btn_handle_t *)data;
    if (handle != nullptr && handle->btn_sem != nullptr) {
        xSemaphoreGive(handle->btn_sem);
    }
}

void app_btn_long_press_cb(void* arg, void *data)
{
    app_btn_handle_t *handle = (app_btn_handle_t *)data;
    if (handle != nullptr) {
        handle->mode = (app_btn_mode_t)(((int)handle->mode + 1) % APP_BTN_MODE_MAX);
        ESP_LOGI(TAG, "Mode switched to %s", handle->mode == APP_BTN_MODE_COLLECT ? "COLLECT" : "INFERENCE");
    }
}

app_btn_handle_t *app_btn_init(void)
{
    esp_err_t ret = ESP_OK;
    app_btn_handle_t* handle = (app_btn_handle_t*)calloc(1, sizeof(app_btn_handle_t));
    if (handle == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate memory for button");
        return nullptr;
    }

    button_config_t btn_cfg = {};
    btn_cfg.long_press_time = 2000;
    button_gpio_config_t btn_gpio_cfg = {};
    btn_gpio_cfg.gpio_num = CONFIG_CONTROL_BUTTON;
    btn_gpio_cfg.active_level = 0;

    ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle->btn);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button");
        free(handle);
        return nullptr;
    }

    ret = iot_button_register_cb(handle->btn, BUTTON_SINGLE_CLICK, nullptr, app_btn_short_press_cb, handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button callback");
        free(handle);
        return nullptr;
    }

    ret = iot_button_register_cb(handle->btn, BUTTON_LONG_PRESS_UP, nullptr, app_btn_long_press_cb, handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register long press callback");
        free(handle);
        return nullptr;
    }

    handle->mode = APP_BTN_MODE_INFERENCE;
    handle->btn_sem = xSemaphoreCreateBinary();
    if (handle->btn_sem == nullptr) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        free(handle);
        return nullptr;
    }

    return handle;
}

app_btn_mode_t app_btn_get_mode(app_btn_handle_t *handle)
{
    return (handle != nullptr) ? handle->mode : APP_BTN_MODE_INFERENCE;
}
