/* Include constants and GPIO functions */
#include "picoplus2.h"
#include "fns.h"

static UART *uart;

void setup_uart(void)
{
    // Enable UART interrupts for testing.
//    NVIC *nvic = (NVIC *)NVIC_ISER;
//    nvic->iser32_63 |= (1 << (UART0_IRQ - 32)) | (1 << (UART1_IRQ - 32));

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;

    // Set the UART function for the GPIO pins used for TX and RX.
    GPIOctrl *gpio = (GPIOctrl *)(IO_BANK0_BASE + (MK_UART_TX * 8));
    gpio->ctrl = MK_UART_FN;
    gpio = (GPIOctrl *)(IO_BANK0_BASE + (MK_UART_RX * 8));
    gpio->ctrl = MK_UART_FN;

    unsigned int *pad_base = (unsigned int *)GPIO0_PAD_ADDR;
    pad_base[MK_UART_TX] = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;
    pad_base[MK_UART_RX] = PADS_IE | PADS_DRIVE_4mA | PADS_SCHMITT;

    // Set the function for the GPIO pins so that the UART peripheral can drive
    // them. Function 0 can be used to read their states.
    if (MK_UART == 0) {
        clrreset->reset = RESETS_UART0;
        while (!(resets->reset_done & RESETS_UART0));
        uart = (UART *)UART0_BASE;
    } else {
        clrreset->reset = RESETS_UART1;
        while (!(resets->reset_done & RESETS_UART1));
        uart = (UART *)UART1_BASE;
    }

    uart->cr = ~UARTCR_EN;

    unsigned int baud_rate_div = (8 * XOSC_FREQ / 115200) + 1;
    uart->ibrd = baud_rate_div >> 7;
    uart->fbrd = (baud_rate_div & 0x7f) >> 1;
    uart->lcr_h = UARTLCR_H_WLEN_8 | UARTLCR_H_FEN;
    uart->cr = UARTCR_RXE | UARTCR_TXE | UARTCR_EN;

    // Enable the RX and RT interrupts for testing.
    //uart->imsc |= 0x50;
}

int rdch_wait(void)
{
    /* Wait until the receive FIFO is not empty. */
    while (uart->fr & UARTFR_RXFE);
    return uart->dr & 0xff;
}

int rdch(void)
{
    return uart->dr & 0xff;
}

void wrch(int c)
{
    // Wait until the transmit FIFO is empty.
    while (uart->fr & UARTFR_TXFF);
    uart->dr = c;
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
    return (uart->fr & UARTFR_RXFE) == 0;
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

void uart_intr(void)
{
    if (uart->imsc & 0x50) {
        uart->icr = 0x50;
    }
}
