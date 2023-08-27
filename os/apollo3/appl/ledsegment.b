implement LEDSegment;

include "draw.m";
include "sys.m";
sys: Sys;

DisplayAddress: con 16r70;
# Commands/offsets
EnableClock:   con 16r21;   # System setup (S=1)
SetRowOutput:  con 16ra0;   # ROW/INT set
SetDimming:    con 16re1;   # Dimming set (P3=P2=P1=0, P0=1)
EnableDisplay: con 16r81;   # Display setup (D=1)

LED_digits := array[10] of {16r3f, 16r06, 16rdb, 16rcf, 16re6, 16red, 16rfd, 16r07, 16rff, 16ref};

LEDSegment: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;

    stdin := sys->fildes(0);
    n := read_input(stdin);

    sys->bind("#J70", "/dev", sys->MAFTER);
    write_ctl();

    dfd := sys->open("/dev/i2c.70.data", sys->ORDWR);
    if (dfd == nil) {
        sys->print("could not open data file\n");
        return;
    }

    write_data(dfd, EnableClock, array[0] of byte);
    write_data(dfd, SetRowOutput, array[0] of byte);
    write_data(dfd, SetDimming, array[0] of byte);
    write_data(dfd, EnableDisplay, array[0] of byte);

    sys->sleep(40);

    b := array[4] of {0, 0, 0, 0};
    p := array[4] of {0, 0, 0, 0};
    # Count up from the lowest digit.
    j := 0;
    # Iterate from the end of the input.
    for (i := len n - 1; i >= 0; i--) {
        c := int n[i];
        if (c == '-')
            b[j++] = 16rc0;
        else if ((c >= 48) && (c < 58))
            b[j++] = LED_digits[c - 48];
        else if (c == '.') {
            p[j] = 16r40;
        }
    }

    send_shape_to_device(dfd, DisplayAddress, b[1], p[1], b[0], p[0], b[3], p[3], b[2], p[2]);
    write_data(dfd, EnableDisplay, array[0] of byte);

    sys->unmount("#J70", "/dev");
}

read_input(stdin: ref sys->FD): array of byte
{
    b := array[5] of byte;
    sys->readn(stdin, b, 5);
    return b;
}

write_ctl()
{
    cfd := sys->open("/dev/i2c.70.ctl", sys->OWRITE);
    if (cfd == nil) {
        sys->print("could not open ctl file\n");
        return;
    }
    b := array of byte sys->sprint("subaddress 1\n");
    sys->write(cfd, b, len b);
}

read_data(dfd: ref sys->FD, offset, length: int): array of byte
{
    b := array[length] of byte;
    sys->seek(dfd, big offset, sys->SEEKSTART);
    sys->readn(dfd, b, length);
    return b;
}

write_data(dfd: ref sys->FD, offset: int, b: array of byte)
{
    sys->seek(dfd, big offset, sys->SEEKSTART);
    sys->write(dfd, b, len b);
}

send_shape_to_device(dfd: ref sys->FD, offset, a, b, c, d, e, f, g, h: int)
{
    ba := array[8] of {byte a, byte b, byte c, byte d,
                       byte e, byte f, byte g, byte h};

    sys->seek(dfd, big offset, sys->SEEKSTART);
    sys->write(dfd, ba[:0], 0);

    sys->seek(dfd, big 0, sys->SEEKSTART);
    sys->write(dfd, ba, len ba);
}
