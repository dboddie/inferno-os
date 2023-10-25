/* Disable interrupts and return the previous state. */
TEXT splhi(SB), THUMB, $-4
	MOVW	$(MACHADDR), R6
	STR_imm(14, 6, 0)   /* m->splpc */

        MRS(0, MRS_BASEPRI) /* load the previous state (enabled=0, disabled!=0) */
        CMP $0, R0
        BEQ splhi_disable
        MOVW $0, R0         /* nothing to do: convert the state to a flag */
        RET
splhi_disable:
        MOVW $0x60, R0
        CPS(1, CPS_I)
	MSR(0, MRS_BASEPRI)
        CPS(0, CPS_I)
	MOVW $1, R0         /* return a flag for the previous state */
	RET

/* Enable interrupts and return the previous state. */
TEXT spllo(SB), THUMB, $-4
        MRS(0, MRS_BASEPRI) /* load the previous state (enabled=0, disabled!=0) */
        CMP $0, R0
        BNE spllo_enable
        MOVW $1, R0         /* nothing to do: convert the state to a flag */
        RET
spllo_enable:
        MOVW $0, R0
        CPS(1, CPS_I)
	MSR(0, MRS_BASEPRI)
        CPS(0, CPS_I)
	RET

/* Set the interrupt enabled state passed in R0. */
TEXT splx(SB), THUMB, $-4
	MOVW	$(MACHADDR), R6
	STR_imm(14, 6, 0)   /* m->splpc */

TEXT splxpc(SB), THUMB, $-4
        CMP     $1, R0
        BNE splx_disable

	MOVW $0, R0         /* enable masked interrupts */
        CPS(1, CPS_I)
        MSR(0, MRS_BASEPRI)
        CPS(0, CPS_I)
        RET
splx_disable:               /* disable/mask the systick interrupt by using */
        MOVW $0x60, R0      /* a base number lower than the systick priority */
        CPS(1, CPS_I)
	MSR(0, MRS_BASEPRI)
        CPS(0, CPS_I)
	RET

TEXT islo(SB), THUMB, $-4
        MRS(0, MRS_BASEPRI) /* load the interrupt disabled state */
        CMP $0, R0          /* a value of zero indicates unmasked exceptions */
        BEQ islo_enabled
        MOVW $0, R0
        RET
islo_enabled:
        MOVW $1, R0
	RET

