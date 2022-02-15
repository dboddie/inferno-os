typedef struct Ureg {
        ulong   sp;
	ulong	r4;
	ulong	r5;
	ulong	r6;
	ulong	r7;
	ulong	r8;
	ulong	r9;
	ulong	r10;
	ulong	r11;
        ulong   exc_r14;
	ulong	r0;
	ulong	r1;
	ulong	r2;
	ulong	r3;
	ulong	r12; /* sb */
	union {
		ulong	r14;
		ulong	link;
		ulong	lr;
	};
	ulong	pc;     /* interrupted addr */
	ulong	psr;    /* status register of interrupted code */
} Ureg;
