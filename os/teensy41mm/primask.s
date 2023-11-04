// See the splhi man page for information about these functions:

/* Disable interrupts and return the previous state. */
TEXT splhi(SB), THUMB, $-4
	MOVW	$(MACHADDR), R6
	STR_imm(14, 6, 0)   /* m->splpc */

        MRS(0, MRS_PRIMASK) /* load the previous interrupt disabled state */
        CMP $0, R0
        BNE splhi_ret       /* return if already disabled */

        RSB $1, R0, R0
	CPS(1, CPS_I)       /* disable interrupts */
splhi_ret:
	RET

/* Enable interrupts and return the previous state. */
TEXT spllo(SB), THUMB, $-4
        MRS(0, MRS_PRIMASK) /* load the previous interrupt disabled state */
        CMP $0, R0
        BEQ spllo_ret       /* return if already enabled */

        RSB $1, R0, R0
	CPS(0, CPS_I)       /* enable interrupts */
spllo_ret:
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
