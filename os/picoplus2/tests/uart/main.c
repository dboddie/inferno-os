#include "u.h"
#include "ureg.h"
#include "../../devices/picoplus2.h"
#include "../../devices/fns.h"

void main(void)
{
    setup_led();

    // Use function 2 (UART) for GPIO pins 4 and 5. This function is needed to
    // drive GPIOs, otherwise function 0 can be used to read their states.
    GPIOctrl *gpio4 = (GPIOctrl *)GPIO4_IO_ADDR;
    gpio4->ctrl = 2;
    GPIOctrl *gpio5 = (GPIOctrl *)GPIO5_IO_ADDR;
    gpio5->ctrl = 2;

    *(unsigned int *)GPIO4_PAD_ADDR = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;
    *(unsigned int *)GPIO5_PAD_ADDR = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;

    XOSC *xosc = (XOSC *)XOSC_BASE;
    xosc->ctrl = XOSC_1_15MHZ;
    xosc->startup = 47;
    xosc->ctrl = XOSC_ENABLE | XOSC_1_15MHZ;

    while (!(xosc->status & XOSC_STABLE));
/*
    PLL *pll = (PLL *)PLL_SYS_BASE;
    pll->cs = 1;
    pll->fbdiv_int = 125;
    pll->prim = (5 << 16) | (2 << 12);
*/

//    *(unsigned int *)CLK_SYS_RESUS_CTRL = 0;

    Clocks *refclk = (Clocks *)CLK_REF_ADDR;
    refclk->ctrl = CLK_REF_CTRL_XOSC_CLKSRC;
    refclk->div = 1 << 16;
    Clocks *sysclk = (Clocks *)CLK_SYS_ADDR;
    sysclk->ctrl = 0;
    sysclk->div = 1 << 16;
    Clocks *periclk = (Clocks *)CLK_PERI_ADDR;
    periclk->div = 1 << 16;
    periclk->ctrl = CLK_PERI_CTRL_ENABLE | CLK_PERI_CTRL_XOSC_CLKSRC;

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_IO_BANK0;
    while (!(resets->reset_done & RESETS_IO_BANK0));

//    resets->reset |= RESETS_UART1;
    clrreset->reset = RESETS_UART1;
    while (!(resets->reset_done & RESETS_UART1));

    UART *uart1 = (UART *)UART1_BASE;
    uart1->cr = ~UARTCR_EN;
    unsigned int baud_rate_div = (8 * XOSC_FREQ / 115200) + 1;
    uart1->ibrd = baud_rate_div >> 7;
    uart1->fbrd = (baud_rate_div & 0x7f) >> 1;
    uart1->lcr_h = UARTLCR_H_WLEN_8 | UARTLCR_H_FEN;
    uart1->cr = UARTCR_RXE | UARTCR_TXE | UARTCR_EN;

//    set_led(1);

    spllo();
    const char s[] = "Hello";

    for (int i = 0; i < 5; i++) {
        uart1->dr = s[i];
        while (uart1->fr & (UARTFR_TXFF | UARTFR_BUSY));
    }
    for (;;) {}
}

void hard_fault(int)
{
}

void usage_fault(int)
{
}

void bus_fault(int)
{
}

void trap_dummy(int)
{
}

void switcher(Ureg *)
{
}

void dumpregs(Ureg *)
{
}
