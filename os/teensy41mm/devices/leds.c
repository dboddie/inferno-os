#include "teensy41mm.h"

void setup_led(void)
{
    // Select GPIO 2 instead of GPIO 7.
    *(unsigned int *)IOMUXC_GPR_GPR27 = 0;
    // Set the MUX for the GPIO_B0_03 pad to use alternate function 5.
    *(unsigned int *)IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 0x05;
    // Set drive strength, DSE to 7.
    *(unsigned int *)IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = 0x38;

    // Set pin 3 as an output.
    *(unsigned int *)GPIO2_GDIR |= 0x08;
}

int get_led(void)
{
    return *(unsigned int *)GPIO2_DR & 0x08;
}

void set_led(int on)
{
    if (on)
        *(unsigned int *)GPIO2_DR |= 0x08;
    else
        *(unsigned int *)GPIO2_DR &= ~0x08;
}
