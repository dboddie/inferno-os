implement Init;

#
# Ben NanoNote
#

include "sys.m";
    sys: Sys;
    print, bind: import sys;

include "sh.m";
    sh: Sh;

include "draw.m";
    draw: Draw;
    Context, Display, Image, Point, Rect: import draw;

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
#    bufio = load Bufio Bufio->PATH;

    sh->system(nil, "mount -c {mntgen} /n");
	
#    print("**\n** Inferno\n** Vita Nuova\n**\n");

#    bind("#c", "/dev", sys->MAFTER);
#    bind("#r", "/dev", sys->MAFTER);		# RTC

    #
    # default namespace
    #
    bind("#c", "/dev", sys->MREPL);		# console
#   bind("#t", "/dev", sys->MAFTER);		# serial port
#    bind("#B", "/dev", sys->MAFTER);            # backlight
#    bind("#L", "/dev", sys->MAFTER);            # LEDs
    bind("#↓", "/dev", sys->MAFTER);          # power
    bind("#Y", "/dev", sys->MAFTER);             # system information
    bind("#u", "/dev", sys->MAFTER);             # USB peripheral device
    bind("#S", "/n/sd", sys->MAFTER);           # microSD card
    bind("#p", "/prog", sys->MREPL);		# prog device
    bind("#d", "/fd", sys->MREPL);

# A simple nested loop test:
#    for (i := 1; i <= 12; i++) {
#        for (j := 1; j <= 12; j++)
#            print("%3d ", i * j);
#
#        print("\n");
#    }

# Use the Draw module to draw on the screen.
#    draw = load Draw Draw->PATH;
#    display := draw->Display.allocate(nil);
#    for (b := 0; b < 256; b++) {
#        display.image.draw(Rect(Point(b, 0), Point(b + 1, 4)), display.rgb(b, 0, 0), display.opaque, Point(0, 0));
#        display.image.draw(Rect(Point(b, 4), Point(b + 1, 8)), display.rgb(0, b, 0), display.opaque, Point(0, 0));
#        display.image.draw(Rect(Point(b, 8), Point(b + 1, 12)), display.rgb(0, 0, b), display.opaque, Point(0, 0));
#        display.image.draw(Rect(Point(b, 12), Point(b + 1, 16)), display.rgb(b, b, b), display.opaque, Point(0, 0));
#    }

#    print("Starting a shell...\n");
    shell := load Sh "/dis/sh.dis";
    args := list of {"sh"};
    spawn shell->init(context, args);

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
