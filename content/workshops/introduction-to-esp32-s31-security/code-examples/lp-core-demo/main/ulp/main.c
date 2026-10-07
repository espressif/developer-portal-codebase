/*
 * LP-core firmware: each LP timer wakeup represents one simulated button
 * press. The variables below are exported to the HP application through the
 * generated ulp_main.h header and remain in LP memory during deep sleep.
 */
#include <stdint.h>

#include "sdkconfig.h"
#include "ulp_lp_core.h"
#include "ulp_lp_core_utils.h"

volatile uint32_t pulse_count;
volatile uint32_t total_pulse_count;
volatile uint32_t last_wakeup_pulse_count;
volatile uint32_t last_wakeup_total_count;
volatile uint32_t hp_wakeup_count;
volatile uint32_t lp_stage;
volatile uint32_t last_wakeup_stage;

int main(void)
{
    /* Stage 1: the LP timer fired and started the LP core. */
    lp_stage = 1;

    pulse_count++;
    total_pulse_count++;
    /* Stage 2: this simulated pulse has been recorded. */
    lp_stage = 2;

    if (pulse_count >= CONFIG_DEMO_PULSES_PER_WAKEUP) {
        /* Stage 3: the configured pulse threshold has been reached. */
        lp_stage = 3;
        last_wakeup_pulse_count = pulse_count;
        last_wakeup_total_count = total_pulse_count;
        pulse_count = 0;
        hp_wakeup_count++;

        /* Stage 4: preserve the final LP stage and request an HP wakeup. */
        lp_stage = 4;
        last_wakeup_stage = lp_stage;
        ulp_lp_core_wakeup_main_processor();
    }

    /*
     * Returning halts the LP core. The LP timer starts it again after the
     * configured interval, avoiding a power-hungry busy-wait loop.
     */
    return 0;
}
