#pragma once

#include "esp_err.h"

typedef struct cloud_manager_t cloud_manager_t;

/**
 * @brief Creates a new cloud manager instance
 */
cloud_manager_t *cloud_manager_create(void);

/**
 * @brief Connects the cloud manager (starts MQTT)
 */
esp_err_t cloud_manager_connect(cloud_manager_t *manager);

/**
 * @brief Disconnects the cloud manager
 */
esp_err_t cloud_manager_disconnect(cloud_manager_t *manager);

/**
 * @brief Sends a temperature value to the cloud
 */
esp_err_t cloud_manager_send_temperature(cloud_manager_t *manager, float temp);

/**
 * @brief Sends an alarm event to the cloud
 */
esp_err_t cloud_manager_send_alarm(cloud_manager_t *manager);

/**
 * @brief Frees the memory
 */
void cloud_manager_delete(cloud_manager_t *manager);