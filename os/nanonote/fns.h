
#define KADDR(p)	((void *)((ulong)(p)|KSEG0))
#define PADDR(p)	((ulong)(p)|KSEG0)
#define DMAADDR(va)	(BUSDRAM |((uintptr)(va)))
#define DMAIO(va)	(BUSIO | ((uintptr)(va)))
#ifndef MASK
#define MASK(v)   ((1UL << (v)) - 1)      /* mask `v' bits wide */
#endif
#define waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
#define procsave(p)	/* Save the mach part of the current */
			/* process state, no need for one cpu */
#define kmapinval()

void	(*serwrite)(char*, int);
void    (*screenputs)(char*, int);

#include "../port/portfns.h"

unsigned int get_vectors_base(void);
void set_vectors_base(unsigned int);

ulong   getsc(void);
ulong   getac(void);
void    setac(ulong);
ulong   getnsac(void);
void    setnsac(ulong);
ulong   getisac(void);

ulong	getsp(void);
ulong	getpc(void);
ulong	getepc(void);
ulong	getprid(void);
ulong	getcause(void);
ulong	getcacheinfo(void);
ulong	getcallerpc(void*);
u32int	lcycles(void);
int	splfhi(void);
int	tas(void *);

ulong   getfpexc(void);
void   setfpexc(ulong);
ulong   getfpsid(void);
ulong   getfpscr(void);
void   setfpscr(ulong);

void	delay(int);
int	islo(void);
void	microdelay(int);
void	idlehands(void);
void	_idlehands(void);

void	coherence(void);
void	clockinit(void);
void    clockintr(Ureg *);
void	trapinit(void);
char *	trapname(int psr);
int	isvalid_va(void *v);
int	isvalid_wa(void *v);

void	vecinit(void);
void	vector0(void);
void	vector100(void);
void	vector180(void);
void	vector200(void);

void	introff(int);
void	intron(int);

void	setpanic(void);
void	dumpregs(Ureg*);
void	dumparound(uint addr);
void    dump_flags(void);
int	(*breakhandler)(Ureg*, Proc*);
void    irqenable(u32int *, u32int, void (*)(Ureg*, void*), void*);
#define intrenable(i, f, a, b, n) irqenable((i), (f), (a))

ulong   getstatus(void);
void    setstatus(ulong);
ulong   getconfig(void);
ulong   getebase(void);

void show_cpu_config(void);

void    kbdinit(void);
void    kbdpoll(void);
void    powerinit(void);
void    powerintr(void);
void	screeninit(void);
void	clockcheck(void);
void	links(void);
void	mdelay(uint);

char*	getconf(char*);
char *	getethermac(void);
void	getramsize(Conf *);

void	drawqlock(void);
void	drawqunlock(void);
int	candrawqlock(void);
void	swcursorinit(void);

int	isaconfig(char *, int, ISAConf *);

uintptr dmaaddr(void *va);
void 	dmastart(int, int, int, void*, void*, int);
int 	dmawait(int);

/* Nanonote specific hardware setup */
void    hwinit(void);

#define PTR2UINT(p)     ((uintptr)(p))
#define UINT2PTR(i)     ((void*)(i))

void fbprint(unsigned int v, unsigned int l, unsigned int colour);
