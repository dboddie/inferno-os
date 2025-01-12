#include "u.h"
#include "ureg.h"

extern void setup_led(void);
extern void set_led(int);

void main(void)
{
    int state = 0;
    setup_led();

    for (;;) {
        set_led(state);
        for (int i = 0; i < 1000000; i++) {}
        state = 1 - state;
    }
}

void hard_fault(int)
{
}

void usage_fault(int)
{
}

void bus_fault(int)
{
}

void trap_dummy(int)
{
}

void switcher(Ureg *)
{
}

void dumpregs(Ureg *)
{
}
