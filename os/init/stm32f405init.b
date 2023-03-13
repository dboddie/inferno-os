implement Init;

# STM32F405, Cortex-M4, Thumb-2

include "draw.m";
include "sys.m";
include "sh.m";
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
#    rd := load Readdir Readdir->PATH;
    sh = load Sh "/dis/tiny/sh.dis";

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

#    sys->bind("#^", "/chan", sys->MBEFORE);
    sys->bind("#c", "/dev", sys->MAFTER);
    sys->bind("#L", "/dev", sys->MAFTER);   # status LED
#    sys->bind("#t", "/dev", sys->MAFTER);
    sys->bind("#u", "/dev/ink", sys->MAFTER);   # UC8159 e-ink driver
    sys->bind("#F", "/dev/flash", sys->MAFTER); # W25Q128 flash driver
    sys->bind("#e", "/env", sys->MREPL | sys->MCREATE);
    sys->bind("#p", "/prog", sys->MREPL);
    sys->bind("#d", "/fd", sys->MREPL);

#    (l, d) := sys->stat("/dis");
#    sys->print("%s %s %s %d\n", d.name, d.uid, d.gid, l);

#    (l, d) = sys->stat("/invalid");
#    sys->print("%s %s %s %d\n", d.name, d.uid, d.gid, l);

#    fd := sys->open("/dev", Sys->OREAD);
#    if (fd == nil) {
#        sys->print("error reading /dev\n");
#        return;
#    }

#    sys->print("fd=%d\n", fd.fd);

#    (nr, b) := sys->dirread(fd);
#    sys->print("%d\n", nr);

#    a := array[128] of byte;
#    stdout := sys->fildes(1);
#
#    for (;;) {
#        n := sys->readn(fd, a, 128);
#        if (n < 0)
#            sys->print("%r");
#        else if (n == 0)
#            break;
#        else
#            sys->write(stdout, a, n);
#    }

#    (b, nr) := rd->init("/dis", Readdir->COMPACT);
#    sys->print("%d\n", nr);
#    sys->print("%r");

#    for (i := 0; i < nr; i++)
#        sys->print("%s\n", b[i].name);

    fd := sys->open("/dev/sysname", sys->OWRITE);
    b := array of byte "stm32f405";
    sys->write(fd, b, len b);

    args: list of string;
    sh->init(context, args);

#    spawn func();

#    for (i := 0; i < 20; i++)
#        sys->print("A %d\n", i);
#    for (;;) {}
}

#func()
#{
#    for (i := 0; i < 20; i++)
#        sys->print("B %d\n", i);
#}
