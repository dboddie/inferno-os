#!/usr/bin/env python3

import sys

cfcr_bits = [
    ("SCS", 31, 1),
    ("LCS", 30, 1),
    ("I2CS", 29, 1),
    ("UCS", 28, 1),
    ("UFR", 25, 7),
    ("MCS", 24, 1),
    ("SCLKOEN", 22, 1),
    ("UPE", 20, 1),
    ("MFR", 16, 0xf),
    ("LFR", 12, 0xf),
    ("PFR", 8, 0xf),
    ("HFR", 4, 0xf),
    ("CFR", 0, 0xf)
    ]

plcr_bits = [
    ("PLLFD", 23, 0x1ff),
    ("PLLRD", 18, 0x1f),
    ("PLLOD", 16, 3),
    ("PLLS", 10, 1),
    ("PLLBP", 9, 1),
    ("PLLEN", 8, 1),
    ("PLLST", 0, 0xff)
    ]

if __name__ == "__main__":

    if len(sys.argv) != 3:
        sys.stderr.write("Usage: %s <CFCR value> <PLCR value>\n" % sys.argv[0])
        sys.exit(1)

    cfcr = int(sys.argv[1], 16)
    plcr = int(sys.argv[2], 16)

    regs = {}

    for rname, value, bits in ("CFCR", cfcr, cfcr_bits), ("PLCR", plcr, plcr_bits):

        print("%s %08x:" % (rname, value))
        d = {}

        for name, shift, mask in bits:
            v = (value >> shift) & mask
            d[name] = v
            print(" ", shift, name, v)

        regs[rname] = d

    EXCLK = 3686400
    od = [1, 2, 2, 4]

    if regs["CFCR"]["SCS"] == 0:
        PLL_freq = (EXCLK * (regs["PLCR"]["PLLFD"] + 2) /
                            (regs["PLCR"]["PLLRD"] + 2)) / od[regs["PLCR"]["PLLOD"]]
        print("PLL frequency", PLL_freq)
