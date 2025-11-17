#ifndef ALARM_H
#define ALARM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alarm_t alarm_t;  // Opaque alarm struct

/**
 * @brief Initialize the alarm system with internal constants.
 * 
 * @return Pointer to the alarm instance, or NULL on failure.
 */
alarm_t* alarm_create(void);

/**
 * @brief Check whether the alarm is currently set.
 * 
 * This is time-aware: it only reevaluates periodically.
 * 
 * @param alarm Pointer to the alarm instance.
 * @return true if alarm is active, false otherwise.
 */
bool is_alarm_set(alarm_t *alarm);

/**
 * @brief Delete the alarm instance and free resources.
 * 
 * @param alarm Pointer to the alarm instance to delete.
 */
void alarm_delete(alarm_t *alarm);

#ifdef __cplusplus
}
#endif

#endif // ALARM_H
