#include "mem.h"
#include "thumb2.h"

THUMB=4

#define DUMMY _dummy(SB)

TEXT vectors(SB), $0
    WORD    $STACKTOP
    WORD    $_start(SB)
    WORD    $DUMMY      /* NMI */
    WORD    $DUMMY      /* HARD_FAULT */
    WORD    $DUMMY      /* MEM_MANAGE */
    WORD    $DUMMY      /* BUS_FAULT */
    WORD    $DUMMY      /* USAGE_FAULT */
    WORD    $DUMMY      /* DUMMY */
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY      /* SVCALL */
    WORD    $DUMMY      /* DUMMY */
    WORD    $DUMMY
    WORD    $DUMMY      /* PENDSV */
    WORD    $_systick(SB)

    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY

TEXT bootblock(SB), $0
    WORD $(0xffffded3)      /* Start of block */
    /* RP2350 (0x1000), ARM (0x000), secure mode (0x20), executable (0x1),
       single block, image type */
    WORD $((0x1021 << 16) | (0x01 << 8) | 0x42)
    WORD $(0x000001ff)      /* No padding, one block, last block */
    WORD $0
    WORD $(0xab123579)      /* End of block */

TEXT _start(SB), THUMB, $-4
    MOVW    $setR12(SB), R1
    MOVW    R1, R12	/* static base (SB) */
    /* After reset, we are in thread mode with main stack pointer (MSP) used. */
    MOVW    $STACK_TOP, R1
    MOVW    R1, SP

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

    BL  ,introff(SB)

    B   ,main(SB)

TEXT _dummy(SB), THUMB, $-4

    MOVW    SP, R0
    B   ,trap_dummy(SB)

TEXT _systick(SB), THUMB, $-4
    PUSH(0x0fff, 1)
    BL ,systick(SB)
    POP(0xfff, 1)

TEXT _hard_fault(SB), THUMB, $-4
/*    MRS(0, MRS_MSP)     Pass the main stack pointer (MSP) to a C function. */

    MOVW    SP, R1      /* Record the interrupted stack pointer. */
    ADD     $0x68, R1   /* Includes FP registers. */

    PUSH(0x0ffa, 1)
    MOVW    SP, R0
    B ,hard_fault(SB)

TEXT _usage_fault(SB), THUMB, $-4
/*     MRS(0, MRS_MSP)     Pass the main stack pointer (MSP) to a C function. */

    /* R0-R3, R12, R14, PC and xPSR are saved on the stack. R0 is stored lowest
       at the address pointed to by the stack pointer. */

    MOVW    SP, R1      /* Record the interrupted stack pointer. */
    ADD     $0x68, R1   /* Includes FP registers. */

    /* Push R1, R4-R11 and LR to complete the set of stacked registers.
       It was found that PUSH(0x0ff2, 1) resulted in an incomplete or
       corrupt set of stacked registers, with the value expected in r4 found
       in r3. */
    PUSH(0x0ffa, 1)
    MOVW    SP, R0
    BL ,usage_fault(SB)
    POP(0x0ffa, 1)

TEXT _nmi(SB), THUMB, $-4
    B ,_nmi(SB)

TEXT _mem_manage(SB), THUMB, $-4
    B ,_mem_manage(SB)

TEXT _bus_fault(SB), THUMB, $-4
    MOVW    SP, R1      /* Record the interrupted stack pointer. */
    ADD     $0x68, R1   /* Includes FP registers. */

    PUSH(0x0ffa, 1)
    MOVW    SP, R0
    B ,bus_fault(SB)

TEXT _svcall(SB), THUMB, $-4
    B ,_svcall(SB)

TEXT _pendsv(SB), THUMB, $-4
    B ,_pendsv(SB)

TEXT get_r10(SB), THUMB, $-4
    MOVW    R10, R0
    RET

TEXT get_r12(SB), THUMB, $-4
    MOVW    R12, R0
    RET

TEXT _dumpregs(SB), THUMB, $-4
    PUSH(0x0fff, 1)
    MOVW SP, R0
    BL ,dumpregs(SB)
    POP(0x0fff, 1)
