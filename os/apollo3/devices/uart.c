/* Include constants and GPIO functions */
#include "fns.h"

/* UARTs */
#define UART0_BASE 0x4001c000
#define UART0_DR 0x4001c000
#define UART0_RSR 0x4001c004
#define UART0_FR 0x4001c018
#define UART0_ILPR 0x4001c020
#define UART0_IBRD 0x4001c024
#define UART0_FBRD 0x4001c028
#define UART0_LCRH 0x4001c02c
#define UART0_CR 0x4001c030
#define UART0_IFLS 0x4001c034
#define UART0_IER 0x4001c038
#define UART0_IES 0x4001c03c
#define UART0_MIS 0x4001c040
#define UART0_IEC 0x4001c044

#define UART_FR_TXBUSY 0x100
#define UART_FR_TXFE 0x80
#define UART_FR_RXFE 0x10
#define UART_FR_BUSY 0x8

#define UART_LCRH_WLEN_8  0x60
#define UART_LCRH_FEN  0x10

#define UART_CR_DTR       0x400
#define UART_CR_RXE       0x200
#define UART_CR_TXE       0x100
#define UART_CR_CLKSEL_24 0x10
#define UART_CR_CLKEN     0x8
#define UART_CR_UARTEN    0x1

#define UART_IFLS_RX_1_2 0x10
#define UART_IFLS_TX_1_2 0x02

void setup_usart(void)
{
    *(unsigned int *)PWR_DEVPWREN |= PWR_UART0;
    while ((*(unsigned int *)PWR_DEVPWRSTATUS & PWR_HCPA) == 0);

    *(unsigned int *)GPIO_padkey = 0x73;
    /* Configure pads 48 for TX and 49 for RX (with input enabled) using function 0. */
    *(unsigned int *)GPIO_padregM = 0x200;
    *(unsigned int *)GPIO_cfgG = 0;
    *(unsigned int *)GPIO_altpadcfgM = 0;
    *(unsigned int *)GPIO_padkey = 0;

    /* F_UART / (16 BR) = IBRD + FBRD
       BR = F_UART / 16 (IBRD + FBRD)
       F_UART = 24 MHz
       BR = 115200 */
    *(unsigned int *)UART0_CR = 0;
    *(unsigned int *)UART0_CR = UART_CR_CLKSEL_24 | UART_CR_CLKEN;

    *(unsigned int *)UART0_IBRD = 13;
    *(unsigned int *)UART0_FBRD = 1;
    *(unsigned int *)UART0_IFLS = UART_IFLS_RX_1_2 | UART_IFLS_TX_1_2;
    *(unsigned int *)UART0_LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    *(unsigned int *)UART0_CR |= UART_CR_UARTEN;
    *(unsigned int *)UART0_CR |= UART_CR_RXE;
    *(unsigned int *)UART0_CR |= UART_CR_TXE;
}

int rdch_wait(void)
{
    /* Wait until the receive FIFO is not empty. */
    while (*(unsigned int *)UART0_FR & UART_FR_RXFE);
    return *(unsigned char *)UART0_DR & 0xff;
}

int rdch(void)
{
    return *(unsigned int *)UART0_DR & 0xff;
}

void wrch(int c)
{
    // Wait until the transmit FIFO is empty.
    while (((*(unsigned int *)UART0_FR) & UART_FR_TXFE) == 0);
    *(unsigned char *)UART0_DR = c & 0xff;
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
    return (*(unsigned int *)UART0_FR & UART_FR_RXFE) == 0;
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
