#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"
#include "version.h"

#include "../port/uart.h"
PhysUart* physuart[1];
extern void uartconsinit(void);

#include "thumb2.h"
#include "devices/fns.h"

Conf conf;
Mach *m = (Mach*)MACHADDR;
Proc *up = 0;

extern int main_pool_pcnt;
extern int heap_pool_pcnt;
extern int image_pool_pcnt;

int apsr_flags = 0;

void _debug(void)
{
    print("AHB_mask=%.8x\r\n", *(int *)MCLK_AHB_mask);
    CMCC *cmcc = (CMCC *)CMCC_base;
    print("CMCC_ctrl=%.8x\r\n", cmcc->ctrl);

    print("control=%.8x\r\n", getcontrol());
    print("primask=%.8x\r\n", getprimask());

    print("SHPR3=%.8lux\r\n", *(int *)SHPR3);
    print("ICTR=%.8lux\r\n", *(int *)ICTR);

    int *ipr0 = (int *)NVIC_IPR0;
    print("IPR20=%.8lux\r\n", ipr0[20]);
}

void confinit(void)
{
    conf.topofmem = MEMORY_TOP;
    conf.base0 = PGROUND((ulong)end);
    conf.base1 = 0;

    conf.npage0 = (conf.topofmem - conf.base0)/BY2PG;
    conf.npage1 = 0;
    conf.npage = conf.npage0 + conf.npage1;
    conf.ialloc = BY2PG;

//    conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
    conf.nproc = 10;
    conf.nmach = 1;

    active.machs = 1;
    active.exiting = 0;
}

static void poolsizeinit(void)
{
    ulong nb = conf.npage*BY2PG;
    poolsize(mainmem, (nb*main_pool_pcnt)/100, 0);
    poolsize(heapmem, (nb*heap_pool_pcnt)/100, 0);
    poolsize(imagmem, (nb*image_pool_pcnt)/100, 1);
}

extern char bdata[];

void main(void)
{
    setup_usart();

    /* Mach is defined in dat.h, edata and end are in port/lib.h */
    memset(m, 0, sizeof(Mach));
    memset(edata, 0, end-edata);

    quotefmtinstall();
    confinit();
    links();                    // in the file generated from the configuration
    xinit();                    // in port/xalloc.c

    poolinit();                 // in port/alloc.c
    poolsizeinit();

    trapinit();                 // in trap.c

//    uartconsinit();
    serwrite = usart_serwrite;

    timersinit();               // in port/portclock.c
    clockinit();                // in clock.c
    printinit();                // in port/devcons.c

    _debug();

    initfrozen();

    kbdinit();

    procinit();                 /* in port/proc.c */
    links();
    chandevreset();

    eve = strdup("inferno");
    userinit();
    schedinit();                /* in port/proc.c and should never return */

    for (;;) {
    }
}

void
init0(void)
{
    Osenv *o;
    char buf[2*KNAMELEN];

    up->nerrlab = 0;

    spllo();

    if(waserror())
        panic("init0 %r");

    /* These are o.k. because rootinit is null.
     * Then early kproc's will have a root and dot. */

    o = up->env;
    o->pgrp->slash = namec("#/", Atodir, 0, 0);
    cnameclose(o->pgrp->slash->name);
    o->pgrp->slash->name = newcname("/");
    o->pgrp->dot = cclone(o->pgrp->slash);

    chandevinit();

    if(!waserror()){
        ksetenv("cputype", "arm", 0);
        snprint(buf, sizeof(buf), "arm %s", conffile);
        ksetenv("terminal", buf, 0);
        poperror();
    }

    poperror();

    disinit("/osinit.dis");
}

void
userinit(void)
{
    Proc *p;
    Osenv *o;

    p = newproc();
    o = p->env;

    o->fgrp = newfgrp(nil);
    o->pgrp = newpgrp();
    o->egrp = newegrp();
    kstrdup(&o->user, eve);

    strcpy(p->text, "interp");

    p->fpstate = FPINIT;

    /*    Kernel Stack
        N.B. The -12 for the stack pointer is important.
        4 bytes for gotolabel's return PC */
    p->sched.pc = (ulong)init0;
    p->sched.sp = (ulong)p->kstack+KSTACK-8;

    ready(p);
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
    int s;
    s = splhi();
    while (--len >= 0) {
    if (*data == '\n')
        wrch('\r');
        wrch(*data++);
    }
    splx(s);
}
