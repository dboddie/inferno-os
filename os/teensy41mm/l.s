#include "mem.h"
#include "thumb2.h"
#include "imx.h"
#include "vectors.s"

THUMB=4

TEXT _start(SB), THUMB, $-4

    /* In GPR16, enable the FlexRAM configuration specified in the GPR17 register. */
    MOVW    $IOMUXC_GPR_GPR16, R0
    MOVW    0(R0), R1
    MOVW    $4, R2
    ORR     R2, R1
    MOVW    R1, 0(R0)

    /* In GPR17, enable DTCM memory for RAM. */
    MOVW    $IOMUXC_GPR_GPR17, R0
    MOVW    $0xaaaaaaaa, R1
    MOVW    R1, 0(R0)
/* Disable instruction cache
    MOVW    $CCR_ADDR, R0
    MOVW    0(R0), R1
    MOVW    $~CCR_IC, R2
    AND     R2, R1
    MOVW    R1, 0(R0)
    ISB
*/
    /* After reset, we are in thread mode with main stack pointer (MSP) used. */
    MOVW    $STACK_TOP, R1
    MOVW    R1, SP

    MOVW    $setR12(SB), R1
    MOVW    R1, R12	/* static base (SB) */

    /* Change the vector table address to point to the table in the payload. */
    MOVW    $SCB_VTOR, R1
    MOVW    $ROM_START, R2
    MOVW    R2, (R1)

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

herehere:

    BL  ,introff(SB)

    B   ,main(SB)

TEXT _dummy(SB), THUMB, $-4

    MOVW    SP, R0
    B   ,_dummy(SB)
