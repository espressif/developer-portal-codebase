/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON response.
 * Adapted from the ssl_client1 example in mbedTLS.
 *
 * SPDX-FileCopyrightText: The Mbed TLS Contributors
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileContributor: 2015-2025 Espressif Systems (Shanghai) CO LTD
 */
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "esp_efuse.h"
#include "esp_event.h"
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
#include "workshop_wifi_connect.h"
#ifdef CONFIG_MBEDTLS_SSL_PROTO_TLS1_3
#include "psa/crypto.h"
#endif

#define WEB_SERVER "www.howsmyssl.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *TAG = "example";
static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

static void log_security_state(void)
{
    esp_efuse_block_t blk;
    ESP_LOGI(TAG, "Secure Boot enabled: %s", esp_secure_boot_enabled() ? "yes" : "no");
    if (esp_efuse_find_purpose(ESP_EFUSE_KEY_PURPOSE_HMAC_UP, &blk)) {
        ESP_LOGI(TAG, "HMAC_UP key in BLOCK_KEY%d", blk - EFUSE_BLK_KEY0);
    }
    if (esp_efuse_find_purpose(ESP_EFUSE_KEY_PURPOSE_SECURE_BOOT_DIGEST0, &blk)) {
        ESP_LOGI(TAG, "SECURE_BOOT_DIGEST0 in BLOCK_KEY%d", blk - EFUSE_BLK_KEY0);
    }
}

static void dump_nvs_prefix(size_t len)
{
    const esp_partition_t *part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs");
    if (part == NULL) {
        ESP_LOGW(TAG, "NVS partition not found");
        return;
    }
    uint8_t *buf = calloc(1, len);
    if (buf == NULL) {
        ESP_LOGE(TAG, "Could not allocate NVS dump buffer");
        return;
    }
    ESP_ERROR_CHECK(esp_partition_read(part, 0, buf, len));
    ESP_LOG_BUFFER_HEXDUMP("nvs_raw", buf, len, ESP_LOG_INFO);
    free(buf);
}

static void https_get_task(void *pvParameters)
{
    char buf[512];
    int ret, flags, len;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    ESP_LOGI(TAG, "Seeding the random number generator");
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
    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif
    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x", -ret);
        goto exit;
    }

    while (1) {
        mbedtls_net_init(&server_fd);
        ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT,
                                       MBEDTLS_NET_PROTO_TCP)) != 0) {
            ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }
        ESP_LOGI(TAG, "Connected.");
        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
        ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }
        ESP_LOGI(TAG, "Verifying peer X.509 certificate...");
        if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
            ESP_LOGW(TAG, "Failed to verify peer certificate!");
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
            ESP_LOGW(TAG, "verification info: %s", buf);
        } else {
            ESP_LOGI(TAG, "Certificate verified.");
        }
        ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));
        ESP_LOGI(TAG, "Writing HTTP request...");
        size_t written_bytes = 0;
        do {
            ret = mbedtls_ssl_write(&ssl, (const unsigned char *)REQUEST + written_bytes,
                                    strlen(REQUEST) - written_bytes);
            if (ret >= 0) {
                ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
            } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
                ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
                goto exit;
            }
        } while (written_bytes < strlen(REQUEST));

        ESP_LOGI(TAG, "Reading HTTP response...");
        do {
            len = sizeof(buf) - 1;
            bzero(buf, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);
#if CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 && CONFIG_MBEDTLS_CLIENT_SSL_SESSION_TICKETS
            if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
                ESP_LOGD(TAG, "got session ticket in TLS 1.3 connection, retry read");
                continue;
            }
#endif
            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                continue;
            }
            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
            }
            if (ret < 0) {
                ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
                break;
            }
            if (ret == 0) {
                ESP_LOGI(TAG, "connection closed");
                break;
            }
            len = ret;
            ESP_LOGD(TAG, "%d bytes read", len);
            for (int i = 0; i < len; i++) {
                putchar(buf[i]);
            }
        } while (1);
        mbedtls_ssl_close_notify(&ssl);

exit:
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);
        if (ret != 0) {
            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        }
        putchar('\n');
        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);
        ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n",
                 esp_get_minimum_free_heap_size());
        for (int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
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
    ESP_ERROR_CHECK(wifi_creds_provision_from_menuconfig_if_missing());
    dump_nvs_prefix(512);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(workshop_wifi_connect());
    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
}
