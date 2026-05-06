#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "provisioning.h"
#include <nvs_flash.h>

static const char *TAG = "app";
provisioning_conf_t config;

void app_main(void)
{
    /* Start provisioning - blocks until WiFi is connected */
     config.wifi_event_group = xEventGroupCreate();
   

    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
   
    
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
     ESP_ERROR_CHECK(provisioning_start(&config));
    
    xEventGroupWaitBits(config.wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);

    while (1) {
        
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}