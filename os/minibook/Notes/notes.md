# Notes

Starting a port of Inferno to the Letux 400 Minibook in parallel to the port of
the Qi Hardware Ben NanoNote.

## Memory map

Linux kernels appear to be loaded at 0x80010000 by U-Boot, according to the
uImage file I found. Looking at some kernel sources, I see that the file
`arch/mips/boot/Makefile` specifies a `LOADADDR` that matches this.

The memory map at https://projects.goldelico.com/p/letux400/page/MemoryMap/

## GPIO

The lists at
https://web.archive.org/web/20120214203947/http://projects.kwaak.net/twiki/bin/view/Epc700/GPIO
and the contents of the `arch/mips/jz4730/proc.c` file in old Letux 400 kernels
show the following:

 * Caps Lock LED is on port 0/A pin 27
 * Num Lock LED is on port 2/C pin 22
 * Scroll Lock LED is on port 0/A pin 9

These appear to be active low.

The Linux kernel uses the GPIO_GPAUR(n) macro
(defined in include/asm-mips/mach-jz4730/regs.h) in the __gpio_as_pwn() macro
(defined in include/asm-mips/mach-jz4730/ops.h) to set the flags in the
high selection register of port 2/C at 0x10010060 + 0x14, clearing bits 28-31
and setting bits 28 and 30 so that the SEL31 and SEL30 pairs of bits both
contain a value of 1. This sets pins 30 and 31 of port 2/C to have function 1.
