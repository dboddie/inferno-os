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
 * Network LED is on port 0/A pin 9 (listed as Scroll Lock in the kwaak list)
