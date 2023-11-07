#include "mem.h"
#include "thumb2.h"

THUMB=4

/* Store the stack pointer and return address in the Label struct passed by the
   caller and return 0. */
TEXT setlabel(SB), THUMB, $-4
	STR_imm(13, 0, 0)
	STR_imm(14, 0, 4)
	MOVW    $0, R0
	RET

/* Update the stack pointer and return address from the values in the Label
   struct passed by the caller and return 1. This causes execution to continue
   after a call to setlabel elsewhere in the kernel. */
TEXT gotolabel(SB), THUMB, $-4
	LDR_imm(14, 0, 4)
	LDR_imm(13, 0, 0)
	MOVW    $1, R0
	RET

TEXT getcallerpc(SB), THUMB, $-4
	MOVW    0(SP), R0
	RET

#include "basepri.s"
/* #include "primask.s" */

TEXT getR12(SB), THUMB, $-4
        MOVW R12, R0
        RET

TEXT getsp(SB), THUMB, $-4
        MOVW    SP, R0
        RET

TEXT getpc(SB), THUMB, $-4
        MOVW    R14, R0
        RET

TEXT getsc(SB), THUMB, $-4
        MRC     CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
        RET

TEXT getapsr(SB), THUMB, $-4
        /* Read APSR into R0 */
        WORD    $(0x8000f3ef | (0 << 4) | (0 << 24))
        RET

TEXT getfpscr(SB), THUMB, $-4
        VMRS(0)
        RET

TEXT setfpscr(SB), THUMB, $-4
    VMSR(0)     /* R0 to FPSCR */
    RET

/* Disable/enable all interrupts - disabled interrupts can cause exceptions to
   escalate to hard faults. */

TEXT introff(SB), THUMB, $-4
    CPS(1, CPS_I)
    RET

TEXT intron(SB), THUMB, $-4
    CPS(0, CPS_I)
    RET

TEXT coherence(SB), THUMB, $-4
        DSB
	ISB
	RET

/* Stack selection */

TEXT getmsp(SB), THUMB, $-4
    MRS(0, MRS_MSP)
    RET

TEXT getpsp(SB), THUMB, $-4
    MRS(0, MRS_PSP)
    RET

TEXT setmsp(SB), THUMB, $-4
    MSR(0, MRS_MSP)
    RET

TEXT setpsp(SB), THUMB, $-4
    MSR(0, MRS_PSP)
    RET

TEXT getprimask(SB), THUMB, $-4
    MRS(0, MRS_PRIMASK)
    RET

TEXT getcontrol(SB), THUMB, $-4
    MRS(0, MRS_CONTROL)
    RET

TEXT setcontrol(SB), THUMB, $-4
    MSR(0, MRS_CONTROL)
    ISB
    RET

/* ulong _tas(ulong*); */
/* Accepts R0 = address of key; returns R0 = 0 (success) or 1 (failure) */
/* Tries to write a lock value to the address. Returns 0 if able to do so, or
   returns 1 to indicate failure.
   (Adapted from emu/Linux/arm-tas-v7.S) */

TEXT _tas(SB), THUMB, $-4
        DMB
	MOVW    R0, R1          /* R1 = address of key */
        MOVW    $0xaa, R2

_tas_loop:
        /* Read the key address to check the lock value. */
        LDREX(0, 1, 0)          /* 0(R1) -> R0 */
        CMP     $0, R0
        BNE     _tas_lockbusy   /* The value loaded was non-zero, so the lock
                                   is in use. */
        STREX(3, 2, 1, 0)       /* R2 -> 0(R1) ? 0 -> R3 : 1 -> R3 (fail) */
        CMP     $0, R3
        BNE     _tas_loop       /* Failed to store, so try again. */
        DMB
        RET

_tas_lockbusy:
        CLREX
        RET
