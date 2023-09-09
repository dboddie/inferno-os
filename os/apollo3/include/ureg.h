typedef struct Ureg {
    ulong r0;
    ulong r1;
    ulong r2;
    ulong r3;
    ulong r4;
    ulong r5;
    ulong r6;
    ulong r7;
    ulong r8;
    ulong r9;
    ulong r10;
    ulong r11;
    union {
        ulong r14;
        ulong link;
        ulong lr;
    };
    ulong pc;   /* interrupted addr */
} Ureg;

typedef struct Ereg {
    /* Stacked in our exception handler */
    union {
        ulong sp;   /* interrupted sp */
        ulong r13;
    };
    ulong r4;
    ulong r5;
    ulong r6;
    ulong r7;
    ulong r8;
    ulong r9;
    ulong r10;
    ulong r11;
    ulong exc_ret;

    /* Automatically stacked by the CPU on exception */
    ulong r0;
    ulong r1;
    ulong r2;
    ulong r3;
    ulong r12;
    union {
        ulong r14;
        ulong link;
        ulong lr;
    };
    ulong pc;   /* interrupted addr */
    ulong xpsr;

    ulong s[16];
    ulong fpscr;
} Ereg;
