/* ARMv7-M Architecture Reference Manual, A7.7.82, mask=2 (nzcvq bits)
   See B5.1.1 for information about the bits. */
#define MSR(Rn, spec) \
    WORD $(0x8800f380 | (Rn & 0xf) | ((spec & 0xff) << 16))
