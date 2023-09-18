#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "thumb2.h"
#include "../port/error.h"

#include "devices/fns.h"
extern void wrch(int);
extern void poolsummary(void);

extern void hzclock(Ureg *);

void trapinit(void)
{
    // Interrupts need to be off at this point. This is done in the loader.

//    *(int *)SHPR1 = (*(int *)SHPR1 & 0xff00ffff) | 0x00440000;
    *(int *)SHPR3 = (*(int *)SHPR3 & 0x00ffffff) | 0xff000000;

    /* Enable the usage fault exception. */
    *(int *)SHCSR_ADDR |= SHCSR_USGFAULTENA;

    /* Enable division by zero trapping. */
    *(int *)CCR_ADDR |= CCR_DIV_0_TRP | CCR_UNALIGN_TRP;
    /* Disable 8-byte stack alignment. */
    *(int *)CCR_ADDR &= ~CCR_STKALIGN;

    /* Disable privileged access, use SP_main as the stack, disable FP
       extension. */
    setcontrol(0);
    disablefpu();
    *(int *)FPCCR_ADDR &= ~FPCCR_LSPEN;
    *(int *)FPCCR_ADDR |= FPCCR_ASPEN;
    /* Enable the FPU again. */
    enablefpu();
    setcontrol(CONTROL_FPCA);
}

void showregs(int sp, int below)
{
    int i;
    for (i = 0; i < 4; i++)
        print("r%d=%.8lux ", i, *(ulong *)(sp + (i*4)));

    print("\n");

    if (below) {
        for (i = 4; i < 13; i++) {
            print("r%d=%.8lux ", i, *(ulong *)(sp - 40 + (i - 4)*4));
            if (i % 4 == 3) print("\n");
        }
    }

    print("sp=%.8lux ", (ulong)sp);
    print("lr=%.8lux ", *(ulong *)(sp + 20));
    print("pc=%.8lux\n", *(ulong *)(sp + 24));
}

void dumpregs(Ureg *uregs)
{
    wrstr(" r0="); wrhex(uregs->r0); wrch(' ');
    wrstr("r1="); wrhex(uregs->r1); wrch(' ');
    wrstr(" r2="); wrhex(uregs->r2); wrch(' ');
    wrstr("r3="); wrhex(uregs->r3); newline();
    wrstr(" r4="); wrhex(uregs->r4); wrch(' ');
    wrstr("r5="); wrhex(uregs->r5); wrch(' ');
    wrstr(" r6="); wrhex(uregs->r6); wrch(' ');
    wrstr("r7="); wrhex(uregs->r7); newline();
    wrstr(" r8="); wrhex(uregs->r8); wrch(' ');
    wrstr("r9="); wrhex(uregs->r9); wrch(' ');
    wrstr("r10="); wrhex(uregs->r10); wrch(' ');
    wrstr("r11="); wrhex(uregs->r11); newline();
    wrstr("r12="); wrhex(get_r12()); wrch(' ');
    wrstr("sp="); wrhex((int)uregs); wrch(' ');
    wrstr("r14="); wrhex(uregs->r14); newline();
}

void dumperegs(Ereg *eregs)
{
    wrstr(" r0="); wrhex(eregs->r0); wrch(' ');
    wrstr("r1="); wrhex(eregs->r1); wrch(' ');
    wrstr(" r2="); wrhex(eregs->r2); wrch(' ');
    wrstr("r3="); wrhex(eregs->r3); newline();
    wrstr(" r4="); wrhex(eregs->r4); wrch(' ');
    wrstr("r5="); wrhex(eregs->r5); wrch(' ');
    wrstr(" r6="); wrhex(eregs->r6); wrch(' ');
    wrstr("r7="); wrhex(eregs->r7); newline();
    wrstr(" r8="); wrhex(eregs->r8); wrch(' ');
    wrstr("r9="); wrhex(eregs->r9); wrch(' ');
    wrstr("r10="); wrhex(eregs->r10); wrch(' ');
    wrstr("r11="); wrhex(eregs->r11); newline();
    wrstr("r12="); wrhex(get_r12()); wrch(' ');
    wrstr("r13="); wrhex(eregs->sp); wrch(' ');
    wrstr("sp="); wrhex((int)eregs); wrch(' ');
    wrstr("r14="); wrhex(eregs->r14); newline();
    wrstr("exc_ret="); wrhex(eregs->exc_ret); wrch(' ');
    wrstr("pc="); wrhex(eregs->pc); wrch(' ');
    wrstr("xpsr="); wrhex(eregs->xpsr); newline();
}

extern int apsr_flags;

