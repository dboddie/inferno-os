THUMB=4

/* These exception handlers will be entered in handler mode, using the main
   stack pointer (MSP). */

TEXT _systick(SB), THUMB, $-4

    /* In handler mode; R0-R3, R12, R14, PC and xPSR from the preempted code
       are saved on the stack. R0 is stored lowest at the address pointed to
       by the stack pointer. */

    MOVW    28(SP), R0          /* Read xPSR */
    MOVW    R0, R2
    MOVW    $0x060fffff, R1
    AND.S   R1, R0              /* Check the exception number and other bits. */
    BNE     _systick_exit       /* Don't interrupt if these are set. */

    /* Mask all exceptions to prevent re-entry. */
    CPS(1, CPS_I)

    /* Store the xPSR flags for the interrupted routine. These will be
       temporarily overwritten and restored later. */
    MOVW    R2, R10

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

    PUSH(0x1001, 0)             /* Save R0 and R12 (will be PC). */
    PUSH(0x0ffe, 1)             /* Save registers R1-R11 as well as R14, in
                                   case the interrupted code uses them. */

    /* Now that R12 (return PC) is safely stacked, enable interrupts again. */
    CPS(0, CPS_I)

    VMRS(0)                     /* Copy FPSCR into R0 */
    PUSH1(0x01, 0)              /* then push it onto the stack. */
    VPUSH(0, 8)                 /* Push D0-D7. */

    MOVW    $setR12(SB), R1
    MOVW    R1, R12             /* Reset static base (SB) */

    MOVW    SP, R0              /* Pass the stack pointer to the switcher. */
    BL      ,switcher(SB)

    VPOP(0, 8)                  /* Recover D0-D7. */
    POP1(0x01, 0)               /* Recover FPSCR into R0 */
    VMSR(0)                     /* then restore it. */

    POP_LR_PC(0x0ffe, 1, 0)     /* Recover R1-R11 and R14 */
    MSR(10, 0)                  /* Restore the status bits in R10. */

    POP_LR_PC(0x0001, 0, 1)     /* then R0 and PC. */
