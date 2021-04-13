#include "mem.h"

void main(void)
{
    // Cases 7 and 8
    unsigned long long i = 0x123456789abcdef0LL;
    unsigned long *j = (unsigned long *)(&i);
    unsigned long k = j[0];
    unsigned long l = j[1];
//    fbprint(j[0], 0);
//    fbprint(j[1], 0);

//    int j = 0x2468ace0;

    // Case 3
//    unsigned long long k = 1;

    // Cases 48 and 1
//    float f = 1.0;
//    unsigned long long i = f;
}
