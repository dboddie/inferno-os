/* Include constants and GPIO functions */
#include "picoplus2.h"
#include "fns.h"

void setup_uart(void)
{
    // Use function 2 (UART) for GPIO pins 4 and 5. This function is needed to
    // drive GPIOs, otherwise function 0 can be used to read their states.
    GPIOctrl *gpio4 = (GPIOctrl *)GPIO4_IO_ADDR;
    gpio4->ctrl = 2;
    GPIOctrl *gpio5 = (GPIOctrl *)GPIO5_IO_ADDR;
    gpio5->ctrl = 2;

    *(unsigned int *)GPIO4_PAD_ADDR = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;
    *(unsigned int *)GPIO5_PAD_ADDR = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_UART1;
    while (!(resets->reset_done & RESETS_UART1));

    UART *uart1 = (UART *)UART1_BASE;
    uart1->cr = ~UARTCR_EN;
    unsigned int baud_rate_div = (2 * XOSC_FREQ / 115200) + 1;
    uart1->ibrd = baud_rate_div >> 7;
    uart1->fbrd = (baud_rate_div & 0x7f) >> 1;
    uart1->lcr_h = UARTLCR_H_WLEN_8 | UARTLCR_H_FEN;
    uart1->cr = UARTCR_RXE | UARTCR_TXE | UARTCR_EN;
}

int rdch_wait(void)
{
    UART *uart1 = (UART *)UART1_BASE;
    /* Wait until the receive FIFO is not empty. */
    while (uart1->fr & UARTFR_RXFE);
    return uart1->dr & 0xff;
}

int rdch(void)
{
    UART *uart1 = (UART *)UART1_BASE;
    return uart1->dr & 0xff;
}

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

void uart_serwrite(char *s, int n)
{
    for (int i = 0; i < n; i++) {
        if (s[i] == '\n')
            wrch('\r');
        wrch(s[i]);
    }
}

int rdch_ready(void)
{
    UART *uart1 = (UART *)UART1_BASE;
    return (uart1->fr & UARTFR_RXFE) == 0;
}

void wrdec(int value)
{
    char ch[10];
    int v = value;
    if (v < 0) {
        wrch(45); v = -v;
    } else if (v == 0) {
        wrch('0');
        return;
    }

    int s = 9;
    for (; s >= 0 && v != 0; s--) {
        int b = v % 10;
        ch[s] = 48 + b;
        v = v / 10;
        if (v == 0) break;
    }

    for (; s < 10; s++)
        wrch(ch[s]);
}
