implement AHT;

include "draw.m";
include "sys.m";
sys: Sys;

AHT: module
{
    init:   fn(nil: ref Draw->Context, args: list of string);
};

init(nil: ref Draw->Context, args: list of string)
{
    sys = load Sys Sys->PATH;

    sys->bind("#J38", "/dev", sys->MAFTER);

    write_ctl();

    dfd := sys->open("/dev/i2c.38.data", sys->ORDWR);
    if (dfd == nil) {
        sys->print("could not open data file\n");
        return;
    }

    b := read_data(dfd, 16r71, 1);
    status := int b[0];
    if ((status & 16r88) == 16r08) {
        wb := array[] of { byte 16r33, byte 16r00 };
        write_data(dfd, 16rac, wb);

        sys->sleep(80);

        b = read_data(dfd, 16r71, 7);
        # SS H2 H1 ht T2 T1 CC
        humidity := (int b[1] << 12) | (int b[2] << 4) | (int b[3] >> 4);
        # The t half-byte is the top four bits of the temperature.
        temperature := ((int b[3] & 16r0f) << 16) | (int b[4] << 8) | int b[5];
        # Using the formula in the datasheet to calculate the temperature, but
        # first calculate the whole part of it, then calculate the temperature in
        # tenths of a degree.
        t_whole := ((temperature * 200) / 16r100000) - 50;
        t_frac := ((temperature * 200) / 16r19999) % 10;
        sys->print("%d.%d\n", t_whole, t_frac);
    }

    sys->unmount("#J38", "/dev");
}

write_ctl()
{
    cfd := sys->open("/dev/i2c.38.ctl", sys->OWRITE);
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
