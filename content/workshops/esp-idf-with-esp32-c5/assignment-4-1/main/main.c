#include <stdio.h>

   #include "freertos/FreeRTOS.h"
   #include "freertos/task.h"

   #include "esp_log.h"
   #include "esp_sleep.h"

   static const char *TAG = "deep_sleep";
static RTC_DATA_ATTR int boot_count = 0;

static void log_wakeup_cause(void)
   {
       uint32_t causes = esp_sleep_get_wakeup_causes();

       if (causes & BIT(ESP_SLEEP_WAKEUP_TIMER)) {
           ESP_LOGI(TAG, "Woken up by the RTC timer");
       } else {
           ESP_LOGI(TAG, "First boot or reset (not a timer wakeup)");
       }
   }

 void app_main(void)
   {
       boot_count++;
       ESP_LOGI(TAG, "Boot count: %d", boot_count);

       log_wakeup_cause();
       const int wakeup_time_sec = 10;
       ESP_LOGI(TAG, "Entering Deep-sleep for %d seconds", wakeup_time_sec);
       ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
       vTaskDelay(pdMS_TO_TICKS(100));
       esp_deep_sleep_start();
   }