void switcher(Ureg *ureg)
{
    int t;

//    wrstr("[switcher] > up="); wrhex((int)up); newline();
//    wrstr("\x1b[1m@\x1b[m");
//    wrstr(">> sp="); wrhex(getsp()); wrstr(" pc="); wrhex(*(ulong *)((ulong)ureg + 52)); newline();
//    print("up=%.8lux\n", up);
//    print("psr=%.8lux\n", ureg->psr);
//    wrstr("in\r\n");
//    _dumpregs();

    if (up) {
        up->pc = ureg->pc;
        up->env->fpu.fpscr = getfpscr();
        savefpregs((double *)&up->env->fpu.regs);
    }

    t = m->ticks;       /* CPU time per proc */
    up = nil;           /* no process at interrupt level */

    hzclock(ureg);

    m->inidle = 0;
    up = m->proc;
    /* The number of clock ticks is partly used to determine preemption. */
    preemption(m->ticks - t);
    m->intr++;

    m->inidle = 0;
    splhi();

    if (rdch_ready())
        kbd_readc();

    if (up) {
        restorefpregs((double *)&up->env->fpu.regs);
        setfpscr(up->env->fpu.fpscr);
    }

//    print("up=%.8lux\n", up);
//    wrstr("<< sp="); wrhex(getsp()); wrstr(" pc="); wrhex(*(ulong *)((ulong)ureg + 52)); newline();
//    wrstr("pc="); wrhex((uint)ureg->pc); wrstr(" r14="); wrhex((uint)ureg->lr); newline();
//    wrstr("out\r\n");
//    _dumpregs();
//    wrstr("< up="); wrhex((int)up); newline();
}

void setpanic(void)
{
    spllo();
    consoleprint = 1;
}

static void linkproc(void)
{
    spllo();
    if (waserror())
	print("error() underflow: %r\n");
    else
	(*up->kpfun)(up->arg);
    pexit("end proc", 1);
}

void kprocchild(Proc *p, void (*func)(void*), void *arg)
{
    p->sched.pc = (ulong)linkproc;
    p->sched.sp = (ulong)p->kstack+KSTACK-8;
//wrstr("kprocchild sp="); wrhex((int)p->sched.sp); newline();
    p->kpfun = func;
    p->arg = arg;
}

void usage_fault(int sp)
{
    /* Entered with sp pointing to an Ereg struct. */
    Ereg *er = (Ereg *)sp;
//    wrstr("Usage fault at "); wrhex(er->pc); newline();
/*
    wrstr("CFSR="); wrhex(*(int *)CFSR_ADDR); newline();
    wrstr("SHCSR="); wrhex(*(int *)SHCSR_ADDR); newline();
    wrstr("FPCCR="); wrhex(*(int *)FPCCR_ADDR); newline();
    wrstr("up="); wrhex((int)up); newline();
    dumperegs(er);
    dumpfpregs(er);
*/
    if ((*(short *)UFSR_ADDR) & UFSR_UNDEFINSTR) {
        if (fpithumb2(er)) {
/*
            wrstr("CFSR="); wrhex(*(int *)CFSR_ADDR); newline();
            wrstr("SHCSR="); wrhex(*(int *)SHCSR_ADDR); newline();
            wrstr("FPCCR="); wrhex(*(int *)FPCCR_ADDR); newline();
            wrstr("up="); wrhex((int)up); newline();
            dumperegs(er);
            dumpfpregs(er);
            newline();
*/
//            *(short *)UFSR_ADDR |= UFSR_UNDEFINSTR;
//            setcontrol(CONTROL_FPCA);
//            wrstr("control="); wrhex(getcontrol()); newline();
//            wrstr("<-- "); wrhex(er->pc); newline();
            return;
        }
    }

    wrstr("Usage fault at "); wrhex((int)er->pc); newline();
    wrstr("UFSR="); wrhex(*(short *)UFSR_ADDR); newline();

    dumperegs((Ereg *)sp);
    poolsummary();
    poolshow();

    for (;;) {}
}

void hard_fault(int sp)
{
    Ereg *er = (Ereg *)sp;

    wrstr("Hard fault at "); wrhex(er->pc); newline();
    wrstr("HFSR="); wrhex(*(int *)HFSR_ADDR); newline();
    int cfsr = *(int *)CFSR_ADDR;
    wrstr("CFSR="); wrhex(cfsr); newline();
    int shcsr = *(int *)SHCSR_ADDR;
    wrstr("SHCSR="); wrhex(shcsr); newline();
    wrstr("FPCCR="); wrhex(*(int *)FPCCR_ADDR); newline();
    wrstr("CONTROL="); wrhex(getcontrol()); newline();
    wrstr("up="); wrhex((int)up); newline();
    wrstr("up->kstack="); wrhex((int)up->kstack); wrstr("..");
    wrhex((int)up->kstack + KSTKSIZE); newline();

    dumperegs(er);
    dumpfpregs(er);

    if (cfsr & 0x200) {
        wrstr("BFAR="); wrhex(*(int *)BFAR_ADDR); newline();
    }

    wrstr("last APSR="); wrhex(apsr_flags); newline();

    int a = sp + sizeof(Ereg);
    for (int i = 0; i < 128; i+=4) {
        wrhex(*(int *)(a + i));
        if ((i & 0xf) == 0xc)
            newline();
        else
            wrch(' ');
    }

//    poolsummary();
//    poolshow();

    for (;;) {}
}

void trap_dummy(int sp)
{
    wrstr("sp="); wrhex(sp); newline();
    wrstr("r0="); wrhex(*(int *)(sp)); newline();
    wrstr("r1="); wrhex(*(int *)(sp + 4)); newline();
    wrstr("pc="); wrhex(*(int *)(sp + 24)); newline();
    wrstr("lr="); wrhex(*(int *)(sp + 20)); newline();
    wrstr("r10="); wrhex(get_r10()); newline();
    wrstr("r12="); wrhex(get_r12()); newline();

    wrstr("stack from sp:\n");
    int i;
    for (i = 0; i < 32; i++)
        print("%.8lux ", *(ulong *)(sp + (i*4)));

    poolsummary();

    poolshow();

    for (;;) {}
}
