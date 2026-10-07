#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "ulp_lp_core.h"
#include "ulp_main.h"

/*
 * WARNING: Plain-text credentials are used only to simplify this power demo.
 * Never commit real credentials or use this approach in production firmware.
 */
#define DEMO_WIFI_SSID             "Umbrella_Corp"
#define DEMO_WIFI_PASSWORD         "zombie22"
#define DEMO_WIFI_HOLD_TIME_MS     10000
#define DEMO_WIFI_CONNECT_TIMEOUT_MS 30000

#define WIFI_CONNECTED_BIT BIT(0)

static const char *TAG = "hp_core";
static const char *PROFILE_TAG = "power_profile";
static EventGroupHandle_t wifi_event_group;
static bool wifi_shutting_down;

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_CONNECTING");
        ESP_LOGI(TAG, "[Wi-Fi stage 2/5] Station started; connecting to \"%s\"",
                 DEMO_WIFI_SSID);
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED &&
               !wifi_shutting_down) {
        ESP_LOGW(TAG, "Wi-Fi disconnected before measurement; retrying");
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = event_data;
        ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_CONNECTED IP=" IPSTR,
                 IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "[Wi-Fi stage 3/5] Connected, IP address: " IPSTR,
                 IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool wifi_credentials_configured(void)
{
    return strcmp(DEMO_WIFI_SSID, "YOUR_WIFI_SSID") != 0 &&
           DEMO_WIFI_SSID[0] != '\0';
}

static void run_wifi_power_demo(void)
{
    ESP_LOGW(TAG, "TEST ONLY: Wi-Fi credentials are stored as plain-text source defines");

    if (!wifi_credentials_configured()) {
        ESP_LOGE(TAG, "Set DEMO_WIFI_SSID and DEMO_WIFI_PASSWORD in main/ulp-demo.c");
        ESP_LOGW(TAG, "Skipping Wi-Fi phase and continuing to the LP-core demo");
        return;
    }

    ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_INIT");
    ESP_LOGI(TAG, "[Wi-Fi stage 1/5] Initializing station");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(err);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif != NULL);

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    esp_event_handler_instance_t wifi_instance;
    esp_event_handler_instance_t ip_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &wifi_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, &ip_instance));

    wifi_config_t wifi_cfg = { 0 };
    strlcpy((char *)wifi_cfg.sta.ssid, DEMO_WIFI_SSID,
            sizeof(wifi_cfg.sta.ssid));
    strlcpy((char *)wifi_cfg.sta.password, DEMO_WIFI_PASSWORD,
            sizeof(wifi_cfg.sta.password));
    wifi_cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;

    wifi_event_group = xEventGroupCreate();
    assert(wifi_event_group != NULL);
    wifi_shutting_down = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    const EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE,
        pdMS_TO_TICKS(DEMO_WIFI_CONNECT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_HOLD_START DURATION_MS=%d",
                 DEMO_WIFI_HOLD_TIME_MS);
        ESP_LOGI(TAG, "[Wi-Fi stage 4/5] Holding connection for %d seconds",
                 DEMO_WIFI_HOLD_TIME_MS / 1000);
        vTaskDelay(pdMS_TO_TICKS(DEMO_WIFI_HOLD_TIME_MS));
        ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_HOLD_DONE");
    } else {
        ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_CONNECT_TIMEOUT");
        ESP_LOGE(TAG, "Wi-Fi connection timed out after %d seconds",
                 DEMO_WIFI_CONNECT_TIMEOUT_MS / 1000);
    }

    ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_DISCONNECTING");
    ESP_LOGI(TAG, "[Wi-Fi stage 5/5] Disconnecting and powering down Wi-Fi");
    wifi_shutting_down = true;
    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, ip_instance));
    esp_netif_destroy_default_wifi(sta_netif);
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    vEventGroupDelete(wifi_event_group);
    wifi_event_group = NULL;
    ESP_LOGI(PROFILE_TAG, "STAGE=WIFI_OFF");
    ESP_LOGI(TAG, "Wi-Fi is fully powered down");
}

static void run_hp_power_demo(void)
{
    ESP_LOGI(PROFILE_TAG, "STAGE=HP_HOLD_START DURATION_MS=%d",
             CONFIG_DEMO_HP_HOLD_TIME_MS);
    ESP_LOGI(TAG, "[HP measurement] Keeping the HP domain running for %d seconds",
             CONFIG_DEMO_HP_HOLD_TIME_MS / 1000);
    vTaskDelay(pdMS_TO_TICKS(CONFIG_DEMO_HP_HOLD_TIME_MS));
    ESP_LOGI(PROFILE_TAG, "STAGE=HP_HOLD_DONE");
    ESP_LOGI(TAG, "[HP measurement] Complete; transitioning to LP-core operation");
}

static void start_lp_core(void)
{
    const size_t binary_size = ulp_main_bin_end - ulp_main_bin_start;
    ESP_LOGI(PROFILE_TAG, "STAGE=LP_LOADING BINARY_BYTES=%u",
             (unsigned int)binary_size);
    ESP_LOGI(TAG, "[HP stage 2/4] Loading %u-byte LP-core firmware",
             (unsigned int)binary_size);
    ESP_ERROR_CHECK(ulp_lp_core_load_binary(ulp_main_bin_start, binary_size));
    ESP_LOGI(TAG, "LP-core firmware loaded into retained LP memory");

    ulp_lp_core_cfg_t cfg = {
        .wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_HP_CPU,
    };
    ESP_LOGI(TAG, "Starting LP core with a %d ms pulse interval",
             CONFIG_DEMO_PULSE_INTERVAL_MS);
    ESP_ERROR_CHECK(ulp_lp_core_run(&cfg));
    ESP_LOGI(PROFILE_TAG,
             "STAGE=LP_STARTED PULSE_INTERVAL_MS=%d TRACE_GPIO=%d"
             " TRACE_WIDTH_MS=%d",
             CONFIG_DEMO_PULSE_INTERVAL_MS,
             CONFIG_DEMO_TRACE_GPIO,
             CONFIG_DEMO_TRACE_PULSE_WIDTH_MS);
    ESP_LOGI(TAG, "LP core started successfully");
}

