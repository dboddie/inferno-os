#include "samd51.h"

void setup_led(void)
{
    *(unsigned int *)PORT_dirset = 1 << 23;
}

int get_led(void)
{
    return *(unsigned int *)PORT_outset & (1 << 23);
}

void set_led(int on)
{
    if (on)
   	    *(unsigned int *)PORT_outset = 1 << 23;
    else
   	    *(unsigned int *)PORT_outclr = 1 << 23;
}
