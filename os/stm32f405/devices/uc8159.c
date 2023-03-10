#include "stm32f405.h"
#include "fns.h"

/* Commands */
#define UC8159_PanelSetting           0x00
#define UC8159_PowerSetting           0x01
#define UC8159_PowerOff               0x02
#define UC8159_PowerOffSequence       0x03
#define UC8159_PowerOn                0x04
#define UC8159_DataStartTransmission  0x10
#define UC8159_DataStop               0x11
#define UC8159_DisplayRefresh         0x12
#define UC8159_PLLControl             0x30
#define UC8159_TemperatureSensorExt   0x41
#define UC8159_VCOMDataSetting        0x50
#define UC8159_TCONSetting            0x60
#define UC8159_TCONResolution         0x61
#define UC8159_SPIFlashControl        0x65
#define UC8159_Revision               0x70
#define UC8159_GetStatus              0x71
#define UC8159_PowerSaving            0xe3

#define UC8159_BUSY 0x02    /* PB1 (I2C_INT) */
#define UC8159_RESET 0x02   /* PC1 (D1) */
#define UC8159_CS 0x10      /* PC4 (CS) */
#define UC8159_DC 0x01      /* PC0 (D0) */

void UC8159_setup_dc_reset_busy(void);
int UC8159_send_command(int cmd, int responses);
void UC8159_busy_wait(int timeout);
int UC8159_send_parameter(int par);
int UC8159_send_command_args(unsigned char *addr);

extern void print(char *, ...);

void UC8159_init(void)
{
    enable_GPIO_B();
    setup_spi(1);
    UC8159_setup_dc_reset_busy();

    /* Increase the baud rate of transfers to the screen. */
/*    SPI *spi1 = (SPI *)SPI1;
    int old = spi1->cr1 & ~(7 << SPI_br_shift);
    spi1->cr1 = old | (4 << SPI_br_shift);*/

    UC8159_busy_wait(1000);

    UC8159_send_command(UC8159_TCONResolution, 0);
    UC8159_send_parameter(640 & 0x300);
    UC8159_send_parameter(640 & 0x0ff);
    UC8159_send_parameter(400 & 0x300);
    UC8159_send_parameter(400 & 0x0ff);

    UC8159_send_command(UC8159_PanelSetting, 0);
    UC8159_send_parameter(0x2f);
    UC8159_send_parameter(8);

    UC8159_send_command(UC8159_PowerSetting, 0);
    UC8159_send_parameter(0x37);
    UC8159_send_parameter(0);
    UC8159_send_parameter(0x23);
    UC8159_send_parameter(0x23);

    UC8159_send_command(UC8159_PLLControl, 0);
    UC8159_send_parameter(0x3c);

    UC8159_send_command(UC8159_TemperatureSensorExt, 0);
    UC8159_send_parameter(0);

    UC8159_send_command(UC8159_VCOMDataSetting, 0);
    UC8159_send_parameter(0x37);

    UC8159_send_command(UC8159_TCONSetting, 0);
    UC8159_send_parameter(0x22);

    UC8159_send_command(UC8159_SPIFlashControl, 0);
    UC8159_send_parameter(0);

    UC8159_send_command(UC8159_PowerSaving, 0);
    UC8159_send_parameter(0xaa);

    UC8159_send_command(UC8159_PowerOffSequence, 0);
    UC8159_send_parameter(0);
}

void UC8159_setup_dc_reset_busy(void)
{
    /* PC4 (CS) should already be set up by the SPI library.
       Use PC0 (D0) for D/C, PB1 (I2C_INT) for the busy line and PC1 (D1) for reset.
       Configure D/C and reset. */
    int cpins = UC8159_RESET | UC8159_DC,    /* 0x3: bits 1 and 0 */
        cpins2 = 0x000f;

    GPIO *gpioc = (GPIO *)GPIO_C;
    /* Pin modes are 2 bits each in the mode register. */
    gpioc->moder = (gpioc->moder & ~cpins2) | (GPIO_Output << 2) | GPIO_Output;
    /* Pull modes are 2 bits each in the pull direction register (clear for no pull). */
    gpioc->pupdr &= ~cpins2;
    /* Output types are single bits in the output type register (clear for push-pull). */
    gpioc->otyper &= ~cpins;
    /* Speeds are 2 bits each in the output speed register. */
    gpioc->ospeedr = (gpioc->ospeedr & ~cpins2) | (GPIO_VeryHighSpeed << 2) | GPIO_VeryHighSpeed;

    /* Configure the busy pin (PB1). */
    int bpin = 0x02, bpin2 = 0x000c;

    GPIO *gpiob = (GPIO *)GPIO_B;
    /* Pin modes are 2 bits each in the mode register. */
    gpiob->moder = (gpiob->moder & ~bpin2) | (GPIO_Input << 2);
    /* Pull modes are 2 bits each in the pull direction register (clear for no pull). */
    gpiob->pupdr &= ~bpin2;

    /* Pull CS and reset high, D/C low. */
    gpioc->odr |= UC8159_CS | UC8159_RESET;
    gpioc->odr &= UC8159_DC;

    /* Reset the display controller. */
    gpioc->odr &= UC8159_RESET;
    wait_ms(100);
    gpioc->odr |= UC8159_RESET;
}

unsigned char UC8159_responses[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int UC8159_send_command(int cmd, int responses)
{
    /* Pull CS and D/C low, send the command byte, then pull them high. */
    GPIO *gpioc = (GPIO *)GPIO_C;
    gpioc->odr &= ~(UC8159_CS | UC8159_DC);
    spi_send_byte((SPI *)SPI1, cmd);
    gpioc->odr |= UC8159_DC;
    int i = 0, res = 0;
    while (i < responses) {
        res = spi_send_byte((SPI *)SPI1, 0xff);
        UC8159_responses[i] = (unsigned char)(res & 0xff);
        i++;
    }
    gpioc->odr |= UC8159_CS;
    return res;
}

void UC8159_busy_wait(int timeout)
{
    GPIO *gpiob = (GPIO *)GPIO_B;
    int t = 0;
    while (t < timeout) {
        if (gpiob->idr & UC8159_BUSY)
            break;

        wait_ms(10);
        t += 10;
    }
}

int UC8159_send_parameter(int par)
{
    /* Pull CS low, leaving D/C high, send the byte, then pull CS high. */
    GPIO *gpioc = (GPIO *)GPIO_C;
    gpioc->odr &= ~UC8159_CS;
    int res = spi_send_byte((SPI *)SPI1, par);
    gpioc->odr |= UC8159_CS;

    return res;
}

void show_response(int n)
{
    int i = 0;
    while (i < n)
        print("%02ux ", UC8159_responses[i++]);
    newline();
}

void UC8159_start(void)
{
    UC8159_send_command(UC8159_DataStartTransmission, 0);
}

void UC8159_finish(void)
{
    //UC8159.send_command(UC8159_DataStop, 0)

    UC8159_send_command(UC8159_PowerOn, 0);
    UC8159_busy_wait(200);

    UC8159_send_command(UC8159_DisplayRefresh, 0);
    UC8159_busy_wait(200);

    UC8159_send_command(UC8159_PowerOff, 0);
    UC8159_busy_wait(200);
}

int UC8159_get_status(void)
{
    UC8159_send_command(UC8159_GetStatus, 1);
    return UC8159_responses[0];
}
