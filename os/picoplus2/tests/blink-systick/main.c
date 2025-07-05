#include "u.h"
#include "ureg.h"
#include "../../devices/rp2350.h"

extern void setup_led(void);
extern void set_led(int);

int state = 0, count = 0;

void start_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    /* The scaled system clock is 48MHz, so set a reset value for 0.25s. */
    tick->reload = 12000000 - 1;
    tick->current = 0;
    tick->control = 5;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
}

void main(void)
{
    setup_led();

    start_timer();

    spllo();

    for (;;) {
        SysTick *tick = (SysTick *)SYSTICK;
        if (tick->current < 6000000)
            set_led(1);
        else
            set_led(0);
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
