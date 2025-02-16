#include "u.h"
#include "ureg.h"
#include "../../devices/picoplus2.h"
#include "../../devices/fns.h"
#include "../../../cortexm/thumb2.h"

void wrch(int c)
{
    UART *uart1 = (UART *)UART1_BASE;
    // Wait until the transmit FIFO is empty.
    while (uart1->fr & UARTFR_TXFF);
    uart1->dr = c;
}

void wrstr(char *s)
{
    for (; *s != 0; s++) {
        wrch((int)*s);
    }
}

void wrhex(int value)
{
    int v = value;
    for (int s = 28; s >= 0; s -= 4) {
        int b = (v >> s) & 0xf;
        if (b > 9)
            wrch(87 + b);
        else
            wrch(48 + b);
    }
}

void newline(void)
{
    wrch(13); wrch(10);
}

void prhex(char *s, unsigned int addr)
{
    wrstr(s);
    wrhex(*(unsigned int *)addr);
    newline();
}

#define prval(expr) \
    wrstr("expr = 0x"); \
    wrhex(expr); \
    newline();

int state = 0, count = 0;

void start_timer(void)
{
    // Make SysTick low priority by assigning a high number to it.
    *(int *)SHPR3 = (*(int *)SHPR3 & 0x00ffffff) | 0xe0000000;

    /* Enable the usage fault exception. */
    *(int *)SHCSR_ADDR |= SHCSR_USGFAULTENA;

    SysTick *tick = (SysTick *)SYSTICK;
    /* The system clock is 12MHz, so set a reset value for 1s. */
    tick->reload = 12000000 - 1;
    tick->current = 0;
    tick->control = 7;  /* 4=processor clock (0=AHB/8, 4=AHB),
                           2=SysTick exception, 1=enable */
}

void main(void)
{
    setup_led();
    start_timer();

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

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_IO_BANK0;
    while (!(resets->reset_done & RESETS_IO_BANK0));

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
    sysclk->ctrl = 0;
    sysclk->div = 1 << 16;
    Clocks *periclk = (Clocks *)CLK_PERI_ADDR;
    periclk->div = 1 << 16;
    periclk->ctrl = CLK_PERI_CTRL_ENABLE | CLK_PERI_CTRL_CLKSRC_PLL_SYS;

    clrreset->reset = RESETS_UART1;
    while (!(resets->reset_done & RESETS_UART1));

    UART *uart1 = (UART *)UART1_BASE;
    uart1->cr = ~UARTCR_EN;
    unsigned int baud_rate_div = (8 * 150000000 / 115200) + 1;
    uart1->ibrd = baud_rate_div >> 7;
    uart1->fbrd = (baud_rate_div & 0x7f) >> 1;
    uart1->lcr_h = UARTLCR_H_WLEN_8 | UARTLCR_H_FEN;
    uart1->cr = UARTCR_RXE | UARTCR_TXE | UARTCR_EN;

    spllo();
    const char s[] = "Hello\r\n";

    for (int i = 0; i < 7; i++) {
        uart1->dr = s[i];
        while (uart1->fr & UARTFR_TXFF);
    }

    prval(getprimask())
    prval(getfaultmask())
    setprimask(0);

    SysTick *tick = (SysTick *)SYSTICK;
    prhex("AIRCR = 0x", AIRCR);
    prval(tick->control)

    for (;;) {
        for (int j = 0; j < 1000000; j++);

        wrhex(tick->current);
        newline();
        prhex("ICSR = 0x", ICSR);
    }
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

void systick(void)
{
    set_led(state);
    state = 1 - state;
}
