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

/* #include "basepri.s" */
#include "primask.s"

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

TEXT savefpregs(SB), THUMB, $-4
    VSTR(0, 0, 0)
    VSTR(1, 0, 8)
    VSTR(2, 0, 16)
    VSTR(3, 0, 24)
    VSTR(4, 0, 32)
    VSTR(5, 0, 40)
    VSTR(6, 0, 48)
    VSTR(7, 0, 56)
    RET

TEXT restorefpregs(SB), THUMB, $-4
    VLDR(0, 0, 0)
    VLDR(1, 0, 8)
    VLDR(2, 0, 16)
    VLDR(3, 0, 24)
    VLDR(4, 0, 32)
    VLDR(5, 0, 40)
    VLDR(6, 0, 48)
    VLDR(7, 0, 56)
    RET

/*
UART0_FR = $0x4001c018
UART_FR_TXFE = 0x80
UART0_DR = $0x4001c000

TEXT wrch(SB), THUMB, $-4
    PUSH(0x6, 1)
    MOVW $0x4001c018, R1
    wrch_loop:
        MOVW 0(R1), R2
        AND $0x80, R2
        CMP $0, R2
        BEQ wrch_loop

    MOVW $0x4001c000, R1
    AND $0xff, R0
    MOVW R0, 0(R1)
    POP(0x6, 1)
*/

/* Disable instruction cache
    MOVW    $CCR_ADDR, R0
    MOVW    0(R0), R1
    MOVW    $~CCR_IC, R2
    AND     R2, R1
    MOVW    R1, 0(R0)
    ISB
*/
