/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.
*/

#include "mem.h"
#include "mips.s"

TEXT    start(SB), $8

    MOVW    $setR30(SB), R30
    MOVW    $123, R2
    RETURN

/* This program appears to cause the Nanonote's U-Boot to restart several times
   before booting from NAND flash. */
