/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.

    Test setting of the status register and subroutine calls.
*/

#include "mem.h"
#include "mips.s"

#define FB_START 0x13050044
#define T0 R8
#define A0 R4

#define Green   0xff77cc44
#define Pink    0xffcc7744

NOSCHED

TEXT    start(SB), $0           /* The storage must be 8-bytes aligned. */

    CONST(Green, A0)
    JAL     fbdraw(SB)
    NOP

    MOVW    $setR30(SB), R30

    /* initialize Mach, including stack */
    MOVW    $MACHADDR, R(MACH)
    ADDU    $(MACHSIZE-BY2V), R(MACH), SP
    MOVW    R(MACH), R1

    JAL     main(SB)
    NOP

//    CONST(Pink, A0)
    MOVW    R2, A0
    JAL     fbdraw(SB)
    NOP

end:
    JMP     end

/* Framebuffer test */

TEXT    fbdraw(SB), $0

    CONST(KSEG1|FB_START, R8)         /* r8 = framebuffer start register  */
    MOVW    0(R8), R8           /* r8 = *(r8 + 0) */
    MOVW    $0xa0000000, R9
    OR      R9, R8

    CONST(0x26000, R9)
    ADDU    R8, R9

loop:
    MOVW    A0, 0(R8)          /* *(r8 + 0) = r4 */
    ADDU    $4, R8
    BNE     R8, R9, loop
    NOP

    RETURN
