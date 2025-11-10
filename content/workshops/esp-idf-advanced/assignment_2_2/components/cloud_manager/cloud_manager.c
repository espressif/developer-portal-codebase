#include <stdio.h>
#include <string.h>
#include "cloud_manager.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

static const char *TAG = "cloud_manager";

struct cloud_manager_t {
    esp_mqtt_client_handle_t client;
    esp_mqtt_client_config_t mqtt_cfg;
};

// Event handler for MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to MQTT broker");
        esp_mqtt_client_subscribe(client, CONFIG_TEMPERATURE_CHANNEL, 0);
        esp_mqtt_client_subscribe(client, CONFIG_ALARM_CHANNEL, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected from MQTT broker");
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Message published (msg_id=%d)", event->msg_id);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        break;
    }
}

cloud_manager_t *cloud_manager_create(void)
{
    cloud_manager_t *manager = calloc(1, sizeof(cloud_manager_t));
    if (!manager) return NULL;


    manager->mqtt_cfg = (esp_mqtt_client_config_t){
        .broker.address.uri = CONFIG_BROKER_URL,
    };

    return manager;
}

esp_err_t cloud_manager_connect(cloud_manager_t *manager)
{

    if(manager == NULL){return ESP_ERR_INVALID_ARG;}
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    manager->client = esp_mqtt_client_init(&manager->mqtt_cfg);
    esp_mqtt_client_register_event(manager->client, ESP_EVENT_ANY_ID, mqtt_event_handler, manager);
    ESP_ERROR_CHECK(example_connect());
    return esp_mqtt_client_start(manager->client);
}

esp_err_t cloud_manager_disconnect(cloud_manager_t *manager)
{
    if (!manager || !manager->client) return ESP_ERR_INVALID_ARG;
    return esp_mqtt_client_stop(manager->client);
}

esp_err_t cloud_manager_send_temperature(cloud_manager_t *manager, float temp)
{
    if (!manager || !manager->client) return ESP_ERR_INVALID_ARG;

    char payload[64];
    snprintf(payload, sizeof(payload), "%.2f", temp);
    ESP_LOGI(TAG, "Temperature: %.2f °C", temp);
    int msg_id = esp_mqtt_client_publish(manager->client, CONFIG_TEMPERATURE_CHANNEL, payload, 0, 1, 0);
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}

esp_err_t cloud_manager_send_alarm(cloud_manager_t *manager)
{
    if (!manager || !manager->client) return ESP_ERR_INVALID_ARG;

    const char *alarm_payload = "ALARM ON!";
    int msg_id = esp_mqtt_client_publish(manager->client, CONFIG_ALARM_CHANNEL, alarm_payload, 0, 1, 0);
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}

void cloud_manager_delete(cloud_manager_t *manager)
{
        if (manager) {
        free(manager);
    }
    
}