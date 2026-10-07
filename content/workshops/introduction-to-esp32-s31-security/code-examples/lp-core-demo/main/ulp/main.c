/*
 * LP-core firmware: count simulated events and emit a short GPIO marker for
 * each count while the HP cores sleep. Exported variables remain in LP memory
 * during deep sleep and are readable by the HP application.
 */
#include <stdint.h>

#include "sdkconfig.h"
#include "ulp_lp_core.h"
#include "ulp_lp_core_gpio.h"
#include "ulp_lp_core_interrupts.h"
#include "ulp_lp_core_lp_timer_shared.h"
#include "ulp_lp_core_utils.h"

volatile uint32_t pulse_count;
volatile uint32_t total_pulse_count;
volatile uint32_t last_wakeup_pulse_count;
volatile uint32_t last_wakeup_total_count;
volatile uint32_t hp_wakeup_count;
volatile uint32_t lp_stage;
volatile uint32_t last_wakeup_stage;
volatile uint32_t lp_paused;

void LP_CORE_ISR_ATTR ulp_lp_core_lp_timer_intr_handler(void)
{
    ulp_lp_core_lp_timer_intr_clear();

    /* Stage 1: the LP timer fired. */
    lp_stage = 1;

    /*
     * Deep-sleep entry can hold the RTC pad after the HP-side setup. Release
     * that hold from the LP domain and restore the output path for each edge.
     */
    rtcio_ll_force_unhold_all();
    rtcio_ll_force_hold_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_init(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_set_output_mode(CONFIG_DEMO_TRACE_GPIO,
                                     RTCIO_LL_OUTPUT_NORMAL);
    ulp_lp_core_gpio_pullup_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_pulldown_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_output_enable(CONFIG_DEMO_TRACE_GPIO);

    pulse_count++;
    total_pulse_count++;

    ulp_lp_core_gpio_set_level(CONFIG_DEMO_TRACE_GPIO, 1);
    ulp_lp_core_delay_us(CONFIG_DEMO_TRACE_PULSE_WIDTH_MS * 1000);
    ulp_lp_core_gpio_set_level(CONFIG_DEMO_TRACE_GPIO, 0);

    /* Stage 2: the event has been counted and marked on GPIO4. */
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
        lp_paused = 1;
        ulp_lp_core_wakeup_main_processor();
    } else {
        ulp_lp_core_lp_timer_set_wakeup_time(
            CONFIG_DEMO_PULSE_INTERVAL_MS * 1000);
    }
}

void LP_CORE_ISR_ATTR ulp_lp_core_lp_pmu_intr_handler(void)
{
    ulp_lp_core_sw_intr_clear();

    if (lp_paused) {
        lp_paused = 0;
        ulp_lp_core_gpio_set_level(CONFIG_DEMO_TRACE_GPIO, 0);
        ulp_lp_core_lp_timer_set_wakeup_time(
            CONFIG_DEMO_PULSE_INTERVAL_MS * 1000);
    }
}

int main(void)
{
    rtcio_ll_force_unhold_all();
    rtcio_ll_force_hold_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_init(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_set_output_mode(CONFIG_DEMO_TRACE_GPIO,
                                     RTCIO_LL_OUTPUT_NORMAL);
    ulp_lp_core_gpio_input_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_pullup_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_pulldown_disable(CONFIG_DEMO_TRACE_GPIO);
    ulp_lp_core_gpio_set_level(CONFIG_DEMO_TRACE_GPIO, 0);
    ulp_lp_core_gpio_output_enable(CONFIG_DEMO_TRACE_GPIO);

    ulp_lp_core_intr_enable();
    ulp_lp_core_sw_intr_enable(true);
    ulp_lp_core_lp_timer_set_wakeup_time(CONFIG_DEMO_PULSE_INTERVAL_MS * 1000);
    ulp_lp_core_lp_timer_intr_enable(true);

    while (1) {
        /* Retain GPIO state and sleep until the next LP timer interrupt. */
        asm volatile("wfi");
    }

    return 0;
}
