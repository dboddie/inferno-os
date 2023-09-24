implement FloatTest;

include "draw.m";
include "math.m";
include "sys.m";
math: Math;
sys: Sys;
print, sprint: import sys;

FloatTest: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;
    math = load Math Math->PATH;

#    for (;;) {
#        assign_test();
#        add_test();
#        compare_test();
#        convert_test();
#        multiply_test();
#        divide_test();
#        print_test();
        math_test();
#    }

    print("Finished\n");
}

assign_test()
{
    print("\nAssignments\n\n");
    print("x := 2.0;\n");
    x := 2.0;
}

add_test()
{
    print("\nAdditions\n\n");
    print("x := 2.0;\n");
    x := 2.0;
    print("a := x + 3.0;\n");
    a := x + 3.0;

    if (a == 5.0)
        print("a == 5.0\n");
    else
        print("FAIL: a != 5.0\n");
}

compare_test()
{
    print("\nComparisons\n\n");
    print("x := 2.0;\n");
    x := 2.0;

    if (x == 2.0)
        print("x == 2.0\n");
    else
        print("FAIL: x != 2.0\n");

    y := 3.0;
    print("y := 3.0;\n");

    if (x == y)
        print("FAIL: x == y\n");
    else
        print("x != y\n");

    if (x < y)
        print("x < y\n");
    else
        print("FAIL: x >= y\n");

    if (x <= y)
        print("x <= y\n");
    else
        print("FAIL: x > y\n");

    if (x > y)
        print("FAIL: x > y\n");
    else
        print("x <= y\n");

    if (x >= y)
        print("FAIL: x >= y\n");
    else
        print("x < y\n");
}

convert_test()
{
    print("\nConversions\n\n");
    print("x := 2.0;\n");
    x := 2.0;

    print("i := big x;\n");
    i := big x;
    if (i == big 2)
        print("i == 2\n");
    else
        print("FAIL: i != 2\n");

    print("j := int x;\n");
    j := int x;
    if (j == 2)
        print("j == 2\n");
    else
        print("FAIL: j != 2\n");

    i = big 2;
    print("i = %bd\n", i);
    print("y := real i;\n");
    y := real i;

    if (y == 2.0)
        print("y == 2.0\n");
    else
        print("FAIL: y != 2.0\n");

    j = -2;
    print("j = %d\n", j);
    print("y := real j;\n");
    y = real j;

    if (y == -2.0)
        print("y == -2.0\n");
    else
        print("FAIL: y != -2.0\n");
}

multiply_test()
{
    a := 2.0;
    b := -3.0;
    c := a * b;

    if (c == -6.0)
        print("c == -6.0\n");
    else
        print("FAIL: c != -6.0\n");
}

divide_test()
{
    a := 3.0;
    b := -2.0;
    c := a / b;

    if (c == -1.5)
        print("c == -1.5\n");
    else
        print("FAIL: c != -1.5\n");
}

print_test()
{
    print("\nPrinting values\n\n");
    x := 2.0;
    print("x := 2.0;\n");
    print("%f\n", x);
    print("%g\n", x);
    x = 1.5;
    print("x = 1.5;\n");
    print("%f\n", x);
    print("%g\n", x);
    x = -2.0;
    print("x = -2.0;\n");
    print("%f\n", x);
    print("%g\n", x);
}

math_test()
{
    print("\nmath module tests\n\n");
    x := 2.0;
    y := math->sqrt(x);
    print("x := %f\n", x);
    print("y := math->sqrt(x);\n");

    s := sprint("%f", y);
    if (s == "1.414214")
        print("y == 1.414214\n");
    else
        print("FAIL: y != 1.414214 (%s)\n", s);

    l := math->log(2.0);
    print("l := log(2.0);\n");
    s = sprint("%f", l);
    if (s == "0.693147")
        print("l == 0.693147\n");
    else
        print("FAIL: l != 0.693147 (%s)\n", s);
    v := math->exp(l);
    print("v := exp(log(2.0));\n");
    print("v = %f\n", v);

    a := math->exp(2.0);
    print("a := exp(2.0);\n");
    s = sprint("%f", a);
    if (s == "7.389056")
        print("a == 7.389056\n");
    else
        print("FAIL: a != 7.389056 (%s)\n", s);
}
