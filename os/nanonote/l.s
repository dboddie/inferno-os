/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.
*/

#include "mem.h"
#include "mips.s"

NOSCHED

/* The storage must be 8-bytes aligned but does not need to perform stack
   operations because this is where the stack pointer is set up. */
TEXT    start(SB), $-8

    MOVW    $setR30(SB), R30

    /* Disable interrupts, FP and ERL, but leave BEV on. */
    MOVW    $BEV, R1
    MOVW    R1, M(STATUS)
    EHB
    MOVW    R0, M(CAUSE)
    EHB

    MOVW    $TLBROFF, R1
    MOVW    R1, M(WIRED)
    EHB
    MOVW    M(CAUSE), R1
    EHB
    OR      $CAUSE_IV, R1
    MOVW    R1, M(CAUSE)
    EHB
    MOVW    R0, M(CONTEXT)
    EHB

    MOVW    $3, R8              /* Enable caching */
    MOVW    R8, M(CONFIG)
    EHB

    /* initialize Mach, including stack */
    MOVW    $MACHADDR, R(MACH)
    /* Set the stack pointer to 1 page (4096 bytes) above the Mach structure. */
    MOVW    $STACKTOP, SP

    JMP     main(SB)
    NOP

/*
 * process switching
 */

TEXT	setlabel(SB), $-8
	MOVW	R29, 0(R1)
	MOVW	R31, 4(R1)
	MOVW	R0, R1
	RETURN

TEXT	gotolabel(SB), $-8
	MOVW	0(R1), R29
	MOVW	4(R1), R31
	MOVW	$1, R1
	RETURN

TEXT	getcallerpc(SB), $-8
	MOVW	0(SP), R1
	RET


/*
 * manipulate interrupts
 */

TEXT	splhi(SB), $-8
	MOVW	R31, 12(R(MACH))	/* save PC in m->splpc */
/*	DI(1)			*/	/* (MIPS32r2) old M(STATUS) into R1 */
	MOVW    M(STATUS), R1
	EHB
	AND     $~IE, R1, R2
	MOVW    R2, M(STATUS)
	EHB
	RETURN

TEXT	splx(SB), $-8
	MOVW	R31, 12(R(MACH))	/* save PC in m->splpc */
TEXT    splxpc(SB), $-8
	MOVW	M(STATUS), R2
	EHB
	AND	$IE, R1
	AND	$~IE, R2
	OR	R2, R1
	MOVW	R1, M(STATUS)
	EHB
	RETURN

TEXT	spllo(SB), $-8
/*	EI(1)			*/	/* (MIPS32r2) old M(STATUS) into R1 */
	MOVW    M(STATUS), R1
	EHB
	OR      $IE, R1, R2
	MOVW    R2, M(STATUS)
	EHB
	RETURN

TEXT	islo(SB), $-8
	MOVW	M(STATUS), R1
	EHB
	AND	$IE, R1
	RETURN

TEXT	coherence(SB), $-8
	BARRIERS(7, R7, cohhb)
	SYNC
	EHB
	RETURN

/* enable an interrupt; bit is in R1 */
TEXT	intron(SB), $0
	MOVW	M(STATUS), R2
	OR	R1, R2
	MOVW	R2, M(STATUS)
	EHB
	RETURN

/* disable an interrupt; bit is in R1 */
TEXT	introff(SB), $0
	MOVW	M(STATUS), R2
	XOR	$-1, R1
	AND	R1, R2
	MOVW	R2, M(STATUS)
	EHB
	RETURN

/*
 * access to CP0 registers
 */

TEXT	getconfig(SB), $-8
	MOVW	M(CONFIG), R1
	RETURN

TEXT	getstatus(SB), $-8
	MOVW	M(STATUS), R1
	EHB
	RETURN

TEXT	setstatus(SB), $0
	MOVW	R1, M(STATUS)
	EHB
	RET

TEXT	setwatchhi0(SB), $0
	MOVW	R1, M(WATCHHI)
	EHB
	RETURN

/*
 * beware that the register takes a double-word address, so it's not
 * precise to the individual instruction.
 */
TEXT	setwatchlo0(SB), $0
	MOVW	R1, M(WATCHLO)
	EHB
	RETURN

