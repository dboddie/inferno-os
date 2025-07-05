#include "u.h"
#include "ureg.h"

extern void setup_led(void);
extern void set_led(int);

void main(void)
{
    setup_led();
    set_led(1);

/*
    GPIOctrl *gpio25 = (GPIOctrl *)GPIO25_IO_ADDR;
    gpio25->ctrl = 5;

    *(unsigned int *)GPIO25_PAD_ADDR = PADS_PUE | PADS_DRIVE_12mA;

    SIOregs *sio = (SIOregs *)SIO_BASE;
    sio->gpio_oe_set = 1 << 25;
    sio->gpio_out_set = (1 << 25);
*/
    for (;;) {}
}

void hard_fault(int)
{
}

void usage_fault(int)
{
}

void bus_fault(int)
{
}

void trap_dummy(int)
{
}

void switcher(Ureg *)
{
}

void dumpregs(Ureg *)
{
}