void app_main(void)
{
    /*
     * USB Serial/JTAG disconnects while the HP domain is in deep sleep.
     * Allow the host monitor time to reconnect before printing the result.
     */
    vTaskDelay(pdMS_TO_TICKS(1000));

    const uint32_t wakeup_causes = esp_sleep_get_wakeup_causes();
    const bool lp_core_wakeup =
        (wakeup_causes & BIT(ESP_SLEEP_WAKEUP_ULP)) != 0;

    ESP_LOGI(PROFILE_TAG, "STAGE=APP_START WAKEUP_MASK=0x%08" PRIx32,
             wakeup_causes);
    ESP_LOGI(TAG, "[HP stage 1/4] HP core booted");
    ESP_LOGI(TAG, "Wakeup causes mask: 0x%08" PRIx32, wakeup_causes);

    if (lp_core_wakeup) {
        ESP_LOGI(PROFILE_TAG, "STAGE=LP_WAKE");
        ESP_LOGI(PROFILE_TAG,
                 "STAGE=PULSES_REPORTED CYCLE=%" PRIu32
                 " TOTAL=%" PRIu32 " HP_WAKEUPS=%" PRIu32,
                 ulp_last_wakeup_pulse_count,
                 ulp_last_wakeup_total_count,
                 ulp_hp_wakeup_count);
        ESP_LOGI(TAG, "[HP stage 2/4] LP-core wakeup detected");
        ESP_LOGI(TAG, "[LP stage %" PRIu32 "/4] HP wake request completed",
                 ulp_last_wakeup_stage);
        ESP_LOGI(TAG, "LP report: pulses in cycle=%" PRIu32
                 ", total pulses=%" PRIu32 ", HP wakeups=%" PRIu32,
                 ulp_last_wakeup_pulse_count,
                 ulp_last_wakeup_total_count,
                 ulp_hp_wakeup_count);
    } else {
        ESP_LOGI(TAG, "[HP stage 2/4] Cold boot: initializing LP core");
        ESP_LOGI(TAG, "Pulse interval=%d ms, wake threshold=%d pulses, trace GPIO=%d",
                 CONFIG_DEMO_PULSE_INTERVAL_MS,
                 CONFIG_DEMO_PULSES_PER_WAKEUP,
                 CONFIG_DEMO_TRACE_GPIO);

        ESP_ERROR_CHECK(rtc_gpio_init(CONFIG_DEMO_TRACE_GPIO));
        ESP_ERROR_CHECK(rtc_gpio_set_direction(CONFIG_DEMO_TRACE_GPIO,
                                               RTC_GPIO_MODE_INPUT_OUTPUT));
        ESP_ERROR_CHECK(rtc_gpio_pullup_dis(CONFIG_DEMO_TRACE_GPIO));
        ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(CONFIG_DEMO_TRACE_GPIO));
        ESP_ERROR_CHECK(rtc_gpio_hold_dis(CONFIG_DEMO_TRACE_GPIO));
        ESP_ERROR_CHECK(rtc_gpio_set_level(CONFIG_DEMO_TRACE_GPIO, 0));
        ESP_ERROR_CHECK(gpio_sleep_sel_dis(CONFIG_DEMO_TRACE_GPIO));
        ESP_LOGI(PROFILE_TAG, "STAGE=TRACE_READY GPIO=%d PULSE_WIDTH_MS=%d",
                 CONFIG_DEMO_TRACE_GPIO,
                 CONFIG_DEMO_TRACE_PULSE_WIDTH_MS);

        run_wifi_power_demo();
        run_hp_power_demo();
        start_lp_core();
    }

    ESP_LOGI(TAG, "[HP stage 3/4] Enabling LP-core wakeup source");
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    /*
     * Keep the RTC peripheral and RTC IO path powered so LP-core GPIO pulses
     * remain visible while the HP domain is in deep sleep.
     */
    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,
                                        ESP_PD_OPTION_ON));
    ESP_LOGI(PROFILE_TAG, "STAGE=RTC_PERIPH_ON FOR_LP_GPIO=1");

    if (lp_core_wakeup) {
        ESP_LOGI(PROFILE_TAG, "STAGE=LP_RESUME_REQUEST");
        ESP_LOGI(TAG, "Resuming LP pulse timer before HP enters deep sleep");
        ulp_lp_core_sw_intr_trigger();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(PROFILE_TAG,
             "STAGE=DEEP_SLEEP_ENTER WAKE_AFTER_PULSES=%d"
             " TRACE_GPIO=%d TRACE_WIDTH_MS=%d",
             CONFIG_DEMO_PULSES_PER_WAKEUP,
             CONFIG_DEMO_TRACE_GPIO,
             CONFIG_DEMO_TRACE_PULSE_WIDTH_MS);
    ESP_LOGI(TAG, "[HP stage 4/4] Entering deep sleep; LP core is now in control");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_deep_sleep_start();
}
