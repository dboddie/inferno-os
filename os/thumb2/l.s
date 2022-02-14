#include "mem.h"
#include "thumb2.h"
#include "vectors.s"

THUMB=4

TEXT _start(SB), THUMB, $-4

    MOVW    $setR12(SB), R1
    MOVW    R1, R12	/* static base (SB) */
    /* After reset, we are in thread mode with main stack pointer (MSP) used. */
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

/* These exception handlers will be entered in handler mode, using the main
   stack pointer (MSP). */

TEXT _systick(SB), THUMB, $-4
    PUSH(0x1ff0, 1)
    MOVW    SP, R0
    BL  ,systick(SB)
    POP(0x1ff0, 1)

TEXT _hard_fault(SB), THUMB, $-4
    MRS(0, MRS_MSP)     /* Pass the main stack pointer (MSP) to a C function. */
    PUSH(0x1ff0, 1)
    BL ,hard_fault(SB)
    POP(0x1ff0, 1)

TEXT _usage_fault(SB), THUMB, $-4
    MRS(0, MRS_MSP)     /* Pass the main stack pointer (MSP) to a C function. */
    PUSH(0x1ff0, 1)
    BL ,usage_fault(SB)
    POP(0x1ff0, 1)
