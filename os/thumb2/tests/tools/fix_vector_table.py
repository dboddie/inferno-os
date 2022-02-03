#!/usr/bin/env python3

import struct, sys

if __name__ == "__main__":

    if len(sys.argv) != 3:
        sys.stderr.write("Usage: %s <binary file containing vector table> <offset after end>\n" % sys.argv[0])
        sys.exit(1)

    end_offset = int(sys.argv[2], 16)

    f = open(sys.argv[1], "r+b")
    f.seek(4, 0)

    while f.tell() < end_offset:
        w = struct.unpack("<I", f.read(4))[0]
        if w != 0:
            f.seek(-4, 1)
            f.write(struct.pack("<I", w | 1))

    sys.exit()
