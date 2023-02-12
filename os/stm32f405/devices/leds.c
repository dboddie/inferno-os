#include "stm32f405.h"

void setup_led(void)
{
    enable_GPIO_A();
    GPIO *gpioa = (GPIO *)GPIO_A;
    /* Set the pin mode using the pair of bits for PA15 and the speed to high */
    gpioa->moder = (gpioa->moder & 0x3fffffff) | (GPIO_Output << 30);
    gpioa->ospeedr = (gpioa->ospeedr & 0x3fffffff) | (GPIO_HighSpeed << 30);
}

void toggle_led(void)
{
    GPIO *gpioa = (GPIO *)GPIO_A;
    gpioa->odr ^= 1 << 15;
}

int get_led(void)
{
    GPIO *gpioa = (GPIO *)GPIO_A;
    return (gpioa->odr & (1 << 15)) != 0 ? 1 : 0;
}

void set_led(int on)
{
    /* Turn on (1) or off (0) the status LED */
    GPIO *gpioa = (GPIO *)GPIO_A;
    if (on)
        gpioa->odr |= 1 << 15;
    else
        gpioa->odr &= ~(1 << 15);
}
