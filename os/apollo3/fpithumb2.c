#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"
#include "fpi.h"

#define fpemudebug 1

// Define a macro for accessing stacked register values.
#define	REG(x) (*(long*)(((char*)(eregs))+roff[(x)]))
// Define a macro for accessing emulated FP registers.
#define FR(x) (*(Internal*)(ufp)->regs[(x)&7])

static Internal zero_internal = {0, 0, 0, 0};

enum {
	N = 1<<31,
	Z = 1<<30,
	C = 1<<29,
	V = 1<<28,
	REGPC = 15,
};

ulong xpsr_flags = 0;

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
static ulong
VFPExpandImm64(ulong imm8)
{
    ulong n;
    n = (imm8 & 0x80) << 24;    // i7 -> sign
    if (imm8 & 0x40)
        n |= 0x3fc00000;        // i6 -> exp
    else
        n |= 0x40000000;
    n |= (imm8 & 0x3f) << 16;   // i5:4 -> exp, i3:0 -> frac
    return n;
}

static ulong
fcmp(Internal *n, Internal *m)
{
	int i;

	if(IsWeird(m) || IsWeird(n)){
		/* BUG: should trap if not masked */
		return V|C;
	}
	i = fpicmp(n, m);
	if(i > 0)
		return C;
	else if(i == 0)
		return C|Z;
	else
		return N;
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

static void
fst(void (*f)(void*, Internal*), ulong ea, int s, int n, FPenv *ufp)
{
	Internal tmp;
	void *mem;

	mem = (void*)ea;
	tmp = FR(s);
	if(fpemudebug)
		print("MOV%c	F%d,#%lux\n", n==8? 'D': 'F', s, ea);
	(*f)(mem, &tmp);
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

extern void dumperegs(Ereg *);

/*
 * Returns the number of FP instructions emulated.
 * A value of zero indicates that an error occurred.
 */
int
fpithumb2(Ereg *eregs)
{
    int n;
/*
    if (up == nil)
        panic("fpithumb2 not in a process");
    FPenv *ufp = &up->env->fpu;

    if (ufp->fpistate != FPACTIVE) {
        ufp->fpistate = FPACTIVE;
        ufp->control = 0;
        ufp->status = (0x01<<28)|(1<<12);   // software emulation, alternative C flag
        for (n = 0; n < 16; n++)
            eregs->s[n] = 0;     // each register contains 0.0 to start with
    }
*/

    print("pc=%lux %04ux %04ux\n", eregs->pc, *(ushort *)eregs->pc, *((ushort *)eregs->pc + 1));

    ushort w0 = *(ushort *)eregs->pc;
    ushort w1 = *(ushort *)(eregs->pc + 2);
    ulong imm, ea;
    ulong Fd, Fm, Fn;   // just register numbers, either R, S or D
    ulong Rt, Rt2, Rn;
    Internal in1, in2, inr;

    if ((w0 == 0xeef1) && (w1 == 0xfa10)) {
        eregs->xpsr &= ~(N|C|Z|V);
        eregs->xpsr |= xpsr_flags;
        print("VMRS XPSR, FPSCR\n");
        eregs->pc += 4;
        return 1;
    }

    switch (w0 & 0xff00) {
    case 0xee00:
        switch (w0 & 0xb0) {    // mask a register bit at 0x40
        case 0x00:
            Rt = w1 >> 12;
            if (w0 & 0x40) {    // MOVWD (A7.7.242)
                Rt2 = w0 & 0xf;
                // For doubles, use even registers.
                Fm = (w1 & 0xf) << 1;
                eregs->s[Fm] = REG(Rt);
                eregs->s[Fm + 1] = REG(Rt2);
                print("VMOV D%uld R%uld R%uld\n", Fm >> 1, Rt, Rt2);
            } else {
                print("### 0xee0x MOVWF unsupported?\n");
                return 0;
            }
            eregs->pc += 4;
            break;
        case 0x10:
            Rt = w1 >> 12;
            if (w0 & 0x40) {    // MOVDW (A7.7.242)
                Rt2 = w0 & 0xf;
                Fm = (w1 & 0xf) << 1;
                REG(Rt) = eregs->s[Fm];
                REG(Rt2) = eregs->s[Fm + 1];
                print("VMOV R%uld R%uld D%uld\n", Rt, Rt2, Fm >> 1);
            } else {
                print("### 0xee1x MOVFW unsupported?\n");
                return 0;
            }
            eregs->pc += 4;
            break;
        case 0x20:              // MULF/MULD
            if ((w1 & 0x100) == 0) {
                print("### MULF unsupported?\n");
                return 0;
            }
            Fd = (w1 >> 12) << 1;
            Fm = (w1 & 0x0f) << 1;
            Fn = (w0 & 0x0f) << 1;
            fpid2i(&in1, &eregs->s[Fn]);
            fpid2i(&in2, &eregs->s[Fm]);
            fmul(in2, in1, &inr);
            print("VMUL D%uld, D%uld, D%uld\n", Fd, Fn, Fm);
            fpii2d(&eregs->s[Fd], &inr);
            eregs->pc += 4;
            break;
        case 0x30:              // ADDF|ADDD|SUBF|SUBD
            if ((w1 & 0x100) == 0) {
                print("### ADDF/SUBF unsupported?\n");
                return 0;
            }
            Fd = (w1 >> 12) << 1;
            Fm = (w1 & 0x0f) << 1;
            Fn = (w0 & 0x0f) << 1;
            fpid2i(&in1, &eregs->s[Fn]);
            fpid2i(&in2, &eregs->s[Fm]);
            if (w1 & 0x40) {    // SUBD
                fsub(in2, in1, &inr);
                print("VSUB D%uld, D%uld, D%uld\n", Fd, Fn, Fm);
            } else {            // ADDD
                fadd(in2, in1, &inr);
                print("VADD D%uld, D%uld, D%uld\n", Fd, Fn, Fm);
            }
            fpii2d(&eregs->s[Fd], &inr);
            eregs->pc += 4;
            break;
        case 0x80:              // DIVF|DIVD
            print("<-- 0xee80 %lux\n", eregs->pc);
            return 0;
        case 0xb0:
            Fd = (w1 >> 12) << 1;

            if ((w1 & 0x40) == 0) {     // MOVF|MOVD (A7.7.236)
                imm = ((w0 & 0xf) << 4) | (w1 & 0xf);
                // Expand the constant into the high register.
                eregs->s[Fd + 1] = VFPExpandImm64(imm);
                eregs->s[Fd] = 0;
                print("VMOV F%uld 0x%ulx\n", Fd, imm);
//                print("%ud %d %ulx %ulx\n", in->s, in->e, in->l, in->h);
            } else if (w0 & 0x4) {      // CMPF|CMPD (A7.7.223)
                if (w0 & 0x1) {
                    fpid2i(&in1, &eregs->s[Fd]);
                    xpsr_flags = fcmp(&in1, &zero_internal);
                    print("VCMP F%uld #0.0\n", Fd >> 1);
                } else {
                    Fm = (w1 & 0x0f) << 1;
                    fpid2i(&in1, &eregs->s[Fd]);
                    fpid2i(&in2, &eregs->s[Fm]);
                    xpsr_flags = fcmp(&in1, &in2);
                    print("VCMP F%uld F%uld\n", Fd >> 1, Fm >> 1);
                }
            } else {                    // MOVF|MOVD (A7.7.237)
                Fm = (w1 & 0xf) << 1;
                print("VMOV F%uld F%uld\n", Fd >> 1, Fm >> 1);
                eregs->s[Fd] = eregs->s[Fm];
                eregs->s[Fd + 1] = eregs->s[Fm + 1];
            }
            eregs->pc += 4;
            break;
        default:
            print("<-- 0xee %lux\n", eregs->pc);
            return 0;
        }
        break;

    case 0xed00:                // MOVF|MOVD (VLDR|VSTR)
        Fd = (w1 >> 12);
        Rn = (w0 & 0x0f);
        imm = (w1 & 0x0f) << 2; // word-aligned offset
        ea = REG(Rn);
        if (w0 & 0x80)
            ea += imm;
        else
            ea -= imm;

        if (w0 & 0x10) {
            if (w1 & 0x100) {
                Fd = Fd << 1;
                eregs->s[Fd] = *(ulong *)ea;
                eregs->s[Fd + 1] = *(ulong *)(ea + 4);
            } else {
                print("### 0xed1x VLDR unsupported?\n");
                return 0;
            }
            print("VLDR F%uld R%uld, #%uld (ea=%lux)\n", Fd, Rn, imm, ea);
        } else {
            if (w1 & 0x100) {
                Fd = Fd << 1;
                *(ulong *)ea = eregs->s[Fd];
                *(ulong *)(ea + 4) = eregs->s[Fd + 1];
            } else {
                print("### 0xedxx VSTR unsupported?\n");
                return 0;
            }
            print("VSTR F%uld R%uld, #%uld (ea=%lux)\n", Fd, Rn, imm, ea);
        }
        eregs->pc += 4;
        break;

    case 0xfe00:                // MOVFD|MOVDF
        print("<-- 0xfe %lux\n", eregs->pc);
        return 0;
    default:
        print("<-- %lux\n", eregs->pc);
        return 0;
    }

    return 1;
}