TEXT	tlbvirt(SB), $0
	EHB
	MOVW	M(TLBVIRT), R1
	EHB
	RETURN

TEXT    getebase(SB), $-8
        MFC0(15, 1, 1)      /* CP0 15.1 -> R1 */
        EHB
        RETURN

TEXT    getprid(SB), $-8
        MOVW    M(PRID), R1
        RETURN

/*
 *  cache manipulation
 */

/*
 *  we avoided using R4, R5, R6, and R7 so gotopc can call us without saving
 *  them, but gotopc is now gone.
 */
TEXT	icflush(SB), $-4			/* icflush(virtaddr, count) */
	MOVW	4(FP), R9
	DI(R10)				/* intrs off, old status -> R10 */
	UBARRIERS(7, R7, ichb);		/* return to kseg1 (uncached) */
	ADDU	R1, R9			/* R9 = last address */
	MOVW	$(~(CACHELINESZ-1)), R8
	AND	R1, R8			/* R8 = first address, rounded down */
	ADDU	$(CACHELINESZ-1), R9
	AND	$(~(CACHELINESZ-1)), R9	/* round last address up */
	SUBU	R8, R9			/* R9 = revised count */
icflush1:
//	CACHE	PD+HWB, (R8)		/* flush D to ram */
	CACHE	PI+HINV, (R8)		/* invalidate in I */
	SUBU	$CACHELINESZ, R9
	BGTZ	R9, icflush1
	ADDU	$CACHELINESZ, R8	/* delay slot */

	BARRIERS(7, R7, ic2hb);		/* return to kseg0 (cached) */
	MOVW	R10, M(STATUS)
	JRHB(31)			/* return and clear all hazards */

TEXT	dcflush(SB), $-4			/* dcflush(virtaddr, count) */
	MOVW	4(FP), R9
	DI(R10)				/* intrs off, old status -> R10 */
	SYNC
	EHB
	ADDU	R1, R9			/* R9 = last address */
	MOVW	$(~(CACHELINESZ-1)), R8
	AND	R1, R8			/* R8 = first address, rounded down */
	ADDU	$(CACHELINESZ-1), R9
	AND	$(~(CACHELINESZ-1)), R9	/* round last address up */
	SUBU	R8, R9			/* R9 = revised count */
dcflush1:
//	CACHE	PI+HINV, (R8)		/* invalidate in I */
	CACHE	PD+HWBI, (R8)		/* flush & invalidate in D */
	SUBU	$CACHELINESZ, R9
	BGTZ	R9, dcflush1
	ADDU	$CACHELINESZ, R8	/* delay slot */
	SYNC
	EHB
	MOVW	R10, M(STATUS)
	JRHB(31)			/* return and clear all hazards */

/*
 * access to program counter and stack pointer
 */

TEXT    getpc(SB), $-8
        MOVW    R31, R1
        RETURN

TEXT    getsp(SB), $-8
        MOVW    SP, R1
        RETURN

TEXT    getepc(SB), $-8
        MOVW    M(EPC), R1
        RETURN

TEXT    getcause(SB), $-8
        MOVW    M(CAUSE), R1
        RETURN

/* target for JALRHB in BARRIERS */
TEXT    barret(SB), $-8
	JMP	(R22)
	NOP

/* vector at KSEG0+0x180, others
   Use R26 (k0) and R27 (k1) as scratch registers */
TEXT    vector180(SB), $-8

        MOVW    $exception(SB), R26
        JMP     (R26)
        NOP

TEXT    vector200(SB), $-8

        MOVW    $interrupt(SB), R26
        JMP     (R26)
        NOP

