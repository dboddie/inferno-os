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

## USB

Useful for understanding what information is available to host and USB device
in a bulk transfer:

https://stackoverflow.com/questions/41855995/when-should-a-usb-device-send-a-zlp-on-a-bulk-pipe

From http://en.qi-hardware.com/wiki/Ingenic_documentation_errata

    22.4.2 Memory Map

    On page 432, it says that the control and status registers of all endpoints
    should exist non-indexed on address BASE+0x100+ep*0x10+offset. This means
    CSR0 should be available on address BASE+0x102. Using this address does not
    work, however. I did not test if the other non-indexed addresses do work,
    or if they don't exist at all.

    22.8.3 READ REQUESTS
    On page 459 it tells how to respond to a read request from the host.
    Following this doesn't work, because one important step is missing: Before
    filling the fifo, the SETUP packet must be acknoledged. The pm says that
    upon receiving the request, instead of setting DataEnd you should fill the
    fifo and set InPktRdy (and DataEnd if it's the last packet). However, it
    only works when doing this _after_, not _instead of_. This can be fixed by
    adding "and the DataEnd bit (D3)" after "the ServicedOutPktRdy bit (D6)" in
    the last sentence of the second paragraph. 

## MMC/SD

U-Boot appears to set the MSC registers to these values:

    STRPCL  0
    STAT    0x140       Clock enabled, CRC write error
    CLKRT   0           CLK_SRC clock rate
    CMDAT   0x409       4-bit bus width, current command is a write operation,
                        response format R1 and R1b

