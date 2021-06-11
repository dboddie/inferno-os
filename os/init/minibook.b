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

bindsd() {
    sys->bind("/n/local/sd/dis", "/dis", Sys->MREPL);
    sys->bind("/n/local/sd/lib", "/lib", Sys->MREPL);
    sys->bind("/n/local/sd/usr", "/usr", Sys->MREPL);
    sys->bind("/n/local/sd/man", "/man", sys->MREPL);
    sys->bind("/n/local/sd/fonts", "/fonts", sys->MREPL);
    sys->bind("/n/local/sd/icons", "/icons", sys->MREPL);
    sys->bind("/n/local/sd/module", "/module", sys->MREPL);
    sys->bind("/n/local/sd/locale", "/locale", sys->MREPL);
    sys->bind("/n/local/sd/services", "/services", sys->MREPL);
    sys->bind("/n/local/sd/tmp", "/tmp", sys->MREPL|sys->MCREATE);
}

init(context: ref Context, nil: list of string)
{
    sys = load Sys Sys->PATH;
    sh = load Sh Sh->PATH;
    disks = load Disks Disks->PATH;
    disks->init();

    print("Ported to the Letux 400 Minibook by David Boddie\n\n");

    sh->system(nil, "mount -c {mntgen} /n");
    sh->system(nil, "mount -c {mntgen} /n/local");
    bind("#S", "/dev", sys->MAFTER);            # microSD card
	
    sh->system(nil, "disk/fdisk -p /dev/sdM0/data > /dev/sdM0/ctl");
    sh->system(nil, "mount -c {disk/kfs -c -A -n main /dev/sdM0/plan9} /n/local/sd");
    sh->system(nil, "disk/kfscmd allow");

    bindsd();

    bind("#c", "/dev", sys->MAFTER);		# console
    bind("#B", "/dev", sys->MAFTER);            # backlight
#    bind("#↓", "/dev", sys->MAFTER);          # power
    bind("#Y", "/dev", sys->MAFTER);            # system information
#    bind("#u", "/n/usb", sys->MAFTER);          # USB subsystem
    bind("#p", "/prog", sys->MREPL);		# prog device
    bind("#d", "/fd", sys->MREPL);
    bind("#κ", "/dev", sys->MAFTER);        # keyboard (kbd)

    # Start the keyboard daemon
    sh->system(nil, "kbdd &");

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
    args := list of {"sh"};
    spawn shell->init(context, args);
}

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
