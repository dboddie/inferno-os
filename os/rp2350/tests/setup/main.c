#include "u.h"
#include "ureg.h"
#include "../../devices/rp2350.h"
#include "../../devices/fns.h"
#include "../../../cortexm/thumb2.h"

int state = 0, count = 0;

extern void spllo(void);
extern void setup_clocks(void);

void main(void)
{
    setup_led();
    setup_clocks();
    setup_uart();
    spllo();

    UART *uart1 = (UART *)UART1_BASE;
    const char s[] = "Hello\r\n";

    for (int i = 0; i < 7; i++) {
        uart1->dr = s[i];
        while (uart1->fr & UARTFR_TXFF);
    }

    start_timer();

    SysTick *tick = (SysTick *)SYSTICK;

    for (;;) {
        for (int j = 0; j < 1000000; j++);

        wrhex(tick->current);
        newline();
    }
}

void hard_fault(int)
{
}

void usage_fault(int)
{
}

void bus_fault(int)
{
}

void trap_dummy(int)
{
}

void switcher(Ureg *)
{
}

void dumpregs(Ureg *)
{
}

void systick(void)
{
    count++;
    if (count < 100) return;
    count = 0;

    set_led(state);
    state = 1 - state;
}
