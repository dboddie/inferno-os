#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "thumb2.h"
#include "../port/error.h"

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
