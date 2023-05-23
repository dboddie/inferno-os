
#define KADDR(p)	((void *)p)
#define PADDR(p)	((ulong)p)
#define DMAADDR(va)	(BUSDRAM |((uintptr)(va)))
#define DMAIO(va)	(BUSIO | ((uintptr)(va)))
#define MASK(v)   ((1UL << (v)) - 1)      /* mask `v' bits wide */
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
void	screeninit(ulong);
void	setpower(int, int);
void	clockcheck(void);
void	armtimerset(int);
void	links(void);
int	fpiarm(Ureg*);
void	mdelay(uint);
void	initfrozen(void);

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

/* Hardware-specific setup */
void    hwinit(void);

void    switcher(Ureg *);
void    usage_fault(int);
void    hard_fault(int);
void    svcall(int);
void    sercom5_rxc_intr(void);
uint    get_r10(void);
uint    get_r12(void);

int getprimask(void);
int getcontrol(void);
void setcontrol(int);
int getmsp(void);
void setmsp(int);
int getpsp(void);
void setpsp(int);

#define PTR2UINT(p)     ((uintptr)(p))
#define UINT2PTR(i)     ((void*)(i))

void    savefp0(uvlong *);
void    savefp1(uvlong *);
void    savefp2(uvlong *);
void    savefp3(uvlong *);
void    savefp4(uvlong *);
void    savefp5(uvlong *);
void    savefp6(uvlong *);
void    savefp7(uvlong *);
void    savefp8(uvlong *);
void    savefp9(uvlong *);
void    savefp10(uvlong *);
void    savefp11(uvlong *);
void    savefp12(uvlong *);
void    savefp13(uvlong *);
void    savefp14(uvlong *);
void    savefp15(uvlong *);
void    savefp16(uvlong *);
void    savefp17(uvlong *);
void    savefp18(uvlong *);
void    savefp19(uvlong *);
void    savefp20(uvlong *);
void    savefp21(uvlong *);
void    savefp22(uvlong *);
void    savefp23(uvlong *);
void    savefp24(uvlong *);
void    savefp25(uvlong *);
void    savefp26(uvlong *);
void    savefp27(uvlong *);
void    savefp28(uvlong *);
void    savefp29(uvlong *);
void    savefp30(uvlong *);
void    savefp31(uvlong *);

void    restfp0(uvlong *);
void    restfp1(uvlong *);
void    restfp2(uvlong *);
void    restfp3(uvlong *);
void    restfp4(uvlong *);
void    restfp5(uvlong *);
void    restfp6(uvlong *);
void    restfp7(uvlong *);
void    restfp8(uvlong *);
void    restfp9(uvlong *);
void    restfp10(uvlong *);
void    restfp11(uvlong *);
void    restfp12(uvlong *);
void    restfp13(uvlong *);
void    restfp14(uvlong *);
void    restfp15(uvlong *);
void    restfp16(uvlong *);
void    restfp17(uvlong *);
void    restfp18(uvlong *);
void    restfp19(uvlong *);
void    restfp20(uvlong *);
void    restfp21(uvlong *);
void    restfp22(uvlong *);
void    restfp23(uvlong *);
void    restfp24(uvlong *);
void    restfp25(uvlong *);
void    restfp26(uvlong *);
void    restfp27(uvlong *);
void    restfp28(uvlong *);
void    restfp29(uvlong *);
void    restfp30(uvlong *);
void    restfp31(uvlong *);
