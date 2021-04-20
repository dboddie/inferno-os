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

void
trapinit(void)
{
/*	memmove((ulong*)UTLBMISS, (ulong*)vector0, 0x80);
	memmove((ulong*)XEXCEPTION, (ulong*)vector0, 0x80);
	memmove((ulong*)CACHETRAP, (ulong*)vector100, 0x80);*/
	memmove((ulong*)EXCEPTION, (ulong*)vector180, 0x80);
	memmove((ulong*)INTERRUPT, (ulong*)vector200, 0x80);

	setstatus(getstatus() & ~BEV);
}

void trapintr(Ureg *ur)
{
    if (ur->cause & 0x400) {
        InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);
        if (ic->pending & InterruptTCU0) {
            clockintr(ur);
            kbdpoll();
            power_check_reset();
        }
    }
}

void trapexc(Ureg *ur)
{
    /* FPU coprocessor causing CPU exception */
    if (((ur->cause >> 28) & 3) == 1 && ((ur->cause >> 2) & EXCMASK) == CCPU) {
        if (fpuemu(ur) >= 0)
            return;
    }
    print("ca=%8.8lux\n", ur->cause);
    for (;;) {}
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
