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
