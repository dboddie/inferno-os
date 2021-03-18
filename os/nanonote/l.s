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
/*	DI(1)			*/	/* old M(STATUS) into R1 */
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
/*	EI(1)			*/	/* old M(STATUS) into R1 */
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

/* vector at KSEG0+0x180, others */
TEXT    vector180(SB), $-8
        MOVW    $exception(SB), R26
        JMP     (R26)
        NOP

TEXT    getepc(SB), $-8
        MOVW    M(EPC), R1
        RETURN

TEXT    getcause(SB), $-8
        MOVW    M(CAUSE), R1
        RETURN
