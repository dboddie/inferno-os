#include "apollo3.h"

void start_timer(void)
{
    *(unsigned int *)CLKGEN_CLKKEY = 0x47;
    /* Set the HFRC (high frequency reference clock) divisor to 1. */
    *(unsigned int *)CLKGEN_CCTRL = 0;
    /* Enable the HFRC. */
    *(unsigned int *)CLKGEN_CLKOUT |= CLKGEN_CLKSEL_HFRC | CLKGEN_CKEN;
    *(unsigned int *)CLKGEN_CLKKEY = 0;

    SysTick *tick = (SysTick *)SYSTICK;
    /* The scaled system clock is 48MHz, so set a reset value for 0.01s. */
    tick->reload = 480000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
}

void pause_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    tick->control ^= 1;
    wrstr("SYSTICK ctl="); wrhex(tick->control); newline();
}

/* With a 48 MHz clock, a count of 48 is 1 microsecond, 48000 is 1ms. */
void _wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int initial = tick->current;
    int diff = delay_ms * 48000;
    int next;

    if (diff > initial) {
        /* Wait while the counter increases until underflow occurs. */
        while (tick->current < initial);
        next = tick->reload - (diff - initial);
    } else
        next = initial - diff;

    while (tick->current > next);
}

void wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int limit_ms = tick->reload / 48000;
    while (delay_ms > limit_ms) {
        _wait_ms(limit_ms);
        delay_ms -= limit_ms;
    }
    _wait_ms(delay_ms);
}
