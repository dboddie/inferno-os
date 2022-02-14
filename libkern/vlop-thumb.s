THUMB = 4

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.43, T3 */
#define LDRimm(Rt,Rn,imm12) \
    WORD $(0x0000f8d0 | Rn | (Rt<<28) | ((imm12 & 0xfff)<<16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.207, T1 */
#define UMULL(Rn,Rm,RdHi,RdLo) \
    WORD $(0x0000fba0 | Rn | (Rm<<16) | (RdHi<<24) | (RdLo<<28))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.84, T2 */
#define	MUL(Rn,Rm,Rd) \
    WORD $(0xf000fb00 | Rn | (Rm<<16) | (Rd<<24))

arg=0

/* replaced use of R10 by R11 because the former can be the data segment base register */

TEXT	_mulv(SB), THUMB, $0
	MOVW	4(FP), R4	/* h0 */
        MOVW    R4, R11
	MOVW	8(FP), R4	/* l0 */
        MOVW    R4, R9
	MOVW	12(FP), R5	/* h1 */
	MOVW	16(FP), R4	/* l1 */
	UMULL(4, 9, 7, 6)       /* l1 * l0 -> h2, l2 (R7, R6) */
	MUL(11, 4, 8)           /* h0 * l1 -> R8 */
	ADD	R8, R7          /* h2 += R8 */
	MUL(9, 5, 8)            /* l0 * h1 -> R8 */
	ADD	R8, R7          /* h2 += R8 */
	MOVW	R6, 4(R(arg))   /* l2 */
	MOVW	R7, 0(R(arg))   /* h2 */
	RET

/* multiply, add, and right-shift, yielding a 32-bit result, while
   using 64-bit accuracy for the multiply -- for fast fixed-point math */

#define UMLAL(Rs,Rm,Rhi,Rlo,S)	WORD	$((14<<28)|(5<<21)|(S<<20)|(Rhi<<16)|(Rlo<<12)|(Rs<<8)|(9<<4)|Rm)

/* Only needed in certain ports */
TEXT	_mularsv(SB), THUMB, $0
/*	MOVW	4(FP), R11 */	/* m1 */
/*	MOVW	8(FP),	R8 */	/* a */
/*	MOVW	12(FP), R4 */	/* rs */
/*	MOVW	$0, R9 */
/*	UMLAL(0, 11, 9, 8, 0) */
/*	MOVW	R8>>R4, R8 */
/*	RSB	$32, R4, R4 */
/*	ORR	R9<<R4, R8, R0 */
/*	RET */
