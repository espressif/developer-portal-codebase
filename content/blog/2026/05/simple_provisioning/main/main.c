#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include "provisioning.h"

static const char *TAG = "app";

void app_main(void)
{
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* Initialize TCP/IP stack and event loop */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    /* Initialize WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Start provisioning */
    provisioning_conf_t prov_config = {
        .pop = "abcd1234",
        .reset_provisioning = true,
    };
    ESP_ERROR_CHECK(provisioning_start(&prov_config));

    /* Block until WiFi is connected */
    ESP_ERROR_CHECK(provisioning_wait_connected(0));

    ESP_LOGI(TAG, "WiFi connected! Starting application...");

    while (1) {
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
