#include "esp_err.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_EVENT    BIT0
#define WIFI_FAIL_EVENT         BIT1

// typedef struct provisioning_conf provisioning_conf_t;

// Public struct (fields visible to users of this header)
typedef struct {
    EventGroupHandle_t wifi_event_group;
} provisioning_conf_t;


esp_err_t start_provisioning(provisioning_conf_t * configuration);
static void get_device_service_name(char *service_name, size_t max);
static void wifi_prov_print_qr(const char *name, const char *transport);