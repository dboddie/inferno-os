#!/usr/bin/env python3

import struct, sys

def encode_vmov_imm(sign, exponent, mantissa, text):

    exp = (exponent + 3)
    aBcd = (sign << 3) | (exp ^ 4)
    efgh = int(16 * mantissa) - 16
    ins = 0x0a00eeb0 | aBcd | (efgh << 16)
    print("0x%08x, /* %s */" % (aBcd | (efgh << 16), text))
    return struct.pack("<I", ins)

if __name__ == "__main__":

    if len(sys.argv) != 2:
        sys.stderr.write("Usage: %s <output file>\n" % sys.argv[0])
        sys.exit(1)

    f = open(sys.argv[1], "wb")
                                    # result = 2**exponent * mantissa
    f.write(encode_vmov_imm(0, 0, 1,      "1 = 2**0 * 1"))
    f.write(encode_vmov_imm(0, 1, 1,      "2 = 2**1 * 1"))
    f.write(encode_vmov_imm(0, 1, 1.5,    "3 = 2**1 * 1.5"))
    f.write(encode_vmov_imm(0, 2, 1,      "4 = 2**2 * 1"))
    f.write(encode_vmov_imm(0, 2, 1.25,   "5 = 2**2 * 1.25"))
    f.write(encode_vmov_imm(0, -1, 1,   "0.5 = 2**-1 * 1"))
    f.write(encode_vmov_imm(0, 3, 1.25,  "10 = 2**3 * 1.25"))
    f.close()

    sys.exit()
