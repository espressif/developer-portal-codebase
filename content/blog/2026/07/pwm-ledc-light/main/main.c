#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mqtt_client.h"


#define LEDC_TIMER        LEDC_TIMER_0
#define LEDC_MODE         LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO    (25)               // Output GPIO pin
#define LEDC_CHANNEL      LEDC_CHANNEL_0
#define LEDC_DUTY_RES     LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FS           (8192)
#define LEDC_DUTY         (6144)            // 50% duty: (2^13) * 50% = 4096
#define LEDC_FREQUENCY    (4000)            // 4 kHz

/* ── Wi-Fi credentials ─────────────────────────────────────────── */
#define WIFI_SSID        "FrancescoHotspot"
#define WIFI_PASS        "ciaociao"
#define WIFI_MAXIMUM_RETRY  5

/* ── MQTT broker ────────────────────────────────────────────────── */
#define MQTT_BROKER_URI  "mqtt://broker.freemqtt.com:1883"
#define MQTT_USERNAME    "freemqtt"
#define MQTT_PASSWORD    "public"
#define MQTT_TOPIC       "c845041f/light"

/* ── Wi-Fi event group ──────────────────────────────────────────── */
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

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


/* ── Wi-Fi event handler ────────────────────────────────────────── */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Retrying Wi-Fi connection...");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ── Wi-Fi initialisation ───────────────────────────────────────── */
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Wait until connected or failed */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                       WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                       pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to Wi-Fi: %s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi: %s", WIFI_SSID);
    }
}

/* ── MQTT event handler ─────────────────────────────────────────── */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event  = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, MQTT_TOPIC, 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("published %.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Last errno: %s",
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id: %d", event->event_id);
        break;
    }
}

/* ── MQTT initialisation ────────────────────────────────────────── */
static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
        },
        .credentials = {
            .username = MQTT_USERNAME,
            .authentication.password = MQTT_PASSWORD,
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


void app_main(void)
{
    // Initialize LEDC peripheral
    ledc_init();
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Connect to Wi-Fi first
    wifi_init_sta();

    // Then start MQTT
    mqtt_app_start();
    ESP_LOGI(TAG, "Setting duty-cycle to 45%%");
    // Set duty cycle to 50%
    ESP_ERROR_CHECK(ledc_set_duty_cycle(0.45));
}