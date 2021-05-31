#!/usr/bin/env python3

import sys

bits = [
    ("BD", 31, 1),
    ("TI", 30, 1),
    ("CE", 28, 3),
    ("DC", 27, 1),
    ("PCI", 26, 1),
    ("IV", 23, 1),
    ("WP", 22, 1),
    ("IP7-2", 10, 0x1f),
    ("IP1-0", 8, 3),
    ("ExcCode", 2, 0x1f)
    ]

excodes = {
    0: "Int - interrupt",
    1: "Mod - TLB page marked read-only",
    2: "TLBL - no matching entry",
    3: "TLBS - miss",
    4: "AdEL - address error on instruction fetch",
    5: "AdES - address error on store",
    6: "IBE - bus error on instruction fetch",
    7: "DBE - bus error on data read",
    8: "Syscall - syscall instruction",
    9: "Bp - break instruction",
    10: "RI - instruction code not recognised",
    11: "CpU - co-processor not enabled",
    12: "Ov - integer arithmetic overflow",
    13: "TRAP - conditional trap",
    14: "Unused - possible cache alias",
    15: "FPE - floating point exception",
    16: "Custom",
    17: "Custom",
    18: "C2E",
    19: "Reserved",
    20: "Reserved",
    21: "Reserved",
    22: "MDMX",
    23: "Watch",
    24: "MCheck - machine check",
    25: "Thread - thread-related exception",
    26: "DSP - DSP instructions not supported or enabled",
    27: "Reserved",
    28: "Reserved",
    29: "Reserved",
    30: "CacheErr",
    31: "Unused"
    }


if __name__ == "__main__":

    if len(sys.argv) != 2:
        sys.stderr.write("Usage: %s <cause value>\n" % sys.argv[0])
        sys.exit(1)

    cause = int(sys.argv[1], 16)

    print("Cause %08x:" % cause)

    d = {}

    for name, shift, mask in bits:
        v = (cause >> shift) & mask
        d[name] = v
        if v != 0:
            print(" ", name, v)

    description = excodes[d["ExcCode"]]
    print("Exception:", description)
