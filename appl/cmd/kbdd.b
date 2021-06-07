implement KbdDaemon;

include "sys.m";
include "draw.m";

KbdDaemon: module {
    init: fn(nil: ref Draw->Context, argv: list of string);
};

init(nil: ref Draw->Context, argv: list of string)
{
    sys := load Sys Sys->PATH;

    f := sys->open("/dev/kbd", Sys->OREAD);
    b := array[1] of byte;
    for (;;) {
        sys->read(f, b, 1);
    }
}
