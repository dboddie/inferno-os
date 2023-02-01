#include "u.h"
#include "../port/lib.h"
#include "isa.h"
#include "interp.h"
#include "raise.h"
#include <kernel.h>

#include "frozen.h"

void
initfrozen(void)
{
    /* Initialise frozen modules using the generated macro defined in frozen/frozen.h. */
    print("initialising frozen modules\n");
    #ifdef InitFrozen
    InitFrozen
    #endif
}
