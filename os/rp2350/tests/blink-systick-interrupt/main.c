#include "u.h"
#include "ureg.h"
#include "../../devices/rp2350.h"

#define SHPR3 0xe000ed20
#define SHCSR_ADDR 0xe000ed24
#define SHCSR_USGFAULTENA (1 << 18)

extern void setup_led(void);
extern void set_led(int);

int state = 0;

void start_timer(void)
{
    // Make SysTick low priority by assigning a high number to it.
    *(int *)SHPR3 = (*(int *)SHPR3 & 0x00ffffff) | 0xe0000000;

    /* Enable the usage fault exception. */
    *(int *)SHCSR_ADDR |= SHCSR_USGFAULTENA;

    SysTick *tick = (SysTick *)SYSTICK;
    /* The scaled system clock is 12MHz, so set a reset value for 1s. */
    tick->reload = 12000000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
}

void main(void)
{
    setup_led();

    start_timer();

    setprimask(0);
    spllo();

    for (;;) {}
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
    set_led(state);
    state = 1 - state;
}
