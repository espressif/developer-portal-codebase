#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{

    ESP_LOGI("main","** GPIO get started tutorial ** ");
    // Zero-initialize the config structure
    gpio_config_t io_conf = {};

    // Disable interrupts
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // Set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;

    // Bit mask of the pins (e.g., GPIO 5)
    io_conf.pin_bit_mask = (1ULL << 5);

    // Disable pull-down and pull-up
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    // Apply the configuration
    gpio_config(&io_conf);

    uint32_t led_state = 0;

    while (1) {
            // Toggle the LED state
            led_state = !led_state;
            // Drive the led
            gpio_set_level(5, led_state);
            // Feedback on terminal
            ESP_LOGI("loop","Status:%s", led_state? "ON":"OFF");
            // Delay of 0.5s
            vTaskDelay(500);
        }
}