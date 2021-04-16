#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "hardware.h"

void kbdinit(void)
{
/*
    GPIO *c_mask = (GPIO *)(GPIO_PORT_C_INTMASK | KSEG1);
    GPIO *c_trig = (GPIO *)(GPIO_PORT_C_TRIG | KSEG1);
    GPIO *c_func = (GPIO *)(GPIO_PORT_C_FUNC | KSEG1);
    GPIO *c_sel = (GPIO *)(GPIO_PORT_C_SEL | KSEG1);
    GPIO *c_dir = (GPIO *)(GPIO_PORT_C_DIR | KSEG1);
    GPIO *c_flag = (GPIO *)(GPIO_PORT_C_FLAG | KSEG1);
    c_mask->set = 0x0003fc00;
    c_trig->clear = 0x0003fc00;
    c_func->clear = 0x0003fc00;
    c_sel->set = 0x0003fc00;
    c_dir->clear = 0x0003fc00;
    c_flag->clear = 0x0003fc00;

    c_mask->clear = 0x0003fc00;

    GPIO *d_pull = (GPIO *)(GPIO_PORT_D_PULL | KSEG1);
    d_pull->clear = 0x25fc0000;

    GPIO *d_mask = (GPIO *)(GPIO_PORT_D_INTMASK | KSEG1);
    GPIO *d_trig = (GPIO *)(GPIO_PORT_D_TRIG | KSEG1);
    GPIO *d_func = (GPIO *)(GPIO_PORT_D_FUNC | KSEG1);
    GPIO *d_sel = (GPIO *)(GPIO_PORT_D_SEL | KSEG1);
    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
    GPIO *d_flag = (GPIO *)(GPIO_PORT_D_FLAG | KSEG1);
    d_mask->set = 0x25fc0000;
    d_trig->clear = 0x25fc0000;
    d_func->clear = 0x25fc0000;
    d_sel->set = 0x25fc0000;
    d_dir->clear = 0x25fc0000;
    d_flag->clear = 0x25fc0000;

    d_mask->clear = 0x25fc0000;
*/

    kbdq = qopen(4*1024, 0, 0, 0);
    qnoblock(kbdq, 1);
}

void kbdpoll(void)
{
/*
    GPIO *d_flag = (GPIO *)(GPIO_PORT_D_FLAG | KSEG1);
//    GPIO *c_flag = (GPIO *)(GPIO_PORT_C_FLAG | KSEG1);
//    GPIO *d_pin = (GPIO *)(GPIO_PORT_D_PIN | KSEG1);
    static int j = 0;
    static int k = 0;
*/
}
