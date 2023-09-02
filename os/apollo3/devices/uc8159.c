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
int UC8159_send_command_args(unsigned char *addr);

extern void print(char *, ...);

void UC8159_init(void)
{
    setup_spi();
    UC8159_setup_dc_reset_busy();

    /* Increase the baud rate of transfers to the screen. */
    spi_set_frequency_divider(4);

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
    // CS should already be set up by the SPI library.

    *(int *)GPIO_padkey = 0x73;

    /* Use D0 for D/C, I2C_INT for the busy line and D1 for reset.
       Configure D/C (0) and reset (1) to use 00 011 1 0 0 (0x1c)
       RSEL=0, FNCSEL=3, STRNG=1, INPEN=0, PULL=0.
       Configure the busy pin as an input with no pull-up or pull-down.
       RSEL=0, FNCSEL=3, STRNG=1, INPEN=1, PULL=0 (00 011 1 1 0 = 0x1e). */
    spi_mask_set(GPIO_padregA, 0xff000000, 0x001e1c1c);     // pads 0-3
    // INTD=0, OUTCFG=1, INCFG=0 (0 01 0 = 0x2)
    spi_mask_set(GPIO_cfgA, 0xfffff000, 0x00000122);        // pads 0-7
    // SR=0, DS=1 (0x1)
    spi_mask_set(GPIO_altpadcfgA, 0xff000000, 0x00010101);  // pads 0-3

    *(int *)GPIO_padkey = 0;

    // Pull CS and reset high, D/C low.
    spi_change_cs(1);
    *(int *)GPIO_wtsA = UC8159_RESET;
    *(int *)GPIO_wtcA = UC8159_DC;

    // Reset the display controller.
    *(int *)GPIO_wtcA = UC8159_RESET;
    wait_ms(100);
    *(int *)GPIO_wtsA = UC8159_RESET;
}

unsigned char UC8159_responses[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int UC8159_send_command(int cmd, int responses)
{
    /* Pull CS and D/C low, send the command byte, then pull them high. */
    spi_change_cs(0);
    *(int *)GPIO_wtcA = UC8159_DC;
    spi_send_byte(cmd);
    *(int *)GPIO_wtsA = UC8159_DC;

    int i = 0, res = 0;
    while (i < responses) {
        res = spi_send_byte(0xff);
        UC8159_responses[i] = (unsigned char)(res & 0xff);
        i++;
    }
    spi_change_cs(1);
    return res;
}

void UC8159_busy_wait(int timeout)
{
    int t = 0;
    while (t < timeout) {
        if (*(int *)GPIO_rdA & UC8159_BUSY)
            break;
        wait_ms(10);
        t += 10;
    }
}

int UC8159_send_parameter(int par)
{
    /* Pull CS low, leaving D/C high, send the byte, then pull CS high. */
    spi_change_cs(0);
    int res = spi_send_byte(par);
    spi_change_cs(1);
    *(int *)GPIO_wtsA = UC8159_DC;

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
