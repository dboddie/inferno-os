#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"
#include "version.h"
#include "io.h"

#include "../port/uart.h"
PhysUart* physuart[1];

Conf conf;
Mach *m = (Mach*)MACHADDR;
Proc *up = 0;

extern int main_pool_pcnt;
extern int heap_pool_pcnt;
extern int image_pool_pcnt;

void confinit(void)
{
    ulong base = PGROUND((ulong)end);

    conf.topofmem = MEMORY_TOP;     /* defined in mem.h */
    conf.base0 = base;

    conf.npage1 = 0;
    conf.npage0 = (conf.topofmem - base)/BY2PG;
    conf.npage = conf.npage0 + conf.npage1;
    conf.ialloc = (((conf.npage*(main_pool_pcnt))/100)/2)*BY2PG;

    conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
    conf.nmach = 1;

    active.machs = 1;
    active.exiting = 0;
}

static void poolsizeinit(void)
{
    ulong nb;
    nb = conf.npage*BY2PG;
    poolsize(mainmem, (nb*main_pool_pcnt)/100, 0);
    poolsize(heapmem, (nb*heap_pool_pcnt)/100, 0);
    poolsize(imagmem, (nb*image_pool_pcnt)/100, 1);
}

void main(void)
{
    /* Mach is defined in dat.h, edata and end are in port/lib.h */
    memset(m, 0, sizeof(Mach));
    memset(edata, 0, end-edata);

    /* Set the exception stack pointer to a page above that */
    m->exc_sp = ESTACKTOP;

    quotefmtinstall();
    confinit();
    xinit();                    /* in port/xalloc.c */
    poolinit();                 /* in port/alloc.c */
    poolsizeinit();
    trapinit();                 /* in trap.c */

    screeninit();               /* in screen.c */
//    power_init();                /* power button handling */

    timersinit();               /* in port/portclock.c */
    clockinit();                /* in clock.c */
    printinit();                /* in port/devcons.c */
    print("\nInferno OS %s Vita Nuova\n", VERSION);

    kbdinit();

    procinit();                 /* in port/proc.c */
    links();                    /* in the generated nanonote.c file */
    chandevreset();

    eve = strdup("inferno");

    userinit();
    schedinit();                /* in port/proc.c and should never return */

    print("to infinite loop\n\n");
    for (;;);
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
        ksetenv("cputype", "spim", 0);
        snprint(buf, sizeof(buf), "spim %s", conffile);
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

    /* Kernel Stack, 4 bytes for gotolabel's return PC */
    p->sched.pc = (ulong)init0;
    p->sched.sp = (ulong)p->kstack+KSTACK;

    ready(p);
}

void    segflush(void * /* p */, ulong /* n */)
{
    //print("segflush: %p %8.8lux\n", p, n);
}

void    idlehands(void) { return; }

void    exit(int) { return; }

/* For some reason, this function from lib9 isn't linked in, so include it here. */
void
exits(char *s)
{
	if(s == 0 || *s == 0)
		exit(0);
	exit(1);
}

void    halt(void) { return; }

void    fpinit(void)
{
}


void
FPsave(FPenv *fps)
{
/*    fps->control = fps->status = getfpscr();

    for (int n = 0; n < 32; n++) {
        int s = splhi();
        savefns[n]((uvlong *)fps->regs[n]);
        splx(s);
        coherence();
    }*/
}

void
FPrestore(FPenv *fps)
{
/*    setfpscr(fps->control);

    for (int n = 0; n < 32; n++) {
        int s = splhi();
        restfns[n]((uvlong *)fps->regs[n]);
        coherence();
        splx(s);
    }*/
}

/* To be moved into trap.c */
void setpanic(void)
{
}

void    reboot(void) { return; }
