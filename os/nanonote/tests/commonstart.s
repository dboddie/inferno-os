/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.

    Test subroutine calls.
*/

#include "mem.h"
#include "mips.s"

SCHED

/* The storage must be 8-bytes aligned but does not need to perform stack
   operations because this is where the stack pointer is set up. */
TEXT    start(SB), $-8

    MOVW    $setR30(SB), R30

    /* Set the stack pointer. */
    MOVW    $0x80010000, SP

    /* Call the main function, written in C. */
    JAL     main(SB)
    NOP

end:
    JMP     end
    NOP
