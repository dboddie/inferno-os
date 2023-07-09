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

// See the splhi man page for information about these functions:

/* Disable interrupts and return the previous state. */
TEXT splhi(SB), THUMB, $-4
	MOVW	$(MACHADDR), R6
	STR_imm(14, 6, 0)   /* m->splpc */

        MRS(0, MRS_PRIMASK) /* load the previous interrupt disabled state */
        RSB $1, R0, R0
	CPS(1, CPS_I)       /* disable interrupts */
	RET

/* Enable interrupts and return the previous state. */
TEXT spllo(SB), THUMB, $-4
        MRS(0, MRS_PRIMASK) /* load the previous interrupt disabled state */
        RSB $1, R0, R0
	CPS(0, CPS_I)       /* enable interrupts */
	RET

/* Set the interrupt enabled state passed in R0. */
TEXT splx(SB), THUMB, $-4
	MOVW	$(MACHADDR), R6
	STR_imm(14, 6, 0)   /* m->splpc */

TEXT splxpc(SB), THUMB, $-4
        CMP     $1, R0
        BNE splx_disable

	CPS(0, CPS_I)       /* enable interrupts */
        RET
splx_disable:
	CPS(1, CPS_I)       /* disable interrupts */
	RET

TEXT islo(SB), THUMB, $-4
        MRS(0, MRS_PRIMASK) /* load the interrupt disabled state */
        RSB $1, R0, R0
	RET

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

TEXT introff(SB), THUMB, $-4
    CPS(1, CPS_I)
    RET

TEXT intron(SB), THUMB, $-4
    CPS(0, CPS_I)
    RET

TEXT coherence(SB), THUMB, $-4
	ISB
        DSB
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

TEXT setprimask(SB), THUMB, $-4
    MSR(0, MRS_PRIMASK)
    ISB
    RET

TEXT getcontrol(SB), THUMB, $-4
    MRS(0, MRS_CONTROL)
    RET

TEXT setcontrol(SB), THUMB, $-4
    MSR(0, MRS_CONTROL)
    ISB
    RET

/* System call */

TEXT callsv(SB), THUMB, $-4
    SVC(0)
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

