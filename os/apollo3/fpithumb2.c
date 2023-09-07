#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"
#include "fpi.h"

#define fpemudebug 0

// Define a macro for accessing stacked register values.
#define	REG(x) (*(long*)(((char*)(eregs))+roff[(x)]))
// Define a macro for accessing emulated FP registers.
#define FR(x) (*(Internal*)(ufp)->regs[(x)&7])

static Internal fpconst[8] = {
    /* s, e, l, h */
    {0, 0x001, 0x00000000, 0x00000000}, /* 0.0 */
    {0, 0x3FF, 0x00000000, 0x08000000},	/* 1.0 */
    {0, 0x400, 0x00000000, 0x08000000},	/* 2.0 */
    {0, 0x400, 0x00000000, 0x0C000000},	/* 3.0 */
    {0, 0x401, 0x00000000, 0x08000000},	/* 4.0 */
    {0, 0x401, 0x00000000, 0x0A000000},	/* 5.0 */
    {0, 0x3FE, 0x00000000, 0x08000000},	/* 0.5 */
    {0, 0x402, 0x00000000, 0x0A000000},	/* 10.0 */
};

#undef OFR
#define	OFR(X)	((ulong)&((Ereg*)0)->X)

/* Define a table mapping register numbers to offsets in the struct containing
   the saved registers. */

static	int	roff[] = {
	OFR(r0), OFR(r1), OFR(r2), OFR(r3),
	OFR(r4), OFR(r5), OFR(r6), OFR(r7),
	OFR(r8), OFR(r9), OFR(r10), OFR(r11),
	OFR(r12), OFR(r13), OFR(r14), OFR(pc),
};



/* A6.4.1

For 32-bit values, the resulting format is:

    31 30       24 23   20 19                    0
     s  e eeeee e   e fff   f000 00000000 00000000
     ^  ^ ----- ^   ^ ---   ^
     i7 |   |   |   |  |    |
       !i6  i6 i5   i4 i321 i0
*/
static ulong
VFPExpandImm32(uchar imm8)
{
    uint n;
    n =  (imm8 & 0x80) << 24;   // i7 -> sign
    if (imm8 & 0x40)
        n |= 0x3f800000;        // i6 -> exp
    else
        n |= 0x40000000;
    n |= (imm8 & 0x30) << 19;   // i5:4 -> exp
    n |= (imm8 & 0x0f) << 19;   // i3:0 -> frac
    return n;
}

/*
    For 64-bit values, the resulting format is:

    63 62      56 55   52 51  48                 32 ...
     s  e eeeeee   ee ee   ffff  00000000 00000000  ...
     ^  ^ ----------- ^^   ----
     i7 |      |      ||    |
       !i6     i6    i5-4  i3-0

    Sets the internal fields using bits of the expanded immediate constant.
*/
static void
VFPExpandImm64(ulong imm8, Internal *in)
{
    print("imm=%lux\n", imm8);
    in->s = (imm8 & 0x80) >> 7; // i7 -> sign
    if (imm8 & 0x40)
        in->e = 0x3fc00000;     // i6 -> exp
    else
        in->e = 0x40000000;

    in->h = ((imm8 & 0x3f) << 16);  // i5:4 -> exp, i3:0 -> frac
    in->l = 0;
}

/*
    Reads the contents of the address, ea, and stores it in the FP register, d.
    The number of bytes used to represent a value is passed in n, and must be
    either 4 or 8.
*/
static void
fld(void (*f)(Internal*, void*), int d, ulong ea, int n, FPenv *ufp)
{
	void *mem;

	mem = (void*)ea;
	(*f)(&FR(d), mem);
	if(fpemudebug)
		print("MOV%c #%lux, F%d\n", n==8? 'D': 'F', ea, d);
}

/*
 * arm binary operations
 */

static void
fadd(Internal m, Internal n, Internal *d)
{
	(m.s == n.s? fpiadd: fpisub)(&m, &n, d);
}

