#include "fns.h"

#define ILI9341_Reset                 0x01
#define ILI9341_ReadDisplayInfo       0x04
#define ILI9341_ReadPowerMode         0x0a
#define ILI9341_ReadMADCtrl           0x0b
#define ILI9341_ReadPixelFormat       0x0c
#define ILI9341_ReadSelfDiag          0x0f
#define ILI9341_SleepOut              0x11
#define ILI9341_DisplayInversionOff   0x20
#define ILI9341_DisplayInversionOn    0x21
#define ILI9341_GammaSet              0x26
#define ILI9341_DisplayOff            0x28
#define ILI9341_DisplayOn             0x29
#define ILI9341_ColumnAddressSet      0x2a
#define ILI9341_PageAddressSet        0x2b
#define ILI9341_MemoryWrite           0x2c
#define ILI9341_MADControl            0x36
#define ILI9341_VerticalScrollAddress 0x37
#define ILI9341_IdleModeOff           0x38
#define ILI9341_IdleModeOn            0x39
#define ILI9341_PixelFormatSet        0x3a
#define ILI9341_DisplayBrightness     0x51
#define ILI9341_ReadDisplayBrightness 0x52
#define ILI9341_ControlDisplay        0x53
#define ILI9341_ReadControlDisplay    0x54
#define ILI9341_FrameControl1         0xb1
#define ILI9341_DisplayFuncControl    0xb6
#define ILI9341_PowerControl1         0xc0
#define ILI9341_PowerControl2         0xc1
#define ILI9341_VCOMControl1          0xc5
#define ILI9341_VCOMControl2          0xc7
#define ILI9341_PositiveGammaCorrect  0xe0
#define ILI9341_NegativeGammaCorrect  0xe1

/* Used the same initialisation as the Adafruit ILI9341 library:
   https://github.com/Adafruit/Adafruit_ILI9341 */
static const char ILI9341_Commands[] = {
    0xef, 0x3, 0x03, 0x80, 0x02,
    0xcf, 0x3, 0x00, 0xc1, 0x30,
    0xed, 0x4, 0x64, 0x03, 0x12, 0x81,
    0xe8, 0x3, 0x85, 0x00, 0x78,
    0xcb, 0x5, 0x39, 0x2c, 0x00, 0x34, 0x02,
    0xf7, 0x1, 0x20,
    0xea, 0x2, 0x00, 0x00,
    ILI9341_PowerControl1, 0x1, 0x23,
    ILI9341_PowerControl2, 0x1, 0x10,
    ILI9341_VCOMControl1, 0x2, 0x3e, 0x28,
    ILI9341_VCOMControl2, 0x1, 0x86,
    ILI9341_MADControl, 0x1, 0x48,
    ILI9341_VerticalScrollAddress, 0x1, 0x00,
    ILI9341_PixelFormatSet, 0x1, 0x55,
    ILI9341_FrameControl1, 0x2, 0x00, 0x18,
    ILI9341_DisplayFuncControl, 0x3, 0x08, 0x82, 0x27, 0xf2, 0x01, 0x00,
    ILI9341_GammaSet, 0x1, 0x01,
    ILI9341_PositiveGammaCorrect, 0xf, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08,
    0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
    ILI9341_NegativeGammaCorrect, 0xf, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07,
    0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f
};

void ILI9341_setup_dc(void);
void ILI9341_change_dc(int high);
int ILI9341_send_command(int cmd, int responses);
int ILI9341_send_parameter(int par);
int ILI9341_send_command_args(char *addr);

void ILI9341_init(void)
{
    setup_spi();
    ILI9341_setup_dc();

    // Increase the baud rate of transfers to the screen.
//    old = __getw(SPI1_cr1) & ~(7 << SPI1_cr1_br_shift)
//    __setw(SPI1_cr1, old | (1 << SPI1_cr1_br_shift))

    ILI9341_send_command(ILI9341_Reset, 0);
    wait_ms(150);

    for (char *addr = ILI9341_Commands; (*addr) != (char)ILI9341_NegativeGammaCorrect;
         addr += *(addr + 1) + 2) {
        ILI9341_send_command_args(addr);
    }

    ILI9341_send_command(ILI9341_SleepOut, 0);
    wait_ms(150);
    ILI9341_send_command(ILI9341_DisplayOn, 0);
    wait_ms(150);
}

