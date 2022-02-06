#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
#include "fns.h"
#include "version.h"
#include "io.h"

#include "../port/uart.h"
PhysUart* physuart[1];
extern void uartconsinit(void);

#include "devices/stm32f405.h"
#include "devices/fns.h"

int interrupts_enabled;

Conf conf;
Mach *m = (Mach*)MACHADDR;
Proc *up = 0;

extern int main_pool_pcnt;
extern int heap_pool_pcnt;
extern int image_pool_pcnt;

void confinit(void)
{
    ulong base = PGROUND((ulong)end);

    conf.topofmem = MEMORY_TOP;
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
    enablefpu();
    setup_system_clock();

    /* Mach is defined in dat.h, edata and end are in port/lib.h */
    memset(m, 0, sizeof(Mach));
    memset(edata, 0, end-edata);

    trapinit();                 // in trap.c

    quotefmtinstall();
    confinit();
    xinit();                    // in port/xalloc.c
    poolinit();                 // in port/alloc.c
    poolsizeinit();

    uartconsinit();
    serwrite = usart_serwrite;

    timersinit();               // in port/portclock.c
    clockinit();                // in clock.c
    printinit();                // in port/devcons.c

    print("OK\r\n");

    setup_LED();
    set_LED(1);
    for (;;) {
    }
}

void reboot(void)
{
    return;
}

void idlehands(void)
{
}

void exit(int)
{
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
    /* Enable CP10 and CP11 coprocessors - see page 7-71 of the Arm Cortex-M4
       Processor Technical Reference Manual. */
//    *(int *)CPACR_ADDR |= (0xf << 20);
}

void    segflush(void *p, ulong n)
{
}
