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
#    sh = load Sh Sh->PATH;

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

#    sys->bind("#c", "/dev", sys->MREPL);

#    shell := load Sh "/dis/sh.dis";
#    args := list of {"sh"};
#    spawn shell->init(context, args);

#    spawn func();

#    for (i := 0; i < 1000; i++)
#        sys->print("A %d\n", i);
    for (;;) {}
}

func()
{
    for (i := 0; i < 1000; i++)
        sys->print("B %d\n", i);
}
