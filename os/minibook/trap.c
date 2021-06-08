#include "u.h"
#include "ureg.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"

#include "hardware.h"

struct {
	char	*name;
	uint	off;
} regname[] = {
	"STATUS", Ureg_status,
	"PC",	Ureg_pc,
	"SP",	Ureg_sp,
	"CAUSE",Ureg_cause,
	"BADADDR", Ureg_badvaddr,
	"TLBVIRT", Ureg_tlbvirt,
	"HI",	Ureg_hi,
	"LO",	Ureg_lo,
	"R31",	Ureg_r31,
	"R30",	Ureg_r30,
	"R28",	Ureg_r28,
	"R27",	Ureg_r27,
	"R26",	Ureg_r26,
	"R25",	Ureg_r25,
	"R24",	Ureg_r24,
	"R23",	Ureg_r23,
	"R22",	Ureg_r22,
	"R21",	Ureg_r21,
	"R20",	Ureg_r20,
	"R19",	Ureg_r19,
	"R18",	Ureg_r18,
	"R17",	Ureg_r17,
	"R16",	Ureg_r16,
	"R15",	Ureg_r15,
	"R14",	Ureg_r14,
	"R13",	Ureg_r13,
	"R12",	Ureg_r12,
	"R11",	Ureg_r11,
	"R10",	Ureg_r10,
	"R9",	Ureg_r9,
	"R8",	Ureg_r8,
	"R7",	Ureg_r7,
	"R6",	Ureg_r6,
	"R5",	Ureg_r5,
	"R4",	Ureg_r4,
	"R3",	Ureg_r3,
	"R2",	Ureg_r2,
	"R1",	Ureg_r1,
};

char *excname[] =
{
	"trap: external interrupt",
	"trap: TLB modification (store to unwritable)",
	"trap: TLB miss (load or fetch)",
	"trap: TLB miss (store)",
	"trap: address error (load or fetch)",
	"trap: address error (store)",
	"trap: bus error (fetch)",
	"trap: bus error (data load or store)",
	"trap: system call",
	"breakpoint",
	"trap: reserved instruction",
	"trap: coprocessor unusable",
	"trap: arithmetic overflow",
	"trap: TRAP exception",
	"trap: VCE (instruction)",
	"trap: floating-point exception",
	"trap: coprocessor 2 implementation-specific", /* used as sys call for debugger */
	"trap: corextend unusable",
	"trap: precise coprocessor 2 exception",
	"trap: TLB read-inhibit",
	"trap: TLB execute-inhibit",
	"trap: undefined 21",
	"trap: undefined 22",
	"trap: WATCH exception",
	"trap: machine checkcore",
	"trap: undefined 25",
	"trap: undefined 26",
	"trap: undefined 27",
	"trap: undefined 28",
	"trap: undefined 29",
	"trap: cache error",
	"trap: VCE (data)",
};

char *fpcause[] =
{
	"inexact operation",
	"underflow",
	"overflow",
	"division by zero",
	"invalid operation",
};

char*
fpexcname(Ureg *ur, ulong fcr31, char *buf, uint size)
{
	int i;
	char *s;
	ulong fppc;

	fppc = ur->pc;
	if(ur->cause & BD)	/* branch delay */
		fppc += 4;
	s = 0;
	if(fcr31 & (1<<17))
		s = "unimplemented operation";
	else{
		fcr31 >>= 7;		/* trap enable bits */
		fcr31 &= (fcr31>>5);	/* anded with exceptions */
		for(i=0; i<5; i++)
			if(fcr31 & (1<<i))
				s = fpcause[i];
	}

	if(s == 0)
		return "no floating point exception";

	snprint(buf, size, "%s fppc=%#lux", s, fppc);
	return buf;
}

void
trapinit(void)
{
    m->exc_sp = ESTACKTOP;

/*  memmove((ulong*)UTLBMISS, (ulong*)vector0, 0x80);
    memmove((ulong*)XEXCEPTION, (ulong*)vector0, 0x80);
    memmove((ulong*)CACHETRAP, (ulong*)vector100, 0x80);*/
    memmove((ulong*)EXCEPTION, (ulong*)vector180, 0x80);
    memmove((ulong*)INTERRUPT, (ulong*)vector200, 0x80);

    setstatus(getstatus() & ~BEV);
}

void trapintr(Ureg *ur)
{
    if (ur->cause & 0x400)
    {
        InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
        if (ic->pending & InterruptOST0) {
            clockintr(ur);
//            mousepoll();
//            power_check_reset();
        }
        else if (ic->pending & InterruptOST1) {
            JZTimer *timer1 = (JZTimer *)(TIMER_BASE1 | KSEG1);
            if (timer1->control & TimerUnder) {
                timer1->control &= ~TimerUnder;
                kbdpoll();
                clockmsec();
            }
        }
        else {
            print("pending=%8.8lux\n\n", ic->pending);
            for (;;) {}
        }
/*        if (ic->pending & InterruptMSC) {
            msc_intr();
        }*/
    }
}

