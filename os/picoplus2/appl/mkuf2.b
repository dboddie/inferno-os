implement mkuf2;

include "draw.m";
include "sys.m";

mkuf2 : module
{
	init: fn(nil: ref Draw->Context, args: list of string);
};

Magic1: con int 16r0A324655;
Magic2: con int 16r9E5D5157;
Magic3: con int 16r0AB16F30;
FamilyId: con int 16rE48BFF59;	# Raspberry Pi, RP2350, secure ARM image
RomAddress: con int 16r10000000;
FamilyIdPresent: con int 16r00002000;


init(nil: ref Draw->Context, args: list of string)
{
	sys := load Sys Sys->PATH;
	stderr := sys->fildes(2);

	if (len args != 3) {
		sys->fprint(stderr, "usage: mkuf2 <kernel> <uf2 file>\n");
		return;
	}

	path := hd tl args;
	uf2path := hd tl tl args;

	(ok, st) := sys->stat(path);
	if (ok < 0) {
		sys->fprint(stderr, "cannot find %s\n", path);
		return;
	}
	total := int st.length / 256;
	if (int st.length % 256 != 0) total++;

	inf := sys->open(path, Sys->OREAD);
	if (inf == nil) {
		sys->fprint(stderr, "cannot open %s\n", path);
		return;
	}

	outf := sys->create(uf2path, Sys->OWRITE, 8r666);
	if (outf == nil) {
		sys->fprint(stderr, "cannot open %s\n", uf2path);
		return;
	}

	addr := RomAddress;

	for (block := 0; block < total; block++) {
		data := array[512 - 32] of byte;
		n := sys->readn(inf, data, 256);
		if (n <= 0) break;
		for (i := n; i < 512 - 32 - 4; i++)
			data[i] = byte 0;

		# Use 256 as the number of bytes, like uf2conv.py.
		header := array[8] of {Magic1, Magic2, FamilyIdPresent, addr, 256, block, total, FamilyId};
		for (i = 0; i < 8; i++) {
			b := bytes(header[i]);
			sys->write(outf, b, 4);
		}
		sys->write(outf, data, 512 - 32 - 4);
		sys->write(outf, bytes(Magic3), 4);
		addr += n;
	}
}

bytes(value: int): array of byte
{
	b := array[4] of byte;
	for (i := 0; i < 4; i++) {
		b[i] = byte(value & 16rFF);
		value >>= 8;
	}
	return b;
}