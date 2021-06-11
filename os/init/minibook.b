implement Init;

#
# Letux 400 Minibook
#

include "sys.m";
    sys: Sys;
    print, bind: import sys;

include "sh.m";
    sh: Sh;

include "draw.m";
    draw: Draw;
    Context, Display, Image, Point, Rect: import draw;

include "disks.m";
    disks: Disks;
    Disk, PCpart, Magic0, Magic1, NTentry, Toffset, TentrySize: import disks;

# From fdisk.b:
TableSize: con TentrySize*NTentry;
Omagic: con TableSize;

#include "bufio.m";
#    bufio: Bufio;

Init: module
{
    init:	fn(nil: ref Context, nil: list of string);
};

err(s: string) { sys->fprint(sys->fildes(2), "init: %s\n", s); }

dobind(f, t: string, flags: int) {
	if(sys->bind(f, t, flags) < 0)
		err(sys->sprint("can't bind %s on %s: %r", f, t));
}

init(context: ref Context, nil: list of string)
{
    sys = load Sys Sys->PATH;
    sh = load Sh Sh->PATH;
    disks = load Disks Disks->PATH;
    disks->init();
#    bufio = load Bufio Bufio->PATH;

    sh->system(nil, "mount -c {mntgen} /n");
	
    print("Ported to the Letux 400 Minibook by David Boddie\n\n");

#    bind("#c", "/dev", sys->MAFTER);
#    bind("#r", "/dev", sys->MAFTER);		# RTC

    #
    # default namespace
    #
    bind("#c", "/dev", sys->MREPL);		# console
#   bind("#t", "/dev", sys->MAFTER);		# serial port
    bind("#B", "/dev", sys->MAFTER);            # backlight
#    bind("#L", "/dev", sys->MAFTER);            # LEDs
#    bind("#↓", "/dev", sys->MAFTER);          # power
    bind("#Y", "/dev", sys->MAFTER);            # system information
#    bind("#u", "/n/usb", sys->MAFTER);          # USB peripheral device
    bind("#S", "/n/sd", sys->MAFTER);           # microSD card
    bind("#p", "/prog", sys->MREPL);		# prog device
    bind("#d", "/fd", sys->MREPL);
    bind("#κ", "/dev", sys->MAFTER);        # keyboard (kbd)

    # Start the keyboard daemon
    sh->system(nil, "kbdd &");

    # Start partfs
    sh->system(nil, "mount -c {partfs /n/sd/data} /n/part");

    # Find partition(s) to mount
    part_spec := array of byte find_partition("/n/sd/data");

    if (part_spec != nil) {
        f := sys->open("/n/part/ctl", sys->OWRITE);
        sys->write(f, part_spec, len part_spec);
        sh->system(nil, "mount -c {disk/kfs /n/part/p1} /n/kfs");
    }

# A simple nested loop test:
#    for (i := 1; i <= 12; i++) {
#        for (j := 1; j <= 12; j++)
#            print("%3d ", i * j);
#
#        print("\n");
#    }

# Use the Draw module to draw on the screen.
    draw = load Draw Draw->PATH;
    display := draw->Display.allocate(nil);
    for (b := 0; b < 256; b++) {
        display.image.draw(Rect(Point(b*2, 0), Point(b*2 + 2, 4)), display.rgb(b, 0, 0), display.opaque, Point(0, 0));
        display.image.draw(Rect(Point(b*2, 4), Point(b*2 + 2, 8)), display.rgb(0, b, 0), display.opaque, Point(0, 0));
        display.image.draw(Rect(Point(b*2, 8), Point(b*2 + 2, 12)), display.rgb(0, 0, b), display.opaque, Point(0, 0));
        display.image.draw(Rect(Point(b*2, 12), Point(b*2 + 2, 16)), display.rgb(b, b, b), display.opaque, Point(0, 0));
    }

#    print("Starting a shell...\n");
    shell := load Sh "/dis/sh.dis";
    #args := list of {"sh", "-c", "dd -if /n/sd/data -bs 32 -count 1 | xd"};
    args := list of {"sh"};
    spawn shell->init(context, args);
#    for (;;) {}

# Spawn a background process and collect its output.
#    c := chan of int;
#    spawn background(c);
#    for (;;) {
#        alt {
#            s := <- c =>
#                print("%d\n", s);
#        }
#    }

# Open console device and write something:
#    fd := sys->open("#c", sys->OWRITE);
#    buf := bufio->fopen(fd, sys->OWRITE);
}

#background(c : chan of int)
#{
#    for (i := 0; i < 100; i++)
#        c <-= i;
#}

find_partition(disk_file_name: string): string
{
    disk := Disk.open(disk_file_name, Sys->OREAD, 1);

    # Read the partition table
    sys->seek(disk.fd, big Toffset, 0);
    table := array[TableSize + 2] of byte;
    disk.readn(table, len table);

    if (int table[Omagic] != Magic0 || int table[Omagic+1] != Magic1)
        sys->print("Failed to find partition table\n");

    for (i := 0; i < TableSize; i += TentrySize) {
        dp := PCpart.extract(table[i:], disk);
        if (dp.ptype == Disks->Type9)
            return sys->sprint("part plan9 %bud %bud\n", dp.offset, dp.offset + dp.size);
    }
    return nil;
}
