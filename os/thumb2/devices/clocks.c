#include "stm32f405.h"

void divide_AHB1_clock(int power_of_two)
{
    uint old = *(uint *)RCC_CFGR;
    uint v = power_of_two + 7;
    *(uint *)RCC_CFGR = (old & 0xffffff0f) | ((v & 0xf) << 4);
}

void divide_APB1_clock(int power_of_two)
{
    uint old = *(uint *)RCC_CFGR;
    uint v = power_of_two + 3;
    *(uint *)RCC_CFGR = (old & 0xffffe3ff) | ((v & 0x7) << 10);
}

void divide_APB2_clock(int power_of_two)
{
    uint old = *(uint *)RCC_CFGR;
    uint v = power_of_two + 3;
    *(uint *)RCC_CFGR = (old & 0xffff1fff) | ((v & 0x7) << 13);
}

void setup_system_clock(void)
{
    RCC *rcc = (RCC *)RCC_CR;

    /* fVCO = fPLL * [PLLN/PLLM]
       fPLL = fVCO / PLLP
       fxxx = fVCO / PLLQ */

    /* HSI=16 MHz, divide by 8, multiply by 168, divide by 2, /7 for USB clock */
    rcc->pllcfgr = (rcc->pllcfgr & 0xf0bc8000) |
                   RCC_PLL_HSI_Source | /* HSI=16MHz */
                   (8 << 0) |           /* divide by 8 (M=8) */
                   (168 << 6) |         /* multiply by 168 (N=168) */
                   (0 << 16) |          /* divide by 2 (P=0) */
                   (7 << 24);           /* divide by 7 for USB clock (Q=7) */

    rcc->cr |= RCC_PLLON;
    while ((rcc->cr & RCC_PLLRDY) != RCC_PLLRDY);

    /* Divide the clock for the AHB clock (168 MHz /4 = 42 MHz) */
    divide_AHB1_clock(2);
    /* Leave the AHB clock undivided for the APB1 clock */
    divide_APB1_clock(0);
    /* Leave the AHB clock undivided for the APB2 clock */
    divide_APB2_clock(0);

    /* Use the PLL as the system clock */
    rcc->cfgr = (rcc->cfgr & ~3) | RCC_CFGR_SW_PLL;
    while ((rcc->cfgr & 3) != RCC_CFGR_SW_PLL);
}

void start_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    /* The scaled system clock is 42MHz, so set a reset value for 0.01s. */
    tick->reload = 420000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
}

/* With a 42 MHz clock, a count of 42 is 1 microsecond, 42000 is 1ms. */
void wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int initial = tick->current;
    int diff = delay_ms * 42000;
    int next;

    if (diff > initial) {
        /* Wait while the counter increases until underflow occurs. */
        while (tick->current < initial);
        next = tick->reload - (diff - initial);
    } else
        next = initial - diff;

    while (tick->current > next);
}
