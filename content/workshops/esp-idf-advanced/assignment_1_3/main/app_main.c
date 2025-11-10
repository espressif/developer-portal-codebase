#include "cloud_manager.h"
#include "temperature_sensor.h"
#include "alarm.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"

#define TEMPERATURE_MEAS_PERIOD_IN_S 5

void app_main(void)
{
    ESP_LOGI("APP", "Starting...");

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    temperature_sensor_t *sensor = temperature_sensor_create();
    alarm_t *alarm = alarm_create();
    cloud_manager_t *cloud = cloud_manager_create();

    ESP_LOGI("APP", "Connecting...");
    ESP_ERROR_CHECK(cloud_manager_connect(cloud));
    ESP_LOGI("APP", "Connected!");

    int64_t last_time = esp_timer_get_time();
    bool previous_alarm_set = false;

    while (1) {
        int64_t now = esp_timer_get_time();
        if ((now - last_time) >= TEMPERATURE_MEAS_PERIOD_IN_S * 1e6) {
            last_time = now;
            float temp;
            if (temperature_sensor_read_celsius(sensor, &temp) == ESP_OK) {
                cloud_manager_send_temperature(cloud, temp);
            } else {
                ESP_LOGW("APP", "Failed to read temperature");
            }
        }

        bool alarm_state = is_alarm_set(alarm);
        if (alarm_state && !previous_alarm_set) {
            ESP_LOGI("APP", "Alarm ON!!");
            cloud_manager_send_alarm(cloud);
        }
        previous_alarm_set = alarm_state;

        vTaskDelay(10);
    }

    cloud_manager_disconnect(cloud);

    cloud_manager_delete(cloud);
    temperature_sensor_delete(sensor);
    alarm_delete(alarm);
}
