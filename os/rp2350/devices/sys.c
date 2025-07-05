#include "fns.h"
#include "thumb2.h"

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
