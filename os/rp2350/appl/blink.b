implement Blink;

include "draw.m";
include "sys.m";
sys: Sys;

Blink: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;

    off := array of byte sys->sprint("%d\n", 0);
    on := array of byte sys->sprint("%d\n", 1);
    states := array[2] of { off, on };
    i := 0;

    for (;;) {
        set_state(states[i]);
        sys->sleep(1000);
        i = 1 - i;
    }
}

set_state(s: array of byte)
{
    fd := sys->open("/dev/leds", sys->OWRITE);
    if (fd != nil)
        sys->write(fd, s, len s);
}
