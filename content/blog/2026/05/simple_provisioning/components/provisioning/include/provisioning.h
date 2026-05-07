#pragma once

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define WIFI_CONNECTED_EVENT    BIT0
#define WIFI_FAIL_EVENT         BIT1

typedef struct {
    EventGroupHandle_t wifi_event_group;
} provisioning_conf_t;

esp_err_t provisioning_start(provisioning_conf_t *configuration);
