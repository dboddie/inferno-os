#include "picoplus2.h"

void
buttons_init(void)
{
    GPIOctrl *gpio45 = (GPIOctrl *)GPIO45_IO_ADDR;
    gpio45->ctrl = 5; // SIO

    *(unsigned int *)GPIO45_PAD_ADDR = PADS_PUE | PADS_IE;
}

int
buttons_user(void)
{
    SIOregs *sio = (SIOregs *)SIO_BASE;
    return sio->gpio_hi_in & (1 << 13); // 32 + 13 = 45
}
