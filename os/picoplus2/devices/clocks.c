#include "picoplus2.h"

extern void setprimask(unsigned int);

void setup_clocks(void)
{
    XOSC *xosc = (XOSC *)XOSC_BASE;
    xosc->ctrl = XOSC_1_15MHZ;
    xosc->startup = 47;
    xosc->ctrl = XOSC_ENABLE | XOSC_1_15MHZ;

    while (!(xosc->status & XOSC_STABLE));

    // Reset the system clock PLL.
    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_PLL_SYS;
    while (!(resets->reset_done & RESETS_PLL_SYS));

    PLL *pll = (PLL *)PLL_SYS_BASE;
    pll->cs = 1;    // refdiv=1
    pll->fbdiv_int = 125;

    // Power up (set low) PLL and oscillator.
    pll->pwr &= ~(PLL_PWR_VCOPD | PLL_PWR_PD);
    while (!(pll->cs & PLL_CS_LOCK));
    pll->prim = (5 << 16) | (2 << 12);

    // Power up (set low) PLL and oscillator.
    pll->pwr &= ~PLL_PWR_POSTDIVD;

    Clocks *refclk = (Clocks *)CLK_REF_ADDR;
    refclk->ctrl = CLK_REF_CTRL_XOSC_CLKSRC;
    refclk->div = 1 << 16;
    Clocks *sysclk = (Clocks *)CLK_SYS_ADDR;
    sysclk->ctrl = CLK_SYS_CTRL_CLKSRC_PLL_SYS | CLK_SYS_CTRL_CLKSRC_CLK_SYS_AUX;
    sysclk->div = 1 << 16;
    Clocks *periclk = (Clocks *)CLK_PERI_ADDR;
    periclk->div = 1;
    periclk->ctrl = CLK_PERI_CTRL_ENABLE | CLK_PERI_CTRL_XOSC_CLKSRC;

    // Reset and configure the USB clock PLL.
    clrreset->reset = RESETS_PLL_USB;
    while (!(resets->reset_done & RESETS_PLL_USB));

    pll = (PLL *)PLL_USB_BASE;
    pll->cs = 1;    // refdiv=1
    pll->fbdiv_int = 64;

    // Power up (set low) PLL and oscillator.
    pll->pwr &= ~(PLL_PWR_VCOPD | PLL_PWR_PD);
    while (!(pll->cs & PLL_CS_LOCK));
    pll->prim = (4 << 16) | (4 << 12);

    // Power up (set low) PLL and oscillator.
    pll->pwr &= ~PLL_PWR_POSTDIVD;

    Clocks *usbclk = (Clocks *)CLK_USB_ADDR;
    usbclk->div = 1;
    usbclk->ctrl = CLK_USB_CTRL_CLKSRC_PLL_USB;

    clrreset->reset = RESETS_IO_BANK0;
    while (!(resets->reset_done & RESETS_IO_BANK0));
}

void start_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    /* The scaled system clock is 12MHz, so set a reset value for 0.01s. */
    tick->reload = 120000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
    setprimask(0);
}

void pause_timer(void)
{
    SysTick *tick = (SysTick *)SYSTICK;
    tick->control ^= 1;
}

/* With a 12 MHz clock, a count of 12 is 1 microsecond, 12000 is 1ms. */
void _wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int initial = tick->current;
    int diff = delay_ms * 12000;
    int next;

    if (diff > initial) {
        /* Wait while the counter increases until underflow occurs. */
        while (tick->current < initial);
        next = tick->reload - (diff - initial);
    } else
        next = initial - diff;

    while (tick->current > next);
}

void wait_ms(int delay_ms)
{
    SysTick *tick = (SysTick *)SYSTICK;
    int limit_ms = tick->reload / 12000;
    while (delay_ms > limit_ms) {
        _wait_ms(limit_ms);
        delay_ms -= limit_ms;
    }
    _wait_ms(delay_ms);
}
