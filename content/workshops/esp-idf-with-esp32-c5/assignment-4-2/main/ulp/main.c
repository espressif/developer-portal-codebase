 #include "ulp_lp_core_utils.h"

   /* Shared with the main CPU */
   volatile uint32_t counter = 0;

   int main(void)
   {
       counter++;

       /* After 5 runs, wake up the main CPU */
       if (counter >= 5) {
           ulp_lp_core_wakeup_main_processor();
       }

       return 0;
   }