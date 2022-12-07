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

    MOVW    $0x1234, R1
    MOVW    R1, R10

    B   ,main(SB)

TEXT _dummy(SB), THUMB, $-4

    MOVW    SP, R0
    B   ,dummy(SB)

/* These exception handlers will be entered in handler mode, using the main
   stack pointer (MSP). */

TEXT _systick(SB), THUMB, $-4

    /* In handler mode; R0-R3, R12, R14, PC and xPSR from the preempted code
       are saved on the stack. R0 is stored lowest at the address pointed to
       by the stack pointer. */

    MOVW    24(SP), R0          /* Record the interrupted PC in the slot for R12. */
    ORR     $1, R0
    MOVW    R0, 16(SP)

    MOVW    $_preswitch(SB), R0
    MOVW    R0, 24(SP)          /* Return to the _preswitch routine instead. */
/*
    MOVW    $in_interrupt(SB), R0
    MOVW    $1, R1
    MOVW    R1, (R0)
*/
    RET

/* When _systick returns, the exception returns and thread mode is entered
   again. The registers from the interrupted code have the values they would
   have if uninterrupted except for PC which points to here. */
TEXT _preswitch(SB), THUMB, $-4

    MOVW R0, R0
    PUSH(0x1000, 0)             /* Save R12 (will be PC). */
    PUSH(0x0bff, 1)             /* Save registers R0-R9, R11 as well as R14, in
                                   case the interrupted code uses them. */
    MOVW    $setR12(SB), R1
    MOVW    R1, R12             /* Reset static base (SB) */
/*
    MOVW    $in_interrupt(SB), R0
    MOVW    $0, R1
    MOVW    R1, (R0)
*/
    MOVW    SP, R0              /* Pass the stack pointer to the switcher. */
    BL      ,switcher(SB)

    POP_LR_PC(0x0bff, 1, 0)     /* Recover R0-R9,R11 and R14 */
    POP_LR_PC(0, 0, 1)          /* then PC. */

TEXT _hard_fault(SB), THUMB, $-4
    MRS(0, MRS_MSP)     /* Pass the main stack pointer (MSP) to a C function. */
    B ,hard_fault(SB)

TEXT _usage_fault(SB), THUMB, $-4
    MRS(0, MRS_MSP)     /* Pass the main stack pointer (MSP) to a C function. */
    B ,usage_fault(SB)

TEXT _nmi(SB), THUMB, $-4
    B ,_nmi(SB)

TEXT _mem_manage(SB), THUMB, $-4
    B ,_mem_manage(SB)

TEXT _bus_fault(SB), THUMB, $-4
    B ,_bus_fault(SB)

TEXT _svcall(SB), THUMB, $-4
    B ,_svcall(SB)

TEXT _pendsv(SB), THUMB, $-4
    B ,_pendsv(SB)

TEXT _uart3(SB), THUMB, $-4
    PUSH(0x1bff, 1)

    MOVW    $setR12(SB), R1
    MOVW    R1, R12             /* Reset static base (SB) */
    BL ,uart3_intr(SB)

    POP_LR_PC(0x1bff, 0, 1)

TEXT get_r10(SB), THUMB, $-4
    MOVW    R10, R0
    RET

TEXT get_r12(SB), THUMB, $-4
    MOVW    R12, R0
    RET
