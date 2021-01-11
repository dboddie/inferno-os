#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"

enum {
    SystimerFreq    = 32768,
    MaxPeriod       = SystimerFreq/HZ,
    MinPeriod       = SystimerFreq/(100*HZ)
};

static void
clockintr(Ureg * ureg, void *)
{
}

void
clockinit(void)
{
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
