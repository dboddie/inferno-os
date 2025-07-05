/* ARMv7-M Architecture Reference Manual, A7.7.82, mask=3 (nzcvqg bits)
   See B5.1.1 for information about the bits. */
#define MSR(Rn, spec) \
    WORD $(0x8000f380 | (Rn & 0xf) | ((3 & 3) << 26) | ((spec & 0xff) << 16))
