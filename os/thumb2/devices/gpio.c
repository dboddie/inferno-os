#include "stm32f405.h"

void enable_GPIO_A(void)
{
    /* Enable GPIO A on the AHB1 bus */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->ahb1enr |= RCC_AHB1_ENABLE_GPIO_A;
}

void enable_GPIO_B(void)
{
    /* Enable GPIO B on the AHB1 bus */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->ahb1enr |= RCC_AHB1_ENABLE_GPIO_B;
}

void enable_GPIO_C(void)
{
    /* Enable GPIO C on the AHB1 bus */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->ahb1enr |= RCC_AHB1_ENABLE_GPIO_C;
}

void setup_LED(void)
{
    enable_GPIO_A();
    GPIO *gpioa = (GPIO *)GPIO_A;
    /* Set the pin mode using the pair of bits for PA15 and the speed to high */
    gpioa->moder = GPIO_Output << 30;
    gpioa->ospeedr = GPIO_HighSpeed << 30;
}

void toggle_led(void)
{
    GPIO *gpioa = (GPIO *)GPIO_A;
    gpioa->odr ^= 1 << 15;
}

void set_LED(int on)
{
    /* Turn on (1) or off (0) the status LED */
    GPIO *gpioa = (GPIO *)GPIO_A;
    if (on)
        gpioa->odr |= 1 << 15;
    else
        gpioa->odr &= ~(1 << 15);
}
