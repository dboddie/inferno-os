# Notes

In **main.c** a call to the `timersinit` function (provided by **port/portclock.c**)
is required to ensure that locks can work, otherwise the "no way out" error
from **port/taslock.c** will be shown.

## Writing an exception handler

With exceptions, care must be taken to include only a small handler to fit in
the region allocated to it. So, the exception handler at KSEG0+0x180 needs to
fit in 0x80 bytes and call a routine elsewhere in memory.

## Loading the initial module

The `/osinit.dis` module fails to load.

The `disinit` function in `os/port/dis.c` obtains 0 from calling

 * the `load` function in `libinterp/load.c` which calls
 * `readmod` in `libinterp/readmod.c` which calls
 * `kopen` in `os/port/sysfile.c` which calls
 * `namec` in `os/port/chan.c`

Comparing with the A7000+ port shows that the same initial names, modes and
permissions are passed to `namec` but that the Nanonote fails to recognise that
paths under `/` do not need to be created.

The problems seem to be due to the assembly language versions of the string
functions in `libkern`. Using C implementations and running `mk nuke` before
building appears to cure the problem.

## Exception in trap

Using `fbprint` in the `trap` function, referring to the `Ureg` structure
pointer, causes an exception to occur when the function returns, showing the
location of the stored registers as the EPC.

## Handling interrupts

When enabling interrupts, unmasking the relevant bits in the interrupt
controller and enabling them in the peripherals, it is necessary to clear the
interrupt flags in any peripherals when they occur. If this is not done then
further interrupts are not delivered.
