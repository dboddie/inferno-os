#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"
#include "../port/error.h"

#include "devices/fns.h"
extern void wrch(int);
extern void showpool(Pool *p);
extern void poolsummary(void);

extern void hzclock(Ureg *);

void trapinit(void)
{
    /* Enable the usage fault exception. */
    *(int *)SHCSR_ADDR |= SHCSR_USGFAULTENA;

    /* Enable division by zero trapping. */
    *(int *)CCR_ADDR |= CCR_DIV_0_TRP | CCR_UNALIGN_TRP;
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

extern int apsr_flags;

void switcher(Ureg *ureg)
{
    int t;

//    wrch('.');
//    wrstr(">> sp="); wrhex(getsp()); wrstr(" pc="); wrhex(*(ulong *)((ulong)ureg + 52)); newline();
//    print("up=%.8lux\n", up);
//    print("psr=%.8lux\n", ureg->psr);
//    wrstr("in\r\n");
//    _dumpregs();

    if (up) up->pc = ureg->pc;

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
        uart3_intr();

//    print("up=%.8lux\n", up);
//    wrstr("<< sp="); wrhex(getsp()); wrstr(" pc="); wrhex(*(ulong *)((ulong)ureg + 52)); newline();
//    wrstr("pc="); wrhex((uint)ureg->pc); wrstr(" r14="); wrhex((uint)ureg->lr); newline();
//    wrstr("out\r\n");
//    _dumpregs();
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
    /* Entered with sp pointing to R4-R12, R0-R3, R12, R14, PC and xPSR. */
    wrstr("Usage fault at "); wrhex(*(int *)(sp + 60)); newline();
    wrstr("UFSR="); wrhex(*(short *)UFSR_ADDR); newline();
//    wrstr("Instruction: "); wrhex(**(int **)(sp + 24)); newline();

/* Step past an FP instruction, setting Thumb mode execution.
    *(int *)(sp + 24) += 4;
    *(int *)(sp + 28) = 0x01000000;
*/
    wrstr("last APSR="); wrhex(apsr_flags); newline();
    wrstr("sp="); wrhex(sp); newline();
    wrstr("r10="); wrhex(get_r10()); newline();
    wrstr("r12="); wrhex(get_r12()); newline();

    wrstr("sp="); wrhex(sp); newline();
    wrstr("r10="); wrhex(get_r10()); wrch(' ');
    wrstr("r12="); wrhex(get_r12()); newline();

    wrstr("r0="); wrhex(*(int *)(sp + 36)); wrch(' ');
    wrstr("r1="); wrhex(*(int *)(sp + 40)); wrch(' ');
    wrstr("r2="); wrhex(*(int *)(sp + 44)); wrch(' ');
    wrstr("r3="); wrhex(*(int *)(sp + 48)); newline();
    wrstr("r4="); wrhex(*(int *)(sp)); wrch(' ');
    wrstr("r5="); wrhex(*(int *)(sp + 4)); wrch(' ');
    wrstr("r6="); wrhex(*(int *)(sp + 8)); wrch(' ');
    wrstr("r7="); wrhex(*(int *)(sp + 12)); newline();
    wrstr("r8="); wrhex(*(int *)(sp + 16)); wrch(' ');
    wrstr("r9="); wrhex(*(int *)(sp + 20)); wrch(' ');
    wrstr("r10="); wrhex(*(int *)(sp + 24)); wrch(' ');
    wrstr("r11="); wrhex(*(int *)(sp + 28)); newline();
    wrstr("r12="); wrhex(*(int *)(sp + 32)); wrch(' ');
    wrstr("lr="); wrhex(*(int *)(sp + 56)); wrch(' ');
    wrstr("pc="); wrhex(*(int *)(sp + 60)); wrch(' ');
    wrstr("xPSR="); wrhex(*(int *)(sp + 64)); newline();

    poolsummary();

    newline();
    showpool(mainmem);
    newline();
    showpool(heapmem);

    for (;;) {}
}

void hard_fault(int sp)
{
    /* Entered with sp pointing to R4-R12, R0-R3, R12, R14, PC and xPSR. */
    wrstr("Hard fault at "); wrhex(*(int *)(sp + 60)); newline();
/*    wrstr("UFSR="); wrhex(*(int *)UFSR_ADDR); newline();
    wrstr("Instruction: "); wrhex(**(int **)(sp + 24)); newline();*/
    int cfsr = *(int *)CFSR_ADDR;
    wrstr("CFSR="); wrhex(cfsr); newline();

    if (cfsr & 0x200) {
        wrstr("BFAR="); wrhex(*(int *)BFAR_ADDR); newline();
    }

    wrstr("last APSR="); wrhex(apsr_flags); newline();
    wrstr("sp="); wrhex(sp); newline();
    wrstr("r10="); wrhex(get_r10()); wrch(' ');
    wrstr("r12="); wrhex(get_r12()); newline();

    wrstr("r0="); wrhex(*(int *)(sp + 36)); wrch(' ');
    wrstr("r1="); wrhex(*(int *)(sp + 40)); wrch(' ');
    wrstr("r2="); wrhex(*(int *)(sp + 44)); wrch(' ');
    wrstr("r3="); wrhex(*(int *)(sp + 48)); newline();
    wrstr("r4="); wrhex(*(int *)(sp)); wrch(' ');
    wrstr("r5="); wrhex(*(int *)(sp + 4)); wrch(' ');
    wrstr("r6="); wrhex(*(int *)(sp + 8)); wrch(' ');
    wrstr("r7="); wrhex(*(int *)(sp + 12)); newline();
    wrstr("r8="); wrhex(*(int *)(sp + 16)); wrch(' ');
    wrstr("r9="); wrhex(*(int *)(sp + 20)); wrch(' ');
    wrstr("r10="); wrhex(*(int *)(sp + 24)); wrch(' ');
    wrstr("r11="); wrhex(*(int *)(sp + 28)); newline();
    wrstr("r12="); wrhex(*(int *)(sp + 32)); wrch(' ');
    wrstr("lr="); wrhex(*(int *)(sp + 56)); wrch(' ');
    wrstr("pc="); wrhex(*(int *)(sp + 60)); wrch(' ');
    wrstr("xPSR="); wrhex(*(int *)(sp + 64)); newline();

    poolsummary();

    poolshow();

    for (;;) {}
}

void dummy(int sp)
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
