implement UsbSh;

include "draw.m";
include "sys.m";
include "sh.m";

UsbSh: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(context: ref Draw->Context, args: list of string)
{
    sys := load Sys Sys->PATH;
    sh := load Sh "/dis/tiny/sh.dis";

    fd := sys->open("/dev/usb/data", sys->ORDWR);

#    sys->dup(infd.fd, 0);
#    sys->dup(outfd.fd, 1);

#    sh->init(context, args);

    for (;;) {
        b := array[64] of byte;
        n := sys->read(fd, b, 1);
        sys->print("%s", string b[:n]);
#        if (n > 0) {
#            sys->write(fd, b, 1);
#        }
    }
}
