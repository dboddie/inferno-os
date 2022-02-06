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

void trapinit(void)
{
    /* Enable the usage fault exception. */
    *(int *)SHCSR_ADDR |= SHCSR_USGFAULTENA;

    /* Enable division by zero trapping. */
    *(int *)CCR_ADDR |= CCR_DIV_0_TRP;
}

void setpanic(void)
{
    spllo();
    consoleprint = 1;
}

void kprocchild(Proc *p, void (*func)(void*), void *arg)
{/*
    p->sched.pc = (ulong)linkproc;
    p->sched.sp = (ulong)p->kstack+KSTACK-8;

    p->kpfun = func;
    p->arg = arg;
*/}

void usage_fault(int msp)
{
/*
    wrstr("Usage fault at "); wrhex(*(int *)(msp + 24)); newline();
    wrstr("UFSR="); wrhex(*(int *)UFSR_ADDR); newline();
    wrstr("Instruction: "); wrhex(**(int **)(msp + 24)); newline();
    *(int *)(msp + 24) += 4;
    *(int *)(msp + 28) = 0x01000000;
*/
    for (;;) {}
}

void hard_fault(void)
{
/*
    wrstr("Hard fault\r\n");
    wrhex(*(int *)HFSR_ADDR); newline();
    wrhex(*(int *)UFSR_ADDR);
*/
    for (;;) {}
}
