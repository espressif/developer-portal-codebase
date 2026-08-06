#include <stdio.h>
   #include <inttypes.h>

   #include "esp_log.h"
   #include "esp_sleep.h"

   #include "ulp_lp_core.h"
   #include "ulp_main.h"

   static const char *TAG = "lp_core";

   extern const uint8_t lp_core_main_bin_start[] asm("_binary_ulp_main_bin_start");
   extern const uint8_t lp_core_main_bin_end[]   asm("_binary_ulp_main_bin_end");

 static void start_lp_core(void)
   {
       ESP_ERROR_CHECK(ulp_lp_core_load_binary(lp_core_main_bin_start,
                       (lp_core_main_bin_end - lp_core_main_bin_start)));

       ulp_lp_core_cfg_t cfg = {
           .wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
           .lp_timer_sleep_duration_us = 1000000,
       };

       ESP_ERROR_CHECK(ulp_lp_core_run(&cfg));
   }

 void app_main(void)
   {
       uint32_t causes = esp_sleep_get_wakeup_causes();

       if (causes & BIT(ESP_SLEEP_WAKEUP_ULP)) {
           ESP_LOGI(TAG, "Woken up by the LP core after %" PRIu32 " counts", ulp_counter);
           return;
       }

       ESP_LOGI(TAG, "First boot, starting the LP core");
       start_lp_core();

       ESP_LOGI(TAG, "Allowing the LP core to wake the main CPU");
       ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());

       ESP_LOGI(TAG, "Entering Deep-sleep, the LP core keeps counting");
       esp_deep_sleep_start();
   }