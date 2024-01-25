#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "thumb2.h"

#include "devices/fns.h"

enum {
    SystimerFreq    = 396000000,
    MaxPeriod       = SystimerFreq / HZ,
    MinPeriod       = SystimerFreq / (100*HZ),
};

void clockinit(void)
{
    setup_system_clock();
    start_timer();
    intron();
}

void clockcheck(void)
{
}

uvlong fastticks(uvlong *hz)
{
    if(hz)
        *hz = HZ;
    return m->ticks;
}

void timerset(uvlong next)
{
    uvlong now;
    long period;

    now = fastticks(nil);
    period = next - now;
    if (period < MinPeriod)
        period = MinPeriod;
    else if (period > MaxPeriod)
        period = MaxPeriod;

    /* TODO: Actually set a timer to go off at now + period. */
}
