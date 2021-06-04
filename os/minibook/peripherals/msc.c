/*
    MMC/SD card (MSC)
*/

#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"
#include "mmc.h"

#define R1_RESP_ERR(v) ((v & 0xffff0000) != 0)
#define R7_RESP_ERR(v) ((v & 0xfffff000) != 0)

static uchar buf[16];
static ulong resp;
static MMC mmc;

void msc_dump(void)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);
    print("%8.8lux\n", msc->clock_control);
    print("%8.8lux\n", msc->status);
    print("%8.8lux\n", msc->clock_rate);
    print("%8.8lux\n", msc->cmd_control);
}

static ulong read_bits(uchar *b, uint shift, ulong mask)
{
    ulong *l = (ulong *)b;
    l += shift / 32;
    ulong offset = shift % 32;
    mask = mask << offset;
    return (*l & mask) >> offset;
}

static void read_cxd(uchar *buf)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    /* Read the contents of the FIFO into a buffer */
    ushort *sptr = (ushort *)buf;
    for (int i = 0; i < 8; i++)
        sptr[7 - i] = msc->resp_fifo;

    /* It looks like the contents of the FIFO can be shifted down by a byte
       for CID/CSD responses, so shift it back up again, starting with the top
       byte which should be the CRC in byte 0. */

    uchar t1 = buf[15];

    for (int i = 0; i < 16; i++) {
        uchar t2 = buf[i];
        buf[i] = t1;
        t1 = t2;
    }
}

static void read_cid(uchar *buf, MMC_CID *cid)
{
    read_cxd(buf);

    cid->manufacturer_id = read_bits(buf, 120, 0xff);
    cid->application_id = read_bits(buf, 104, 0xffff);
    for (int i = 0; i < 5; i++)
        cid->product_name[4 - i] = (char)buf[8 + i];
    cid->product_name[5] = 0;
    cid->revision = read_bits(buf, 56, 0xff);
    cid->serial_number = read_bits(buf, 24, 0xffffffff);
    cid->manufacturing_date = read_bits(buf, 8, 0xfff);
    cid->crc = read_bits(buf, 0, 0xff);
}

static int read_csd(uchar *buf, MMC_CSD *csd)
{
    read_cxd(buf);

    csd->version = (uchar)read_bits(buf, 126, 0x3);

    ulong _csize, csize_mult, blocklen;

    switch (csd->version)
    {
    case 0: /* CSD version 1 */
        _csize = read_bits(buf, 62, 0xfff);
        csize_mult = read_bits(buf, 47, 0x7);
        blocklen = read_bits(buf, 80, 0xf);
        if (blocklen < 9)
            blocklen = read_bits(buf, 22, 0xf);

        csd->block_len = (ushort)(1 << blocklen);
        csd->card_size = csd->block_len * (1 << (csize_mult + 2)) * (_csize + 1);
        csd->speed = read_bits(buf, 96, 0xff);
        break;

    case 1: /* CSD version 2 */
        _csize = read_bits(buf, 48, 0x3fffff);
        blocklen = read_bits(buf, 80, 0xf);
        if (blocklen < 9)
            blocklen = read_bits(buf, 22, 0xf);

        csd->card_size = (_csize + 1) * 1024;
        csd->block_len = (ushort)(1 << blocklen);
        csd->speed = read_bits(buf, 96, 0xff);
        break;

    default:
        return 1;
    }

    return 0;
}

static void print_cid(MMC_CID *cid)
{
    print("man=%2.2ux app=%4.4ux ", cid->manufacturer_id, cid->application_id);
    print("name=%s\n", cid->product_name);
    print("rev=%2.2ux ser=%8.8ux ", cid->revision, cid->serial_number);
    print("date=%2.2ux\n", cid->manufacturing_date);
}

static void print_buf(uchar *buf)
{
    for (int i = 15; i >= 0; i--)
        print("%2.2ux", buf[i]);
    print("\n");
}

static void print_csd(MMC_CSD *csd)
{
    print("CSD: ver=%d size=%lud bl=%d\n", csd->version, csd->card_size,
                                           csd->block_len);
}

static void msc_send_command(ulong cmd, ulong resp_format, ulong arg)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    /* Stop the clock - Linux does a stop and start for each command but the
       SoC documentation doesn't suggest this */
    msc->clock_control = MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    msc->cmd_index = cmd;
    msc->cmd_arg = arg;
    msc->cmd_control &= ~MSC_CmdCtrl_BusWidth;
    msc->cmd_control |= MSC_CmdCtrl_FourBit;
    msc->cmd_control &= ~MSC_CmdCtrl_ResponseFormat;
    msc->cmd_control |= resp_format;
    msc->cmd_control &= ~MSC_CmdCtrl_DataEnabled;
    msc->cmd_control &= ~MSC_CmdCtrl_Busy;
    msc->cmd_control &= ~MSC_CmdCtrl_Init;

    /* Start the clock and the operation */
    msc->clock_control = MSC_ClockCtrl_StartClock | MSC_ClockCtrl_StartOp;
    while (!(msc->status & MSC_Status_ClockEnabled))
        ;

    /* Wait until the command completes */
    while (!(msc->status & MSC_Status_EndCmdRes))
        ;
}

