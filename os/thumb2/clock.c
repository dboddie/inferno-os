#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"

#include "devices/fns.h"

enum {
    SystimerFreq    = 100,
    MaxPeriod       = SystimerFreq / HZ,
    MinPeriod       = SystimerFreq / (100*HZ),
};

void clockinit(void)
{
    start_timer();
}

void systick(void)
{
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
}
