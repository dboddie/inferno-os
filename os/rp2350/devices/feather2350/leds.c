#include "../rp2350.h"

void setup_led(void)
{
    // Use function 5 (SIO) for the LED GPIO (7). This function is needed to
    // drive GPIOs, otherwise function 0 can be used to read their states.
    GPIOctrl *gpio7 = (GPIOctrl *)GPIO7_IO_ADDR;
    gpio7->ctrl = 5;

    *(unsigned int *)GPIO7_PAD_ADDR = PADS_PUE;

    SIOregs *sio = (SIOregs *)SIO_BASE;
    sio->gpio_oe_set = 1 << 7;
    sio->gpio_out_clr = (1 << 7);
}

int get_led(void)
{
    SIOregs *sio = (SIOregs *)SIO_BASE;
    return sio->gpio_in & (1 << 7);
}

void set_led(int on)
{
    SIOregs *sio = (SIOregs *)SIO_BASE;
    if (on)
        sio->gpio_out_set = (1 << 7);
    else
        sio->gpio_out_clr = (1 << 7);
}
