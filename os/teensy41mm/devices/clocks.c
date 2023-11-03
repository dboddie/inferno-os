#include "teensy41mm.h"

void setup_system_clock(void)
{
    // Divide the peripheral clock by 1 and enable it.
    *(unsigned int *)CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_PODF_MASK;
    *(unsigned int *)CCM_CSCMR1 |= CCM_CSCMR1_PERCLK_SEL;
    // Divide the UART clock by 1 and enable it.
    *(unsigned int *)CCM_CSCDR1 &= ~CCM_CSCDR1_UART_CLK_PODF_MASK;
    *(unsigned int *)CCM_CSCDR1 |= CCM_CSCDR1_UART_CLK_SEL;
}

void start_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    /* The default CPU clock is 396MHz, so set a reset value for 0.01s. */
    tick->reload = 3960000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock; 2=SysTick exception, 1=enable */
}

void pause_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    tick->control ^= 1;
//    wrstr("SYSTICK ctl="); wrhex(tick->control); newline();
}

void _wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int initial = tick->current;
    int diff = delay_ms * 396000;
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
    int limit_ms = tick->reload / 396000;
    while (delay_ms > limit_ms) {
        _wait_ms(limit_ms);
        delay_ms -= limit_ms;
    }
    _wait_ms(delay_ms);
}
