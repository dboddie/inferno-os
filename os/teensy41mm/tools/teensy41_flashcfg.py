#!/usr/bin/env python3

import os, struct, sys

start_addr = 0x60000000
image_vector_table_addr = start_addr + 0x1000
bootdata_addr = image_vector_table_addr + 0x20
csf_addr = 0
program_start = bootdata_addr + 0x10


class Text:
    def __init__(self, b): self.b = b
    def data(self): return self.b

class Num:
    def __init__(self, n, l):
        self.n = n
        self.l = l
    def data(self):
        l = []
        v = self.n
        for i in range(self.l):
            l.append(v & 0xff)
            v = v >> 8
        return bytes(l)


# See 9.6.3.1 (FlexSPI Configuration Block) in the i.MX RT1060 Processor
# Reference Manual, revision 3.
flexspi_cfg = [
    Text(b"FCFB"),          # Tag
    Num(0x56010000, 4),     # Version
    Num(0, 4),
    # readSampleClkSrc, csHoldTime, csSetupTime, columnAddressWidth
    Num(1, 1), Num(3, 1), Num(3, 1), Num(0, 1),

    # deviceModeCfgEnable, reserved, waitTimeCfgCommands
    Num(0, 1), Num(0, 1), Num(0, 2),
    Num(0, 4),              # deviceModeSeq
    Num(0, 4),              # deviceModeArg
    Num(0, 1), Num(0, 3),   # configCmdEnable, reserved

    Num(0, 4),              # configCmdSeqs
    Num(0, 4),              # configCmdSeqs
    Num(0, 4),              # configCmdSeqs
    Num(0, 4),

    Num(0, 4),              # configCmdArgs
    Num(0, 4),              # configCmdArgs
    Num(0, 4),              # configCmdArgs
    Num(0, 4),

    Num(0, 4),              # controllerMiscOption
    # deviceType, sflashPadType, serialClkFreq, lutCustomSeqEnable
    Num(1, 1), Num(4, 1), Num(8, 1), Num(0, 1),
    Num(0, 4),
    Num(0, 4),

    Num(0x01000000, 4),     # sflashA1Size
    Num(0, 4),              # sflashA2Size
    Num(0, 4),              # sflashB1Size
    Num(0, 4),              # sflashB2Size

    Num(0, 4),              # csPadSettingOverride
    Num(0, 4),              # sclkPadSettingOverride
    Num(0, 4),              # dataPadSettingOverride
    Num(0, 4),              # dqsPadSettingOverride

    Num(0, 4),              # timeoutInMs
    Num(0, 4),              # commandInterval
    Num(0, 4),              # dataValidTime
    Num(0, 2), Num(0, 2),   # busyOffset, busyBitPolarity

    # lookupTable
    Num(0x0A1804EB, 4),
    Num(0x26043206, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x24040405, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x00000406, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x08180420, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x081804D8, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x08180402, 4),
    Num(0x00002004, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0x00000460, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    # lutCustomSeq
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    # Reserved
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),
    Num(0, 4),

    # See 9.6.3.2 (Serial NOR configuration block) in the i.MX RT1060 Processor
    # Reference Manual, revision 3.

    Num(256, 4),            # pageSize
    Num(4096, 4),           # sectorSize
    Num(1, 1), Num(0, 3),   # ipCmdSerialClkFreq
    Num(0, 4),              # reserved

    Num(0x00010000, 4),     # block size
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved

    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved

    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
    Num(0, 4),              # reserved
]

if __name__ == "__main__":

    if len(sys.argv) != 3:
        sys.stderr.write("usage: %s <program binary file> <output file>\n" % sys.argv[0])
        sys.exit(1)

    p = open(sys.argv[1], "rb")
    pd = p.read()
    p.close()

    f = open(sys.argv[2], "wb")
    for obj in flexspi_cfg:
        f.write(obj.data())

    while f.tell() < 4096:
        f.write(b"\xff\xff\xff\xff")

    # Read the reset vector address from the program code.
    entry_addr = struct.unpack("<I", pd[4:8])[0]

    # The image vector table must be 4096 bytes into the ROM image.
    # See 9.7.1 (Image Vector Table and Boot Data).
    image_vector_table = [
        Num(0xd1, 1), Num(0x2000, 2), Num(0x41, 1), # tag, length, version
        Num(entry_addr, 4),     # entry: address of first instruction to execute
        Num(0, 4),
        Num(0, 4),                          # dcd (optional, so left as null)
        Num(bootdata_addr, 4),
        Num(image_vector_table_addr, 4),    # self
        Num(csf_addr, 4),       # command sequence file
        Num(0, 4),
    ]

    for obj in image_vector_table:
        f.write(obj.data())

    image_length = program_start + len(pd) - start_addr

    boot_data = [
        Num(start_addr, 4),     # absolute address of the image
        Num(image_length, 4),   # length of the image
        Num(0, 4),              # plugin flag
        Num(0, 4),              # (padding)
    ]

    for obj in boot_data:
        f.write(obj.data())

    i = 0
    while i < 0x50:
        f.write(b"\xff\xff\xff\xff")
        i += 4

    f.write(pd)

    f.close()
    sys.exit()
