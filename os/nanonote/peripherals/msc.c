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

MMC_CID mmc_cid;

void msc_dump(void)
{
    MMC *mmc = (MMC *)(MSC_BASE | KSEG1);
    print("%8.8lux\n", mmc->clock_control);
    print("%8.8lux\n", mmc->status);
    print("%8.8lux\n", mmc->clock_rate);
    print("%8.8lux\n", mmc->cmd_control);
}

void print_cid(MMC_CID *cid)
{
    int i;
    print("%2.2ux%4.4ux", cid->manufacturer_id, cid->application_id);
    for (i = 0; i < 5; i++) print("%2.2ux", (uchar)cid->product_name[i]);
    print("%2.2ux%8.8ux%4.4ux%2.2ux\n", cid->revision, cid->serial_number,
                                        cid->manufacturing_date, cid->crc);

    print("man=%2.2ux app=%4.4ux name=", cid->manufacturer_id, cid->application_id);
    for (i = 0; i < 5; i++) print("%c", cid->product_name[i]);
    print("\nrev=%2.2ux ser=%8.8ux", cid->revision, cid->serial_number);
    print(" date=%2.2ux\n", cid->manufacturing_date);
}

void msc_enter_spi(void)
{
    MMC *mmc = (MMC *)(MSC_BASE | KSEG1);
    mmc->cmd_index = 0;
    mmc->cmd_arg = 0;
    mmc->cmd_control &= ~MMC_CmdCtrl_ResponseFormat;
    mmc->cmd_control |= 1;
    mmc->cmd_control &= ~MMC_CmdCtrl_DataEnabled;
    mmc->cmd_control &= ~MMC_CmdCtrl_Busy;
    mmc->cmd_control |= MMC_CmdCtrl_Init;

    /* Start the operation */
    mmc->clock_control |= MMC_ClockCtrl_StartOp;

    /* Wait until the command completes */
    while (!(mmc->status & MMC_Status_EndCmdRes))
        ;
}
void msc_send_command(ulong cmd, ulong resp_format, ulong arg)
{
    MMC *mmc = (MMC *)(MSC_BASE | KSEG1);
    mmc->cmd_index = cmd;
    mmc->cmd_arg = arg;
    mmc->cmd_control &= ~MMC_CmdCtrl_BusWidth;
    mmc->cmd_control |= MMC_CmdCtrl_FourBit;
    mmc->cmd_control &= ~MMC_CmdCtrl_ResponseFormat;
    mmc->cmd_control |= resp_format;
    mmc->cmd_control &= ~MMC_CmdCtrl_DataEnabled;
    mmc->cmd_control &= ~MMC_CmdCtrl_Busy;
    mmc->cmd_control &= ~MMC_CmdCtrl_Init;

    /* Start the operation */
    mmc->clock_control |= MMC_ClockCtrl_StartOp;

    /* Wait until the command completes */
    while (!(mmc->status & MMC_Status_EndCmdRes))
        ;
}

ulong msc_response(void)
{
    MMC *mmc = (MMC *)(MSC_BASE | KSEG1);
    /* Bits 32-48 (command number, top 8 reserved bits (0)) */
    ulong resp = (mmc->resp_fifo & 0xff) << 24;
    resp |= (mmc->resp_fifo & 0xffff) << 8;
    ulong t = mmc->resp_fifo;
    //print("%4.4ux\n", t >> 8);
    return resp | (t & 0xff);
}

static ulong resp, voltages, ccs, cid, rca, csd;

void msc_reset(void)
{
    MMC *mmc = (MMC *)(MSC_BASE | KSEG1);

    /* Stop the clock */
    mmc->clock_control |= MMC_ClockCtrl_StopClock;
    while (mmc->status & MMC_Status_ClockEnabled)
        ;

    /* Reset the card */
    mmc->clock_control |= MMC_ClockCtrl_Reset;
    while (mmc->status & MMC_Status_IsResetting)
        ;

    mmc->clock_rate = 6;

    /* Start the clock */
    mmc->clock_control |= MMC_ClockCtrl_StartClock;
    while (!(mmc->status & MMC_Status_ClockEnabled))
        ;

    /* Send CMD0 with response type 1 */
    msc_send_command(CMD_GO_IDLE_STATE, 1, 0);

    /* Send CMD52
    msc_send_command(CMD_IO_RW_DIRECT, 5, 0x88000c08);
    print("%4.4ux\n", mmc->resp_fifo % 0xff); */

    /* CMD8 (R7) */
    msc_send_command(CMD_SEND_IF_COND, 1, 0x1aa);
    resp = msc_response();
    print("CMD8: %8.8lux\n", resp);

    if ((mmc->status & MMC_Status_CRCResError) || R7_RESP_ERR(resp)) {
        print("IF_COND: %8.8lux\n", resp);
        return;
    }

    ulong sdhc = SD_OCR_HCS;
    if ((resp & 0xff) == 0xaa) {
        if (resp & 0x100)
            voltages = 0xff8000;
        else
            voltages = 0;
    } else {
        print("Not an SD card\n");
        return;
    }

    /* ACMD41 = CMD55 + CMD41 */
    ulong c = 0;
    const ulong limit = 1000;

    for (c = 0; c < limit; c++)
    {
        msc_send_command(CMD_APP_CMD, 1, 0);
        resp = msc_response();
        //print("CMD55: %8.8lux\n", resp);
        if (resp != 0x120) {
            print("APP_CMD: %8.8lux\n", resp);
            return;
        }

        /* Supported voltages must be passed in SD mode */
        msc_send_command(CMD_SD_APP_OP_COND, 3, sdhc | voltages);
        resp = msc_response();
        //print("ACMD41: %8.8lux\n", resp);
        if ((resp & 0xff8000) != 0x00ff8000) {  // for the NanoNote
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
    print("CMD58: %8.8lux\n", resp);

    voltages = resp & 0xff8000;
    ccs = resp & SD_OCR_HCS;
    print("V: %4.4lux CCS: %d\n", voltages, ccs == SD_OCR_HCS);

    msc_send_command(CMD_ALL_SEND_CID, 2, 0);
    ushort *ptr = (ushort *)&mmc_cid;
    for (int i = 0; i < 8; i++)
        ptr[i] = mmc->resp_fifo;

    print("CMD2:\n");
    print_cid(&mmc_cid);

    msc_send_command(CMD_SEND_RELATIVE_ADDR, 6, 0);
    resp = msc_response();
    rca = resp >> 16;
    print("CMD3: %8.8lux\n", resp);

    msc_send_command(CMD_SELECT_CARD, 1, rca << 16);
    resp = msc_response();
    if (R1_RESP_ERR(resp)) {
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

void msc_init(void)
{
    //InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    /* Propagate the UDC clock by clearing the appropriate bit */
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~CGU_MSC;

    /* Unmask the MSC interrupt */
    //ic->mask_clear = InterruptMSC;

    msc_reset();
}
