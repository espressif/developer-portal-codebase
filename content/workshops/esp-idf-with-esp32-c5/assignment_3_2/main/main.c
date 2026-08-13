#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "cJSON.h"

/* Change this string to tell firmware versions apart (e.g. "Hello world v1"
 * on the running firmware and "Hello world v2" on the firmware you serve). */

#define FIRMWARE_VERSION_MESSAGE "Hello world v1.3"

static const char *TAG = "simple_ota";

/* Binary semaphore to signal when we are connected to Wi-Fi. */
static SemaphoreHandle_t s_wifi_connected;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected, retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xSemaphoreGive(s_wifi_connected);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_connected = xSemaphoreCreateBinary();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to SSID: %s", CONFIG_WIFI_SSID);

    /* Wait until we are connected and have an IP address. */
    xSemaphoreTake(s_wifi_connected, portMAX_DELAY);
}

static bool remote_version_matches(void)
{
    ESP_LOGI(TAG, "Checking firmware version at: %s", CONFIG_FIRMWARE_VERSION_URL);

    esp_http_client_config_t config = {
        .url = CONFIG_FIRMWARE_VERSION_URL,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to query version: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    esp_http_client_fetch_headers(client);

    char buffer[256];
    int len = esp_http_client_read_response(client, buffer, sizeof(buffer) - 1);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (len <= 0) {
        ESP_LOGW(TAG, "Empty version response");
        return false;
    }
    buffer[len] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        ESP_LOGW(TAG, "Invalid version JSON: %s", buffer);
        return false;
    }

    bool matches = false;
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (cJSON_IsString(version) && version->valuestring != NULL) {
        ESP_LOGI(TAG, "Available firmware version: %s", version->valuestring);
        matches = (strcmp(version->valuestring, FIRMWARE_VERSION_MESSAGE) == 0);
    } else {
        ESP_LOGW(TAG, "Version field missing in response");
    }
    cJSON_Delete(root);
    return matches;
}

static void do_firmware_upgrade(void)
{
    ESP_LOGI(TAG, "OTA started");
    ESP_LOGI(TAG, "Downloading firmware from: %s", CONFIG_FIRMWARE_UPGRADE_URL);

    esp_http_client_config_t http_config = {
        .url = CONFIG_FIRMWARE_UPGRADE_URL,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    /* Simplified esp_https_ota interface: downloads, writes and validates
     * the new firmware in a single blocking call. */
    esp_err_t ret = esp_https_ota(&ota_config);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA ended successfully!");
    } else {
        ESP_LOGE(TAG, "OTA ended with error: %s", esp_err_to_name(ret));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "\n\n%s\n\n", FIRMWARE_VERSION_MESSAGE);

    /* Initialize NVS, required by the Wi-Fi driver. */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    if (remote_version_matches()) {
        ESP_LOGI(TAG, "Already running the latest firmware (%s), skipping OTA",
                 FIRMWARE_VERSION_MESSAGE);
    } else {
        do_firmware_upgrade();
    }
    ESP_LOGI(TAG, "\n\n%s\n\n", FIRMWARE_VERSION_MESSAGE);
    vTaskDelay(10000);
    esp_restart();
}