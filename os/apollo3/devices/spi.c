#include "fns.h"
#include "iom.h"

#define IOM3_BASE 0x50007000

void
spi_mask_set(int addr, int mask, int set)
{
    *(int *)addr = (*(int *)addr & mask) | set;
}

void
setup_spi(void)
{
    /* The system clock should already be set up at this point. */

    /* The SparkFun Arduino code defines IOM 3 for SPI and uses pads 38 for
       MOSI/COPI, 42 for SCK and 43 for MISO/CIPO. The pad used for CS_ is 41. */
    spi_mask_set(PWR_DEVPWREN, 0xffffffff, PWR_IOM3);
    int depends = PWR_HCPC | PWR_MCUL;
    while ((*(int *)PWR_DEVPWRSTATUS & depends) == 0);

    *(int *)GPIO_padkey = 0x73;
    /* Configure pads 38 (COPI), 42 (SCK) and 43 (CIPO) for function 5 (IOM3).
       COPI (38): 0x2c = 00 fn=101 ds=1 0 0 */
    spi_mask_set(GPIO_padregJ, 0xff00ffff, 0x002c0000); // pads 36-39
    /* CS_ (41): 0x1e = 00 fn=011 ds=1 ie=1 0
       SCK (42): 0x2e = 00 fn=101 ds=1 ie=1 0
       CIPO (43): 0x2e = 00 fn=101 ds=1 ie=1 0 */
    spi_mask_set(GPIO_padregK, 0x000000ff, 0x2e2e1e00); // pads 40-43
    // COPI (38): 0x2 = 0 oc=01 0
    spi_mask_set(GPIO_cfgE, 0xf0ffffff, 0x02000000);    // pads 32-39
    /* CS_ (41): 0x2 = 0 oc=01 0
       SCK (42): 0x2 = 0 oc=01 0
       CIPO (43): 0x2 = 0 oc=01 0 */
    spi_mask_set(GPIO_cfgF, 0xffff000f, 0x00002220);       // pads 40-47
    // COPI (38): 0x01 = 0 hds=1
    spi_mask_set(GPIO_altpadcfgJ, 0xff00ffff, 0x00010000); // pads 36-39
    /* CS_ (41): 0x01 = 0 hds=1
       SCK (42): 0x01 = 0 hds=1
       CIPO (43): 0x01 = 0 hds=1 */
    spi_mask_set(GPIO_altpadcfgK, 0x000000ff, 0x01010100); // pads 40-43
    *(int *)GPIO_padkey = 0;

    /* The SparkFun code sets the IOM3_CLKCFG register to 0xf01.
       This means TOTPER=0, LOWPER=0, DIVEN=0, DIV3=1, FSEL=0x7, IOCLKEN=1
       which translates to divide by 3, HFRC/64 as the input clock, and clock
       enabled.
       HFRC/64 = 48000000/64 = 750000.
       Divide by 3 to get 250000.
       According to 8.2.2.1 of the Apollo3 datasheet, the output clock is half
       this frequency. */
    *(int *)(IOM3_BASE + IOM_CLKCFG) = 0xf01;

    // Disable IOM3 interrupts.
    *(int *)(IOM3_BASE + IOM_INTEN) = 0;

    // Configure full duplex communication.
    *(int *)(IOM3_BASE + IOM_MSPICFG) = IOM_MSPICFG_FULLDUP;

    // Enable the SPI submodule.
    *(int *)(IOM3_BASE + IOM_SUBMODCTRL) = IOM_SMOD0TYPE_SPIMASTER | IOM_SMOD0EN;
}

void
spi_set_frequency_divider(int divider)
{
    int i = 0;
    while ((i < 7) && (divider >= 1)) {
        divider >>= 1;
        i++;
    }
    *(int *)(IOM3_BASE + IOM_CLKCFG) = 0x801 | (i << IOM_CLKCFG_FSEL_SHIFT);
}

void
spi_change_cs(int high)
{
    if (high)
        *(int *)GPIO_wtsB = 0x200;  // 41 - 32 = 9 -> 0x200
    else
        *(int *)GPIO_wtcB = 0x200;
}

int
spi_send_byte(int b)
{
    while (*(int *)(IOM3_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CMDSTAT_ACTIVE);

    // Push the byte into the FIFO and send a command without an offset.
    *(int *)(IOM3_BASE + IOM_FIFOPUSH) = b & 0xff;
    *(int *)(IOM3_BASE + IOM_CMD) = (1 << IOM_CMD_TSIZE_SHIFT) | IOM_CMD_WRITE;

    while (*(int *)(IOM3_BASE + IOM_CMDSTAT) & IOM_CMDSTAT_CMDSTAT_ACTIVE);

    return *(int *)(IOM3_BASE + IOM_FIFOPOP) & 0xff;
}
