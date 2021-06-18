/*
    MMC/SD card (MSC)
    Based heavily on rpi's sdmmc.c but doesn't use a separate implementation to
    access the card
*/

#include "u.h"
#include "../../port/lib.h"
#include "../port/error.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "hardware.h"
#include "peripherals/mmc.h"
#include "../port/sd.h"

#define R1_RESP_ERR(v) ((v & 0xffff0000) != 0)
#define R7_RESP_ERR(v) ((v & 0xfffff000) != 0)

static uchar buf[16];
static ulong resp;
static MMC mmc;

/* Export the SD interface for this implementation */
extern SDifc sdmscifc;

typedef struct {
	SDev	*dev;
	SDio	*io;
	/* SD card registers */
	u16int	rca;
	u32int	ocr;
	u32int	cid[4];
	u32int	csd[4];
} Ctlr;

void msc_dump(void)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);
    print("%8.8lux\n", msc->clock_control);
    print("%8.8lux\n", msc->status);
    print("%8.8lux\n", msc->clock_rate);
    print("%8.8lux\n", msc->cmd_control);
}

/* Read at most 32 bits from a long integer in b, starting at the position
   given by shift and applying the mask at that position.
*/
static ulong read_bits(uchar *b, uint shift, ulong mask)
{
    ulong *l = (ulong *)b;
    ulong v = 0;

    /* Seek to the shift position */
    l += shift / 32;

    int sh = shift % 32;
    if (sh != 0) {
        /* Extract bits part way through a word */
        v = (*l >> sh) & mask;
        mask >>= (32 - sh);
        l++;
    }

    if (mask)
        v |= (*l & mask) << (32 - sh);

    return v;
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

    uvlong _csize;
    ulong csize_mult, blocklen;

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

        /* Card size in bytes: (_csize + 1) * 512K */
        csd->card_size = (_csize + 1) * 1024 * 512;
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
    print("CSD: ver=%d size=%llud bl=%d\n", csd->version, csd->card_size,
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

/* Read a number of blocks from the card address (specified in blocks) into the
   destination buffer.
*/
ulong msc_read_block(ulong card_addr, ulong *dest)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    if (!mmc.ccs)
        card_addr = card_addr * mmc.csd.block_len;

    /* Stop the clock - Linux does a stop and start for each command but the
       SoC documentation doesn't suggest this */
    msc->clock_control = MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    msc->number_of_blocks = 1;
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

    /* Wait until the transaction is complete */
    while (!(msc->status & MSC_Status_DataTranDone))
        ;

    return (msc->status & MSC_Status_CRCReadError);
}

ulong msc_write_block(ulong card_addr, ulong *src)
{
    MSC *msc = (MSC *)(MSC_BASE | KSEG1);

    if (!mmc.ccs)
        card_addr = card_addr * mmc.csd.block_len;

    /* Stop the clock - Linux does a stop and start for each command but the
       SoC documentation doesn't suggest this */
    msc->clock_control = MSC_ClockCtrl_StopClock;
    while (msc->status & MSC_Status_ClockEnabled)
        ;

    msc->number_of_blocks = 1;
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

    /* Wait until the transaction is complete */
    while (!(msc->status & MSC_Status_PrgDone))
        ;

    return (msc->status & MSC_Status_CRCWriteErrors);
}

static int
msc_init(void)
{
    //InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    mmc_sd = &mmc;
    mmc_sd->rca = 0;

    /* Propagate the MSC clock by clearing the appropriate bit */
    *(ulong*)(CGU_MSCR | KSEG1) &= ~CGU_MSC;

    /* Unmask the MSC interrupt */
    //ic->mask_clear = InterruptMSC;

    return 0;
}

static SDev*
msc_pnp(void)
{
    SDev *sdev;
    Ctlr *ctl;

    if(msc_init() < 0)
        return nil;
    sdev = malloc(sizeof(SDev));
    if(sdev == nil)
        return nil;
    ctl = malloc(sizeof(Ctlr));
    if(ctl == nil){
        free(sdev);
        return nil;
    }
    sdev->idno = 'M';
    sdev->ifc = &sdmscifc;
    sdev->nunit = 1;
    sdev->ctlr = ctl;
    ctl->dev = sdev;
    return sdev;
}

static int
msc_inquiry(char *inquiry, int inqlen)
{
    return snprint(inquiry, inqlen, "Ingenic MMC/SDC Controller");
}

static int
msc_verify(SDunit *unit)
{
    int n;
//    Ctlr *ctl = unit->dev->ctlr;
    n = msc_inquiry((char*)&unit->inquiry[8], sizeof(unit->inquiry)-8);
    if(n < 0)
        return 0;
    unit->inquiry[0] = SDperdisk;
    unit->inquiry[1] = SDinq1removable;
    unit->inquiry[4] = sizeof(unit->inquiry)-4;
    return 1;
}

static int
msc_enable(SDev* dev)
{
    USED(dev);
    return 1;
}

static int
msc_online(SDunit *unit)
{
    Ctlr *ctl = unit->dev->ctlr;

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
        return 0;
    }

    ulong sdhc = SD_OCR_HCS;
    if ((resp & 0xff) == 0xaa) {
        if (resp & 0x100)
            mmc.voltages = 0xff8000;
        else
            mmc.voltages = 0;
    } else {
        print("Not an SD card\n");
        return 0;
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
            return 0;
        }

        /* Supported voltages must be passed in SD mode */
        msc_send_command(CMD_SD_APP_OP_COND, 3, sdhc | mmc.voltages);
        resp = msc_response();
        if ((resp & 0xff8000) != 0x00ff8000) {  // for the Minibook
            print("SD_APP_OP_COND: %8.8lux\n", resp);
            return 0;
        }

        if (resp & SD_OCR_READY)
            break;
    }

    if (c == limit) {
        print("ACMD41: %8.8lux\n", resp);
        return 0;
    }

    /* CMD58 (read OCR) */
    msc_send_command(58, 3, 0);
    resp = msc_response();
    ctl->ocr = resp;

    mmc.voltages = resp & 0xff8000;
    mmc.ccs = resp & SD_OCR_HCS;

    msc_send_command(CMD_ALL_SEND_CID, 2, 0);
    read_cid(buf, &mmc.cid);

    msc_send_command(CMD_SEND_RELATIVE_ADDR, 6, 0);
    resp = msc_response();
    mmc.rca = resp >> 16;
    ctl->rca = resp >> 16;

    msc_send_command(CMD_SEND_CID, 2, mmc.rca << 16);
    read_cid(buf, &mmc.cid);
    memmove(ctl->cid, buf, sizeof ctl->cid);

    msc_send_command(CMD_SEND_CSD, 2, mmc.rca << 16);
    if (read_csd(buf, &mmc.csd)) {
        print("CSD?\n");
        print_buf(buf);
        return 0;
    }
    memmove(ctl->csd, buf, sizeof ctl->csd);
    unit->sectors = mmc.csd.card_size / 512;
    unit->secsize = 512;

    if (mmc.csd.speed == 0x32 || mmc.csd.speed == 0x5a)
        msc->clock_rate = 0;

    if (msc_select_card(mmc.rca)) {
        print("CMD7: %8.8lux\n", resp);
        return 0;
    }

    msc_send_command(CMD_SET_BLOCKLEN, 1, 512);
    resp = msc_response();
    if (R1_RESP_ERR(resp)) {
        print("CMD16: %8.8lux\n", resp);
        return 0;
    }

    return 1;
}