TEXT    interrupt(SB), $-4      /* Don't generate save and restore PC instructions */

        /* Save SP so that we can call routines */
        MOVW    SP, R26

        MOVW    $ESTACKTOP, SP
        SUBU    $UREGSIZE, SP
	OR	$7, SP          /* conservative rounding for compatibility with */
	XOR	$7, SP          /* MIPS64 */

        /* Push registers onto the exception stack */
        /* R29 is SP, stored in R26 above */
        MOVW    R26, Ureg_sp(SP)

        MOVW    M(STATUS), R26
        EHB
        MOVW    R26, Ureg_status(SP)
        MOVW    M(CAUSE), R26
        MOVW    R26, Ureg_cause(SP)
        MOVW    M(EPC), R26
        MOVW    R26, Ureg_pc(SP)

        MOVW    R31, Ureg_r31(SP)
        MOVW    R30, Ureg_r30(SP)

        MOVW    R28, Ureg_r28(SP)
        MOVW    R25, Ureg_r25(SP)
        MOVW    R24, Ureg_r24(SP)
        MOVW    R23, Ureg_r23(SP)
        MOVW    R22, Ureg_r22(SP)
        MOVW    R21, Ureg_r21(SP)
        MOVW    R20, Ureg_r20(SP)
        MOVW    R19, Ureg_r19(SP)
        MOVW    R18, Ureg_r18(SP)
        MOVW    R17, Ureg_r17(SP)
        MOVW    R16, Ureg_r16(SP)
        MOVW    R15, Ureg_r15(SP)
        MOVW    R14, Ureg_r14(SP)
        MOVW    R13, Ureg_r13(SP)
        MOVW    R12, Ureg_r12(SP)
        MOVW    R11, Ureg_r11(SP)
        MOVW    R10, Ureg_r10(SP)
        MOVW    R9, Ureg_r9(SP)
        MOVW    R8, Ureg_r8(SP)
        MOVW    R7, Ureg_r7(SP)
        MOVW    R6, Ureg_r6(SP)
        MOVW    R5, Ureg_r5(SP)
        MOVW    R4, Ureg_r4(SP)
        MOVW    R3, Ureg_r3(SP)
        MOVW    R2, Ureg_r2(SP)
        MOVW    R1, Ureg_r1(SP)

	MOVW	HI, R1
	MOVW	LO, R2
	MOVW	R1, Ureg_hi(SP)
	MOVW	R2, Ureg_lo(SP)

        MOVW    SP, R1
        JAL     trapintr(SB)
        SUBU    $Notuoffset, SP

        ADDU    $Notuoffset, SP

        /* Pop registers from the stack */

	MOVW	Ureg_hi(SP), R1
	MOVW	Ureg_lo(SP), R2
	MOVW	R1, HI
	MOVW	R2, LO

        MOVW    Ureg_r31(SP), R31
        MOVW    Ureg_r30(SP), R30

        MOVW    Ureg_r28(SP), R28
        MOVW    Ureg_r25(SP), R25
        MOVW    Ureg_r24(SP), R24
        MOVW    Ureg_r23(SP), R23
        MOVW    Ureg_r22(SP), R22
        MOVW    Ureg_r21(SP), R21
        MOVW    Ureg_r20(SP), R20
        MOVW    Ureg_r19(SP), R19
        MOVW    Ureg_r18(SP), R18
        MOVW    Ureg_r17(SP), R17
        MOVW    Ureg_r16(SP), R16
        MOVW    Ureg_r15(SP), R15
        MOVW    Ureg_r14(SP), R14
        MOVW    Ureg_r13(SP), R13
        MOVW    Ureg_r12(SP), R12
        MOVW    Ureg_r11(SP), R11
        MOVW    Ureg_r10(SP), R10
        MOVW    Ureg_r9(SP), R9
        MOVW    Ureg_r8(SP), R8
        MOVW    Ureg_r7(SP), R7
        MOVW    Ureg_r6(SP), R6
        MOVW    Ureg_r5(SP), R5
        MOVW    Ureg_r4(SP), R4
        MOVW    Ureg_r3(SP), R3
        MOVW    Ureg_r2(SP), R2
        MOVW    Ureg_r1(SP), R1

        MOVW    Ureg_status(SP), R26
        MOVW    R26, M(STATUS)
        EHB
        MOVW    Ureg_cause(SP), R26
        MOVW    R26, M(CAUSE)
        EHB
        MOVW    Ureg_pc(SP), R26
        MOVW    R26, M(EPC)
        EHB

        MOVW    Ureg_sp(SP), R26
        MOVW    R26, SP
        ERET

