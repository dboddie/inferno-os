/* Include constants and GPIO functions */
#include "stm32f405.h"

#define USART3_sr  0x40004800
#define USART3_dr  0x40004804
#define USART3_brr 0x40004808
#define USART3_cr1 0x4000480c
#define USART3_cr2 0x40004810

/* Status */
#define USART_TransComplete    0x40
#define USART_RecvComplete     0x20

/* Control 1 */
#define USART_Over8            0x8000
#define USART_Enable           0x2000
#define USART_WordLength       0x1000
#define USART_RecvIntEnable    0x20
#define USART_TransEnable      0x8
#define USART_RecvEnable       0x4

/* Control 2 */
#define USART_StopBits 0x3000

static void enable_usart(void)
{
    /* The USART peripheral is on the APB1 bus and must be enabled.
       See the entry for USART3 in the memory map in the reference manual. */
    RCC *rcc = (RCC *)RCC_CR;
    rcc->apb1enr |= RCC_APB1_ENABLE_USART3;
}

void setup_usart(void)
{
    enable_usart();
    /* The USART pins are accessed via GPIO B */
    enable_GPIO_B();

    GPIO *gpiob = (GPIO *)GPIO_B;
    /* Set the pin modes for pins 10 (TX) and 11 (RX) to alternate function 7
       as described in the datasheet (stm32f405rg-1851084.pdf) */
    gpiob->moder = (GPIO_Alternate << 20) | (GPIO_Alternate << 22);
    gpiob->afrh = (7 << 8) | (7 << 12);
    /* Set the speed */
    gpiob->ospeedr = (GPIO_HighSpeed << 20) | (GPIO_HighSpeed << 22);

    /* usartdiv = div_mantissa + (div_fraction / 8 * (2 - over8)) */
    /* baud rate = fCK / (8 * (2 - over8) * usartdiv) */
    /* For over8=0, usartdiv = fCK / (16 * baud) */
    USART *usart = (USART *)USART3;

/*  int fck = 42000000;
    int baud = 115200;*/
    usart->brr = 0x16c;

    /* 8 data bits, 1 stop bit */
    usart->cr1 &= ~USART_WordLength;
    usart->cr2 &= ~USART_StopBits;

    /* Enable TX and RX, set over8=0 */
    usart->cr1 = USART_TransEnable | USART_RecvEnable;

    /* Enable the USART */
    usart->cr1 |= USART_Enable;

    /* Wait until the USART is ready to transmit */
    while (!(usart->sr & USART_TransComplete));
}

void enable_usart_intr(int enable)
{
    USART *usart = (USART *)USART3;
    if (enable) {
        NVIC *nvic = (NVIC *)NVIC_ISER;
        nvic->iser32_63 |= (1 << 7);
        usart->cr1 |= USART_RecvIntEnable;
    } else {
        NVIC_clear *nvic_clr = (NVIC_clear *)NVIC_ICER;
        nvic_clr->icer32_63 |= (1 << 7);
        usart->cr1 &= ~USART_RecvIntEnable;
    }
}

int rdch_wait(void)
{
    USART *usart = (USART *)USART3;
    while (!(usart->sr & USART_RecvComplete));
    return usart->dr;
}

int rdch(void)
{
    USART *usart = (USART *)USART3;
    return usart->dr;
}

void wrch(int c)
{
    USART *usart = (USART *)USART3;
    usart->dr = c;
    while (!(usart->sr & USART_TransComplete));
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
    USART *usart = (USART *)USART3;
    return (usart->sr & USART_ReadNotEmpty);
}
