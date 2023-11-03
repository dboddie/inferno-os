#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"
#include "version.h"

#include "thumb2.h"
#include "devices/fns.h"

Conf conf;
Mach *m = (Mach*)MACHADDR;
Proc *up = 0;

void main(void)
{
    setup_uart();

    for (int i = 0; i < 256; i++)
        wrch(i);
}

void reboot(void)
{
    print("rebooting...\n");
    *(uint *)SCB_AIRCR = SCB_AIRCR_VECTKEYRESET | SCB_AIRCR_SYSRESETREQ;
}

void idlehands(void)
{
}

void exit(int)
{
    reboot();
}

void halt(void)
{
}

void FPsave(FPenv *fps)
{
}

void FPrestore(FPenv *fps)
{
}

void fpinit(void)
{
    enablefpu();
}

void    segflush(void *p, ulong n)
{
}

void uartputs(char *data, int len)
{
/*
    int s;
    s = splhi();
    while (--len >= 0) {
    if (*data == '\n')
        wrch('\r');
        wrch(*data++);
    }
    splx(s);
*/
}

/* From trap.c */

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