void
trap(Ureg *ur)
{
	int ecode, user, cop, fpchk, fpures;
	ulong fpfcr31;
	char buf[2*ERRMAX], buf1[ERRMAX], *fpexcep;
	static int dumps;

	ecode = (ur->cause>>2)&EXCMASK;
	user = ur->status&KUSER;

	fpchk = 0;

	/* clear EXL in status
	setstatus(getstatus() & ~EXL); */

/*	clockintr = 0;*/
	switch(ecode){
	case CINT:
		trapintr(ur);
		break;

	case CFPE:
		panic("FP exception but no FPU");
		break;

	case CWATCH:
		fpwatch(ur);
		break;

	case CCPU:
		cop = (ur->cause>>28)&3;

		if (up && cop == 1) {
			/* no fpu, so we can only emulate fp ins'ns */
                        fpures = fpuemu(ur);
			if (fpures >= 0)
				fpchk = 1;
			break;
		}
		/* Fallthrough */

	default:
		if (ecode == CADREL || ecode == CADRES)
			print("kernel addr exception for va %#p pid %#ld %s\n",
				ur->badvaddr, (up? up->pid: 0),
				(up? up->text: ""));
		print("cpu%d: kernel %s pc=%#lux\n",
			m->machno, excname[ecode], ur->pc);

		dumpregs(ur);
		dumpstack();
		if(m->machno == 0)
			spllo();
		exit(1);
	}

//        print("%d %8.8lux\n", fpchk, fpures);
	if(fpchk) {
		fpfcr31 = up->fpsave.fpstatus;
		if((fpfcr31>>12) & ((fpfcr31>>7)|0x20) & 0x3f) {
			spllo();
			fpexcep	= fpexcname(ur, fpfcr31, buf1, sizeof buf1);
			snprint(buf, sizeof buf, "sys: fp: %s", fpexcep);
		}
	}

	splhi();

	/* delaysched set because we held a lock or because our quantum ended */
/*	if(up && up->delaysched && clockintr){
		sched();
		splhi();
	}
*/
	/* restore EXL in status
	setstatus(getstatus() | EXL); */
}

static void
getpcsp(ulong *pc, ulong *sp)
{
	*pc = getcallerpc(&pc);
	*sp = (ulong)&pc-4;
}

void
callwithureg(void (*fn)(Ureg*))
{
	Ureg ureg;

	memset(&ureg, 0, sizeof ureg);
	getpcsp((ulong*)&ureg.pc, (ulong*)&ureg.sp);
	ureg.r31 = getcallerpc(&fn);
	fn(&ureg);
}

static void
_dumpstack(Ureg *ureg)
{
	ulong l, v, top, i;
	extern ulong etext;

for (;;) {}

	if(up == 0)
		return;

	print("ktrace /kernel/path %.8lux %.8lux %.8lux\n",
		ureg->pc, ureg->sp, ureg->r31);
	top = (ulong)up->kstack + KSTACK;
	i = 0;
	for(l=ureg->sp; l < top; l += BY2WD) {
		v = *(ulong*)l;
		if(KTZERO < v && v < (ulong)&etext) {
			print("%.8lux=%.8lux ", l, v);
			if((++i%4) == 0){
				print("\n");
				delay(200);
			}
		}
	}
	print("\n");
}

void
dumpstack(void)
{
	callwithureg(_dumpstack);
}

static ulong
R(Ureg *ur, int i)
{
	uchar *s;

	s = (uchar*)ur;
	return *(ulong*)(s + regname[i].off - Uoffset);
}

void
dumpregs(Ureg *ur)
{
	int i;

	if(up)
		print("registers for %s %lud\n", up->text, up->pid);
	else
		print("registers for kernel\n");

	for(i = 0; i < nelem(regname); i += 2)
		print("%s\t%#.8lux\t%s\t%#.8lux\n",
			regname[i].name,   R(ur, i),
			regname[i+1].name, R(ur, i+1));
}

static void
linkproc(void)
{
	spllo();
	if (waserror())
		print("error() underflow: %r\n");
	else
		(*up->kpfun)(up->arg);
	pexit("end proc", 1);
}

void
kprocchild(Proc *p, void (*func)(void*), void *arg)
{
	p->sched.pc = (ulong)linkproc;
	p->sched.sp = (ulong)p->kstack+KSTACK;

	p->kpfun = func;
	p->arg = arg;
}
