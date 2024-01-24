#include "thumb2.h"
#include "fns.h"

/*
int getcpacr(void)
{
    return *(int *)CPACR_ADDR;
}
*/
int getcpuid(void)
{
    return *(int *)CPUID_ADDR;
}

void enablefpu(void)
{
    /* Enable CP10 and CP11 coprocessors - see page 7-71 of the Arm Cortex-M4
       Processor Technical Reference Manual. */
    *(int *)CPACR_ADDR |= (0xf << 20);
    coherence();
}

void disablefpu(void)
{
    /* Disable CP10 and CP11 coprocessors - see page 7-71 of the Arm Cortex-M4
       Processor Technical Reference Manual. */
    *(int *)CPACR_ADDR &= ~(0xf << 20);
    coherence();
}

int getmvfr(int i)
{
    /* Return the contents of the MVFR<i> register containing FP feature
       information. */
    switch (i) {
    case 0:
        return *(int *)MVFR0;
    case 1:
        return *(int *)MVFR1;
    case 2:
        return *(int *)MVFR2;
    default:
        ;
    }
    return 0;
}

short getufsr(void)
{
    return *(short *)UFSR_ADDR;
}

void clearufsr(short bits)
{
    *(short *)UFSR_ADDR |= bits;
}

int getfpccr(void)
{
    return *(int *)FPCCR_ADDR;
}

int getccr(void)
{
    return *(int *)CCR_ADDR;
}

int getcfsr(void)
{
    return *(int *)CFSR_ADDR;
}

int setcfsr(int bits)
{
    return *(int *)CFSR_ADDR |= bits;
}
