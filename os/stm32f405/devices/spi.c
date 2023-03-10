#include "stm32f405.h"
#include "fns.h"

static int spi_init = 0;

extern void print(char *, ...);

void setup_spi1(void)
{
    /* The SPI pins can be accessed via GPIO A, using PA4-7 in alternate
       function 5. Because PC4 is adjacent to PA5-7, we use that instead. */
    enable_GPIO_A();
    enable_GPIO_C();

    /* Set the pin modes for GPIO A pins 5 [SCK], 6 [MISO] and 7 [MOSI] to
       alternate function 5 as described in the datasheet. */
    GPIO *gpioa = (GPIO *)GPIO_A;
    unsigned int apins = 0xe0, apins2 = 0xfc00, apins4 = 0xfff00000;
    /* Alternative functions for pins 0-7 are 4 bits each in the low register. */
    gpioa->afrl = (gpioa->afrl & ~apins4) | 0x55500000;
    /* Pin modes are 2 bits each in the mode register. */
    gpioa->moder = (gpioa->moder & ~apins2) | (GPIO_Alternate << 14) |
                   (GPIO_Alternate << 12) | (GPIO_Alternate << 10);
    /* Output types are single bits in the output type register (clear for push-pull). */
    gpioa->otyper &= ~apins;
    /* Speeds are 2 bits each in the output speed register. */
    gpioa->ospeedr = (gpioa->ospeedr & ~apins2) | (GPIO_VeryHighSpeed << 14) |
                     (GPIO_VeryHighSpeed << 12) | (GPIO_VeryHighSpeed << 10);

    /* Set the pin mode GPIO C pin 4. */
    GPIO *gpioc = (GPIO *)GPIO_C;
    unsigned int cpin = 0x10, cpin2 = 0x0300;
    /* Pin modes are 2 bits each in the mode register. */
    gpioc->moder = (gpioc->moder & ~cpin2) | (GPIO_Output << 8);
    /* Pull modes are 2 bits each in the pull direction register (clear for no pull). */
    gpioc->pupdr &= ~cpin2;
    /* Output types are single bits in the output type register (clear for push-pull). */
    gpioc->otyper &= ~cpin;
    /* Speeds are 2 bits each in the output speed register. */
    gpioc->ospeedr = (gpioc->ospeedr & ~cpin2) | (GPIO_VeryHighSpeed << 8);

    /* Set the clock phase [0] and polarity [0], master, baud rate divider
       [6 (128)], SPI enable [1], MSB first [0], SSI and SSM enabled,
       full duplex [0], 8-bit data [DFF=0] */
    SPI *spi1 = (SPI *)SPI1;
    spi1->cr1 = SPI_mstr | (6 << SPI_br_shift) | SPI_ssi | SPI_ssm | SPI_spe;
    spi1->cr2 = 0;
}

void setup_spi3(void)
{
}

void setup_spi(int iface)
{
    if (spi_init & (1 << iface)) return;
    spi_init |= (1 << iface);

    /* The SPI pins are on the APB2 bus and must be enabled. See the entry for
       SPI1 in the memory map in the reference manual. */
    RCC *rcc = (RCC *)RCC_CR;
    switch (iface) {
    case 1:
        rcc->apb2enr |= RCC_APB2_SPI1;
        setup_spi1();
        break;
    case 3:
        rcc->apb1enr |= RCC_APB1_SPI3;
        setup_spi3();
        break;
    default:
        ;
    }
}

void spi_set_frequency_divider(SPI *spi, int divider)
{
    /* Increase the baud rate of transfers to the screen. */
    uint old = spi->cr1 & ~(7 << SPI_br_shift);
    spi->cr1 = old | ((divider & 7) << SPI_br_shift);
}

void spi_wait_for_not_busy(SPI *spi)
{
    while ((spi->sr & SPI_bsy) != 0);
}

void spi_wait_for_txe(SPI *spi)
{
    while ((spi->sr & SPI_txe) == 0);
}

void spi_wait_for_rxne(SPI *spi)
{
    while ((spi->sr & SPI_rxne) == 0);
}

int spi_send_byte(SPI *spi, int b)
{
    /*_wrhex(b, 2)
    wrch(32)*/

    spi_wait_for_txe(spi);
    spi->dr = b;
    spi_wait_for_rxne(spi);
    int res = spi->dr;

    /*_wrhex(res, 2)
    newline()*/

    return res;
}

int spi_wait_for_response(SPI *spi, int n, int delay_ms)
{
    /* Send padding bits. */
    int res = 0xff;
    int t = 0;
    while (res == 0xff)
    {
        res = spi_send_byte(spi, 0xff);
        t++;
        if (t == n)
            break;

        wait_ms(delay_ms);
    }
    return res;
}
