#include "../thumb2.h"

/* Only provide a main function to hold test code in. */
void main(void)
{
    /* Assembler format : Vendor format */
    /* MOVF $const, Fd  : VMOV Sd, #imm */
    float x = 0;
    float y = 1.0;

    /* ADDF Fn, Fd      : VADD Sd, Sn, Sm */
    float a = x + y;

    float two = 2.0;
    float three = 3.0;
    float four = 4.0;
    float five = 5.0;
    float point_five = 0.5;
    float ten = 10.0;
    float z = y;

    float eleven = 11.0;
    float twelve = 12.0;

    int *f = (int *)0x20004000;
    int *g = (int *)0x10003000;
    *(int **)0x40008000 = f;

    /* MOVF Fx, offset(Rn) : VSTR Sn, [Rn, #offset] */
    *(float *)0x20001000 = y;
    *(float *)0x20001004 = 1.0;
    float fs[3];
    fs[0] = x;
    float *fp;
    fp = (float *)0x20002000;
    fp[0] = x;
    fp[1] = 1.0;

    /* MOVF offset(Rn), Fx : VLDR Sn, [Rn, #offset] */
    z = fs[0];
    y = *(float *)0x20001000;
    x = z;
    fs[2] = y;
    fs[2] = z;

//    float *f = (float *)0x20002000;
//    float s = 1.0;
//    *f = s;
}
