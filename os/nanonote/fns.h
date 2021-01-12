
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
ulong	getcpsr(void);
ulong	getspsr(void);
ulong	getcpuid(void);
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
void	trapinit(void);
void    power_init(void);
char *	trapname(int psr);
int	isvalid_va(void *v);
int	isvalid_wa(void *v);
void	setr13(int, void*);
void	vectors(void);
void	vtable(void);
void	setpanic(void);
void	dumpregs(Ureg*);
void	dumparound(uint addr);
void    dump_flags(void);
int	(*breakhandler)(Ureg*, Proc*);
void    irqenable(u32int *, u32int, void (*)(Ureg*, void*), void*);
#define intrenable(i, f, a, b, n) irqenable((i), (f), (a))

void	cachedwbinv(void);
void	cachedwbse(void*, int);
void	cachedwbinvse(void*, int);
void	cachedinvse(void*, int);
void	cacheiinvse(void*, int);
void	cacheiinv(void);
void	cacheuwbinv(void);

void    kbdinit(void);
void	mmuinit1(void);
void	mmuinvalidateaddr(u32int);
void	screeninit(void);
void	setpower(int, int);
void	clockcheck(void);
void	armtimerset(int);
void	links(void);
int	fpiarm(Ureg*);
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

void fbprint(unsigned int v, unsigned int l);
