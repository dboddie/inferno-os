# mandel.b
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

    render(-0.5, 0.0, 3.2, 2.4, 16);
}

pix := array[480] of byte;

render(ox, oy, xlength, ylength: real, iterations: int)
{
    x1 := y1 := 0;
    x2 := w := 320;
    y2 := h := 240;

    xscale, yscale: real;

    xscale = xlength/(real w);
    yscale = ylength/(real h);

    w2 := w/2;
    h2 := h/2;

    x := x1;
    while (x < x2) {

        r := ox + (real (x - w2) * xscale);

        y := y1;
        while (y < y2) {

            ti := i := oy + (real (h2 - y) * yscale);
            tr := r;

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
                draw(-1, y);

            else if (count < iterations) {
                c := count - low;
                draw(c, y);
            } else
                draw(0, y);

            y += 1;
        }

        sys->write(sys->fildes(1), pix, 480);
        x += 1;
    }
}

draw(value: int, y: int)
{
    # Pixel format is RGB 565 with blue in the lowest 5 bits.
    # High byte sent first.
    low := (y * 2) + 1;
    high := y * 2;

    if (value < 0) {
        pix[low] = byte 16r1f;
        pix[high] = byte 0;
    } else if (value == 0)
        pix[low] = pix[high] = byte 0;
    else {
        g: int;
        if (value <= 7)
            g = value * 9;
        else
            g = (15 - value) * 9;

        rgb := ((value * 2) << 11) | (g << 5) | (31 - (value * 2));
        pix[high] = byte (rgb >> 8);
        pix[low] = byte (rgb & 16rff);
    }
}
