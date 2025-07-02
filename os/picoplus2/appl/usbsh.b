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
    sh := load Sh Sh->PATH;

    fd := sys->open("/dev/usb/data", sys->ORDWR);

    sys->dup(fd.fd, 0);
    sys->dup(fd.fd, 1);
    sys->dup(fd.fd, 2);

    sh->init(context, args);
}
