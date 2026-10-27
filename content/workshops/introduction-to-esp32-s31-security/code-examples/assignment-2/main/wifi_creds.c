/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */
#include "wifi_creds.h"

#include <string.h>

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

static const char *TAG = "wifi_creds";
static const char *NS = "workshop_wifi";
static const char *KEY_SSID = "ssid";
static const char *KEY_PASS = "pass";

esp_err_t wifi_creds_provision_from_menuconfig_if_missing(void)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err == ESP_OK) {
        size_t len = 0;
        err = nvs_get_str(h, KEY_SSID, NULL, &len);
        nvs_close(h);
        if (err == ESP_OK && len > 1) {
            ESP_LOGI(TAG, "Wi-Fi credentials already in NVS");
            return ESP_OK;
        }
    }

    err = nvs_open(NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
#if CONFIG_EXAMPLE_CONNECT_WIFI
    err = nvs_set_str(h, KEY_SSID, CONFIG_EXAMPLE_WIFI_SSID);
    if (err == ESP_OK) {
        err = nvs_set_str(h, KEY_PASS, CONFIG_EXAMPLE_WIFI_PASSWORD);
    }
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Stored lab Wi-Fi credentials in NVS");
    }
    return err;
#else
    nvs_close(h);
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t wifi_creds_load(wifi_config_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }

    size_t ssid_len = sizeof(cfg->sta.ssid);
    size_t pass_len = sizeof(cfg->sta.password);
    err = nvs_get_str(h, KEY_SSID, (char *)cfg->sta.ssid, &ssid_len);
    if (err == ESP_OK) {
        err = nvs_get_str(h, KEY_PASS, (char *)cfg->sta.password, &pass_len);
    }
    nvs_close(h);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded Wi-Fi SSID from NVS: %s", cfg->sta.ssid);
    }
    return err;
}