static ulong msc_response(void)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);
    /* Bits 32-48 (command number, top 8 reserved bits (0)) */
    ulong resp = (msc->resp_fifo & 0xff) << 24;
    resp |= (msc->resp_fifo & 0xffff) << 8;
    ulong t = msc->resp_fifo;
    return resp | (t & 0xff);
}

static int msc_select_card(ulong rca)
{
    msc_send_command(CMD_SELECT_CARD, 1, rca << 16);
    resp = msc_response();
    if (R1_RESP_ERR(resp))
        return 1;

    return 0;
}

void msc_reset(void)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    /* Stop the clock */
    msc->clock_control |= MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    /* Reset the card */
    msc->clock_control |= MSC_ClockCtrl_Reset;
    while (msc->status & MSC_Status_IsResetting)
        ;

    msc->clock_rate = 6;

    /* Send CMD0 with response type 1 */
    msc_send_command(CMD_GO_IDLE_STATE, 1, 0);

    /* Send CMD52
    msc_send_command(CMD_IO_RW_DIRECT, 5, 0x88000c08);
    print("%4.4ux\n", msc->resp_fifo % 0xff); */

    /* CMD8 (R7) */
    msc_send_command(CMD_SEND_IF_COND, 1, 0x1aa);
    resp = msc_response();

    if ((msc->status & MSC_Status_CRCResError) || R7_RESP_ERR(resp)) {
        print("IF_COND: %8.8lux\n", resp);
        return;
    }

    ulong sdhc = SD_OCR_HCS;
    if ((resp & 0xff) == 0xaa) {
        if (resp & 0x100)
            mmc.voltages = 0xff8000;
        else
            mmc.voltages = 0;
    } else {
        print("Not an SD card\n");
        return;
    }

    /* ACMD41 = CMD55 + CMD41 */
    ulong c;
    const ulong limit = 1000;

    for (c = 0; c < limit; c++)
    {
        msc_send_command(CMD_APP_CMD, 1, 0);
        resp = msc_response();
        if (resp != 0x120) {
            print("APP_CMD: %8.8lux\n", resp);
            return;
        }

        /* Supported voltages must be passed in SD mode */
        msc_send_command(CMD_SD_APP_OP_COND, 3, sdhc | mmc.voltages);
        resp = msc_response();
        if ((resp & 0xff8000) != 0x00ff8000) {  // for the Minibook
            print("SD_APP_OP_COND: %8.8lux\n", resp);
            return;
        }

        if (resp & SD_OCR_READY)
            break;
    }

    if (c == limit) {
        print("ACMD41: %8.8lux\n", resp);
        return;
    }

    /* CMD58 (read OCR) */
    msc_send_command(58, 3, 0);
    resp = msc_response();

    mmc.voltages = resp & 0xff8000;
    mmc.ccs = resp & SD_OCR_HCS;

    msc_send_command(CMD_ALL_SEND_CID, 2, 0);
    read_cid(buf, &mmc.cid);

    msc_send_command(CMD_SEND_RELATIVE_ADDR, 6, 0);
    resp = msc_response();
    mmc.rca = resp >> 16;

    msc_send_command(CMD_SEND_CID, 2, mmc.rca << 16);
    read_cid(buf, &mmc.cid);

    msc_send_command(CMD_SEND_CSD, 2, mmc.rca << 16);
    if (read_csd(buf, &mmc.csd)) {
        print("CSD?\n");
        print_buf(buf);
        return;
    }

    if (mmc.csd.speed == 0x32 || mmc.csd.speed == 0x5a)
        msc->clock_rate = 0;

    if (msc_select_card(mmc.rca)) {
        print("CMD7: %8.8lux\n", resp);
        return;
    }

    msc_send_command(CMD_SET_BLOCKLEN, 1, 512);
    resp = msc_response();
    if (R1_RESP_ERR(resp)) {
        print("CMD16: %8.8lux\n", resp);
        return;
    }
}

