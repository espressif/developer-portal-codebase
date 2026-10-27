/*
 * HTTPS GET example using plain Mbed TLS sockets.
 *
 * SPDX-FileCopyrightText: The Mbed TLS Contributors
 * SPDX-FileCopyrightText: 2015-2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: Apache-2.0
 */
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "esp_efuse.h"
#include "esp_event.h"
#include "esp_flash.h"
#include "esp_flash_encrypt.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_partition.h"
#include "esp_secure_boot.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/error.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "nvs_flash.h"
#include "wifi_creds.h"
#include "workshop_ota.h"
#include "workshop_wifi_connect.h"

#define WEB_SERVER "www.howsmyssl.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *TAG = "example";
static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n\r\n";

static void https_get_task(void *pv_parameters)
{
    char buf[512];
    int ret = 0;
    int flags;

    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ssl_config_init(&conf);
    ESP_LOGI(TAG, "Attaching the certificate bundle...");
    ret = esp_crt_bundle_attach(&conf);
    if (ret < 0) {
        ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");
    if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto cleanup;
    }
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif
    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x", -ret);
        goto cleanup;
    }

    mbedtls_net_init(&server_fd);
    ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT,
                                    MBEDTLS_NET_PROTO_TCP)) != 0) {
        ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
        goto close_connection;
    }
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
            goto close_connection;
        }
    }

    flags = mbedtls_ssl_get_verify_result(&ssl);
    if (flags != 0) {
        ESP_LOGE(TAG, "Failed to verify peer certificate");
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
        ESP_LOGE(TAG, "verification info: %s", buf);
        ret = -1;
        goto close_connection;
    }
    ESP_LOGI(TAG, "Certificate verified.");
    ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

    size_t written = 0;
    while (written < strlen(REQUEST)) {
        ret = mbedtls_ssl_write(&ssl, (const unsigned char *)REQUEST + written,
                                strlen(REQUEST) - written);
        if (ret > 0) {
            written += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
            goto close_connection;
        }
    }

    do {
        memset(buf, 0, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, sizeof(buf) - 1);
        if (ret > 0) {
            for (int i = 0; i < ret; ++i) {
                putchar(buf[i]);
            }
        }
    } while (ret > 0 || ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    mbedtls_ssl_close_notify(&ssl);

close_connection:
    mbedtls_net_free(&server_fd);
cleanup:
    if (ret != 0) {
        mbedtls_strerror(ret, buf, sizeof(buf));
        ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
    }
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_x509_crt_free(&cacert);
    vTaskDelete(NULL);
}

static void dump_nvs_prefix(size_t len)
{
    const esp_partition_t *part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs");
    if (part == NULL) {
        return;
    }
    uint8_t *buf = calloc(1, len);
    if (buf == NULL) {
        return;
    }
    ESP_ERROR_CHECK(esp_partition_read(part, 0, buf, len));
    ESP_LOG_BUFFER_HEXDUMP("nvs_raw", buf, len, ESP_LOG_INFO);
    free(buf);
}

static void log_security_state(void)
{
    esp_efuse_block_t block;

    ESP_LOGI(TAG, "Secure Boot enabled: %s", esp_secure_boot_enabled() ? "yes" : "no");
    if (esp_efuse_find_purpose(ESP_EFUSE_KEY_PURPOSE_HMAC_UP, &block)) {
        ESP_LOGI(TAG, "HMAC_UP key in BLOCK_KEY%d", block - EFUSE_BLK_KEY0);
    }
    if (esp_efuse_find_purpose(ESP_EFUSE_KEY_PURPOSE_SECURE_BOOT_DIGEST0, &block)) {
        ESP_LOGI(TAG, "SECURE_BOOT_DIGEST0 in BLOCK_KEY%d", block - EFUSE_BLK_KEY0);
    }
    if (esp_efuse_find_purpose(ESP_EFUSE_KEY_PURPOSE_XTS_AES_128_KEY, &block)) {
        ESP_LOGI(TAG, "XTS_AES_128_KEY in BLOCK_KEY%d", block - EFUSE_BLK_KEY0);
    }
}

static void log_flash_encryption_demo(void)
{
    static const char test_str[32] = "workshop flash encryption test";
    char buf[sizeof(test_str)] = {0};
    const esp_partition_t *part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (part == NULL) {
        ESP_LOGE(TAG, "Partition \"storage\" not found");
        return;
    }

    ESP_LOGI(TAG, "Flash encryption enabled: %s",
             esp_efuse_is_flash_encryption_enabled() ? "yes" : "no");
    ESP_LOGI(TAG, "Partition \"%s\" treated as encrypted: %s",
             part->label, part->encrypted ? "yes" : "no");
    ESP_ERROR_CHECK(esp_partition_erase_range(part, 0, part->size));
    ESP_ERROR_CHECK(esp_partition_write(part, 0, test_str, sizeof(test_str)));

    ESP_ERROR_CHECK(esp_partition_read(part, 0, buf, sizeof(buf)));
    ESP_LOGI(TAG, "esp_partition_read: %s",
             memcmp(buf, test_str, sizeof(test_str)) == 0
                 ? "matches test string"
                 : "does not match test string");

    ESP_ERROR_CHECK(esp_flash_read(NULL, buf, part->address, sizeof(buf)));
    ESP_LOGI(TAG, "esp_flash_read (raw): %s",
             memcmp(buf, test_str, sizeof(test_str)) == 0
                 ? "matches test string (plaintext)"
                 : "does not match test string (ciphertext)");
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, sizeof(buf), ESP_LOG_INFO);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    log_security_state();
    log_flash_encryption_demo();
    ESP_ERROR_CHECK(wifi_creds_provision_from_menuconfig_if_missing());
    dump_nvs_prefix(512);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(workshop_wifi_connect());

    workshop_ota_confirm_running_app();
    workshop_ota_check_and_update();
    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
}
