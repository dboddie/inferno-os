#include "u.h"
#include "ureg.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"

#include "hardware.h"

void
trapinit(void)
{
/*	memmove((ulong*)UTLBMISS, (ulong*)vector0, 0x80);
	memmove((ulong*)XEXCEPTION, (ulong*)vector0, 0x80);
	memmove((ulong*)CACHETRAP, (ulong*)vector100, 0x80);*/
	memmove((ulong*)EXCEPTION, (ulong*)vector180, 0x80);
	memmove((ulong*)INTERRUPT, (ulong*)vector200, 0x80);

	setstatus(getstatus() & ~BEV);
}

void trapintr(Ureg *ureg)
{
    if (ureg->cause & 0x400) {
        InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
        if (ic->pending & InterruptTCU0) {
            clockintr(ureg);
            kbdpoll();
        }
        if (ic->pending & InterruptGPIO3) {
            powerintr();
        }
    }
}

void trapexc(Ureg *ureg)
{
    fbprint(getepc(), 10, 0xff0000);
    fbprint(getcause(), 11, 0xff8000);
    fbprint(getstatus(), 12, 0xffff00);
    for (;;) {}
}
