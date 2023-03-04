implement Ink;

include "draw.m";
include "sys.m";

BUFSIZE: con 160;

Ink: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys := load Sys Sys->PATH;
    stdout := sys->fildes(1);

    if (len args != 1) {
        sys->fprint(sys->fildes(2), "usage: ink (stdin) (stdout)\n");
        return;
    }

    args = tl args;

    f := sys->fildes(1);
    if (f == nil) {
        sys->fprint(sys->fildes(2), "can't write data\n");
        return;
    }

    glyphs := array[190] of {
        16r00000000, 16r00000000, # <space>
        16r08080808, 16r00000800, # !
        16r00001414, 16r00000000, # "
        16r123F1200, 16r00123F12, # #
        16r1C023C08, 16r00081E20, # $
        16r08122502, 16r00205224, # %
        16r54081408, 16r005C2222, # &
        16r00000808, 16r00000000, # '
        16r04040408, 16r00000804, # (
        16r08080804, 16r00000408, # )
        16r14083E08, 16r00000000, # *
        16r3E080800, 16r00000808, # +
        16r00000000, 16r04080800, # ,
        16r1E000000, 16r00000000, # -
        16r00000000, 16r000C0C00, # .
        16r04081020, 16r00000102, # /
        16r2D21211E, 16r001E2121, # 0
        16r08080A0C, 16r003E0808, # 1
        16r1820211E, 16r003F0204, # 2
        16r1C20211E, 16r001E2120, # 3
        16r090A0C08, 16r0008083F, # 4
        16r1F01013F, 16r001F2020, # 5
        16r1F01211E, 16r001E2121, # 6
        16r0810203F, 16r00010204, # 7
        16r1E21211E, 16r001E2121, # 8
        16r3E21211E, 16r001E2120, # 9
        16r00080800, 16r00000808, # :
        16r00080800, 16r00040808, # ;
        16r02040810, 16r00100804, # <
        16r003E0000, 16r0000003E, # =
        16r20100804, 16r00040810, # >
        16r1020221C, 16r00080008, # ?
        16r1519110E, 16r001E0119, # @
        16r3F21211E, 16r00212121, # A
        16r1F21211F, 16r001F2121, # B
        16r0101211E, 16r001E2101, # C
        16r2121110F, 16r000F1121, # D
        16r3F01013F, 16r003F0101, # E
        16r3F01013F, 16r00010101, # F
        16r0101211E, 16r001E2139, # G
        16r3F212121, 16r00212121, # H
        16r0808083E, 16r003E0808, # I
        16r0808083F, 16r00060908, # J
        16r07091121, 16r00211109, # K
        16r01010101, 16r003F0101, # L
        16r212D3321, 16r00212121, # M
        16r29252321, 16r00212131, # N
        16r2121211E, 16r001E2121, # O
        16r1F21211F, 16r00010101, # P
        16r2121211E, 16r201E3129, # Q
        16r1F21211F, 16r00211109, # R
        16r1E01211E, 16r001E2120, # S
        16r0808087F, 16r00080808, # T
        16r21212121, 16r001E2121, # U
        16r22224141, 16r00081414, # V
        16r21212121, 16r0021332D, # W
        16r0C122121, 16r00212112, # X
        16r08142241, 16r00080808, # Y
        16r0408103F, 16r003F0102, # Z
        16r0404043C, 16r003C0404, # [
        16r08040201, 16r00002010, # \
        16r1010101E, 16r001E1010, # ]
        16r00221408, 16r00000000, # ^
        16r00000000, 16r003F0000, # _
        16r00000804, 16r00000000, # `
        16r111E0000, 16r002E1111, # a
        16r221E0202, 16r001E2222, # b
        16r221C0000, 16r001C2202, # c
        16r223C2020, 16r003C2222, # d
        16r221C0000, 16r003C023E, # e
        16r1E02221C, 16r00020202, # f
        16r223C0000, 16r1C203C22, # g
        16r221E0202, 16r00222222, # h
        16r08080008, 16r00100808, # i
        16r10100010, 16r0C121010, # j
        16r0A120202, 16r0022120E, # k
        16r08080808, 16r00100808, # l
        16r2A1E0000, 16r002A2A2A, # m
        16r221E0000, 16r00222222, # n
        16r221C0000, 16r001C2222, # o
        16r221E0000, 16r02021E22, # p
        16r223C0000, 16r60203C22, # q
        16r261A0000, 16r00020202, # r
        16r023C0000, 16r001E201C, # s
        16r041E0404, 16r00180404, # t
        16r22220000, 16r001C2222, # u
        16r22220000, 16r00081422, # v
        16r22220000, 16r00142A2A, # w
        16r14220000, 16r00221408, # x
        16r22220000, 16r1C22203C, # y
        16r103E0000, 16r003E0408, # z
        16r04080810, 16r00100808, # {
        16r08080808, 16r00080808, # |
        16r20101008, 16r00081010, # }
        16r00192600, 16r00000000  # ~
        };

    b := array[BUFSIZE] of byte;
    c := 0;
    text := array[40] of byte;
    rest := array[40] of byte;

    for (y := 0; y < 400; y += 16) {

        nr := 0;
        for (; nr < 40; nr++) {
            if (sys->read(sys->fildes(0), text[nr:], 1) <= 0)
                break;
            else if (int text[nr] < 32)
                break;
        }
        while (nr < 40) text[nr++] = byte 32;

        for (dy := 0; dy < 8; dy++) {
            i := 0;

            for (r := 0; r < 2; r++)
            {
                for (col := 0; col < 40; col++)
                {
                    n: int;
                    if (col < len text)
                        n = int text[col];
                    else
                        n = 32;

                    if (n < ' ' || n > '~') n = 32;

                    # Select the 32 bits for the top or bottom half of the glyph.
                    g := glyphs[((n - 32) * 2) + (dy / 4)];

                    # Obtain the bits for the current row and write the pixels.
                    bits := g >> (8 * (dy % 4));

                    for (mask := 1; mask != 16r100; mask <<= 2) {
                        v := byte 16r11;
                        if ((bits & (mask >> 1)) != 0)
                            v |= byte 16r22;

                        b[i++] = v;

                        v = byte 16r11;
                        if ((bits & mask) != 0)
                            v |= byte 16r22;

                        b[i++] = v;
                    }

                    if (i == BUFSIZE) {
                        if (sys->write(f, b, BUFSIZE) != BUFSIZE) return;
                        i = 0;
                    }
                }
            }
        }
    }
}
