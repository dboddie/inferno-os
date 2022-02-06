#include "thumb2.h"
#include "vectors.s"

THUMB=4
STACK_TOP=0x20020000

TEXT _start(SB), THUMB, $-4

    MOVW    $setR12(SB), R1
    MOVW    R1, R12	/* static base (SB) */
    MOVW    $STACK_TOP, R1
    MOVW    R1, SP

    /* Copy initial values of data from after the end of the text section to
       the beginning of the data section. */
    MOVW    $etext(SB), R1
    MOVW    $bdata(SB), R2
    MOVW    $edata(SB), R3

_start_loop:
    CMP     R3, R2              /* Note the reversal of the operands */
    BGE     _end_start_loop

    MOVW    (R1), R4
    MOVW    R4, (R2)
    ADD     $4, R1
    ADD     $4, R2
    B       _start_loop

_end_start_loop:

    BL  ,introff(SB)

    MOVW    $0, R1
    MOVW    $interrupts_enabled(SB), R2
    MOVW    R1, 0(R2)

    B   ,main(SB)

TEXT _dummy(SB), THUMB, $-4

    B   ,_dummy(SB)

TEXT _systick(SB), THUMB, $-4
    MRS(0, MRS_MSP)                 /* Assuming MSP not PSP, save SP before
                                       usage_fault changes it. */

    PUSH(0xf0, 1)               /* followed by R4-R7, */
    BL  ,systick(SB)
    POP(0xf0, 1)                /* Recover R4-R7 */

TEXT _hard_fault(SB), THUMB, $-4
    B   ,hard_fault(SB)

TEXT _usage_fault(SB), THUMB, $-4
    MRS(0, MRS_MSP)                 /* Assuming MSP not PSP, save SP before
                                       usage_fault changes it. */

    PUSH(0xf0, 1)               /* followed by R4-R7, */
    BL ,usage_fault(SB)         /* then call a C function to skip an instruction */
    POP(0xf0, 1)                /* Recover R4-R7 */
