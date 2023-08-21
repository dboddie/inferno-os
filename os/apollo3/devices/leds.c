#include "apollo3.h"

void setup_led(void)
{
    *(unsigned int *)GPIO_padkey = 0x73;

    *(unsigned int *)GPIO_padregE = (*(unsigned int *)GPIO_padregE & 0x00ffffff) | 0x1e000000;
    *(unsigned int *)GPIO_cfgC = (*(unsigned int *)GPIO_cfgC & 0xffff0fff) | 0x2000;
    *(unsigned int *)GPIO_altpadcfgE = (*(unsigned int *)GPIO_altpadcfgE & 0x00ffffff) | 0x01000000;

    *(unsigned int *)GPIO_padkey = 0;
}

int get_led(void)
{
    return *(unsigned int *)GPIO_wtA & (1 << 19);
}

void set_led(int on)
{
    if (on)
        *(unsigned int *)GPIO_wtsA |= 1 << 19;
    else
        *(unsigned int *)GPIO_wtcA |= 1 << 19;
}
