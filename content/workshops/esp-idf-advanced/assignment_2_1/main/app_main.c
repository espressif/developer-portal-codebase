#include "cloud_manager.h"
#include "temperature_sensor.h"
#include "alarm.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"

#define TEMPERATURE_MEAS_PERIOD_US (5 * 1000000)
#define ALARM_CHECK_PERIOD_US      (200 * 1000)

ESP_EVENT_DEFINE_BASE(TEMP_EVENT_BASE);
ESP_EVENT_DEFINE_BASE(ALARM_EVENT_BASE);

static bool previous_alarm_set = false;

typedef enum {
    TEMP_EVENT_MEASURE,
} temp_event_id_t;

typedef enum {
    ALARM_EVENT_CHECK,
} alarm_event_id_t;

static temperature_sensor_t *sensor = NULL;
static alarm_t *alarm = NULL;
static cloud_manager_t *cloud = NULL;

static void temp_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    float temp;
    if (temperature_sensor_read_celsius(sensor, &temp) == ESP_OK) {
        cloud_manager_send_temperature(cloud, temp);
    } else {
        ESP_LOGW("APP", "Failed to read temperature");
    }
}

static void alarm_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    
    bool alarm_state = is_alarm_set(alarm);
    if (alarm_state && !previous_alarm_set) {
        ESP_LOGI("APP","Alarm ON!!");
        cloud_manager_send_alarm(cloud);
    }
    previous_alarm_set = alarm_state;
}

static void temp_timer_callback(void* arg) {
    esp_event_post(TEMP_EVENT_BASE, TEMP_EVENT_MEASURE, NULL, 0, 0);
}

static void alarm_timer_callback(void* arg) {
    esp_event_post(ALARM_EVENT_BASE, ALARM_EVENT_CHECK, NULL, 0, 0);
}

void app_main(void)
{
    ESP_LOGI("APP", "Starting...");

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    sensor = temperature_sensor_create();
    alarm = alarm_create();
    cloud = cloud_manager_create();

    ESP_LOGI("APP","Connecting...");
    ESP_ERROR_CHECK(cloud_manager_connect(cloud));
    ESP_LOGI("APP","Connected");

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(TEMP_EVENT_BASE, TEMP_EVENT_MEASURE, temp_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(ALARM_EVENT_BASE, ALARM_EVENT_CHECK, alarm_event_handler, NULL));

    // Create and start periodic timers
    const esp_timer_create_args_t temp_timer_args = {
        .callback = &temp_timer_callback,
        .name = "temp_timer"
    };
    esp_timer_handle_t temp_timer;
    ESP_ERROR_CHECK(esp_timer_create(&temp_timer_args, &temp_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(temp_timer, TEMPERATURE_MEAS_PERIOD_US));

    const esp_timer_create_args_t alarm_timer_args = {
        .callback = &alarm_timer_callback,
        .name = "alarm_timer"
    };
    esp_timer_handle_t alarm_timer;
    ESP_ERROR_CHECK(esp_timer_create(&alarm_timer_args, &alarm_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(alarm_timer, ALARM_CHECK_PERIOD_US));

    // The main task can now just sleep
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Cleanup (unreachable in this example)
    cloud_manager_disconnect(cloud);
    cloud_manager_delete(cloud);
    temperature_sensor_delete(sensor);
    alarm_delete(alarm);
}