static void
fsub(Internal m, Internal n, Internal *d)
{
	m.s ^= 1;
	(m.s == n.s? fpiadd: fpisub)(&m, &n, d);
}

static void
fsubr(Internal m, Internal n, Internal *d)
{
	n.s ^= 1;
	(n.s == m.s? fpiadd: fpisub)(&n, &m, d);
}

static void
fmul(Internal m, Internal n, Internal *d)
{
	fpimul(&m, &n, d);
}

static void
fdiv(Internal m, Internal n, Internal *d)
{
	fpidiv(&m, &n, d);
}

static void
fdivr(Internal m, Internal n, Internal *d)
{
	fpidiv(&n, &m, d);
}

/*
 * arm unary operations
 */

static void
fmov(Internal *m, Internal *d)
{
	*d = *m;
}

static void
fmovn(Internal *m, Internal *d)
{
	*d = *m;
	d->s ^= 1;
}

/*
 * Returns the number of FP instructions emulated.
 * A value of zero indicates that an error occurred.
 */
int
fpithumb2(Ereg *eregs)
{
    int n;

    if (up == nil)
        panic("fpithumb2 not in a process");

    FPenv *ufp = &up->env->fpu;

    if (ufp->fpistate != FPACTIVE) {
        ufp->fpistate = FPACTIVE;
        ufp->control = 0;
        ufp->status = (0x01<<28)|(1<<12);   /* software emulation, alternative C flag */
        for (n = 0; n < 8; n++)
            FR(n) = fpconst[0];     // each register contains 0.0 to start with
    }

    /* Try to read a sequence of floating point instructions to avoid the
       overhead of repeated exceptions. */
    for (n = 0;;n++)
    {
        print("pc=%lux\n", eregs->pc);
        print("%04ux %04ux\n", *(ushort *)eregs->pc, *((ushort *)eregs->pc + 1));

        ushort w0 = *(ushort *)eregs->pc;
        ushort w1 = *(ushort *)(eregs->pc + 2);
        ulong imm, ea;
        ulong Rd, Rn, Rm;   // just register numbers, either R, S or D
        Internal *in;

        switch (w0 & 0xff00) {
        case 0xee00:
            switch (w0 & 0xb0) {
            case 0x00:              // MOVWF
            case 0x10:              // MOVFW
            case 0x20:              // MULF/MULD
            case 0x30:              // ADDF|ADDD|SUBF|SUBD
            case 0x80:              // DIVF|DIVD
                break;
            case 0xb0:              // CMPF|CMPD|MOVF|MOVD
                if ((w1 & 0x40) == 0) {
                    Rd = w1 >> 12;
                    imm = ((w0 & 0xf) << 4) | (w1 & 0xf);
                    in = &FR(Rd);
                    VFPExpandImm64(imm, in);
                    eregs->pc += 4;
                }
                break;
            default:
                return n;
            }
            break;
        case 0xfe00:                // MOVFD|MOVDF
            break;
        case 0xed00:                // MOVF|MOVD (VLDR|VSTR)
            Rd = ((w0 & 0x40) >> 2) | (w1 >> 24);
            Rn = (w0 & 0x0f);
            imm = (w1 & 0x0f) << 2; // word-aligned offset
            ea = REG(Rn);
            if (w0 & 0x80)
                ea += imm;
            else
                ea -= imm;
            print("Rd=%d Rn=%d imm=%d ea=%lux\n", Rd, Rn, imm, ea);
            print("sp=%lux r13=%lux\n", (ulong)eregs, REG(13), (ulong)eregs + sizeof(Ereg));
            if (w1 & 0x100)
                fld(fpid2i, Rd, ea, 8, ufp);
            else
                fld(fpid2i, Rd, ea, 4, ufp);

            eregs->pc += 4;
            break;
        default:
            return n;
        }
    }

    return n;
}
