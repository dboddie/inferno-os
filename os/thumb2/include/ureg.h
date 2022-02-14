typedef struct Ureg {
	uint	r4;
	uint	r5;
	uint	r6;
	uint	r7;
	uint	r8;
	uint	r9;
	uint	r10;
	uint	r11;
	uint	r12; /* sb */
	uint	r0;
	uint	r1;
	uint	r2;
	uint	r3;
	union {
		uint	r14;
		uint	link;
	};
	uint	pc;     /* interrupted addr */
	uint	psr;    /* status register of interrupted code */
} Ureg;
