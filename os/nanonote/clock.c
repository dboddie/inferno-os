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

static void
clockintr(Ureg *, void *)
{
}

void
clockinit(void)
{
    /* Propagate the TCU clock by clearing the appropriate bit */
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~CGU_TCU;

    JZTimer *tm = (JZTimer *)(TIMER_BASE | KSEG1);

    /* Disable all timers, set their stop and mask bits, clear their flag bits */
    tm->counter_enable_clear = TimerCounterAll;
    tm->stop_set = TimerCounterAll;
    tm->mask_set = TimerCounterAll;
    tm->flag_clear = TimerCounterAll;

    /* Clear timer0 stop bit, scale the clock speed down and use external source,
       set the half and full registers. */
    tm->stop_clear = TimerCounter0;
    tm->control0 = TimerPrescale256 | TimerSourceExt;
    tm->data_half0 = 0;
    tm->data_full0 = 12000000 / 256;

    /* Reset counter0, clear its full match flag, unmask its interrupt to
       enable it, then enable the counter itself */
    tm->counter0 = 0;
    tm->flag_clear = TimerCounter0;
    tm->mask_clear = TimerCounter0;
    tm->counter_enable_set = TimerCounter0;
}

extern void fbprint(unsigned int v, unsigned int l, unsigned int colour);
void
clocktest(void)
{
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    print("%8.8lux\n", ic->source);
    print("%8.8lux\n", ic->mask);
    ic->mask_clear = InterruptTCU0;
    print("%8.8lux\n", ic->mask);
    print("%8.8lux\n", ic->pending);

    intron(INTMASK);
    spllo(); /* Enable interrupts */
    print("%8.8lux\n", getstatus());
    print("%8.8lux\n", getcause());

    JZTimer *tm = (JZTimer *)(TIMER_BASE | KSEG1);
    print("%8.8lux\n", tm->mask);

    for (;;) {
        fbprint(tm->counter0, 10, 0xffffff);
    }
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
timerset(uvlong next)
{
    uvlong now;
    long period;

    now = fastticks(nil);
    period = next - now;
    if (period < MinPeriod)
        period = MinPeriod;
    else if (period > MaxPeriod)
        period = MaxPeriod;
}

void mdelay(uint delay)
{
}
