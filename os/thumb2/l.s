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

    MOVW    $0, R1
    MOVW    $interrupts_enabled(SB), R2
    MOVW    R1, 0(R2)

    B   ,main(SB)

TEXT _dummy(SB), THUMB, $-4

    B   ,_dummy(SB)

TEXT _systick(SB), THUMB, $0

    BL  ,systick(SB)
    RET