ulong msc_read(ulong card_addr, ulong *dest, ushort blocks)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    if (!mmc.ccs)
        card_addr = card_addr * mmc.csd.block_len;

    /* Stop the clock - Linux does a stop and start for each command but the
       SoC documentation doesn't suggest this */
    msc->clock_control = MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    msc->number_of_blocks = blocks;
    msc->block_length = mmc.csd.block_len;

    msc->cmd_index = CMD_READ_SINGLE_BLOCK;
    msc->cmd_arg = card_addr;
    msc->cmd_control &= ~MSC_CmdCtrl_BusWidth;
    msc->cmd_control |= MSC_CmdCtrl_FourBit;
    msc->cmd_control &= ~MSC_CmdCtrl_ResponseFormat;
    msc->cmd_control |= 1;
    msc->cmd_control |= MSC_CmdCtrl_DataEnabled;
    msc->cmd_control &= ~MSC_CmdCtrl_WriteRead;
    msc->cmd_control &= ~MSC_CmdCtrl_StreamBlock;
    msc->cmd_control &= ~MSC_CmdCtrl_Busy;
    msc->cmd_control &= ~MSC_CmdCtrl_Init;

    /* Start the clock and the operation */
    msc->clock_control = MSC_ClockCtrl_StartClock | MSC_ClockCtrl_StartOp;
    while (!(msc->status & MSC_Status_ClockEnabled))
        ;

    /* Wait until the command completes */
    while (!(msc->status & MSC_Status_EndCmdRes))
        ;

    ulong blocks_left = blocks;

    while ((blocks_left > 0) && !(msc->status & MSC_Status_CRCReadError))
    {
        /* Read until a whole block has been received */
        ulong read = 0;
        ulong words = msc->block_length/4;

        while ((read < words) && !(msc->status & MSC_Status_CRCReadError))
        {
            if (!(msc->status & MSC_Status_DataFIFOEmpty)) {
                dest[read] = msc->rec_data_fifo;
                read++;
            }
        }
        blocks_left--;
    }

    /* Wait until the transaction is complete */
    while (!(msc->status & MSC_Status_DataTranDone))
        ;

    /* Stop transmission after multiple blocks have been received */
    if (blocks > 1)
        msc_send_command(CMD_STOP_TRANSMISSION, 1, 0);

    return (msc->status & MSC_Status_CRCReadError);
}

ulong msc_write(ulong card_addr, ulong *src, ushort blocks)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    if (!mmc.ccs)
        card_addr = card_addr * mmc.csd.block_len;

    /* Stop the clock - Linux does a stop and start for each command but the
       SoC documentation doesn't suggest this */
    msc->clock_control = MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    msc->number_of_blocks = blocks;
    msc->block_length = mmc.csd.block_len;

    msc->cmd_index = CMD_WRITE_BLOCK;
    msc->cmd_arg = card_addr;
    msc->cmd_control &= ~MSC_CmdCtrl_BusWidth;
    msc->cmd_control |= MSC_CmdCtrl_FourBit;
    msc->cmd_control &= ~MSC_CmdCtrl_ResponseFormat;    // response format
    msc->cmd_control |= 1;                              // is R1
    msc->cmd_control |= MSC_CmdCtrl_DataEnabled;
    msc->cmd_control |= MSC_CmdCtrl_WriteRead;
    msc->cmd_control &= ~MSC_CmdCtrl_StreamBlock;
    msc->cmd_control &= ~MSC_CmdCtrl_Busy;
    msc->cmd_control &= ~MSC_CmdCtrl_Init;

    /* Start the clock and the operation */
    msc->clock_control = MSC_ClockCtrl_StartClock | MSC_ClockCtrl_StartOp;
    while (!(msc->status & MSC_Status_ClockEnabled))
        ;

    /* Wait until the command completes */
    while (!(msc->status & MSC_Status_EndCmdRes))
        ;

    ulong blocks_left = blocks;

    while ((blocks_left > 0) && !(msc->status & MSC_Status_CRCWriteErrors))
    {
        /* Write until a whole block has been written */
        ulong written = 0;
        ulong words = msc->block_length/4;

        while ((written < words) && !(msc->status & MSC_Status_CRCWriteErrors))
        {
            if (!(msc->status & MSC_Status_DataFIFOFull)) {
                msc->trans_data_fifo = src[written];
                written++;
            }
        }
        blocks_left--;
    }

    /* Wait until the transaction is complete */
    while (!(msc->status & MSC_Status_PrgDone))
        ;

    /* Stop transmission after multiple blocks have been sent */
    if (blocks > 1)
        msc_send_command(CMD_STOP_TRANSMISSION, 1, 0);

    return (msc->status & MSC_Status_CRCWriteErrors);
}

void msc_init(void)
{
    //InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    mmc_sd = &mmc;
    mmc_sd->rca = 0;

    /* Propagate the MSC clock by clearing the appropriate bit */
    *(ulong*)(CGU_MSCR | KSEG1) &= ~CGU_MSC;

    /* Unmask the MSC interrupt */
    //ic->mask_clear = InterruptMSC;

    msc_reset();
}
