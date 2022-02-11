implement Init;

# Thumb-2

include "draw.m";
include "sys.m";

draw: Draw;
sys: Sys;

Init: module
{
    init:	fn(nil: ref Draw->Context, nil: list of string);
};

init(context: ref Draw->Context, nil: list of string)
{
    sys = load Sys Sys->PATH;

    sys->print("**\n** Inferno\n** Vita Nuova\n**\n");
    for (;;) {}
}
