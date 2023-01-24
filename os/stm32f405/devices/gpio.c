#include "stm32f405.h"

void enable_GPIO_B(void)
{
    /* Enable GPIO B on the AHB1 bus */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->ahb1enr |= RCC_AHB1_ENABLE_GPIO_B;
}
#ifdef ALL_GPIO
void enable_GPIO_C(void)
{
    /* Enable GPIO C on the AHB1 bus */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->ahb1enr |= RCC_AHB1_ENABLE_GPIO_C;
}
#endif
