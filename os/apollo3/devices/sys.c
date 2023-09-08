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
}

void disablefpu(void)
{
    /* Disable CP10 and CP11 coprocessors - see page 7-71 of the Arm Cortex-M4
       Processor Technical Reference Manual. */
    *(int *)CPACR_ADDR &= ~(0xf << 20);
}
