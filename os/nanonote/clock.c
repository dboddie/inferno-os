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
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~(CGU_TCU | CGU_RTC);
    fbprint(*(ulong*)(CGU_CLKGR | KSEG1), 0, 0xff0000);

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

    // Reset counter0, clear its full match flag, unmask its interrupt, then
    // enable it
    tm->counter0 = 0;
    tm->flag_clear = TimerCounter0;
    tm->mask_clear = TimerCounter0;
    tm->counter_enable_set = TimerCounter0;
}

extern void fbprint(unsigned int v, unsigned int l, unsigned int colour);
void
clocktest(void)
{
    JZTimer *tm = (JZTimer *)(TIMER_BASE | KSEG1);

    fbprint(tm->stop, 1, 0xffffff);
    fbprint(tm->counter_enable, 2, 0xffffff);
    fbprint(tm->mask, 3, 0xffffff);

    fbprint(tm->control0, 4, 0xffff00);
    fbprint(tm->data_half0, 5, 0xffff00);
    fbprint(tm->data_full0, 6, 0xffff00);

    ulong i = 0;
    for (;;) {
        fbprint(tm->counter0, 7, 0xffffff);
        fbprint(i++, 8, 0xff);
    }
/*
    RTC *rtc = (RTC *)(RTC_BASE | KSEG1);
    fbprint(rtc->control, 1, 0xffffff);
    fbprint(rtc->second, 2, 0xffff00);
    fbprint(rtc->second_alarm, 3, 0xffff00);
    fbprint(rtc->regulator, 4, 0xff00ff);

    for (;;) {
        fbprint(rtc->second, 5, 0xffffff);
    }
*/
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
