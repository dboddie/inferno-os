#include "samd51.h"

void enable_PORT(void)
{
    /* Enable the clock for PORT. */
    *(unsigned int *)MCLK_APBB_mask |= MCLK_APBB_PORT;
}
