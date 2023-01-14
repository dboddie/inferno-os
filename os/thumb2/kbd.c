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
    enable_usart_intr(0);
}

void uart3_intr(void)
{
    int c = rdch();
    /* Filter backspace */
    if (c == 127) {
        kbdputc(kbdq, 8);
        wrch(8);
        wrch(32);
        wrch(8);
        return;
    }
    /* Process characters normally */
    kbdputc(kbdq, c);
    wrch(c);
    /* Add additional newlines for carriage returns */
    if (c == 13) {
        kbdputc(kbdq, 10);
        wrch(10);
    }
}