void ILI9341_setup_dc(void)
{
    // Use D1 for D/C.
    // Output, no pull-up, push-pull, very high speed.

    *(int *)GPIO_padkey = 0x73;
    // D/C (1): 0x1e = 00 fn=011 ds=1 ie=1 0
    spi_mask_set(GPIO_padregA, 0xffff00ff, 0x00001e00); // pads 0-3
    // D/C (1): 0x2 = 0 oc=01 0
    spi_mask_set(GPIO_cfgA, 0xffffff0f, 0x00000020); // pads 0-7
    // D/C (1): 0x01 = 0 hds=1
    spi_mask_set(GPIO_altpadcfgA, 0xffff00ff, 0x00000100); // pads 0-3
    *(int *)GPIO_padkey = 0;

    // Pull CS and D/C high.
    spi_change_cs(1);
    ILI9341_change_dc(1);
}

void ILI9341_change_dc(int high)
{
    if (high)
        *(int *)GPIO_wtsA = 0x2;
    else
        *(int *)GPIO_wtcA = 0x2;
}

unsigned char ILI9341_responses[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int ILI9341_send_command(int cmd, int responses)
{
    // Pull CS and D/C low, send the command byte, then pull them high.
    spi_change_cs(0);
    ILI9341_change_dc(0);
    spi_send_byte(cmd);
    ILI9341_change_dc(1);

    int res = 0;
    int i = 0;
    while (i < responses) {
        res = spi_send_byte(cmd);
        ILI9341_responses[i] = res & 0xff;
        i++;
    }
    spi_change_cs(1);
    return res;
}

int ILI9341_send_parameter(int par)
{
    // Pull CS low, leaving D/C high, send the byte, then pull CS high.
    spi_change_cs(0);
    int res = spi_send_byte(par);
    spi_change_cs(1);
    return res;
}

int ILI9341_send_command_args(char *addr)
{
    int cmd = (int)*addr;
    int args = (int)*(addr + 1);

    int res = ILI9341_send_command(cmd, 0);

    int offset = 2;
    while (args > 0) {
        res = ILI9341_send_parameter(*(unsigned char *)(addr + offset));
        args--;
        offset++;
    }

    return res;
}

void ILI9341_set_window(int x0, int y0, int x1, int y1)
{
    ILI9341_send_command(ILI9341_ColumnAddressSet, 0);
    ILI9341_send_parameter(x0 >> 8);
    ILI9341_send_parameter(x0 & 0xff);
    ILI9341_send_parameter(x1 >> 8);
    ILI9341_send_parameter(x1 & 0xff);
    ILI9341_send_command(ILI9341_PageAddressSet, 0);
    ILI9341_send_parameter(y0 >> 8);
    ILI9341_send_parameter(y0 & 0xff);
    ILI9341_send_parameter(y1 >> 8);
    ILI9341_send_parameter(y1 & 0xff);
}

void ILI9341_fill_rect(int x0, int y0, int x1, int y1, int rgb)
{
    ILI9341_set_window(x0, y0, x1, y1);
    ILI9341_send_command(ILI9341_MemoryWrite, 0);

    int high = rgb >> 8;
    int low = rgb & 0xff;

    spi_change_cs(0);
    int i = 0;
    int l = (x1 - x0 + 1) * (y1 - y0 + 1);
    while (i < l) {
        spi_send_byte(high);
        spi_send_byte(low);
        i++;
    }
    spi_change_cs(1);
}

void ILI9341_begin(void)
{
    ILI9341_set_window(0, 0, 239, 319);
    ILI9341_send_command(ILI9341_MemoryWrite, 0);
    spi_change_cs(0);
}

void ILI9341_end(void)
{
    spi_change_cs(1);
}

void ILI9341_start(void)
{
    ILI9341_set_window(0, 0, 239, 319);
    ILI9341_send_command(ILI9341_MemoryWrite, 0);
}
