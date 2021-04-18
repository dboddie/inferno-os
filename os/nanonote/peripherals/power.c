#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"

void powerinit(void)
{
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
    ic->mask_clear = InterruptGPIO3;

    GPIO *d_mask = (GPIO *)(GPIO_PORT_D_INTMASK | KSEG1);
    GPIO *d_trig = (GPIO *)(GPIO_PORT_D_TRIG | KSEG1);
    GPIO *d_func = (GPIO *)(GPIO_PORT_D_FUNC | KSEG1);
    GPIO *d_sel = (GPIO *)(GPIO_PORT_D_SEL | KSEG1);
    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
    GPIO *d_flag = (GPIO *)(GPIO_PORT_D_FLAG | KSEG1);
    d_mask->set = GPIO_Power;
    d_trig->clear = GPIO_Power;
    d_func->clear = GPIO_Power;
    d_sel->set = GPIO_Power;
    d_dir->clear = GPIO_Power;
    d_flag->clear = GPIO_Power;

    d_mask->clear = GPIO_Power;
}

void powerintr(void)
{
    GPIO *d_flag = (GPIO *)(GPIO_PORT_D_FLAG | KSEG1);
    static int j = 0;

    if (d_flag->data & GPIO_Power) {
        /* Clear all flags */
        d_flag->clear = GPIO_Power;
    }
}
