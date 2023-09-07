implement FloatTest;

include "draw.m";
include "sys.m";
sys: Sys;

FloatTest: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;
    x := 2.0;
#    sys->print("%f\n", x);
}
