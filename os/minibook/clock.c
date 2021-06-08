#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "hardware.h"

enum {
    SystimerFreq    = 32768,
    MaxPeriod       = SystimerFreq/HZ,
    MinPeriod       = SystimerFreq/(100*HZ)
};

extern void hzclock(Ureg *);

void
clockintr(Ureg *ureg)
{
    JZTimer *timer0 = (JZTimer *)(TIMER_BASE0 | KSEG1);

    if (timer0->control & TimerUnder) {
        timer0->control &= ~TimerUnder;
        hzclock(ureg);
    }
}

static uint msec_counter = 0;

/* Called by the timer interrupt handler in trapintr */
void
clockmsec(void)
{
    msec_counter++;
}

void
clockinit(void)
{
    /* Propagate the OST clock by clearing the appropriate bit */
    *(ulong*)(CGU_MSCR | KSEG1) &= ~CGU_OST;

    /* Set up the OST to use the RTC and enable underflow interrupts */
    *(ulong *)(TIMER_OTER | KSEG1) = 0;

    JZTimer *timer0 = (JZTimer *)(TIMER_BASE0 | KSEG1);
    timer0->control = TimerUnderIntEn | TimerRTCCLK;
    timer0->data = timer0->counter = 327;

    JZTimer *timer1 = (JZTimer *)(TIMER_BASE1 | KSEG1);
    timer1->control = TimerUnderIntEn | TimerRTCCLK;
    timer1->data = timer1->counter = 32;

    /* Enable timers 0 and 1 */
    *(ulong *)(TIMER_OTER | KSEG1) = Timer0 | Timer1;

    /* Enable interrupts for the OST0 and OST1 timers */
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    ic->mask_clear = InterruptOST0 | InterruptOST1;

    /* Enable the relevant interrupt in the core */
    intron(0x400);
}

void
clockcheck(void)
{
    return;
}

uvlong
fastticks(uvlong *hz)
{
    if(hz)
        *hz = HZ;
    return m->ticks;
}

void
timerset(uvlong /* next */)
{/*
    uvlong now;
    long period;

    now = fastticks(nil);
    period = next - now;
    if (period < MinPeriod)
        period = MinPeriod;
    else if (period > MaxPeriod)
        period = MaxPeriod;*/
}

void mdelay(uint delay)
{
    uint now = msec_counter;
    uint next = now + delay;

    /* If an integer overflow occurred, wait until the counter wraps around */
    while (next < now)
        now = msec_counter;

    while (now < next)
        now = msec_counter;
}
