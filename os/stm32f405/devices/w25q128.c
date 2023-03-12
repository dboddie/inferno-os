#include "fns.h"

const char W25Q128_DeviceID[6] = {0x04, 0x02, 0x90, 0x00, 0x00, 0x00};
const char W25Q128_ReadStatus1[3] = {0x01, 0x01, 0x05};
const char W25Q128_ReadStatus2[3] = {0x01, 0x01, 0x35};
const char W25Q128_ReadStatus3[3] = {0x01, 0x01, 0x15};
const char W25Q128_WriteEnable[3] = {0x01, 0x00, 0x06};
const char W25Q128_WriteDisable[3] = {0x01, 0x00, 0x04};
const char W25Q128_GlobalUnlock[3] = {0x01, 0x00, 0x98};

/* Worst case times for erase and write operations, plus 10% or 1ms. */
#define W25Q128_times_sector 440
#define W25Q128_times_block32 1760
#define W25Q128_times_block64 2200
#define W25Q128_times_chip 200000
#define W25Q128_times_pageprogram 4

char W25Q128_response[3] = {0x00, 0x00, 0x00};

#define W25Q128_PageProgram 0x02
#define W25Q128_ReadData 0x03
#define W25Q128_SectorErase 0x20
#define W25Q128_32KBlockErase 0x52
#define W25Q128_64KBlockErase 0xd8
#define W25Q128_ChipErase 0xc7

#define W25Q128_Status1_Busy 0x01
#define W25Q128_Status1_WriteEnable 0x02

void W25Q128_init()
{
    setup_spi(3);
    spi_set_frequency_divider((SPI *)SPI3, 0);
}

void spi3_change_cs(int high)
{
    GPIO *gpioc = (GPIO *)GPIO_C;
    if (high)
        gpioc->odr |= 0x08;
    else
        gpioc->odr &= ~0x08;
}

int W25Q128_send_command(const char cmd[])
{
    int res = 0;
    spi3_change_cs(0);

    char wr = cmd[0];
    char rd = cmd[1];

    for (int i = 0; i < wr; i++)
        spi_send_byte((SPI *)SPI3, cmd[2 + i]);

    for (int i = 0; i < rd; i++) {
        res = spi_send_byte((SPI *)SPI3, 0xff);
        W25Q128_response[i] = res;
    }

    spi3_change_cs(1);

    return res;
}

void W25Q128_print_response(const char cmd[])
{
    int rd = cmd[1];

    for (int i = 0; i < rd; i++) {
        if (i != 0)
            wrch(' ');

        wrhex(W25Q128_response[i]);
    }
    newline();
}

int W25Q128_busy_wait(int timeout)
{
    for (int t = 0; t < timeout; t++) {
        if ((W25Q128_send_command(W25Q128_ReadStatus1) & W25Q128_Status1_Busy) == 0)
            return 1;

        wait_ms(1);
    }
    return 0;
}

void W25Q128_read_data(int addr, char *a, int l)
{
    spi3_change_cs(0);

    spi_send_byte((SPI *)SPI3, W25Q128_ReadData);
    spi_send_byte((SPI *)SPI3, addr >> 16);
    spi_send_byte((SPI *)SPI3, (addr >> 8) & 0xff);
    spi_send_byte((SPI *)SPI3, addr & 0xff);

    for (int i = 0; i < l; i++)
        a[i] = spi_send_byte((SPI *)SPI3, 0xff);

    spi3_change_cs(1);
}

int W25Q128_wait_for(int bit, int set, int timeout)
{
    if (set)
        set = bit;

    for (int t = 0; t < timeout; t++) {
        if ((W25Q128_send_command(W25Q128_ReadStatus1) & bit) == set)
            return 1;

        wait_ms(1);
    }
    return 0;
}

/* Write a page at the given address using data from the offset into the byte array. */
int W25Q128_write_page_offset(int addr, char *a, int l, int offset, int wait)
{
    W25Q128_send_command(W25Q128_WriteEnable);
    if (!W25Q128_wait_for(W25Q128_Status1_WriteEnable, 1, 4))
        return 0;

    spi3_change_cs(0);
    spi_send_byte((SPI *)SPI3, W25Q128_PageProgram);
    spi_send_byte((SPI *)SPI3, addr >> 16);
    spi_send_byte((SPI *)SPI3, (addr >> 8) & 0xff);
    spi_send_byte((SPI *)SPI3, addr & 0xff);

    int f;
    int i = offset;
    if ((i + 256) < l)
        f = i + 256;
    else
        f = l;

    for (; i < f; i++)
        spi_send_byte((SPI *)SPI3, a[i]);

    spi3_change_cs(1);

    if (wait)
        return W25Q128_busy_wait(W25Q128_times_pageprogram);
    else
        return 1;
}

/* addr should really be 256 byte page-aligned; a should have 1 <= length <= 256. */
int W25Q128_write_page(int addr, char *a, int wait)
{
    return W25Q128_write_page_offset(addr, a, 256, 0, wait);
}

int W25Q128_erase_region(int addr, int cmd, int wait)
{
    W25Q128_send_command(W25Q128_WriteEnable);
    if (!W25Q128_wait_for(W25Q128_Status1_WriteEnable, 1, 4))
        return 0;

    spi3_change_cs(0);
    spi_send_byte((SPI *)SPI3, cmd);
    spi_send_byte((SPI *)SPI3, addr >> 16);
    spi_send_byte((SPI *)SPI3, (addr >> 8) & 0xff);
    spi_send_byte((SPI *)SPI3, addr & 0xff);
    spi3_change_cs(1);

    if (wait) {
        if (cmd == W25Q128_SectorErase)
            return W25Q128_busy_wait(W25Q128_times_sector);
        else if (cmd == W25Q128_32KBlockErase)
            return W25Q128_busy_wait(W25Q128_times_block32);
        else
            return W25Q128_busy_wait(W25Q128_times_block64);
    }
    return 1;
}

int W25Q128_erase_chip(void)
{
    W25Q128_send_command(W25Q128_WriteEnable);
    if (!W25Q128_wait_for(W25Q128_Status1_WriteEnable, 1, 4))
        return 0;

    spi3_change_cs(0);
    spi_send_byte((SPI *)SPI3, W25Q128_ChipErase);
    spi3_change_cs(1);

    return W25Q128_busy_wait(W25Q128_times_chip);
}

int W25Q128_unlock(void)
{
    W25Q128_send_command(W25Q128_WriteEnable);
    if (!W25Q128_wait_for(W25Q128_Status1_WriteEnable, 1, 4))
        return 0;

    W25Q128_send_command(W25Q128_GlobalUnlock);
    return 1;
}

void W25Q128_test(void)
{
    W25Q128_send_command(W25Q128_DeviceID);
    W25Q128_print_response(W25Q128_DeviceID);
}
