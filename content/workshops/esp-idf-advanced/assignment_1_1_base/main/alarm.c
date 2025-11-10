#include <stdlib.h>
#include "alarm.h"
#include "esp_random.h"
#include "esp_timer.h" // for esp_timer_get_time()

// Define internal behavior constants
#define ALARM_THRESHOLD_PERCENT     2     // 2% chance
#define ALARM_REFRESH_INTERVAL_MS   1000   // reevaluate every 1000 ms

// Internal alarm structure (hidden from user)
struct alarm_t {
    int64_t last_check_time_us;
    bool last_state;
};

alarm_t* alarm_create(void)
{
    alarm_t *alarm = malloc(sizeof(alarm_t));
    if (!alarm) return NULL;

    alarm->last_check_time_us = 0;
    alarm->last_state = false;

    return alarm;
}

bool is_alarm_set(alarm_t *alarm)
{
    if (!alarm) return false;

    int64_t now_us = esp_timer_get_time();
    int64_t elapsed_us = now_us - alarm->last_check_time_us;

    if (elapsed_us >= ALARM_REFRESH_INTERVAL_MS * 1000) {
        uint32_t rand_val = esp_random() % 100;
        alarm->last_state = rand_val < ALARM_THRESHOLD_PERCENT;
        alarm->last_check_time_us = now_us;
    }

    return alarm->last_state;
}

void alarm_delete(alarm_t *alarm)
{
    if (alarm) {
        free(alarm);
    }
}
