/* Include constants and GPIO functions */
#include "fns.h"

/* SERCOM5 is on AHB-APB bridge D at 0x43000400. */
#define SERCOM5_ctrla 0x43000400
#define SERCOM5_ctrla_dord 0x40000000
#define SERCOM5_ctrla_rxpo_mask 0x00300000
#define SERCOM5_ctrla_rxpo_shift 20
#define SERCOM5_ctrla_sampr_mask 0x0000e000
#define SERCOM5_ctrla_sampr_shift 13
#define SERCOM5_ctrla_mode_mask 0x0000001c
#define SERCOM5_ctrla_mode_shift 2
#define SERCOM5_ctrla_enable 0x2
#define SERCOM5_ctrla_swrst 0x1

#define SERCOM5_ctrlb 0x43000404
#define SERCOM5_ctrlb_rxen 0x20000
#define SERCOM5_ctrlb_txen 0x10000

#define SERCOM5_ctrlc 0x43000408
#define SERCOM5_baud 0x4300040c

#define SERCOM5_intclr 0x43000414
#define SERCOM5_intclr_rxc 0x4
#define SERCOM5_intclr_dre 0x1

#define SERCOM5_intset 0x43000416
#define SERCOM5_intset_rxc 0x4
#define SERCOM5_intset_dre 0x1

#define SERCOM5_intflag 0x43000418
#define SERCOM5_intflag_rxc 0x4
#define SERCOM5_intflag_dre 0x1

#define SERCOM5_syncbusy 0x4300041c
#define SERCOM5_syncbusy_enable 0x2
#define SERCOM5_syncbusy_swrst 0x1

#define SERCOM5_data 0x43000428

void setup_usart(void)
{
    enable_PORT();

    /* Configure the peripheral functions of PB30 and PB31 (TX and RX on the
       MicroMod processor board) to function D (0x3). Each byte contains the
       functions for two pins, so byte 15 contains the functions for these pins. */
    *(unsigned char *)(PORT_pmux + 0x80 + 15) = 0x33;

    /* Configure the pins to use alternative peripheral control. */
    *(unsigned char *)(PORT_pincfg + 0x80 + 30) = PORT_pincfg_drvstr | PORT_pincfg_pmuxen;
    *(unsigned char *)(PORT_pincfg + 0x80 + 31) = PORT_pincfg_drvstr | PORT_pincfg_pmuxen;

    /* Enable generator 1 for the SERCOM5 peripheral. */
    *(unsigned int *)(GCLK_GENCTRL_base + 4) = GCLK_GENCTRL_idc | GCLK_GENCTRL_genen | GCLK_GENCTRL_src_DFLL;

    /* Use generator 1 for peripheral 35 (SERCOM5_CORE). */
    *(unsigned int *)(GCLK_PCHCTRL_base + (35 * 4)) = GCLK_PCHCTRL_chen | 1;

    /* Enable the SERCOM5 clock. */
    *(unsigned int *)MCLK_APBD_mask |= MCLK_APBD_SERCOM5;

    /* Reset the SERCOM5 peripheral. */
    *(unsigned int *)SERCOM5_ctrla = SERCOM5_ctrla_swrst;

    while ((*(unsigned int *)SERCOM5_ctrla & SERCOM5_ctrla_swrst) &&
          (*(unsigned int *)(SERCOM5_syncbusy & SERCOM5_syncbusy_swrst)));

    /* The USART is asynchronous by default. */
    unsigned int flags = SERCOM5_ctrla_dord | /* little-endian */
            (1 << SERCOM5_ctrla_rxpo_shift) | /* pad[1] *.
            /* 16x over-sampling, fractional baud rate generation */
            (1 << SERCOM5_ctrla_sampr_shift) |
            (1 << SERCOM5_ctrla_mode_shift); /* internal clock */

    *(unsigned int *)SERCOM5_ctrla = flags;

    /* Set the baud rate.
    #f_BAUD = 9600
    #f_ref = 48000000
    #S = 16
    #FP = 4
    #BAUD = 312 */
    *(unsigned int *)SERCOM5_baud = 0x8138;

    /* Enable receive and transmit. */
    *(unsigned int *)SERCOM5_ctrlb = SERCOM5_ctrlb_rxen | SERCOM5_ctrlb_txen;

    /* Enable the peripheral. */
    *(unsigned int *)SERCOM5_ctrla = flags | SERCOM5_ctrla_enable;

    /* Wait for the peripheral to be ready. */
    while (*(unsigned int *)SERCOM5_syncbusy & SERCOM5_syncbusy_enable);
}

void enable_usart_rxc_intr(int enable)
{
    if (enable) {
        NVIC *nvic = (NVIC *)NVIC_ISER;
        nvic->iser64_95 |= (1 << 4);
        *(unsigned char *)SERCOM5_intset = SERCOM5_intset_rxc;
    } else {
        NVIC_clear *nvic_clr = (NVIC_clear *)NVIC_ICER;
        nvic_clr->icer64_95 |= (1 << 4);
        *(unsigned char *)SERCOM5_intclr = SERCOM5_intclr_rxc;
    }
}

int rdch_wait(void)
{
    while (!(*(unsigned int *)SERCOM5_intflag & SERCOM5_intflag_rxc));
    return *(unsigned int *)SERCOM5_data;
}

int rdch(void)
{
    return *(unsigned int *)SERCOM5_data;
}

void wrch(int c)
{
    /* Wait for the data register to be empty. */
    while (!(*(unsigned int *)SERCOM5_intflag & SERCOM5_intflag_dre));
    *(unsigned int *)SERCOM5_data = c;
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

void usart_serwrite(char *s, int n)
{
    for (int i = 0; i < n; i++) {
        if (s[i] == '\n')
            wrch('\r');
        wrch(s[i]);
    }
}

int rdch_ready(void)
{
    return *(unsigned int *)SERCOM5_intflag & SERCOM5_intflag_rxc;
}
