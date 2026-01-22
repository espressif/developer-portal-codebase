#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO           9
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "button_example";

// ISR callback
static void IRAM_ATTR button_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    ESP_EARLY_LOGI(TAG, "GPIO[%d] interrupt triggered", gpio_num);
}

void app_main(void)
{
    // Configure direction and pull-up
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);          // input only[[ESP32 GPIO fn](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html#functions)]
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);         // enable internal pull-up[[ESP32 GPIO fn](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html#functions)]

    // Configure interrupt type (falling edge for active‑low button)
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);        // same edge as in the workshop example[[Assign 2.2 code](https://developer.espressif.com/workshops/esp-idf-advanced/assignment-2-2/#reading-gpio-code)]

    // Install GPIO ISR service and register per‑pin handler
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);           //[[Assign 2.2 solution](https://developer.espressif.com/workshops/esp-idf-advanced/assignment-2-2/#assignment-solution-code)]
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void *)BUTTON_GPIO);

    ESP_LOGI(TAG, "Button on GPIO %d configured with interrupt", BUTTON_GPIO);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}