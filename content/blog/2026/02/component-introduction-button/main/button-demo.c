#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "iot_button.h"
#include "button_gpio.h"

// Define button GPIO and active level
#define BUTTON_IO_NUM           9 // GPIO number for the button
#define BUTTON_ACTIVE_LEVEL     0 // Active level for the button (0 for active low, 1 for active high)

static const char *TAG = "BUTTON";

// Callback functions for button events
static void button_single_click_event_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    ESP_LOGI(TAG, "Button single click!");
}

static void button_double_click_event_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    ESP_LOGI(TAG, "Button double click!");
}

static void button_long_press_event_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    ESP_LOGI(TAG, "Button long press!");
}

static void button_repeat_event_cb(void *arg, void *data)
{
    (void)arg;
    (void)data;
    ESP_LOGI(TAG, "Button press repeat!");
}

void app_main(void)
{
    // Create button configurations
    const button_config_t btn_cfg = {
        .long_press_time = 5000,      // Long press time in milliseconds
        .short_press_time = 200,      // Short press time in milliseconds
    };

    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = BUTTON_IO_NUM,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .disable_pull = false,
    };

    // Button handle
    button_handle_t btn;
    // Create a new button device
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &btn);
    ESP_ERROR_CHECK(ret);

    // Register callback for button press
    ret = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, NULL, button_double_click_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP, NULL, button_long_press_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, NULL, button_repeat_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
}
