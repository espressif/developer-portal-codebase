#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Provisioning configuration structure
 *
 * Contains user-facing configuration options for provisioning.
 * All fields have sensible defaults when zero-initialized.
 */
typedef struct {
    const char *pop;                /*!< Proof of possession string (NULL defaults to "abcd1234") */
    const char *service_name_prefix; /*!< SoftAP SSID prefix (NULL defaults to "PROV_") */
    const char *service_key;        /*!< SoftAP password (NULL = open network) */
    bool reset_provisioning;        /*!< If true, erase stored credentials and force re-provisioning */
} provisioning_conf_t;

/**
 * @brief Start WiFi provisioning
 *
 * Registers event handlers, initializes the provisioning manager,
 * and either starts SoftAP-based provisioning (if device is not yet
 * provisioned) or connects to the stored WiFi network.
 *
 * The event group and connection state are managed internally.
 * Use provisioning_wait_connected() to block until a connection
 * is established.
 *
 * @param config Pointer to provisioning configuration (NULL for all defaults)
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already started
 */
esp_err_t provisioning_start(const provisioning_conf_t *config);

/**
 * @brief Block until WiFi is connected or timeout expires
 *
 * @param timeout_ms Maximum time to wait in milliseconds (0 = wait forever)
 * @return ESP_OK if connected, ESP_ERR_TIMEOUT on timeout,
 *         ESP_FAIL if connection failed, ESP_ERR_INVALID_STATE if not started
 */
esp_err_t provisioning_wait_connected(uint32_t timeout_ms);

/**
 * @brief Check if WiFi is currently connected
 *
 * Non-blocking check of connection status.
 *
 * @return true if connected, false otherwise
 */
bool provisioning_is_connected(void);

/**
 * @brief Stop provisioning and release all resources
 *
 * Unregisters all event handlers, stops WiFi, and frees internal state.
 * After calling this, provisioning_start() can be called again if needed.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if not started
 */
esp_err_t provisioning_stop(void);
