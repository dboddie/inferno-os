#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/uart.h"

#include "devices/fns.h"

void kbdinit(void)
{
    kbdq = qopen(1024, 0, nil, nil);
    qnoblock(kbdq, 1);
    enable_usart_intr(1);
}

void uart3_intr(void)
{
    int c = rdch();
    kbdputc(kbdq, c);
    wrch(c);
    if (c == 13) {
        kbdputc(kbdq, 10);
        wrch(10);
    }
}
