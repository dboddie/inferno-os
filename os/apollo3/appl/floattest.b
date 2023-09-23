implement FloatTest;

include "draw.m";
include "math.m";
include "sys.m";
math: Math;
sys: Sys;
print: import sys;

FloatTest: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;
    math = load Math Math->PATH;

#    for (;;) {
        assign_test();
        add_test();
        compare_test();
        convert_test();
        multiply_test();
        divide_test();
        print_test();
        math_test();
#    }

    sys->print("Finished\n");
}

assign_test()
{
    sys->print("\nAssignments\n\n");
    sys->print("x := 2.0;\n");
    x := 2.0;
}

add_test()
{
    sys->print("\nAdditions\n\n");
    sys->print("x := 2.0;\n");
    x := 2.0;
    sys->print("a := x + 3.0;\n");
    a := x + 3.0;

    if (a == 5.0)
        sys->print("a == 5.0\n");
    else
        sys->print("FAIL: a != 5.0\n");
}

compare_test()
{
    sys->print("\nComparisons\n\n");
    sys->print("x := 2.0;\n");
    x := 2.0;

    if (x == 2.0)
        sys->print("x == 2.0\n");
    else
        sys->print("FAIL: x != 2.0\n");

    y := 3.0;
    sys->print("y := 3.0;\n");

    if (x == y)
        sys->print("FAIL: x == y\n");
    else
        sys->print("x != y\n");

    if (x < y)
        sys->print("x < y\n");
    else
        sys->print("FAIL: x >= y\n");

    if (x <= y)
        sys->print("x <= y\n");
    else
        sys->print("FAIL: x > y\n");

    if (x > y)
        sys->print("FAIL: x > y\n");
    else
        sys->print("x <= y\n");

    if (x >= y)
        sys->print("FAIL: x >= y\n");
    else
        sys->print("x < y\n");
}

convert_test()
{
    sys->print("\nConversions\n\n");
    sys->print("x := 2.0;\n");
    x := 2.0;

    sys->print("i := big x;\n");
    i := big x;
    if (i == big 2)
        sys->print("i == 2\n");
    else
        sys->print("FAIL: i != 2\n");

    sys->print("j := int x;\n");
    j := int x;
    if (j == 2)
        sys->print("j == 2\n");
    else
        sys->print("FAIL: j != 2\n");

    i = big 2;
    sys->print("i = %bd\n", i);
    sys->print("y := real i;\n");
    y := real i;

    if (y == 2.0)
        sys->print("y == 2.0\n");
    else
        sys->print("FAIL: y != 2.0\n");

    j = -2;
    sys->print("j = %d\n", j);
    sys->print("y := real j;\n");
    y = real j;

    if (y == -2.0)
        sys->print("y == -2.0\n");
    else
        sys->print("FAIL: y != -2.0\n");
}

multiply_test()
{
    a := 2.0;
    b := -3.0;
    c := a * b;

    if (c == -6.0)
        sys->print("c == -6.0\n");
    else
        sys->print("FAIL: c != -6.0\n");
}

divide_test()
{
    a := 3.0;
    b := -2.0;
    c := a / b;

    if (c == -1.5)
        sys->print("c == -1.5\n");
    else
        sys->print("FAIL: c != -1.5\n");
}

print_test()
{
    sys->print("\nPrinting values\n\n");
    x := 2.0;
    sys->print("x := 2.0;\n");
    sys->print("%f\n", x);
    sys->print("%g\n", x);
    x = 1.5;
    sys->print("x = 1.5;\n");
    sys->print("%f\n", x);
    sys->print("%g\n", x);
    x = -2.0;
    sys->print("x = -2.0;\n");
    sys->print("%f\n", x);
    sys->print("%g\n", x);
}

math_test()
{
    x := 2.0;
    y := math->sqrt(x);
    sys->print("x := %f\n", x);
    sys->print("y := %f\n", y);
}
