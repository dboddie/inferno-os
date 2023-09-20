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

/* These exception handlers will be entered in handler mode, using the main
   stack pointer (MSP). */

TEXT _systick(SB), THUMB, $-4

    /* In handler mode; R0-R3, R12, R14, PC and xPSR from the preempted code
       are saved on the stack. R0 is stored lowest at the address pointed to
       by the stack pointer. */

    MOVW    $SHCSR_ADDR, R0
    MOVW    (R0), R0
    AND.S   $0xff, R0
    BNE     _systick_exit       /* Don't interrupt a currently active exception. */

    MOVW    $CFSR_ADDR, R0
    MOVW    (R0), R0
    SRL     $16, R0
    AND.S   $0x1, R0
    BNE     _systick_exit       /* Don't interrupt a currently active exception. */

    MOVW    28(SP), R0          /* Read xPSR */
    MOVW    R0, R2
    MOVW    $0x060fffff, R1
    AND.S   R1, R0              /* Check the exception number and other bits. */
    BNE     _systick_exit       /* Don't interrupt if these are set. */

    /* Store the xPSR flags for the interrupted routine. These will be
       temporarily overwritten and restored later. */
    MOVW    $apsr_flags(SB), R1
    MOVW    R2, (R1)

    /* Record the interrupted PC in the slot for R12. */
    MOVW    24(SP), R0
    ORR     $1, R0
    MOVW    R0, 16(SP)

    /* Clear the condition flags before jumping into the switcher. */
    MOVW    $0x07ffffff, R0
    AND     R2, R0
    MOVW    R0, 28(SP)

    MOVW    $_preswitch(SB), R0
    ORR     $1, R0
    MOVW    R0, 24(SP)          /* Return to the _preswitch routine instead. */

_systick_exit:
    RET

/* When _systick returns, the exception returns and thread mode is entered
   again. The registers from the interrupted code have the values they would
   have if uninterrupted except for R12 which contains the interrupted PC and
   PC which points to here. */
TEXT _preswitch(SB), THUMB, $-4

    MOVW R0, R0
    PUSH(0x1000, 0)             /* Save R12 (will be PC). */
    PUSH(0x0bff, 1)             /* Save registers R0-R9, R11 as well as R14, in
                                   case the interrupted code uses them. */
    VMRS(0)                     /* Copy FPSCR into R0 */
    PUSH(0x0001, 0)             /* then push it onto the stack. */
    VPUSH(0, 8)                 /* Push D0-D7. */

    MOVW    $setR12(SB), R1
    MOVW    R1, R12             /* Reset static base (SB) */

    MOVW    SP, R0              /* Pass the stack pointer to the switcher. */
    BL      ,switcher(SB)

    MOVW    $apsr_flags(SB), R1
    MOVW    (R1), R1
    MSR(1, 0)                /* Restore the status bits. */

    VPOP(0, 8)                  /* Recover D0-D7. */
    POP(0x0001, 0)              /* Recover FPSCR into R0 */
    VMSR(0)                     /* then restore it. */

    POP_LR_PC(0x0bff, 1, 0)     /* Recover R0-R9, R11 and R14 */
    POP_LR_PC(0, 0, 1)          /* then PC. */

TEXT _hard_fault(SB), THUMB, $-4
/*    MRS(0, MRS_MSP)     Pass the main stack pointer (MSP) to a C function. */

    MOVW    SP, R1      /* Record the interrupted stack pointer. */
    ADD     $0x68, R1   /* Includes FP registers. */

    PUSH(0x0ff2, 1)
    MOVW    SP, R0
    B ,hard_fault(SB)

TEXT _usage_fault(SB), THUMB, $-4
/*     MRS(0, MRS_MSP)     Pass the main stack pointer (MSP) to a C function. */

    /* R0-R3, R12, R14, PC and xPSR are saved on the stack. R0 is stored lowest
       at the address pointed to by the stack pointer. */

    MOVW    SP, R1      /* Record the interrupted stack pointer. */
    ADD     $0x68, R1   /* Includes FP registers. */

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

    PUSH(0x0ff2, 1)
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
