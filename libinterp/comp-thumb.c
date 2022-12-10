#include "lib9.h"
#include "isa.h"
#include "interp.h"
#include "raise.h"

#define	RESCHED 1	/* check for interpreter reschedule */

enum
{
	R0	= 0,		// why wasn't this used ?
	R1	= 1,
	R2	= 2,
	R3	= 3,
	R4	= 4,
	R5	= 5,
	R6	= 6,
	R7	= 7,
	R8	= 8,
	R9	= 9,
	R10	= 10,		// unused
	R11	= 11,		// unused
	R12	= 12,		/* C's SB */
	R13	= 13,		/* C's SP */
	R14	= 14,		/* Link Register */
	R15	= 15,		/* PC */

	RSB		= R12,
	RLINK	= R14,
	RPC		= R15,

	RTMP = R11,	/* linker temp */
	RHT	= R8,		/* high temp */
	RFP	= R7,		/* Frame Pointer */
	RMP	= R6,		/* Module Pointer */
	RREG	= R5,		/* Pointer to REG */
	RA3	= R4,		/* gpr 3 */
	RA2	= R3,		/* gpr 2 2+3 = L */
	RA1	= R2,		/* gpr 1 */
	RA0	= R1,		/* gpr 0 0+1 = L */
	RCON	= R0,		/* Constant builder */

	EQ	= 0,
	NE	= 1,
	CS	= 2,
	CC	= 3,
	MI	= 4,
	PL	= 5,
	VS	= 6,
	VC	= 7,
	HI	= 8,
	LS	= 9,
	GE	= 10,
	LT	= 11,
	GT	= 12,
	LE	= 13,
	AL	= 14,
	NV	= 15,

	And = 0,
	Eor = 1,
	Lsl = 2,
	Lsr = 3,
	Asr = 4,
	Adc = 5,
	Sbc = 6,
	Ror = 7,
	Tst = 8,
	Neg = 9,
	Cmp = 10,
	Cmn = 11,
	Orr = 12,
	Mul = 13,
	Bic = 14,
	Mvn = 15,

	Mov = 16,
	Cmpi = 17,
	Add = 18,
	Sub = 19,

	Cmph = 19,
	Movh = 20,

	Lea	= 100,		/* macro memory ops */
	Ldw,
	Ldb,
	Stw,
	Stb,

	NCON	= (0x3fc-8)/4,

	SRCOP	= (1<<0),
	DSTOP	= (1<<1),
	WRTPC	= (1<<2),
	TCHECK	= (1<<3),
	NEWPC	= (1<<4),
	DBRAN	= (1<<5),
	THREOP	= (1<<6),

	ANDAND	= 1,
	OROR	= 2,
	EQAND	= 3,

	MacFRP	= 0,
	MacRET,
	MacCASE,
	MacCOLR,
	MacMCAL,
	MacFRAM,
	MacMFRA,
	MacRELQ,
	NMACRO
};

static	ulong	macro[NMACRO];
	void	(*comvec)(void);

int
compile(Module *m, int size, Modlink *ml)
{
	return 0;
}
