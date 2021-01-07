/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.
*/

#include "mem.h"
#include "mips.s"

#define FB_START 0xb3050044
#define T0 R8

NOSCHED

TEXT    start(SB), $0           /* The storage must be 8-bytes aligned. */

    CONST(FB_START, R8)         /* r8 = framebuffer start register */
    MOVW    0(R8), R8           /* r8 = *(r8 + 0) */
    MOVW    $0xa0000000, R9
    OR      R8, R9, R8

    CONST(0x26000, R9)
    ADDU    R9, R8, R9

    CONST(0xff77cc44, R10)

loop:
    MOVW    R10, 0(R8)          /* *(r8 + 0) = r10 */
    ADDU    $4, R8
    BNE     R8, R9, loop
    NOP

end:
    JMP     end
