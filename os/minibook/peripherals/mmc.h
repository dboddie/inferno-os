enum {
    CMD_GO_IDLE_STATE         = 0,
    CMD_SEND_OP_COND          = 1,
    CMD_ALL_SEND_CID          = 2,
    CMD_SEND_RELATIVE_ADDR    = 3,
    CMD_SET_DSR               = 4,
    CMD_IO_SEND_OP_COND       = 5,
    CMD_SWITCH_FUNC           = 6,
    CMD_SELECT_CARD           = 7,
    CMD_SEND_IF_COND          = 8,
    CMD_SEND_EXT_CSD          = 8,
    CMD_SEND_CSD              = 9,
    CMD_SEND_CID              = 10,
    CMD_READ_DAT_UNTIL_STOP   = 11,
    CMD_STOP_TRANSMISSION     = 12,
    CMD_SEND_STATUS           = 13,
    CMD_GO_INACTIVE_STATE     = 15,
    CMD_SET_BLOCKLEN          = 16,
    CMD_READ_SINGLE_BLOCK     = 17,
    CMD_READ_MULTIPLE_BLOCK   = 18,
    CMD_WRITE_DAT_UNTIL_STOP  = 20,
    CMD_WRITE_BLOCK           = 24,
    CMD_WRITE_MULTIPLE_BLOCK  = 25,
    CMD_PROGRAM_CID           = 26,
    CMD_PROGRAM_CSD           = 27,
    CMD_SET_WRITE_PROT        = 28,
    CMD_CLR_WRITE_PROT        = 29,
    CMD_SEND_WRITE_PROT       = 30,
    CMD_TAG_SECTOR_START      = 32,
    CMD_TAG_SECTOR_END        = 33,
    CMD_UNTAG_SECTOR          = 34,
    CMD_TAG_ERASE_GROUP_START = 35,
    CMD_TAG_ERASE_GROUP_END   = 36,
    CMD_UNTAG_ERASE_GROUP     = 37,
    CMD_ERASE                 = 38,
    CMD_FAST_IO               = 39,
    CMD_GO_IRQ_STATE          = 40,
    CMD_SD_APP_OP_COND        = 41,
    CMD_LOCK_UNLOCK           = 42,
    CMD_IO_RW_DIRECT          = 52,
    CMD_IO_RW_EXTENDED        = 53,
    CMD_APP_CMD               = 55,
    CMD_GEN_CMD               = 56,
    CMD_RW_MULTIPLE_REGISTER  = 60,
    CMD_RW_MULTIPLE_BLOCK     = 61,

    SD_OCR_READY              = 0x80000000,
    SD_OCR_HCS                = 0x40000000
};

/* CID struct layout does not reflect layout of data from cards */
typedef struct {
    u8int manufacturer_id;
    u16int application_id;
    char product_name[6];
    u8int revision;
    u32int serial_number;
    u16int manufacturing_date;
    u8int crc;
} MMC_CID;

/* CSD struct is abstract */
typedef struct {
    uchar version;
    uchar speed;
    ushort block_len;
    ulong card_size;     /* in blocks */
} MMC_CSD;

typedef struct {
    MMC_CID cid;
    MMC_CSD csd;
    ushort rca;
    ulong voltages, ccs;
} MMC;

MMC *mmc_sd;

void msc_init(void);
ulong msc_read(ulong card_addr, ulong *dest, ushort blocks);
ulong msc_write(ulong card_addr, ulong *src, ushort blocks);
