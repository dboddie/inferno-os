typedef struct ISAConf ISAConf;
typedef struct Lock Lock;
typedef struct Ureg Ureg;
typedef struct Label Label;
typedef struct FPenv FPenv;
typedef struct I2Cdev I2Cdev;
typedef struct PhysUart PhysUart;
typedef struct Mach Mach;
typedef struct MMMU	MMMU;
typedef struct FPU FPU;
typedef ulong  Instr;
typedef struct Conf Conf;
typedef u32int PTE;
typedef struct Soc      Soc;
typedef struct Softtlb	Softtlb;

struct Lock
{
	ulong   key;
	ulong   sr;
	ulong   pc;
	int pri;
};

struct Label
{
	ulong   sp;
	ulong   pc;
};

/*
 * floating point registers
 */
enum
{
	FPINIT,
	FPACTIVE,
	FPINACTIVE,
	FPEMU,

	/* bits or'd with the state */
	FPILLEGAL= 0x100,
};

enum {
        Nfpregs = 32,   /* floats; half as many doubles */
};

/* In Plan 9 this struct is called FPsave: */
struct FPenv
{
	ulong	status;
	ulong   control;
	ulong   regs[Nfpregs][3];
	int     fpistate;
        uintptr pc;
};

struct FPU
{
	FPenv env;
};

struct Conf
{
	ulong   nmach;      /* processors */
	ulong   nproc;      /* processes */
	ulong   npage;      /* total physical pages of memory */
	ulong   npage0;     /* total physical pages of memory */
	ulong   npage1;     /* total physical pages of memory */
	ulong   base0;      /* base of bank 0 */
	ulong   base1;      /* base of bank 1 */
	ulong   ialloc;     /* max interrupt time allocation in bytes */
	ulong   topofmem;   /* top addr of memory */
	int     monitor;    /* flag */
};

#include "../port/portdat.h"

/* See Plan 9's 9/rb/dat.h */
struct Mach
{
	/* the following are all known by l.s and cannot be moved */
	int	machno;			/* physical id of processor FIRST */
	Softtlb*stb;			/* Software tlb simulation SECOND */
	Proc*	proc;			/* process on this processor THIRD */
	ulong	splpc;			/* pc that called splhi() FOURTH */
	ulong	tlbfault;		/* # of tlb faults FIFTH */
	ulong	ktlbfault;
	ulong	utlbfault;
	ulong	exc_sp;                 /* exception stack pointer */

	/* the following is safe to move */
	ulong	tlbpurge;
	ulong	ticks;			/* of the clock since boot time */
	Label	sched;			/* scheduler wakeup */
	void*	alarm;			/* alarms bound to this clock */
	int	lastpid;		/* last pid allocated on this machine */
	int	knext;
	int	speed;			/* cpu speed */
	ulong	delayloop;		/* for the delay() routine */
	ulong	fairness;		/* for runproc */
	int	inclockintr;
	int	ilockdepth;
	uvlong	cyclefreq;		/* Frequency of user readable cycle counter */

	/* for per-processor timers */
	ulong	lastcount;
	uvlong	fastticks;
	ulong	hz;
	ulong	maxperiod;
	ulong	minperiod;

	Proc*	readied;		/* for runproc */
	ulong	schedticks;		/* next forced context switch */

	int	pfault;
	int	cs;
	int	syscall;
	int	load;
	int	intr;
	int	hashcoll;		/* soft-tlb hash collisions */
	int	paststartup;		/* for putktlb */
};

extern Mach *m;
extern Proc *up;

/* offsets known by l.s */
struct Softtlb
{
	ulong	virt;
	ulong	phys0;
	ulong	phys1;
};

struct
{
	Lock;
	int machs;          /* bitmap of active CPUs */
	int exiting;        /* shutdown */
} active;