TEXT    exception(SB), $-4      /* Don't generate save and restore PC instructions */

        /* Save SP so that we can call routines */
        MOVW    SP, R26

        MOVW    $ESTACKTOP, SP
        SUBU    $UREGSIZE, SP
	OR	$7, SP          /* conservative rounding for compatibility with */
	XOR	$7, SP          /* MIPS64 */

        /* Push registers onto the exception stack */
        /* R29 is SP, stored in R26 above */
        MOVW    R26, Ureg_sp(SP)

        MOVW    M(STATUS), R26
        EHB
        MOVW    R26, Ureg_status(SP)
        MOVW    M(CAUSE), R26
        MOVW    R26, Ureg_cause(SP)
        MOVW    M(EPC), R26
        MOVW    R26, Ureg_pc(SP)

        MOVW    R31, Ureg_r31(SP)
        MOVW    R30, Ureg_r30(SP)

        MOVW    R28, Ureg_r28(SP)
        MOVW    R25, Ureg_r25(SP)
        MOVW    R24, Ureg_r24(SP)
        MOVW    R23, Ureg_r23(SP)
        MOVW    R22, Ureg_r22(SP)
        MOVW    R21, Ureg_r21(SP)
        MOVW    R20, Ureg_r20(SP)
        MOVW    R19, Ureg_r19(SP)
        MOVW    R18, Ureg_r18(SP)
        MOVW    R17, Ureg_r17(SP)
        MOVW    R16, Ureg_r16(SP)
        MOVW    R15, Ureg_r15(SP)
        MOVW    R14, Ureg_r14(SP)
        MOVW    R13, Ureg_r13(SP)
        MOVW    R12, Ureg_r12(SP)
        MOVW    R11, Ureg_r11(SP)
        MOVW    R10, Ureg_r10(SP)
        MOVW    R9, Ureg_r9(SP)
        MOVW    R8, Ureg_r8(SP)
        MOVW    R7, Ureg_r7(SP)
        MOVW    R6, Ureg_r6(SP)
        MOVW    R5, Ureg_r5(SP)
        MOVW    R4, Ureg_r4(SP)
        MOVW    R3, Ureg_r3(SP)
        MOVW    R2, Ureg_r2(SP)
        MOVW    R1, Ureg_r1(SP)

	MOVW	HI, R1
	MOVW	LO, R2
	MOVW	R1, Ureg_hi(SP)
	MOVW	R2, Ureg_lo(SP)

        MOVW    SP, R1
        JAL     trap(SB)
        SUBU    $Notuoffset, SP

        ADDU    $Notuoffset, SP

        /* Pop registers from the stack */

	MOVW	Ureg_hi(SP), R1
	MOVW	Ureg_lo(SP), R2
	MOVW	R1, HI
	MOVW	R2, LO

        MOVW    Ureg_r31(SP), R31
        MOVW    Ureg_r30(SP), R30

        MOVW    Ureg_r28(SP), R28
        MOVW    Ureg_r25(SP), R25
        MOVW    Ureg_r24(SP), R24
        MOVW    Ureg_r23(SP), R23
        MOVW    Ureg_r22(SP), R22
        MOVW    Ureg_r21(SP), R21
        MOVW    Ureg_r20(SP), R20
        MOVW    Ureg_r19(SP), R19
        MOVW    Ureg_r18(SP), R18
        MOVW    Ureg_r17(SP), R17
        MOVW    Ureg_r16(SP), R16
        MOVW    Ureg_r15(SP), R15
        MOVW    Ureg_r14(SP), R14
        MOVW    Ureg_r13(SP), R13
        MOVW    Ureg_r12(SP), R12
        MOVW    Ureg_r11(SP), R11
        MOVW    Ureg_r10(SP), R10
        MOVW    Ureg_r9(SP), R9
        MOVW    Ureg_r8(SP), R8
        MOVW    Ureg_r7(SP), R7
        MOVW    Ureg_r6(SP), R6
        MOVW    Ureg_r5(SP), R5
        MOVW    Ureg_r4(SP), R4
        MOVW    Ureg_r3(SP), R3
        MOVW    Ureg_r2(SP), R2
        MOVW    Ureg_r1(SP), R1

        MOVW    Ureg_status(SP), R26
        MOVW    R26, M(STATUS)
        EHB
        MOVW    Ureg_cause(SP), R26
        MOVW    R26, M(CAUSE)
        EHB
        MOVW    Ureg_pc(SP), R26
        MOVW    R26, M(EPC)
        EHB

        MOVW    Ureg_sp(SP), R26
        MOVW    R26, SP
        ERET
