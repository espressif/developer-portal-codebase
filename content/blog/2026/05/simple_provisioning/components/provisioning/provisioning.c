#include "provisioning.h"
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <network_provisioning/manager.h>
#include <network_provisioning/scheme_softap.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_netif.h>

#define PROV_QR_VERSION         "v1"
#define PROV_TRANSPORT_SOFTAP   "softap"
#define QRCODE_BASE_URL         "https://espressif.github.io/esp-jumpstart/qrcode.html"

#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

static const char *TAG = "provisioning";

static const char *DEFAULT_POP = "abcd1234";
static const char *DEFAULT_PREFIX = "PROV_";

/* Internal state */
static EventGroupHandle_t s_wifi_event_group = NULL;
static bool s_started = false;
static bool s_connected = false;

/* ============================================================================
 * Internal helpers
 * ============================================================================ */

static void wifi_prov_print_qr(const char *name, const char *pop, const char *transport)
{
    if (!name || !transport) {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};

    snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
         ",\"pop\":\"%s\",\"transport\":\"%s\"}",
         PROV_QR_VERSION, name, pop ? pop : "", transport);
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s",
             QRCODE_BASE_URL, payload);
}

static void get_device_service_name(char *service_name, size_t max, const char *prefix)
{
    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* ============================================================================
 * Event handlers
 * ============================================================================ */

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        s_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        s_connected = false;
        esp_wifi_connect();
        break;
    case WIFI_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "SoftAP transport: Connected!");
        break;
    case WIFI_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
        break;
    default:
        break;
    }
}

static void provisioning_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    switch (event_id) {
    case NETWORK_PROV_START:
        ESP_LOGI(TAG, "Provisioning started");
        break;
    case NETWORK_PROV_WIFI_CRED_RECV: {
        wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
        ESP_LOGI(TAG, "Received Wi-Fi credentials"
                 "\n\tSSID     : %s\n\tPassword : %s",
                 (const char *) wifi_sta_cfg->ssid,
                 (const char *) wifi_sta_cfg->password);
        break;
    }
    case NETWORK_PROV_WIFI_CRED_FAIL: {
        network_prov_wifi_sta_fail_reason_t *reason = (network_prov_wifi_sta_fail_reason_t *)event_data;
        ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                 "\n\tPlease reset to factory and retry provisioning",
                 (*reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) ?
                 "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        break;
    }
    case NETWORK_PROV_WIFI_CRED_SUCCESS:
        ESP_LOGI(TAG, "Provisioning successful");
        break;
    case NETWORK_PROV_END: {
        esp_err_t err = network_prov_mgr_deinit();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to de-initialize provisioning manager: %s", esp_err_to_name(err));
        }
        break;
    }
    default:
        break;
    }
}

static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* ============================================================================
 * Public API
 * ============================================================================ */

esp_err_t provisioning_start(const provisioning_conf_t *config)
{
    if (s_started) {
        ESP_LOGW(TAG, "Provisioning already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Resolve defaults */
    const char *pop = (config && config->pop) ? config->pop : DEFAULT_POP;
    const char *prefix = (config && config->service_name_prefix) ? config->service_name_prefix : DEFAULT_PREFIX;
    const char *service_key = config ? config->service_key : NULL;
    bool reset = config ? config->reset_provisioning : false;

    /* Create internal event group */
    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &provisioning_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    /* Configuration for the provisioning manager */
    network_prov_mgr_config_t mgr_config = {
        .scheme = network_prov_scheme_softap,
        .scheme_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE
    };

    ESP_ERROR_CHECK(network_prov_mgr_init(mgr_config));

    if (reset) {
        ESP_LOGI(TAG, "Resetting provisioning data");
        network_prov_mgr_reset_wifi_provisioning();
    }

    bool provisioned = false;
    ESP_ERROR_CHECK(network_prov_mgr_is_wifi_provisioned(&provisioned));

    s_started = true;
    s_connected = false;

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");

        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name), prefix);

        network_prov_security_t security = NETWORK_PROV_SECURITY_1;
        network_prov_security1_params_t *sec_params = (network_prov_security1_params_t *) pop;

        ESP_ERROR_CHECK(network_prov_mgr_start_provisioning(security, sec_params, service_name, service_key));

        wifi_prov_print_qr(service_name, pop, PROV_TRANSPORT_SOFTAP);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");
        ESP_ERROR_CHECK(network_prov_mgr_deinit());
        wifi_init_sta();
    }

    return ESP_OK;
}

esp_err_t provisioning_wait_connected(uint32_t timeout_ms)
{
    if (!s_started) {
        ESP_LOGE(TAG, "Provisioning not started");
        return ESP_ERR_INVALID_STATE;
    }

    TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                            pdFALSE,
                                            pdFALSE,
                                            ticks);

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        return ESP_FAIL;
    }

    return ESP_ERR_TIMEOUT;
}

bool provisioning_is_connected(void)
{
    return s_connected;
}

esp_err_t provisioning_stop(void)
{
    if (!s_started) {
        ESP_LOGW(TAG, "Provisioning not started");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Stopping provisioning and cleaning up");

    /* Unregister all event handlers */
    esp_event_handler_unregister(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &provisioning_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);

    /* Stop WiFi */
    esp_wifi_stop();

    /* Delete internal event group */
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    s_started = false;
    s_connected = false;

    return ESP_OK;
}
