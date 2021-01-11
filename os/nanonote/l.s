/*
    Little endian MIPS32 for Ben Nanonote
    Contains parts of Plan 9's 9/rb port.
*/

#include "mem.h"
#include "mips.s"

SCHED

/* The storage must be 8-bytes aligned but does not need to perform stack
   operations because this is where the stack pointer is set up. */
TEXT    start(SB), $-8

    MOVW    $setR30(SB), R30

    /* Set the stack pointer to . */
    MOVW    $(MACHADDR+BY2PG), SP

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

TEXT	getcallerpc(SB), $0
	MOVW	0(SP), R1
	RET


/*
 * manipulate interrupts
 */

TEXT	splhi(SB), $0
	EHB
	MOVW	R31, 12(R(MACH))	/* save PC in m->splpc */
	DI(1)				/* old M(STATUS) into R1 */
	EHB
	RETURN

TEXT	splx(SB), $0
	EHB
	MOVW	R31, 12(R(MACH))	/* save PC in m->splpc */
	MOVW	M(STATUS), R2
	AND	$IE, R1
	AND	$~IE, R2
	OR	R2, R1
	MOVW	R1, M(STATUS)
	EHB
	RETURN

TEXT	spllo(SB), $0
	EHB
	EI(1)				/* old M(STATUS) into R1 */
	EHB
	RETURN

TEXT	islo(SB), $0
	MOVW	M(STATUS), R1
	AND	$IE, R1
	RETURN

TEXT	coherence(SB), $-8
	BARRIERS(7, R7, cohhb)
	SYNC
	EHB
	RETURN
