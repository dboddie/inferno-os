implement MakeColour;

include "draw.m";
include "sys.m";

BUFSIZE: con 160;

MakeColour: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys := load Sys Sys->PATH;
    stdout := sys->fildes(1);

    if (len args != 1) {
        sys->fprint(sys->fildes(2), "usage: makecol\n");
        return;
    }

    f := sys->open("/dev/data", sys->OWRITE);
    if (f == nil) {
        sys->fprint(sys->fildes(2), "can't write data\n");
        return;
    }

    b := array[BUFSIZE] of byte;
    c := 0;

    for (i := 0; i < 8; i++) {
        cb := byte c;
        for (j := 0; j < BUFSIZE; j++)
            b[j] = cb;

        for (j = 0; j < 100; j++)
            if (sys->write(f, b, BUFSIZE) != BUFSIZE) return;

        c = (c + 1) % 7;
    }
}
