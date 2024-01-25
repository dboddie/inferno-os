/* Include constants and GPIO functions */
#include "fns.h"

/* UARTs */
#define LPUART6 0x40198000

#define LPUART_GLOBAL   0x08
#define LPUART_PINCFG   0x0c
#define LPUART_BAUD     0x10
#define LPUART_STAT     0x14
#define LPUART_CTRL     0x18
#define LPUART_DATA     0x1c
#define LPUART_FIFO     0x28
#define LPUART_WATER    0x2c

#define LPUART_STAT_TDRE    0x800000
#define LPUART_STAT_TC      0x400000
#define LPUART_STAT_RDRF    0x200000
#define LPUART_CTRL_RE      0x40000
#define LPUART_CTRL_TE      0x80000
#define LPUART_CTRL_TXINV   0x10000000
#define LPUART_RXCOUNT_MASK 0x07000000
#define LPUART_TXCOUNT_MASK 0x00000700

void setup_uart(void)
{
    setup_system_clock();

    // Set the MUX for the GPIO_AD_B0_02 and GPIO_AD_B0_03 pads to use
    // alternate function 2.
    *(unsigned int *)IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 = 0x02;
    *(unsigned int *)IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 = 0x02;
    // Configure the TX and RX pads.
    // TX: SRE=1, DSE=3, SPEED=3
    // 1 | (3 << 3) | (3 << 6)
    *(unsigned int *)IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_02 = 0xd9;
    // RX: DSE=7, PKE=1, PUE=1, PUS=3, HYS=1
    // (7 << 3) | (1 << 12) | (1 << 13) | (3 << 14) | (1 << 16)
    *(unsigned int *)IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_03 = 0x1f038;
    // Select LPUART6 in the daisy chain.
    *(unsigned int *)IOMUXC_LPUART6_RX_SELECT_INPUT = 1;
    *(unsigned int *)IOMUXC_LPUART6_TX_SELECT_INPUT = 1;

    // Enable the LPUART6 clock.
    *(unsigned int *)CCM_CCGR3 |= CCM_CCGR3_CG3;

    *(unsigned int *)(LPUART6 + LPUART_PINCFG) = 0;
    // UART clock is 24 MHz, target baud rate is 115200 Hz.
    // 24000000 / 115200 = 208.333..., oversampling ratio of 16 (15).
    *(unsigned int *)(LPUART6 + LPUART_BAUD) = 8 | (25 << 24);
    // TXWATER=2, RXWATER=2
    *(unsigned int *)(LPUART6 + LPUART_WATER) = 2 | (2 << 16);
    // Enable RX and TX FIFOs.
    *(unsigned int *)(LPUART6 + LPUART_FIFO) |= (1 << 3) | (1 << 7);
    *(unsigned int *)(LPUART6 + LPUART_CTRL) = LPUART_CTRL_RE | LPUART_CTRL_TE;
}

int rdch(void)
{
    return *(unsigned int *)(LPUART6 + LPUART_DATA) & 0xff;
}

int rdch_ready(void)
{
    return (*(unsigned int *)(LPUART6 + LPUART_WATER) & LPUART_RXCOUNT_MASK) != 0;
}

int rdch_wait(void)
{
    /* Wait until the receive FIFO contains data. */
    while (!rdch_ready());
    return rdch();
}

void wrch(int c)
{
    // Wait until the transmit FIFO is empty.
    while ((*(unsigned int *)(LPUART6 + LPUART_STAT) & LPUART_STAT_TDRE) == 0);
    *(unsigned char *)(LPUART6 + LPUART_DATA) = c & 0xff;
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