/*
TEXT _idlehands(SB), $-4
	BARRIERS
	MOVW	CPSR, R3
	BIC	$(PsrDirq|PsrDfiq), R3, R1		// spllo
	MOVW	R1, CPSR

	MOVW	$0, R0				// wait for interrupt
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEintr), CpCACHEwait
	ISB

	MOVW	R3, CPSR			// splx
	RET

TEXT getfpsid(SB), $-4
    VMRS(FPSID, 0)      // FP ctrl register 0 (FPSID) to R0
    RET

TEXT getfpexc(SB), $-4
    VMRS(FPEXC, 0)      // FP ctrl register 8 (FPEXC) to R0
    RET

TEXT setfpexc(SB), $-4
    VMSR(0, FPEXC)      // R0 to FP ctrl register 8 (FPEXC)
    RET

TEXT getfpscr(SB), $-4
    VMRS(FPSCR, 0)      // R0 to FPSCR
    RET

TEXT setfpscr(SB), $-4
    VMSR(0, FPSCR)      // R0 to FPSCR
    RET

TEXT savefp0(SB), $-4
    VSTR(0)
    RET

TEXT savefp1(SB), $-4
    VSTR(1)
    RET

TEXT savefp2(SB), $-4
    VSTR(2)
    RET

TEXT savefp3(SB), $-4
    VSTR(3)
    RET

TEXT savefp4(SB), $-4
    VSTR(4)
    RET

TEXT savefp5(SB), $-4
    VSTR(5)
    RET

TEXT savefp6(SB), $-4
    VSTR(6)
    RET

TEXT savefp7(SB), $-4
    VSTR(7)
    RET

TEXT savefp8(SB), $-4
    VSTR(8)
    RET

TEXT savefp9(SB), $-4
    VSTR(9)
    RET

TEXT savefp10(SB), $-4
    VSTR(10)
    RET

TEXT savefp11(SB), $-4
    VSTR(11)
    RET

TEXT savefp12(SB), $-4
    VSTR(12)
    RET

TEXT savefp13(SB), $-4
    VSTR(13)
    RET

TEXT savefp14(SB), $-4
    VSTR(14)
    RET

TEXT savefp15(SB), $-4
    VSTR(15)
    RET

TEXT savefp16(SB), $-4
    VSTR(16)
    RET

TEXT savefp17(SB), $-4
    VSTR(17)
    RET

TEXT savefp18(SB), $-4
    VSTR(18)
    RET

TEXT savefp19(SB), $-4
    VSTR(19)
    RET

TEXT savefp20(SB), $-4
    VSTR(20)
    RET

TEXT savefp21(SB), $-4
    VSTR(21)
    RET

TEXT savefp22(SB), $-4
    VSTR(22)
    RET

TEXT savefp23(SB), $-4
    VSTR(23)
    RET

TEXT savefp24(SB), $-4
    VSTR(24)
    RET

TEXT savefp25(SB), $-4
    VSTR(25)
    RET

TEXT savefp26(SB), $-4
    VSTR(26)
    RET

TEXT savefp27(SB), $-4
    VSTR(27)
    RET

TEXT savefp28(SB), $-4
    VSTR(28)
    RET

TEXT savefp29(SB), $-4
    VSTR(29)
    RET

TEXT savefp30(SB), $-4
    VSTR(30)
    RET

TEXT savefp31(SB), $-4
    VSTR(31)
    RET

TEXT restfp0(SB), $-4
    VLDR(0)
    RET

TEXT restfp1(SB), $-4
    VLDR(1)
    RET

TEXT restfp2(SB), $-4
    VLDR(2)
    RET

TEXT restfp3(SB), $-4
    VLDR(3)
    RET

TEXT restfp4(SB), $-4
    VLDR(4)
    RET

TEXT restfp5(SB), $-4
    VLDR(5)
    RET

TEXT restfp6(SB), $-4
    VLDR(6)
    RET

TEXT restfp7(SB), $-4
    VLDR(7)
    RET

TEXT restfp8(SB), $-4
    VLDR(8)
    RET

TEXT restfp9(SB), $-4
    VLDR(9)
    RET

TEXT restfp10(SB), $-4
    VLDR(10)
    RET

TEXT restfp11(SB), $-4
    VLDR(11)
    RET

TEXT restfp12(SB), $-4
    VLDR(12)
    RET

TEXT restfp13(SB), $-4
    VLDR(13)
    RET

TEXT restfp14(SB), $-4
    VLDR(14)
    RET

TEXT restfp15(SB), $-4
    VLDR(15)
    RET

TEXT restfp16(SB), $-4
    VLDR(16)
    RET

TEXT restfp17(SB), $-4
    VLDR(17)
    RET

TEXT restfp18(SB), $-4
    VLDR(18)
    RET

TEXT restfp19(SB), $-4
    VLDR(19)
    RET

TEXT restfp20(SB), $-4
    VLDR(20)
    RET

TEXT restfp21(SB), $-4
    VLDR(21)
    RET

TEXT restfp22(SB), $-4
    VLDR(22)
    RET

TEXT restfp23(SB), $-4
    VLDR(23)
    RET

TEXT restfp24(SB), $-4
    VLDR(24)
    RET

TEXT restfp25(SB), $-4
    VLDR(25)
    RET

TEXT restfp26(SB), $-4
    VLDR(26)
    RET

TEXT restfp27(SB), $-4
    VLDR(27)
    RET

TEXT restfp28(SB), $-4
    VLDR(28)
    RET

TEXT restfp29(SB), $-4
    VLDR(29)
    RET

TEXT restfp30(SB), $-4
    VLDR(30)
    RET

TEXT restfp31(SB), $-4
    VLDR(31)
    RET
*/
