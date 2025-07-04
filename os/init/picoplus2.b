implement Init;

# RP2350, Cortex-M33, Thumb-2

include "draw.m";
include "sys.m";
include "sh.m";

draw: Draw;
sys: Sys;
sh: Sh;

Init: module
{
    init: fn(nil: ref Draw->Context, nil: list of string);
};

init(context: ref Draw->Context, nil: list of string)
{
    sys = load Sys Sys->PATH;
    sh = load Sh Sh->PATH; # "/dis/tiny/sh.dis";

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");

    sys->bind("#c", "/dev", sys->MAFTER);
    sys->bind("#L", "/dev", sys->MAFTER);   # status LED
    sys->bind("#Y", "/dev", sys->MAFTER);   # system information
    sys->bind("#e", "/env", sys->MREPL | sys->MCREATE);
    sys->bind("#p", "/prog", sys->MREPL);
    sys->bind("#d", "/fd", sys->MREPL);
    sys->bind("#u", "/dev/usb", sys->MAFTER);   # USB device

    fd := sys->open("/dev/sysname", sys->OWRITE);
    b := array of byte "picoplus2";
    sys->write(fd, b, len b);

#    usbinfd := sys->open("/dev/usb/data", sys->OWRITE);
#    sys->dup(usbinfd.fd, 1);
#    sys->dup(1, 2);

    usbfd := sys->open("/dev/usb/data", sys->ORDWR);
    sys->dup(usbfd.fd, 0);
    sys->dup(usbfd.fd, 1);
    sys->dup(usbfd.fd, 2);
    args := "sh"::"-i"::nil;

#    args: list of string;
    sh->init(context, args);
}