static int
msc_rctl(SDunit *unit, char *p, int l)
{
    Ctlr *ctl;
    int i, n;

    ctl = unit->dev->ctlr;
    assert(unit->subno == 0);
    if (unit->sectors == 0){
        msc_online(unit);
        if (unit->sectors == 0)
            return 0;
    }
    n = snprint(p, l, "rca %4.4ux ocr %8.8ux\ncid ", ctl->rca, ctl->ocr);
    for(i = nelem(ctl->cid)-1; i >= 0; i--)
        n += snprint(p+n, l-n, "%8.8ux", ctl->cid[i]);
    n += snprint(p+n, l-n, " csd ");
    for(i = nelem(ctl->csd)-1; i >= 0; i--)
        n += snprint(p+n, l-n, "%8.8ux", ctl->csd[i]);
    n += snprint(p+n, l-n, "\ngeometry %llud %ld\n",
        unit->sectors, unit->secsize);
    return n;
}

static long
msc_bio(SDunit *unit, int lun, int write, void *data, long nb, uvlong bno)
{
    int len;
    uchar *buf;
    ulong b;

    USED(lun);
    assert(unit->subno == 0);
    if (unit->sectors == 0)
        error("media change");
    buf = data;
    len = unit->secsize;

    if (write) {
	for (b = bno; b < bno + nb; b++) {
            msc_write_block(b, (ulong*)buf);
            buf += len;
        }
    } else {
	for (b = bno; b < bno + nb; b++) {
            msc_read_block(b, (ulong*)buf);
            buf += len;
        }
    }

    return nb * len;
}

static int
msc_rio(SDreq*)
{
    return -1;
}

SDifc sdmscifc = {
    .name	= "msc",
    .pnp	= msc_pnp,
    .enable	= msc_enable,
    .verify	= msc_verify,
    .online	= msc_online,
    .rctl	= msc_rctl,
    .bio	= msc_bio,
    .rio	= msc_rio,
};
