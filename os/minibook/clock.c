#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "hardware.h"

enum {
    SystimerFreq    = 12000000 / 256,
    MaxPeriod       = SystimerFreq/HZ,
    MinPeriod       = SystimerFreq/(100*HZ)
};

extern void hzclock(Ureg *);

void
clockintr(Ureg *ureg)
{
    JZTimer *timer = (JZTimer *)(TIMER_BASE0 | KSEG1);
    static int i = 0;
    static int j = 0;

    if (timer->control & TimerUnder) {
        timer->control &= ~TimerUnder;
        hzclock(ureg);
    }
}

void
clockinit(void)
{
    /* Propagate the OST clock by clearing the appropriate bit */
    *(ulong*)(CGU_MSCR | KSEG1) &= ~CGU_OST;

    /* Disable all timers */
    *(ulong *)TIMER_OTER = 0;

    JZTimer *timer = (JZTimer *)(TIMER_BASE0 | KSEG1);
    timer->control = TimerRTCCLK;
    timer->data = timer->counter = 32768;

    /* Enable timer 0 */
    *(ulong *)TIMER_OTER = Timer0;

    /* Enable interrupts for the OST0 timer */
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    ic->mask_clear = InterruptOST0;
    intron(INTMASK);
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

void mdelay(uint /* delay */)
{
}
