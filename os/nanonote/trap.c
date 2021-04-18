#include "u.h"
#include "ureg.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"

#include "hardware.h"

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
