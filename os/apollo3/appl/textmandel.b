# textmandel.b
#
# Written in 2023 by David Boddie <david@boddie.org.uk>
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along with
# this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

implement Mandelbrot;

include "sys.m";
    sys: Sys;
include "draw.m";
include "math.m";
    math: Math;

Mandelbrot: module
{
    init: fn(ctx: ref Draw->Context, args: list of string);
};

displayed := 0;

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;
    math = load Math Math->PATH;

    render(-0.5, 0.0, 3.0, 16);
}

render(ox, oy, length: real, iterations: int)
{
    x1 := y1 := 0;
    x2 := w := 80;
    y2 := h := 40;

    xscale, yscale: real;

    xscale = length/(real w);
    yscale = length/(real h);

    w2 := w/2;
    h2 := h/2;

    y := y1;
    while (y < y2) {

        i := oy + (real (h2 - y) * yscale);

        x := x1;
        while (x < x2) {

            tr := r := ox + (real (x - w2) * xscale);
            ti := i;

            count := 0;

            while (count < iterations) {
                temp := (tr*tr) - (ti*ti) + r;
                ti = (2.0*tr*ti) + i;
                tr = temp;

                r2 := (tr*tr) + (ti*ti);
                if (r2 > 4.0)
                    break;

                count += 1;
            }

            low := iterations - 16;
            if (count <= low)
                draw(-1);

            else if (count < iterations) {
                c := count - low;
                draw(c);
            } else
                draw(0);

            x += 1;
        }
        sys->print("\n");

        y += 1;
    }
}

esc := array[1] of {byte 16r1b};
chars := "0123456789abcdef";

draw(value: int)
{
    if (value < 0) {
        sys->print(".");
    } else if (value == 0)
        sys->print(" ");
    else {
        b := array[1] of byte;
        b[0] = byte chars[value];
        sys->write(sys->fildes(1), b, 1);
    }
}
