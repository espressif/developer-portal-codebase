#include <stdio.h>
#include "led_toggle.h"
#include "esp_err.h"

  esp_err_t led_config(led_gpio_t * led_gpio){

        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask =  (1ULL<<led_gpio->gpio_nr);
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        return gpio_config(&io_conf);
    }

    esp_err_t led_drive(led_gpio_t * led_gpio,bool level){
        led_gpio->status = level;
        return gpio_set_level(led_gpio->gpio_nr, level); // accende il LED
    }

    esp_err_t led_toggle(led_gpio_t * led_gpio){
        if(led_gpio->status){
            led_gpio->status = false;
            led_drive(led_gpio, false);
        }else{
            led_gpio->status = true;
            led_drive(led_gpio,true);
        }
        return ESP_OK;
    }