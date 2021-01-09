/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.

    Test subroutine calls.
*/

#include "mem.h"
#include "mips.s"

#define T0 R8
#define A0 R4

#define Green   0xff77cc44

SCHED

/* The storage must be 8-bytes aligned but does not need to perform stack
   operations because this is where the stack pointer is set up. */
TEXT    start(SB), $-8

    MOVW    $setR30(SB), R30

    /* Set the stack pointer. */
    MOVW    $0x80010000, SP

    /* Call a function to obtain a pixel value to make the framebuffer blue. */
    JAL     main(SB)
    NOP

    /* Copy the return value into the first parameter to the fbdraw routine. */
    MOVW    R1, A0
    JAL     fbdraw(SB)
    NOP

end:
    JMP     end
    NOP

/* Framebuffer test */

TEXT    fbdraw(SB), $-8

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
