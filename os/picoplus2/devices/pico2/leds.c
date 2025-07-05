#include "../rp2350.h"

void setup_led(void)
{
    // Use function 5 (SIO) for the LED GPIO (25). This function is needed to
    // drive GPIOs, otherwise function 0 can be used to read their states.
    GPIOctrl *gpio25 = (GPIOctrl *)GPIO25_IO_ADDR;
    gpio25->ctrl = 5;

    *(unsigned int *)GPIO25_PAD_ADDR = PADS_PUE;

    SIOregs *sio = (SIOregs *)SIO_BASE;
    sio->gpio_oe_set = 1 << 25;
    sio->gpio_out_clr = (1 << 25);
}

int get_led(void)
{
    SIOregs *sio = (SIOregs *)SIO_BASE;
    return sio->gpio_in & (1 << 25);
}

void set_led(int on)
{
    SIOregs *sio = (SIOregs *)SIO_BASE;
    if (on)
        sio->gpio_out_set = (1 << 25);
    else
        sio->gpio_out_clr = (1 << 25);
}
