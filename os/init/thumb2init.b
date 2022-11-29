implement Init;

# Thumb-2

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
    #sh = load Sh "/dis/tiny/sh.dis";

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

    #sys->bind("#c", "/dev", sys->MREPL);
    #sys->bind("#e", "/env", sys->MREPL);
    #sys->bind("#p", "/prog", sys->MREPL);

    #args: list of string;
    #sh->init(context, args);

    spawn func();

    i := 0;
    for (;; i++)
        sys->print("A %d\n", i);
}

func()
{
    for (i := 0; i < 1000; i++)
        sys->print("B %d\n", i);
}
