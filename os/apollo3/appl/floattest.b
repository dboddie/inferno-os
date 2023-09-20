implement FloatTest;

include "draw.m";
include "sys.m";
sys: Sys;
print: import sys;

FloatTest: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;

#    for (;;) {
#        assign_test();
#        add_test();
#        compare_test();
#        convert_test();
        print_test();
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

print_test()
{
    sys->print("\nPrinting values\n\n");
    sys->print("x := 2.0;\n");
    x := 2.0;
    sys->print("%f\n", x);
    x = 1.0;
    sys->print("%f\n", x);
}
