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

void
clockintr(Ureg *ureg)
{
    static int i = 0;
    JZTimer *tm = (JZTimer *)(TIMER_BASE | KSEG1);
    tm->flag_clear = TimerCounter0;

    fbprint((unsigned int)ureg, 0, 0x80ff80);
    hzclock(ureg);
    fbprint(i++, 1, 0x0080ff);
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
    tm->data_full0 = MaxPeriod;

    /* Reset counter0, clear its full match flag, unmask its interrupt to
       enable it, then enable the counter itself */
    tm->counter0 = 0;
    tm->flag_clear = TimerCounter0;
    tm->mask_clear = TimerCounter0;
    tm->counter_enable_set = TimerCounter0;

    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    ic->mask_clear = InterruptTCU0;
    intron(INTMASK);
}

extern void fbprint(unsigned int v, unsigned int l, unsigned int colour);
void
clocktest(void)
{
    spllo(); /* Enable interrupts */

    JZTimer *tm = (JZTimer *)(TIMER_BASE | KSEG1);

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
