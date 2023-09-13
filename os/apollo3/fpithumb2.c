#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"
#include "fpi.h"

//#define fpudebug

enum {
	N = 1<<31,
	Z = 1<<30,
	C = 1<<29,
	V = 1<<28,
	REGPC = 15,
};

// Define a macro for accessing stacked register values.
#define	REG(x) (*(long*)(((char*)(er))+roff[(x)]))

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

void
dumpfpregs(Ereg *er)
{
    for (int i = 0; i < 16; i++) {
        wrch(' '); wrstr("s"); wrdec(i); wrstr("="); wrhex(er->s[i]);
        if ((i & 3) == 3)
            newline();
    }
    wrstr("fpscr="); wrhex(er->fpscr); newline();
}

void
dumpfpreg(Ereg *er, int i)
{
    wrstr("D"); wrdec(i>>1); wrstr("=");
    wrhex(er->s[i+1]); wrch(':'); wrhex(er->s[i]);
    newline();
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

/*
   The original ternary expressions with pointer dereferencing cause problems
   when compiled. Perhaps pointers to functions are handled incorrectly.
*/
static void
fadd(Internal m, Internal n, Internal *d)
{
    if (m.s == n.s)
        fpiadd(&m, &n, d);
    else
        fpisub(&m, &n, d);
}

static void
fsub(Internal m, Internal n, Internal *d)
{
    m.s ^= 1;
    if (m.s == n.s)
        fpiadd(&m, &n, d);
    else
        fpisub(&m, &n, d);
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

ulong xpsr_flags = 0;
static Internal zero_internal = {0, 0, 0, 0};

int
fpithumb2(Ereg *er)
{
    FPenv *ufp;

//#ifdef fpudebug
    wrstr("pc="); wrhex(er->pc); wrch(' ');
    wrhex(*(ushort *)er->pc); wrch(' ');
    wrhex(*((ushort *)er->pc + 1));
    newline();
//#endif

    ufp = &up->env->fpu;
    if(ufp->fpistate != FPACTIVE) {
        ufp->fpistate = FPACTIVE;
        ufp->control = 0;
        ufp->status = (0x01<<28)|(1<<12);	/* software emulation, alternative C flag */
        for (int n = 0; n < 16; n++)
            er->s[n] = 0;
    }

    ushort w0 = *(ushort *)er->pc;
    ushort w1 = *(ushort *)(er->pc + 2);
    ulong imm, ea;
    ulong Fd, Fm, Fn;   // just register numbers, either R, S or D
    ulong Rt, Rt2, Rn;
    Internal in1, in2, inr;

    if (((w1 & 0x0f00) == 0xa00) && ((w0 & 0xeeb7) != 0xeeb7)) {
        wrstr("Single precision FP undefined instruction at ");
        wrhex(er->pc); newline();
        wrhex(*(ushort *)er->pc); wrch(' ');
        wrhex(*((ushort *)er->pc + 1));
        newline();
        return 0;
    }

    switch (w0 & 0xffb0) {
    case 0xeeb0:
    {
        if ((w1 & 0x40) == 0) {
            // MOVD (A7.7.236)
            Fd = (w1 >> 12) << 1;
            imm = ((w0 & 0xf) << 4) | (w1 & 0xf);
            // Expand the constant into the high register.
            er->s[Fd + 1] = VFPExpandImm64(imm);
            er->s[Fd] = 0;
#ifdef fpudebug
            wrstr("VMOV D"); wrdec(Fd>>1); wrstr(", #0x"); wrhex(imm); newline();
#endif

        } else if ((w1 & 0xc0) == 0xc0) {
            if ((w1 & 0xf00) == 0xb00) {
                // MOVDF - Vd:D M:Vm - multiply the numbers by two in any case
                Fd = ((w1 >> 12) << 2) | ((w0 & 0x40) >> 4);
                Fm = ((w1 & 0xf) << 1) | (w1 & 0x20);
                fpid2i(&in1, &er->s[Fm]);
                fpii2s(&er->s[Fd], &in1);
#ifdef fpudebug
                wrstr("MOVDF D"); wrdec(Fm>>1); wrstr(" S"); wrdec(Fd>>1); newline();
#endif
            } else {
                // MOVFD - D:Vd Vm:M - multiply the numbers by two in any case
                Fd = ((w1 >> 12) << 1) | ((w0 & 0x40) >> 1);
                Fm = ((w1 & 0xf) << 2) | ((w1 & 0x20) >> 4);
                fpis2i(&in1, &er->s[Fm]);
                fpii2d(&er->s[Fd], &in1);
#ifdef fpudebug
                wrstr("MOVDF S"); wrdec(Fm>>1); wrstr(" D"); wrdec(Fd>>1); newline();
#endif
            }

        } else if (w0 & 0x4) {
            // CMPD (A7.7.223)
            Fd = (w1 >> 12) << 1;
            if (w0 & 0x1) {
                fpid2i(&in1, &er->s[Fd]);
                xpsr_flags = fcmp(&in1, &zero_internal);
#ifdef fpudebug
                wrstr("VCMP D"); wrdec(Fd>>1); wrstr(", #0.0\r\n");
#endif
            } else {
                Fm = (w1 & 0x0f) << 1;
                fpid2i(&in1, &er->s[Fd]);
                fpid2i(&in2, &er->s[Fm]);
                xpsr_flags = fcmp(&in1, &in2);
#ifdef fpudebug
                wrstr("VCMP D"); wrdec(Fd>>1); wrstr(", D"); wrdec(Fm>>1); newline();
#endif
            }
            er->fpscr = (er->fpscr & 0x0fffffff) | xpsr_flags;
        } else {
            // MOVD (A7.7.237)
            Fd = (w1 >> 12) << 1;
            Fm = (w1 & 0xf) << 1;
#ifdef fpudebug
            wrstr("VMOV D"); wrdec(Fd >> 1); wrstr(" D"); wrdec(Fm >> 1); newline();
#endif
            er->s[Fd] = er->s[Fm];
            er->s[Fd + 1] = er->s[Fm + 1];
        }
        er->pc += 4;
        return 1;
    }
    // DIVD (A7.7.229)
    case 0xee80:
    {
        Fd = (w1 >> 12) << 1;
        Fm = (w1 & 0x0f) << 1;
        Fn = (w0 & 0x0f) << 1;
        fpid2i(&in1, &er->s[Fn]);
        fpid2i(&in2, &er->s[Fm]);
        fpidiv(&in2, &in1, &inr);
#ifdef fpudebug
        wrstr("VDIV D"); wrdec(Fd>>1); wrstr(", D"); wrdec(Fn>>1); wrstr(", D"); wrdec(Fm>>1); newline();
#endif
        fpii2d(&er->s[Fd], &inr);
        er->pc += 4;
        return 1;

    // ADDD (A7.7.222), SUBD (A7.7.257)
    case 0xee30:
        Fd = (w1 >> 12) << 1;
        Fm = (w1 & 0x0f) << 1;
        Fn = (w0 & 0x0f) << 1;
        fpid2i(&in1, &er->s[Fn]);
        fpid2i(&in2, &er->s[Fm]);
        if (w1 & 0x40) {
#ifdef fpudebug
            wrstr("VSUB D"); wrdec(Fd>>1); wrstr(", D"); wrdec(Fn>>1); wrstr(", D"); wrdec(Fm>>1); newline();
#endif
            fsub(in2, in1, &inr);
        } else {
#ifdef fpudebug
            wrstr("VADD D"); wrdec(Fd>>1); wrstr(", D"); wrdec(Fn>>1); wrstr(", D"); wrdec(Fm>>1); newline();
#endif
            fadd(in2, in1, &inr);
        }
        fpii2d(&er->s[Fd], &inr);
//        dumpfpreg(er, Fd); dumpfpreg(er, Fn); dumpfpreg(er, Fm);
        er->pc += 4;
        return 1;
    }
    // MULD (A7.7.245)
    case 0xee20:
    {
        Fd = (w1 >> 12) << 1;
        Fm = (w1 & 0x0f) << 1;
        Fn = (w0 & 0x0f) << 1;
        fpid2i(&in1, &er->s[Fn]);
        fpid2i(&in2, &er->s[Fm]);
        fpimul(&in2, &in1, &inr);
#ifdef fpudebug
        wrstr("VMUL D"); wrdec(Fd>>1); wrstr(", D"); wrdec(Fn>>1); wrstr(", D"); wrdec(Fm>>1); newline();
#endif
        fpii2d(&er->s[Fd], &inr);
        er->pc += 4;
        return 1;
    }
    // MOVDW (A7.7.242)
    case 0xee10:
    {
        if ((w0 & 0x40) == 0)
            break;

        Rt = w1 >> 12;
        Rt2 = w0 & 0xf;
        Fm = (w1 & 0xf) << 1;
        REG(Rt) = er->s[Fm];
        REG(Rt2) = er->s[Fm + 1];
#ifdef fpudebug
        wrstr("VMOV R"); wrdec(Rt); wrstr(", R"); wrdec(Rt2), wrstr(", D"); wrdec(Fm>>1); newline();
#endif
        er->pc += 4;
        return 1;
    }
    // MOVWD (A7.7.242)
    case 0xee00:
    {
        Rt = w1 >> 12;
        Rt2 = w0 & 0xf;
        Fm = (w1 & 0xf) << 1;
        er->s[Fm] = REG(Rt);
        er->s[Fm + 1] = REG(Rt2);
#ifdef fpudebug
        wrstr("VMOV D"); wrdec(Fm>>1); wrstr(", R"); wrdec(Rt), wrstr(", R"); wrdec(Rt2); newline();
#endif
        er->pc += 4;
        return 1;
    }
    // MOVD (A7.7.256)
    case 0xed00:
    case 0xed80:
    {
        Fd = (w1 >> 12);
        Rn = (w0 & 0x0f);
        imm = (w1 & 0x0f) << 2; // word-aligned offset
        ea = REG(Rn);
        if (w0 & 0x80)
            ea += imm;
        else
            ea -= imm;

        Fd = Fd << 1;
        *(ulong *)ea = er->s[Fd];
        *(ulong *)(ea + 4) = er->s[Fd + 1];

#ifdef fpudebug
        wrstr("VSTR D"); wrdec(Fd); wrstr(", R"); wrdec(Rn); wrstr(", #0x"); wrhex(imm);
        wrstr(" ("); wrhex(ea); wrstr(")"); newline();
#endif
        er->pc += 4;
        return 1;
    }
    // MOVD (A7.7.233)
    case 0xed10:
    case 0xed90:
    {
        Fd = (w1 >> 12);
        Rn = (w0 & 0x0f);
        imm = (w1 & 0x0f) << 2; // word-aligned offset
        ea = REG(Rn);
        if (w0 & 0x80)
            ea += imm;
        else
            ea -= imm;

        Fd = Fd << 1;
        er->s[Fd] = *(ulong *)ea;
        er->s[Fd + 1] = *(ulong *)(ea + 4);

#ifdef fpudebug
        wrstr("VLDR D"); wrdec(Fd); wrstr(", R"); wrdec(Rn); wrstr(", #0x"); wrhex(imm);
        wrstr(" ("); wrhex(ea); wrstr(")"); newline();
#endif
        er->pc += 4;
        return 1;
    }
    }

    return 0;
}
