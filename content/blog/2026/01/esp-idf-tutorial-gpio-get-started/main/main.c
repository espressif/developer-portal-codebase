#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO 5
static const char * TAG = "LED_TEST"; 

void app_main(void)
{

    ESP_LOGI(TAG,"** GPIO get started tutorial ** ");
    gpio_set_direction(LED_GPIO,GPIO_MODE_OUTPUT);
    gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
    uint32_t led_state = 0;

    while (1) {
            // Toggle the LED state
            led_state = !led_state;
            // Drive the led
            gpio_set_level(LED_GPIO, led_state);
            // Feedback on terminal
            ESP_LOGI(TAG,"Status:%s", led_state? "ON":"OFF");
            // Delay of 0.5s
            vTaskDelay(pdMS_TO_TICKS(500));
        }
}