#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"

#include "hardware.h"

void kbdinit(void)
{
//    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
//    d_dir->clear = 0x0003fc00;

    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    ic->mask_clear = InterruptGPIO0 | InterruptGPIO1 | InterruptGPIO2 | InterruptGPIO3;

    GPIO *d_mask = (GPIO *)(GPIO_PORT_D_INTMASK | KSEG1);
    GPIO *d_trig = (GPIO *)(GPIO_PORT_D_TRIG | KSEG1);
    GPIO *d_func = (GPIO *)(GPIO_PORT_D_FUNC | KSEG1);
    GPIO *d_sel = (GPIO *)(GPIO_PORT_D_SEL | KSEG1);
    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
    GPIO *d_flag = (GPIO *)(GPIO_PORT_D_FLAG | KSEG1);
    d_mask->set = 0x20000000;
    d_trig->clear = 0x20000000;
    d_func->clear = 0x20000000;
    d_sel->set = 0x20000000;
    d_dir->clear = 0x20000000;
    d_flag->clear = 0x20000000;

    d_mask->clear = 0x20000000;

    kbdq = qopen(4*1024, 0, 0, 0);
    qnoblock(kbdq, 1);
}
