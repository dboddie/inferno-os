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

TEXT    getebase(SB), $-8
        MFC0(15, 1, 1)      /* CP0 15.1 -> R1 */
        EHB
        RETURN

TEXT    getprid(SB), $-8
        MOVW    M(PRID), R1
        RETURN

/*
 * access to program counter and stack pointer
 */

TEXT    getpc(SB), $-8
        MOVW    R31, R1
        RETURN

TEXT    getsp(SB), $-8
        MOVW    SP, R1
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

TEXT    exception(SB), $-8

        /* Use k1 to access the Mach structure (dat.h)
           then load exception stack pointer m->exc_sp */
/*
        MOVW    $(MACHADDR), R27
        MOVW    28(R27), R27
*/
        MOVW    $ESTACKTOP, R27
        SUBU    $UREGSIZE, R27

        /* Save SP so that we can call routines */
        MOVW    SP, R26

        /* Push registers onto the exception stack */
        MOVW    R31, Ureg_r31(R27)
        MOVW    R30, Ureg_r30(R27)
        MOVW    R26, Ureg_sp(R27)
        MOVW    R28, Ureg_r28(R27)

                                    /* R29 is SP, stored in R26 above */
                                    /* R26 and R27 are scratch registers */
        MOVW    R25, Ureg_r25(R27)
        MOVW    R24, Ureg_r24(R27)
        MOVW    R23, Ureg_r23(R27)
        MOVW    R22, Ureg_r22(R27)
        MOVW    R21, Ureg_r21(R27)
        MOVW    R20, Ureg_r20(R27)
        MOVW    R19, Ureg_r19(R27)
        MOVW    R18, Ureg_r18(R27)
        MOVW    R17, Ureg_r17(R27)
        MOVW    R16, Ureg_r16(R27)
        MOVW    R15, Ureg_r15(R27)
        MOVW    R14, Ureg_r14(R27)
        MOVW    R13, Ureg_r13(R27)
        MOVW    R12, Ureg_r12(R27)
        MOVW    R11, Ureg_r11(R27)
        MOVW    R10, Ureg_r10(R27)
        MOVW    R9, Ureg_r9(R27)
        MOVW    R8, Ureg_r8(R27)
        MOVW    R7, Ureg_r7(R27)
        MOVW    R6, Ureg_r6(R27)
        MOVW    R5, Ureg_r5(R27)
        MOVW    R4, Ureg_r4(R27)
        MOVW    R3, Ureg_r3(R27)
        MOVW    R2, Ureg_r2(R27)
        MOVW    R1, Ureg_r1(R27)
/*
        MOVW    R26, SP
*/
        MOVW    $(0x10002010 | KSEG1), R26
        MOVW    $1, R27                     /* Clear the flag for timer 0 */
        MOVW    R27, 0x18(R26)
        ERET

TEXT    getepc(SB), $-8
        MOVW    M(EPC), R1
        RETURN

TEXT    getcause(SB), $-8
        MOVW    M(CAUSE), R1
        RETURN
