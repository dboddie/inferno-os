/* Include constants and GPIO functions */
#include "fns.h"
#include "thumb2.h"

/* SERCOM1 is on AHB-APB bridge A at 0x40003400. */
#define SERCOM1_base 0x40003400
#define SERCOM5_base 0x43000400

#define SERCOM_base SERCOM1_base

#define SERCOM_ctrla SERCOM_base
#define SERCOM_ctrlb (SERCOM_base + 0x04)
#define SERCOM_ctrlc (SERCOM_base + 0x08)
#define SERCOM_baud (SERCOM_base + 0x0c)
#define SERCOM_intclr (SERCOM_base + 0x14)
#define SERCOM_intset (SERCOM_base + 0x16)
#define SERCOM_intflag (SERCOM_base + 0x18)
#define SERCOM_syncbusy (SERCOM_base + 0x1c)
#define SERCOM_data (SERCOM_base + 0x28)

#define SERCOM_ctrla_dord 0x40000000
#define SERCOM_ctrla_rxpo_mask 0x00300000
#define SERCOM_ctrla_rxpo_shift 20
#define SERCOM_ctrla_sampr_mask 0x0000e000
#define SERCOM_ctrla_sampr_shift 13
#define SERCOM_ctrla_mode_mask 0x0000001c
#define SERCOM_ctrla_mode_shift 2
#define SERCOM_ctrla_enable 0x2
#define SERCOM_ctrla_swrst 0x1

#define SERCOM_ctrlb_rxen 0x20000
#define SERCOM_ctrlb_txen 0x10000

#define SERCOM_intclr_rxc 0x4
#define SERCOM_intclr_dre 0x1

#define SERCOM_intset_rxc 0x4
#define SERCOM_intset_dre 0x1

#define SERCOM_intflag_rxc 0x4
#define SERCOM_intflag_dre 0x1

#define SERCOM_syncbusy_enable 0x2
#define SERCOM_syncbusy_swrst 0x1

void setup_usart(void)
{
    enable_PORT();

    /* Configure the peripheral functions of PA16 and PA17 (SCL and SDA on
       the MicroMod processor board) to function C (0x2). Each byte contains
       the functions for two pins, so byte 8 contains the functions for
       these pins. */
    *(unsigned char *)(PORT_pmux + 8) = 0x22;

    /* Configure the pins to use alternative peripheral control. */
    *(unsigned char *)(PORT_pincfg + 16) = PORT_pincfg_drvstr | PORT_pincfg_pmuxen;
    *(unsigned char *)(PORT_pincfg + 17) = PORT_pincfg_drvstr | PORT_pincfg_pmuxen;

    /* Enable generator 1 for the SERCOM1 peripheral. */
    *(unsigned int *)(GCLK_GENCTRL_base + 4) = GCLK_GENCTRL_idc | GCLK_GENCTRL_genen | GCLK_GENCTRL_src_DFLL;

    /* Use generator 1 for peripheral 8 (SERCOM1_CORE). */
    *(unsigned int *)(GCLK_PCHCTRL_base + (8 * 4)) = GCLK_PCHCTRL_chen | 1;

    /* Enable the SERCOM1 clock. */
    *(unsigned int *)MCLK_APBA_mask |= MCLK_APBA_SERCOM1;

    /* Reset the SERCOM1 peripheral. */
    *(unsigned int *)SERCOM_ctrla = SERCOM_ctrla_swrst;

    while ((*(unsigned int *)SERCOM_ctrla & SERCOM_ctrla_swrst) &&
          (*(unsigned int *)(SERCOM_syncbusy & SERCOM_syncbusy_swrst)));

    /* The USART is asynchronous by default. */
    unsigned int flags = SERCOM_ctrla_dord | /* little-endian */
            (1 << SERCOM_ctrla_rxpo_shift) | /* pad[1] */
            (1 << SERCOM_ctrla_sampr_shift) | /* 16x over-sampling, fractional baud rate generation */
            (1 << SERCOM_ctrla_mode_shift); /* internal clock */

    *(unsigned int *)SERCOM_ctrla = flags;

    /* Set the baud rate.
    #f_BAUD = 9600
    #f_ref = 48000000
    #S = 16
    #FP = 4
    #BAUD = 312 */
    *(unsigned int *)SERCOM_baud = 0x8138;

    /* Enable receive and transmit. */
    *(unsigned int *)SERCOM_ctrlb = SERCOM_ctrlb_rxen | SERCOM_ctrlb_txen;

    /* Enable the peripheral. */
    *(unsigned int *)SERCOM_ctrla = flags | SERCOM_ctrla_enable;

    /* Wait for the peripheral to be ready. */
    while (*(unsigned int *)SERCOM_syncbusy & SERCOM_syncbusy_enable);
}

void enable_usart_rxc_intr(int enable)
{
    if (enable) {
        NVIC *nvic = (NVIC *)NVIC_ISER;
        /* Lines 50-53 are used for SERCOM1 interrupts, with RXC (bit 3 in
           SERCOM_intflag) being exposed on line 53. */
        nvic->iser32_63 |= (1 << 21);
        *(unsigned char *)SERCOM_intset = SERCOM_intset_rxc;
    } else {
        NVIC_clear *nvic_clr = (NVIC_clear *)NVIC_ICER;
        nvic_clr->icer32_63 |= (1 << 21);
        *(unsigned char *)SERCOM_intclr = SERCOM_intclr_rxc;
    }
}

int rdch_wait(void)
{
    while (!(*(unsigned int *)SERCOM_intflag & SERCOM_intflag_rxc));
    return *(unsigned int *)SERCOM_data;
}

int rdch(void)
{
    return *(unsigned int *)SERCOM_data;
}

void wrch(int c)
{
    /* Wait for the data register to be empty. */
    while (!(*(unsigned int *)SERCOM_intflag & SERCOM_intflag_dre));
    *(unsigned int *)SERCOM_data = c;
}

void wrstr(char *s)
{
    for (; *s != 0; s++) {
        wrch((int)*s);
    }
}

void _wrhex(int n, int digits)
{
    int shift = 4 * digits;
    while (shift != 0) {
        shift -= 4;
        int v = (n >> shift) & 0xf;
        if (v < 10)
            wrch(48 + v);
        else
            wrch(87 + v);
    }
}

void wrhex(int value)
{
    _wrhex(value, 8);
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
    return *(unsigned int *)SERCOM_intflag & SERCOM_intflag_rxc;
}
