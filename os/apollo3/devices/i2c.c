#include "u.h"
#include "../../port/lib.h"
#include "../mem.h"
#include "../dat.h"
#include "../fns.h"
#include "../io.h"
#include "../../port/error.h"
#include "apollo3.h"
#include "iom.h"

static void
i2c_mask_set(int addr, int mask, int set)
{
    *(int *)addr = (*(int *)addr & mask) | set;
}

static void
i2c_set_address(int addr)
{
    *(int *)(IOM4_BASE + IOM_DEVCFG) = addr & 0xff;
}

static void
i2c_wait_for_command(void)
{
    while ((*(int *)(IOM4_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CMDSTAT_ACTIVE) != 0);
}

void
i2csetup(int /* polling */)
{
    /* The SparkFun Arduino code defines IOM 4 for I2C and uses pad 39 for SCL
       and pad 40 for SDA. */
    i2c_mask_set(PWR_DEVPWREN, 0xffffffff, PWR_IOM4);
    int depends = PWR_HCPC | PWR_MCUL;
    while ((*(int *)(PWR_DEVPWRSTATUS) & depends) == 0);

    *(int *)GPIO_padkey = 0x73;
    /* Configure pads 48 for TX and 49 for RX (with input enabled) using
       function 0. */
    // SCL, SDA: FNCSEL=4, STRNG=INPEN=PULL=1
    i2c_mask_set(GPIO_padregJ, 0x00ffffff, 0x27000000);
    i2c_mask_set(GPIO_padregK, 0xffffff00, 0x00000027);
    // SCL, SDA: OD=1
    i2c_mask_set(GPIO_cfgE, 0x0fffffff, 0x40000000);
    i2c_mask_set(GPIO_cfgF, 0xfffffff0, 0x00000004);
    // SCL, SDA: DS=1
    i2c_mask_set(GPIO_altpadcfgJ, 0x00ffffff, 0x01000000);
    i2c_mask_set(GPIO_altpadcfgK, 0xffffff00, 0x00000001);
    *(int *)GPIO_padkey = 0;

    /* The SparkFun code sets the IOM4_CLKCFG register to 0x773b1201.
       This means TOTPER=0x77, LOWPER=0x3b, DIVEN=1, DIV3=0, FSEL=0x2, IOCLKEN=1
       which translates to counts of 0x78 and 0x3c for the programmable divider,
       divider enabled, no divide by 3, HFRC/2 as the input clock, and clock
       enabled.
       HFRC/2 = 24MHz, 0x78 = 120, so the cycle time is 24MHz/120 = 200kHz.
       According to 8.2.2.1 of the Apollo3 datasheet, the output clock is half
       this frequency. */
    *(int *)(IOM4_BASE + IOM_CLKCFG) = 0x773b1201;

    // Disable IOM4 interrupts.
    *(int *)(IOM4_BASE + IOM_INTEN) = 0;

    // Enable the I2C submodule.
    *(int *)(IOM4_BASE + IOM_SUBMODCTRL) = IOM_SMOD1TYPE_I2CMASTER | IOM_SMOD1EN;
}

long
i2crecv(I2Cdev *d, void *buf, long len, ulong offset)
{
//    print("i2crecv %ld %uld\n", len, offset);
    if (len > 32) return -1;

    // If not in subaddress mode ignore the offset.
    if (!d->salen)
        offset = 0;

    i2c_set_address(d->addr);

    *(int *)(IOM4_BASE + IOM_OFFSETHI) = offset >> 8;
    *(int *)(IOM4_BASE + IOM_CMD) =
            ((offset & 0xff) << IOM_CMD_OFFSETLO_SHIFT) |
            (1 << IOM_CMD_OFFSET_CNT_SHIFT) |
            (len << IOM_CMD_TSIZE_SHIFT) |
            IOM_CMD_READ;

    i2c_wait_for_command();

    int err = *(int *)(IOM4_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CMDSTAT_ERR;
    int rem = *(int *)(IOM4_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CTSIZE_MASK;

    if (err != 0)
        return -1;
    else if ((len > 0) && (rem > 0))
        return -1;

    /* Copy the bytes from the read FIFO (32 bytes into the FIFO buffer) to the
       supplied buffer. */
    for (int i = 0; i < len; i += 4) {
        int w = *(int *)(IOM4_BASE + IOM_FIFOPOP);
        rem = (len - i);
        if (rem >= 4)
            memmove((void *)((int)buf + i), (void *)(&w), 4);
        else
            memmove((void *)((int)buf + i), (void *)(&w), rem);
    }
    return len;
}

long
i2csend(I2Cdev *d, void *buf, long len, ulong offset)
{
//    print("i2csend %ld %uld\n", len, offset);
    if (len > 32) return -1;

    // If not in subaddress mode ignore the offset.
    if (!d->salen)
        offset = 0;

    i2c_set_address(d->addr);

    /* Copy the bytes from the supplied buffer into the write FIFO. */
    for (int i = 0; i < len; i += 4) {
        int w;
        memmove((void *)(&w), (void *)((int)buf + i), 4);
        *(int *)(IOM4_BASE + IOM_FIFOPUSH) = w;
    }

    *(int *)(IOM4_BASE + IOM_OFFSETHI) = offset >> 8;
    *(int *)(IOM4_BASE + IOM_CMD) =
           ((offset & 0xff) << IOM_CMD_OFFSETLO_SHIFT) |
           (1 << IOM_CMD_OFFSET_CNT_SHIFT) |
           (len << IOM_CMD_TSIZE_SHIFT) |
           IOM_CMD_WRITE;

    i2c_wait_for_command();

    if (*(int *)(IOM4_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CMDSTAT_ERR)
        return -1;
    else
        return len;
}
