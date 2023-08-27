/* I/O Master (IOM) */
#define IOM0_BASE         0x50004000
#define IOM4_BASE         0x50008000

#define IOM_FIFO          0
#define IOM_FIFOPTR       0x100
#define IOM_FIFOTHR       0x104
#define IOM_FIFOPOP       0x108
#define IOM_FIFOPUSH      0x10c
#define IOM_FIFOCTRL      0x110
#define IOM_FIFOLOC       0x114
#define IOM_INTEN         0x200
#define IOM_INTSTAT       0x204
#define IOM_INTCLR        0x208
#define IOM_INTSET        0x20c
#define IOM_CLKCFG        0x210
#define IOM_SUBMODCTRL    0x214
#define IOM_CMD           0x218
#define IOM_DCX           0x21c
#define IOM_OFFSETHI      0x220
#define IOM_CMDSTAT       0x224
#define IOM_DMATRIGEN     0x240
#define IOM_DMACFG        0x280
#define IOM_DMATOTCOUNT   0x288
#define IOM_DMATARGADDR   0x28c
#define IOM_STATUS        0x2b4
#define IOM_MSPICFG       0x300
#define IOM_DEVCFG        0x404

#define IOM_FIFOCTRL_FIFORSTN 0x2   /* clear this to reset the FIFO */

#define IOM_INT_THR     0x2
#define IOM_INT_CMDCMP  0x1

#define IOM_SMOD1TYPE_I2CMASTER 0x20
#define IOM_SMOD1EN 0x10
#define IOM_SMOD0TYPE_SPIMASTER 0
#define IOM_SMOD0EN 0x1

#define IOM_CMD_OFFSETLO_SHIFT 24
#define IOM_CMD_TSIZE_SHIFT 8
#define IOM_CMD_OFFSET_CNT_SHIFT 5
#define IOM_CMD_WRITE 0x1
#define IOM_CMD_READ 0x2

#define IOM_CMDSTAT_CTSIZE_MASK 0xfff00
#define IOM_CMDSTAT_CMDSTAT_MASK 0xe0
#define IOM_CMDSTAT_CMDSTAT_ERR 0x20
#define IOM_CMDSTAT_CMDSTAT_ACTIVE 0x40
#define IOM_CMDSTAT_CMDSTAT_IDLE 0x80
#define IOM_CMDSTAT_CMDSTAT_WAIT 0xc0
#define IOM_CMDSTAT_CMDSTAT_CCMD 0x1f

#define IOM_DMATRIGEN_DTHREN    0x2
#define IOM_DMATRIGEN_DCMDCMPEN 0x1

#define IOM_DMACFG_DMADIR_P2M 0     /* read */
#define IOM_DMACFG_DMADIR_READ 0    /* for convenience */
#define IOM_DMACFG_DMADIR_M2P 0x2   /* write */
#define IOM_DMACFG_DMADIR_WRITE 0x2 /* for convenience */
#define IOM_DMACFG_DMAEN      0x1

#define IOM_DMATOTCOUNT_TOTCOUNT_MASK 0xfff

#define IOM_DMATARGADDR_ADDR28    0x10000000
#define IOM_DMATARGADDR_ADDR_MASK 0x001fffff

#define IOM_STATUS_IDLE 0x4
#define IOM_STATUS_CMD 0x2
/* The error status bit (0x1) is not used. */
