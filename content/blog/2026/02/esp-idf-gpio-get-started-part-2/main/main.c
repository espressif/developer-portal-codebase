#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUTTON_GPIO   9   // boot pin on the ESP32-C61-DevkitC-1 board

void app_main(void)
{
    // Configure direction as input
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);  

    // Enable internal pull-up (button to GND)
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY); 
    while (1) {
        int level = gpio_get_level(BUTTON_GPIO);
        if (level == 0) {
            printf("Button pressed\n");
        } else {
            printf("Button released\n");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}