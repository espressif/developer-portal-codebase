#include <stdio.h>
#define ESP_WIFI_SSID "TEST_WORKSHOP"
#define ESP_WIFI_PASS "test_esp"
#define ESP_WIFI_CHANNEL 1
#define MAX_STA_CONN 2
#include "esp_wifi.h"
#include "string.h"
#include "esp_log.h"
#include "esp_http_server.h"


static const char* TAG = "main";

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char* resp_str = "<h1>Hello World</h1>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static const httpd_uri_t hello_world_uri= {
    .uri       = "/",               
    .method    = HTTP_GET,          
    .handler   = hello_get_handler, 
    .user_ctx  = NULL               
};


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                  int32_t event_id, void* event_data){
    printf("Evento n°: %ld!\n", event_id);
}

void wifi_init_softap(){
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // sempre iniziare da qui

    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_softap completata. SSID:%s password:%s canale:%d",
             ESP_WIFI_SSID, ESP_WIFI_PASS, ESP_WIFI_CHANNEL);
}

httpd_handle_t start_webserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Server avviato con successo, registrazione degli handler URI...");
        return server;
    }

    ESP_LOGE(TAG, "Impossibile avviare il server");
    return NULL;
}


void app_main(void)
{
    wifi_init_softap();
    httpd_handle_t server = start_webserver();
    httpd_register_uri_handler(server,&hello_world_uri);
}