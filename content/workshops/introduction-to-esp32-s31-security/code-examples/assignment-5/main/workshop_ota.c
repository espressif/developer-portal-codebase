/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>

#include "esp_app_desc.h"
#include "esp_check.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "workshop_ota.h"

static const char *TAG = "workshop_ota";

extern const uint8_t ota_server_cert_pem_start[] asm("_binary_ca_cert_pem_start");

void workshop_ota_confirm_running_app(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;

    ESP_LOGI(TAG, "Running partition: %s, version: %s",
             running->label, esp_app_get_description()->version);
    if (esp_ota_get_state_partition(running, &state) == ESP_OK &&
            state == ESP_OTA_IMG_PENDING_VERIFY) {
#if CONFIG_WORKSHOP_OTA_FAIL_SELF_TEST
        ESP_LOGE(TAG, "Self-test failed, rolling back to the previous firmware");
        esp_ota_mark_app_invalid_rollback_and_reboot();
#endif
        ESP_LOGI(TAG, "New firmware passed the self-test, cancelling rollback");
        ESP_ERROR_CHECK(esp_ota_mark_app_valid_cancel_rollback());
    }
}

esp_err_t workshop_ota_check_and_update(void)
{
    esp_http_client_config_t http_config = {
        .url = CONFIG_WORKSHOP_OTA_URL,
        .cert_pem = (const char *)ota_server_cert_pem_start,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };
    esp_https_ota_handle_t handle = NULL;

    ESP_LOGI(TAG, "Checking %s", CONFIG_WORKSHOP_OTA_URL);
    ESP_RETURN_ON_ERROR(esp_https_ota_begin(&ota_config, &handle),
                        TAG, "Could not connect to the OTA server");

    esp_app_desc_t new_desc;
    esp_err_t err = esp_https_ota_get_img_desc(handle, &new_desc);
    if (err != ESP_OK) {
        esp_https_ota_abort(handle);
        ESP_LOGE(TAG, "Could not read the image header");
        return err;
    }

    const esp_app_desc_t *running = esp_app_get_description();
    ESP_LOGI(TAG, "Running version: %s, server version: %s",
             running->version, new_desc.version);
    if (strcmp(new_desc.version, running->version) == 0) {
        ESP_LOGI(TAG, "Firmware is up to date");
        esp_https_ota_abort(handle);
        return ESP_OK;
    }

    const esp_partition_t *last_invalid = esp_ota_get_last_invalid_partition();
    esp_app_desc_t invalid_desc;
    if (last_invalid != NULL &&
            esp_ota_get_partition_description(last_invalid, &invalid_desc) == ESP_OK &&
            strcmp(new_desc.version, invalid_desc.version) == 0) {
        ESP_LOGW(TAG, "Version %s was rolled back before, skipping it", new_desc.version);
        esp_https_ota_abort(handle);
        return ESP_OK;
    }

    while ((err = esp_https_ota_perform(handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
    }
    if (err != ESP_OK) {
        esp_https_ota_abort(handle);
        ESP_LOGE(TAG, "Download failed (%s)", esp_err_to_name(err));
        return err;
    }

    err = esp_https_ota_finish(handle);
    if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
        ESP_LOGE(TAG, "Image rejected: signature or hash verification failed");
        return err;
    }
    ESP_RETURN_ON_ERROR(err, TAG, "OTA finish failed");

    ESP_LOGI(TAG, "OTA succeeded, rebooting into the new firmware");
    esp_restart();
    return ESP_OK;
}
