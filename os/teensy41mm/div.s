THUMB=4

Q   = 7
N   = 7
D   = 1
CC  = 3
TMP = 11

#define SDIV(d, n, m) \
    WORD $(0xf0f0fb90 | (n & 0xf) | ((m & 0xf) << 16) | ((d & 0xf) << 24))

#define UDIV_low(d, n, m) (0xfbb0 | (n & 0xf))
#define UDIV_high(d, n, m) (0xf000 | ((d & 0xf) << 8) | 0xf0 | (m & 0xf))

#define UDIV(d, n, m) \
    WORD $(UDIV_low(d, n, m) | (UDIV_high(d, n, m) << 16))

#define MUL2(d, n, m) \
    WORD $(0xf000fb00 | (n & 0xf) | ((m & 0xf) << 16) | ((d & 0xf) << 24))

TEXT _div(SB), THUMB, $0

    MOVW 4(SP), R1
    SDIV(Q, N, D)
    RET

TEXT _divu(SB), THUMB, $0

    MOVW 4(SP), R1
    UDIV(Q, N, D)
    RET

TEXT _mod(SB), THUMB, $0
TEXT _modu(SB), THUMB, $0

    MOVW 4(SP), R1
    SDIV(TMP, N, D)         /* TMP = N DIV D */
    MUL2(D, D, TMP)          /* D = D * (N DIV D) */
    SUB R(D), R(Q)          /* Q = N - D * (N DIV D) */
    RET

/* From ARM Architecture Reference Manual Thumb-2 Supplement, A-16:
   x MOD y = x - y * (x DIV y) */
