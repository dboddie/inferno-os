implement Init;

# Apollo3, Cortex-M7, Thumb-2

include "draw.m";
include "sys.m";
include "sh.m";

draw: Draw;
sys: Sys;
sh: Sh;

Init: module
{
    init:	fn(nil: ref Draw->Context, nil: list of string);
};

init(context: ref Draw->Context, nil: list of string)
{
    sys = load Sys Sys->PATH;
    sh = load Sh "/dis/tiny/sh.dis";

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

#    sys->bind("#^", "/chan", sys->MBEFORE);
    sys->bind("#c", "/dev", sys->MAFTER);
    sys->bind("#L", "/dev", sys->MAFTER);   # status LED
#    sys->bind("#t", "/dev", sys->MAFTER);
#    sys->bind("#u", "/dev/ink", sys->MAFTER);   # UC8159 e-ink driver
    sys->bind("#Y", "/dev", sys->MAFTER);   # system information
    sys->bind("#e", "/env", sys->MREPL | sys->MCREATE);
    sys->bind("#p", "/prog", sys->MREPL);
    sys->bind("#d", "/fd", sys->MREPL);

    fd := sys->open("/dev/sysname", sys->OWRITE);
    b := array of byte "teensy41mm";
    sys->write(fd, b, len b);

    args: list of string;
    sh->init(context, args);
}
