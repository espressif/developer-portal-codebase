    #include "driver/gpio.h"

    typedef struct {
        int gpio_nr;
        bool status;
    }led_gpio_t;

    esp_err_t led_config(led_gpio_t * led_gpio);
    esp_err_t led_drive(led_gpio_t * led_gpio, bool level);
    esp_err_t led_toggle(led_gpio_t * led_gpio);