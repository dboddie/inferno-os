implement Init;

# Thumb-2

include "draw.m";
include "sys.m";
#include "sh.m";
#include "env.m";
#include "readdir.m";

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
    #rd := load Readdir Readdir->PATH;
    #sh = load Sh "/dis/tiny/sh.dis";

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

    #(a, l) := rd->init("/", Readdir->COMPACT);
    #sys->print("%d\n", l);

    #for (i := 0; i < l; i++)
    #    sys->print("%s\n", a[i].name);

    #sys->bind("#c", "/dev", sys->MREPL);
    #sys->bind("#e", "/env", sys->MREPL);
    #sys->bind("#p", "/prog", sys->MREPL);

    #args: list of string;
    #sh->init(context, args);

    spawn func();

    for (i := 0; i < 100; i++)
        sys->print("A %d\n", i);
#    for (;;) {}
}

func()
{
    for (i := 0; i < 100; i++)
        sys->print("B %d\n", i);
}
