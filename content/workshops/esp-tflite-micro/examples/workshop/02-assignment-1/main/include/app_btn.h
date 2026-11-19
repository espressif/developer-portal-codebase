/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "iot_button.h"
#include "button_gpio.h"

typedef enum {
    APP_BTN_MODE_COLLECT = 0,
    APP_BTN_MODE_INFERENCE,
    APP_BTN_MODE_MAX,
} app_btn_mode_t;

typedef struct {
    button_handle_t btn;
    SemaphoreHandle_t btn_sem;
    volatile app_btn_mode_t mode;
} app_btn_handle_t;

/**
 * @brief Initialize the button
 *
 * @return app_btn_handle_t* Pointer to the button handle
 */
app_btn_handle_t *app_btn_init(void);

/**
 * @brief Get the mode of the button
 *
 * @param handle Pointer to the button handle
 * @return app_btn_mode_t Mode of the button
 */
app_btn_mode_t app_btn_get_mode(app_btn_handle_t *handle);
