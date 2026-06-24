#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#define LEDC_TIMER        LEDC_TIMER_0
#define LEDC_MODE         LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO    (25)               // Output GPIO pin
#define LEDC_CHANNEL      LEDC_CHANNEL_0
#define LEDC_DUTY_RES     LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FS           (8192)
#define LEDC_DUTY         (6144)            // 50% duty: (2^13) * 50% = 4096
#define LEDC_FREQUENCY    (4000)            // 4 kHz

const static char * TAG = "main";

static void ledc_init(void)
{
    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .gpio_num   = LEDC_OUTPUT_IO,
        .duty       = 0,   // Start at 0% duty cycle
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

esp_err_t ledc_set_duty_cycle(float duty){
    printf("duty: %d\n",(int)(duty*LEDC_FS));
    esp_err_t ret = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, (int) (duty*LEDC_FS));
    if(ret != ESP_OK){
        return ret;
    }
    ret = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    return ret;
}

void app_main(void)
{
    // Initialize LEDC peripheral
    ledc_init();
    ESP_LOGI(TAG, "Setting duty-cycle to 75%%");
    // Set duty cycle to 50%
    ESP_ERROR_CHECK(ledc_set_duty_cycle(0.60));
}