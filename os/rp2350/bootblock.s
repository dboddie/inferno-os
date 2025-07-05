/*
   This file contains a boot block definition to be stored in the first 4K of
   the binary where the bootloader can find it.
   See section 5.9.5 of the RP2350 Datasheet.
*/

TEXT bootblock(SB), $0
    WORD $(0xffffded3)      /* Start of block */
    /* RP2350 (0x1000), ARM (0x000), secure mode (0x20), executable (0x1),
       single block, image type */
    WORD $((0x1021 << 16) | (0x01 << 8) | 0x42)
    WORD $(0x000001ff)      /* No padding, one block, last block */
    WORD $0
    WORD $(0xab123579)      /* End of block */
