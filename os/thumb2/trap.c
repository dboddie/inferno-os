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
    wrstr("r12="); wrhex(uregs->r12); wrch(' ');
    wrstr("sp="); wrhex((unsigned long)uregs); wrch(' ');
    wrstr("r14="); wrhex(uregs->r14);
    wrstr("pc="); wrhex(uregs->pc); newline();
}

void switcher(Ureg *ureg)
{
    int t;

//    wrch('.');
//    print("sp=%.8lux\n", getsp());
//    print("up=%.8lux\n", up);
//    print("psr=%.8lux\n", ureg->psr);
//    dumpregs(ureg);

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

//    print("up=%.8lux\n", up);
//    print("<< sp=%.8lux\n", getsp());
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
    wrstr("Usage fault at "); wrhex(*(int *)(sp + 24)); newline();
    wrstr("UFSR="); wrhex(*(short *)UFSR_ADDR); newline();
//    wrstr("Instruction: "); wrhex(**(int **)(sp + 24)); newline();

/* Step past an FP instruction, setting Thumb mode execution.
    *(int *)(sp + 24) += 4;
    *(int *)(sp + 28) = 0x01000000;
*/
    wrstr("sp="); wrhex(sp); newline();
    wrstr("r0="); wrhex(*(int *)(sp)); newline();
    wrstr("r1="); wrhex(*(int *)(sp + 4)); newline();
    wrstr("pc="); wrhex(*(int *)(sp + 24)); newline();
    wrstr("lr="); wrhex(*(int *)(sp + 20)); newline();

    for (;;) {}
}

void hard_fault(int sp)
{
    wrstr("Hard fault at "); wrhex(*(int *)(sp + 24)); newline();
/*    wrstr("UFSR="); wrhex(*(int *)UFSR_ADDR); newline();
    wrstr("Instruction: "); wrhex(**(int **)(sp + 24)); newline();*/
    int cfsr = *(int *)CFSR_ADDR;
    wrstr("CFSR="); wrhex(cfsr); newline();

    if (cfsr & 0x200) {
        wrhex(*(int *)BFAR_ADDR);
    }

    showregs(sp, 1);

    for (;;) {}
}
