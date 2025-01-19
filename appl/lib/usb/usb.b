implement Usb;

include "sys.m";
	sys: Sys;
include "usb.m";
include "string.m";
	str: String;

init()
{
	sys = load Sys Sys->PATH;
	str = load String String->PATH;
}

dprint(n: int, s: string)
{
	if(usbdebug)
		sys->fprint(sys->fildes(n), "%s: %s", argv0, s);
}

get2(b: array of byte): int
{
	return int b[0] | (int b[1] << 8);
}

put2(buf: array of byte, v: int)
{
	buf[0] = byte v;
	buf[1] = byte (v >> 8);
}

get4(b: array of byte): int
{
	return int b[0] | (int b[1] << 8) | (int b[2] << 16) | (int b[3] << 24);
}

put4(buf: array of byte, v: int)
{
	buf[0] = byte v;
	buf[1] = byte (v >> 8);
	buf[2] = byte (v >> 16);
	buf[3] = byte (v >> 24);
}

bigget2(b: array of byte): int
{
	return int b[1] | (int b[0] << 8);
}

bigput2(buf: array of byte, v: int)
{
	buf[1] = byte v;
	buf[0] = byte (v >> 8);
}

bigget4(b: array of byte): int
{
	return int b[3] | (int b[2] << 8) | (int b[1] << 16) | (int b[0] << 24);
}

bigput4(buf: array of byte, v: int)
{
	buf[3] = byte v;
	buf[2] = byte (v >> 8);
	buf[1] = byte (v >> 16);
	buf[0] = byte (v >> 24);
}

#
# epN.M -> N
#
nameid(s: string): int
{
	(nil, r) := str->splitr(s, "p");
	if(r == nil)
		return -1;
	(n, nil) := str->toint(r, 0);
	return n;
}

openep(d: ref Dev, id: int): ref Dev
{
	if(d.cfd == nil || d.usb == nil){
		sys->werrstr("device not configured");
		return nil;
	}
	ud := d.usb;
	if(id < 0 || id >= len ud.ep || ud.ep[id] == nil) {
		sys->werrstr("bad enpoint number");
		return nil;
	}
	ep := ud.ep[id];
	mode := "rw";
	if(ep.dir == Ein)
		mode = "r";
	if(ep.dir == Eout)
		mode = "w";
	name := sys->sprint("/dev/usb/ep%d.%d", d.id, id);
	if(devctl(d, sys->sprint("new %d %d %s", id, ep.etype, mode)) < 0){
		if(usbdebug)
			dprint(2, sys->sprint("%s: new: %r\n", d.dir));
		return nil;
	}
	epd := opendev(name);
	if(epd == nil)
		return nil;
	epd.id = id;
	if(devctl(epd, sys->sprint("maxpkt %d", ep.maxpkt)) < 0)
		sys->fprint(sys->fildes(2), "%s: %s: openep: maxpkt: %r\n",
			argv0, epd.dir);
	else if(usbdebug)
		dprint(2, sys->sprint("%s: maxpkt %d\n", epd.dir, ep.maxpkt));
	epd.maxpkt = ep.maxpkt;
	ac := ep.iface.altc[0];
	if(ep.ntds > 1 && devctl(epd, sys->sprint("ntds %d", ep.ntds)) < 0)
		sys->fprint(sys->fildes(2), "%s: %s: openep: ntds: %r\n",
			argv0, epd.dir);
	else if(usbdebug)
		dprint(2, sys->sprint("%s: ntds %d\n", epd.dir, ep.ntds));

	#
	# For iso endpoints and high speed interrupt endpoints the pollival is
	# actually 2ⁿ and not n.
	# The kernel usb driver must take that into account.
	# It's simpler this way.
	#

	if(ac != nil && (ep.etype == Eintr || ep.etype == Eiso) && ac.interval != 0)
		if(devctl(epd, sys->sprint("pollival %d", ac.interval)) < 0)
			sys->fprint(sys->fildes(2), "%s: %s: openep: pollival: %r\n",
				argv0, epd.dir);
	return epd;
}

opendev(fname: string): ref Dev
{
	d := ref Dev(nil, 0, nil, nil, 0, nil, nil);

	d.dfd = nil;
	#
	# Don't ask me why this is necessary; some corner
	# cases of concatenation don't seem to work
	#
	d.dir = sys->sprint("%s", fname);
	ctlname := sys->sprint("%s/ctl", fname);
	d.cfd = sys->open(ctlname, Sys->ORDWR);
	d.id = nameid(fname);
	if(d.cfd == nil) {
		sys->werrstr(sys->sprint("can't open endpoint %s: %r", d.dir));
		d.dir = nil;
		d = nil;
		return nil;
	}
	if(usbdebug)
		dprint(2, sys->sprint("opendev %s\n", fname));
	return d;
}

opendevdata(d: ref Dev, mode: int): ref Sys->FD
{
	datname := sys->sprint("%s/data", d.dir);
	d.dfd = sys->open(datname, mode);
	return d.dfd;
}

loaddevconf(d: ref Dev, n: int): int
{
	buf := array[1024] of {* => byte 0};

	if(d == nil || d.usb == nil || d.usb.conf == nil){
		sys->fprint(sys->fildes(2), "%s: Internal error, no Dev in loaddevconf\n",
			argv0);
		return -1;
	}
	if(n >= len d.usb.conf) {
		sys->werrstr("loaddevconf: bug: out of configurations in device");
		sys->fprint(sys->fildes(2), "%s: %r\n", argv0);
		return -1;
	}
	typ := Rd2h|Rstd|Rdev;
	nr := usbcmd(d, typ, Rgetdesc, Dconf<<8|n, 0, buf, 1024);
	if(nr < Dconflen)
		return -1;
	if(d.usb.conf[n] == nil) {
		d.usb.conf[n] = ref Conf(0, 0, 0, nil);
		d.usb.conf[n].iface = array[Niface] of ref Iface;
	}
	return parseconf(d.usb, d.usb.conf[n], buf, nr);
}

mkep(d: ref Usbdev, id: int): ref Ep
{
	d.ep[id] = ref Ep(0, 0, 0, 0, id, 0, 0, nil, nil);
	return d.ep[id];
}

loaddevstr(d: ref Dev, sid: int): string
{
	buf := array[128] of {* => byte 0};
	if(sid == 0)
		return "none";
	typ := Rd2h|Rstd|Rdev;
	nr := usbcmd(d, typ, Rgetdesc, Dstr<<8|sid, 0, buf, len buf);
	s := "";
	if(nr <= 2 || (nr & 1) != 0)
		s = "none";
	for(i := 2; i < nr; i += 2)
		s += sys->sprint("%c", get2(buf[i:i+2]));
	return s;
}

loaddevdesc(d: ref Dev): int
{
	buf := array[Ddevlen+255] of {* => byte 0};

	typ := Rd2h|Rstd|Rdev;
	nr := len buf;
	if((nr = usbcmd(d, typ, Rgetdesc, Ddev<<8|0, 0, buf, nr)) < 0)
		return -1;
	#
	# Several hubs are returning descriptors of 17 bytes, not 18.
	# We accept them and leave number of configurations as zero.
	# (a get configuration descriptor also fails for them!)
	#
	if(nr < Ddevlen) {
		sys->print("%s: %s: warning: device with short descriptor\n",
			argv0, d.dir);
		if(nr < Ddevlen-1){
			sys->werrstr(sys->sprint("short device descriptor (%d bytes)", nr));
			return -1;
		}
	}
	d.usb = ref Usbdev(big 0, 0, 0, nil, nil, nil, 0, 0, 0, 0, 0,
		array[Nconf] of ref Conf, array[Nep] of ref Ep, array[Nddesc] of ref Desc);
	ep0 := mkep(d.usb, 0);
	ep0.dir = Eboth;
	ep0.etype = Econtrol;
	ep0.maxpkt = d.maxpkt = 8;		# a default
	nr = parsedev(d, buf, nr);
	if(nr >= 0){
		d.usb.vendor = loaddevstr(d, d.usb.vsid);
		if(d.usb.vendor != "none") {
			d.usb.product = loaddevstr(d, d.usb.psid);
			d.usb.serial = loaddevstr(d, d.usb.ssid);
		}
	}
	return nr;
}

configdev(d: ref Dev): int
{
	if(d.dfd == nil)
		opendevdata(d, Sys->ORDWR);
	if(loaddevdesc(d) < 0)
		return -1;
	for(i := 0; i < d.usb.nconf; i++)
		if(loaddevconf(d, i) < 0)
			return -1;
	return 0;
}

closeconf(c: ref Conf)
{
	if(c == nil)
		return;
	for(i := 0; i < len c.iface; i++)
		if(c.iface[i] != nil){
			for(a := 0; a < len c.iface[i].altc; a++)
				c.iface[i].altc[a] = nil;
			c.iface[i] = nil;
		}
	c = nil;
}

closedev(d: ref Dev)
{
	if(d == nil)
		return;
	if(usbdebug)
		dprint(2, sys->sprint("closedev %s\n", d.dir));
	d.cfd = d.dfd = nil;
	d.dir = nil;
	ud := d.usb;
	d.usb = nil;
	if(ud != nil){
		ud.vendor = nil;
		ud.product = nil;
		ud.serial = nil;
		for(i := 0; i < len ud.ep; i++)
			ud.ep[i] = nil;
		for(i = 0; i < len ud.ddesc; i++)
			ud.ddesc[i] = nil;
		for(i = 0; i < len ud.conf; i++)
			closeconf(ud.conf[i]);
		ud = nil;
	}
	d = nil;
}

reqstr(typ: int, req: int): string
{
	buf: string;
	ds := array[] of { "dev", "if", "ep", "oth" };

	if(typ & Rd2h)
		buf = "d2h";
	else
		buf = "h2d";
	if(typ & Rclass)
		buf = "|cls";
	else if(typ & Rvendor)
		buf += "|vnd";
	else
		buf += "|std";
	buf += sys->sprint("|%s", ds[typ&3]);

	case req {
	Rgetstatus => buf += " getsts";
	Rclearfeature => buf += " clrfeat";
	Rsetfeature => buf += " setfeat";
	Rsetaddress => buf += " setaddr";
	Rgetdesc => buf += " getdesc";
	Rsetdesc => buf += " setdesc";
	Rgetconf => buf += " getcnf";
	Rsetconf => buf += " setcnf";
	Rgetiface => buf += " getif";
	Rsetiface => buf += " setif";
	}
	return buf;
}

cmdreq(d: ref Dev, typ: int, req: int, value: int, index: int,
	data: array of byte, count: int): int
{
	ndata: int;
	wp: array of byte;
	tmp := array[2] of {* => byte 0};
	hexd: string;

	if(data == nil){
		wp = array[8] of {* => byte 0};
		ndata = 0;
	}
	else{
		ndata = count;
		wp = array[ndata+8] of {* => byte 0};
	}
	wp[0] = byte typ;
	wp[1] = byte req;
	put2(tmp, value);
	wp[2:] = tmp;
	put2(tmp, index);
	wp[4:] = tmp;
	put2(tmp, count);
	wp[6:] = tmp;
	if(data != nil)
		wp[8:] = data;
	if(usbdebug > 2){
		hexd = hexstr(wp, ndata+8);
		rs := reqstr(typ, req);
		sys->fprint(sys->fildes(2), "%s: %s val %d|%d idx %d cnt %d out[%d] %s\n",
			d.dir, rs, value>>8, value & 16rFF,
			index, count, ndata+8, hexd);
		hexd = nil;
	}
	n := sys->write(d.dfd, wp, 8+ndata);
	wp = nil;
	if(n < 0)
		return -1;
	if(n != 8+ndata){
		if(usbdebug)
			dprint(2, sys->sprint("cmd: short write: %d\n", n));
		return -1;
	}
	return n;
}

cmdrep(d: ref Dev, buf: array of byte, nb: int): int
{
	n := sys->read(d.dfd, buf, nb);
	if(n > 0 && usbdebug > 2){
		hexd := hexstr(buf, n);
		sys->fprint(sys->fildes(2), "%s: in[%d] %s\n", d.dir, n, hexd);
		hexd = nil;
	}
	return n;
}

usbcmd(d: ref Dev, typ: int, req: int, value: int, index: int,
	data: array of byte, count: int): int
{
	err: string;
	i, nerr: int;

	#
	# Some devices do not respond to commands some times.
	# Others even report errors but later work just fine. Retry.
	#
	r := -1;
	for(i = nerr = 0; i < Uctries; i++){
		if(typ & Rd2h)
			r = cmdreq(d, typ, req, value, index, nil, count);
		else
			r = cmdreq(d, typ, req, value, index, data, count);
		if(r > 0){
			if((typ & Rd2h) == 0)
				break;
			r = cmdrep(d, data, count);
			if(r > 0)
				break;
			if(r == 0)
				sys->werrstr("no data from device");
		}
		nerr++;
		if(err == nil)
			err = sys->sprint("%r");
		sys->sleep(Ucdelay);
	}
	if(r > 0 && i >= 2){
		# let the user know the device is not in good shape
		sys->fprint(sys->fildes(2), "%s: usbcmd: %s: required %d attempts (%s)\n",
			argv0, d.dir, i, err);
	}
	return r;
}

unstall(dev: ref Dev, ep: ref Dev, dir: int): int
{
	if(dir == Ein)
		dir = 16r80;
	else
		dir = 0;
	r := Rh2d|Rstd|Rep;
	if(usbcmd(dev, r, Rclearfeature, Fhalt, ep.id|dir, nil, 0)<0){
		sys->werrstr(sys->sprint("unstall: %s: %r", ep.dir));
		return -1;
	}
	if(devctl(ep, "clrhalt") < 0){
		sys->werrstr(sys->sprint("clrhalt: %s: %r", ep.dir));
		return -1;
	}
	return 0;
}

#
# To be sure it uses a single write.
#
devctl(dev: ref Dev, msg: string): int
{
	return sys->write(dev.cfd, array of byte msg, len msg);
}

classname(c: int): string
{
	cnames := array[] of {
		"none", "audio", "comms", "hid", "",
		"", "", "printer", "storage", "hub", "data"
	};

	if(c >= 0 && c < len cnames)
		return cnames[c];
	else
		return sys->sprint("%d", c);
}

hexstr(a: array of byte, n: int): string
{
	b := a;
	s := "";
	for(i := 0; i < n; i++)
		s += sys->sprint(" %.2ux", int b[i]);
	return s;
}

seprintiface(i: ref Iface): string
{
	edir := array[] of {"in", "out", "inout"};
	etype := array[] of {"ctl", "iso", "bulk", "intr"};

	s2 := sys->sprint("\t\tiface csp %s.%ud.%ud\n",
		classname(int i.csp & 16rff), int ((i.csp)>>8)&16rff, int ((i.csp)>>16)&16rff);
	for(j := 0; j < Naltc; j++){
		a := i.altc[j];
		if(a == nil)
			break;
		s2 += sys->sprint("\t\t  alt %d attr %d ival %d", j, a.attrib, a.interval);
		s2 += sys->sprint("\n");
	}
	for(j = 0; j < Nep; j++){
		ep := i.ep[j];
		if(ep == nil)
			break;
		eds := ets := "";
		if(ep.dir <= len edir)
			eds = edir[ep.dir];
		if(ep.etype <= len etype)
			ets = etype[ep.etype];
		s2 += sys->sprint(
			"\t\t  ep id %d addr %d dir %s type %s itype %d maxpkt %d ntds %d\n",
			ep.id, ep.addr, eds, ets, ep.isotype,
			ep.maxpkt, ep.ntds);
	}
	return s2;
}

seprintconf(d: ref Usbdev, ci: int): string
{
	c := d.conf[ci];
	s2 := sys->sprint("\tconf: cval %d attrib %x %d mA\n",
		c.cval, c.attrib, c.milliamps);
	for(i := 0; i < Niface; i++)
		if(c.iface[i] == nil)
			break;
		else
			s2 += seprintiface(c.iface[i]);
	for(i = 0; i < Nddesc; i++)
		if(d.ddesc[i] == nil)
			break;
		else if(d.ddesc[i].conf == c){
			hexd := hexstr(d.ddesc[i].data.serialize(),
				int d.ddesc[i].data.bLength);
			s2 += sys->sprint("\t\tdev desc %x[%d]: %s\n",
				int d.ddesc[i].data.bDescriptorType,
				int d.ddesc[i].data.bLength, hexd);
			hexd = nil;
		}
	return s2;
}

Ufmt(d: ref Dev): string
{
	if(d == nil)
		return "nil device\n";
	ud := d.usb;
	if(ud == nil)
		return sys->sprint("%s %bd refs\n", d.dir, big 0);
	s := sys->sprint("%s csp %s.%ud.%ud",
		d.dir, classname(int ud.csp & 16rff), (int ud.csp >> 8) & 16rff,
		(int ud.csp >> 16) & 16rff);
	s += sys->sprint(" vid %#ux did %#ux", ud.vid, ud.did);
	s += sys->sprint(" refs %bd\n", big 0);
	s += sys->sprint("\t%s %s %s\n", ud.vendor, ud.product, ud.serial);
	for(i := 0; i < Nconf; i++){
		if(ud.conf[i] == nil)
			break;
		else
			s += seprintconf(ud, i);
	}

	return s;
}

parsedev(xd: ref Dev, b: array of byte, n: int): int
{
	dd := ref DDev;

	d := xd.usb;
	dd.populate(b);
	if(usbdebug>1){
		hexd := hexstr(b, Ddevlen);
		sys->fprint(sys->fildes(2), "%s: parsedev %s: %s\n", argv0, xd.dir, hexd);
		hexd = nil;
	}
	if(int dd.bLength < Ddevlen){
		sys->werrstr(sys->sprint("short dev descr. (%d < %d)",
			int dd.bLength, Ddevlen));
		return -1;
	}
	if(int dd.bDescriptorType != Ddev){
		sys->werrstr(sys->sprint("%d is not a dev descriptor",
			int dd.bDescriptorType));
		return -1;
	}
	d.csp = big dd.bDevClass | (big dd.bDevSubClass << 8)
		| (big dd.bDevProtocol << 16);
	d.ep[0].maxpkt = xd.maxpkt = int dd.bMaxPacketSize0;
	d.class = int dd.bDevClass;
	d.nconf = int dd.bNumConfigurations;
	if(d.nconf == 0 && usbdebug)
		dprint(2, sys->sprint("%s: no configurations\n", xd.dir));
	d.vid = get2(dd.idVendor);
	d.did = get2(dd.idProduct);
	d.vsid = int dd.iManufacturer;
	d.psid = int dd.iProduct;
	d.ssid = int dd.iSerialNumber;
	if(n > Ddevlen && usbdebug > 1)
		dprint(2, sys->sprint("%s: parsedev: %d bytes left",
			xd.dir, n - Ddevlen));
	return Ddevlen;
}

parseiface(d: ref Usbdev,  c: ref Conf, b: array of byte, n: int): (int, ref Iface, ref Altc)
{
	dip := ref DIface;

	if(n < Difacelen){
		sys->werrstr("short interface descriptor");
		return (-1, nil, nil);
	}
	dip.populate(b);
	ifid := int dip.bInterfaceNumber;
	if(ifid < 0 || ifid >= len c.iface) {
		sys->werrstr(sys->sprint("bad interface number %d", int ifid));
		return (-1, nil, nil);
	}
	if(c.iface[ifid] == nil)
		c.iface[ifid] = ref Iface(0, big 0,
			array[Naltc] of ref Altc, array[Nep] of ref Ep);
	ip := c.iface[ifid];
	class := dip.bInterfaceClass;
	subclass := dip.bInterfaceSubClass;
	proto := dip.bInterfaceProtocol;
	ip.csp = big class | (big subclass << 8) | (big proto << 16);
	if(d.csp == big 0)				# use csp from 1st iface
		d.csp = ip.csp;		# if device has none
	if(d.class == 0)
		d.class = int class;
	ip.id = ifid;
	if(c == d.conf[0] && ifid == 0)	# ep0 was already there
		d.ep[0].iface = ip;
	altid := int dip.bAlternateSetting;
	if(altid < 0 || altid >= len ip.altc){
		sys->werrstr(sys->sprint("bad alternate conf. number %d", altid));
		return (-1, nil, nil);
	}
	if(ip.altc[altid] == nil)
		ip.altc[altid] = ref Altc(0, 0);
	return (Difacelen, ip, ip.altc[altid]);
}

parseendpt(d: ref Usbdev, c: ref Conf, ip: ref Iface, altc: ref Altc,
	b: array of byte, n: int): (int, ref Ep)
{
	dep := ref DEp;

	if(n < Deplen){
		sys->werrstr("short endpoint descriptor");
		return (-1, nil);
	}
	dep.populate(b);
	altc.attrib = int dep.bmAttributes;	# here?
	altc.interval = int dep.bInterval;

	epid := int dep.bEndpointAddress & 16rF;
	if(int dep.bEndpointAddress & 16r80)
		dir := Ein;
	else
		dir = Eout;
	ep := d.ep[epid];
	if(ep == nil){
		ep = mkep(d, epid);
		ep.dir = dir;
	}
	else if((ep.addr & 16r80) != (int dep.bEndpointAddress & 16r80))
		ep.dir = Eboth;
	ep.maxpkt = get2(dep.wMaxPacketSize);
	ep.ntds = 1 + ((ep.maxpkt >> 11) & 3);
	ep.maxpkt &= 16r7FF;
	ep.addr = int dep.bEndpointAddress;
	ep.etype = int dep.bmAttributes & 16r03;
	ep.isotype = (int dep.bmAttributes>>2) & 16r03;
	ep.conf = c;
	ep.iface = ip;
	for(i := 0; i < len ip.ep; i++)
		if(ip.ep[i] == nil)
			break;
	if(i == len ip.ep){
		sys->werrstr(sys->sprint(
			"parseendpt: bug: too many end points on interface with csp %#bux",
			ip.csp));
		sys->fprint(sys->fildes(2), "%s: %r\n", argv0);
		return (-1, nil);
	}
	ip.ep[i] = ep;
	return (Dep, ep);
}

dname(dtype: int): string
{
	case dtype {
	Ddev =>	return "device";
	Dconf => 	return "config";
	Dstr => 	return "string";
	Diface =>	return "interface";
	Dep =>	return "endpoint";
	Dreport =>	return "report";
	Dphysical =>	return "phys";
	* =>	return "desc";
	}
}

parsedesc(d: ref Usbdev, c: ref Conf, b: array of byte, n: int): int
{
	r: int;
	ip: ref Iface;
	ep: ref Ep;
	altc: ref Altc;

	tot := 0;
	ip = nil;
	ep = nil;
	altc = nil;
	for(nd := 0; nd < len d.ddesc; nd++)
		if(d.ddesc[nd] == nil)
			break;
	while(n > 2 && int b[0] != 0 && int b[0] <= n) {
		leng := int b[0];
		if(usbdebug > 1) {
			hexd := hexstr(b, leng);
			sys->fprint(sys->fildes(2), "%s:\t\tparsedesc %s %x[%d] %s\n",
				argv0, dname(int b[1]), int b[1], int b[0], hexd);
			hexd = nil;
		}
		case int b[1] {
		Ddev or Dconf =>
			sys->werrstr(sys->sprint("unexpected descriptor %d", int b[1]));
			if(usbdebug)
				dprint(2, sys->sprint("parsedesc: %r"));
		Diface =>
			(r, ip, altc) = parseiface(d, c, b, n);
			if(r < 0) {
				if(usbdebug)
					dprint(2, sys->sprint("parsedesc: %r\n"));
				return -1;
			}
		Dep =>
			if(ip == nil || altc == nil){
				sys->werrstr("unexpected endpoint descriptor");
				break;
			}
			(r, ep) = parseendpt(d, c, ip, altc, b, n);
			if(r < 0) {
				if(usbdebug)
					dprint(2, sys->sprint("parsedesc: %r\n"));
				return -1;
			}
		* =>
			if(nd == len d.ddesc) {
				sys->fprint(sys->fildes(2),
					"%s: parsedesc: too many device-specific descriptors for device %s %s\n",
					argv0, d.vendor, d.product);
				break;
			}
			d.ddesc[nd] = ref Desc(c, ip, ep, altc, ref DDesc);
			d.ddesc[nd].data.populate(b);
			++nd;
		}
		n -= leng;
		b = b[leng:];
		tot += leng;
	}
	return tot;
}

parseconf(d: ref Usbdev, c: ref Conf, b: array of byte, n: int): int
{
	dc := ref DConf;

	dc.populate(b);
	if(usbdebug > 1) {
		hexd := hexstr(b, Dconflen);
		sys->fprint(sys->fildes(2), "%s:\tparseconf  %s\n", argv0, hexd);
		hexd = nil;
	}
	if(int dc.bLength < Dconflen){
		sys->werrstr("short configuration descriptor");
		return -1;
	}
	if(int dc.bDescriptorType != Dconf){
		sys->werrstr("not a configuration descriptor");
		return -1;
	}
	c.cval = int dc.bConfigurationValue;
	c.attrib = int dc.bmAttributes;
	c.milliamps = int dc.MaxPower*2;
	l := get2(dc.wTotalLength);
	if(n < l){
		sys->werrstr("truncated configuration info");
		return -1;
	}
	n -= Dconflen;
	b2 := b[Dconflen:];
	if(n > 0 && (nr := parsedesc(d, c, b2, n)) < 0)
		return -1;
	n -= nr;
	if(n > 0 && usbdebug>1)
		sys->fprint(sys->fildes(2), "%s:\tparseconf: %d bytes left\n", argv0, n);
	return l;
}

DDesc.populate(d: self ref DDesc, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.bbytes = array[int b[0]-2] of byte;
	for(i := 0; i < int b[0] - 2; ++i)
		d.bbytes[i] = b[i+2];
}

DDesc.serialize(d: self ref DDesc): array of byte
{
	l := len d.bbytes;
	b := array[l+2] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	for(i := 0; i < l; ++i)
		b[i+2] = d.bbytes[i];
	return b;
}

DDev.populate(d: self ref DDev, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.bcdUSB = array[2] of byte;
	d.bcdUSB[0] = b[2];
	d.bcdUSB[1] = b[3];
	d.bDevClass = b[4];
	d.bDevSubClass = b[5];
	d.bDevProtocol = b[6];
	d.bMaxPacketSize0 = b[7];
	d.idVendor = array[2] of byte;
	d.idVendor[0] = b[8];
	d.idVendor[1] = b[9];
	d.idProduct = array[2] of byte;
	d.idProduct[0] = b[10];
	d.idProduct[1] = b[11];
	d.bcdDev = array[2] of byte;
	d.bcdDev[0] = b[12];
	d.bcdDev[1] = b[13];
	d.iManufacturer = b[14];
	d.iProduct = b[15];
	d.iSerialNumber = b[16];
	d.bNumConfigurations = b[17];
}

DDev.serialize(d: self ref DDev): array of byte
{
	b := array[Ddevlen] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	b[2] = d.bcdUSB[0];
	b[3] = d.bcdUSB[1];
	b[4] = d.bDevClass;
	b[5] = d.bDevSubClass;
	b[6] = d.bDevProtocol;
	b[7] = d.bMaxPacketSize0;
	b[8] = d.idVendor[0];
	b[9] = d.idVendor[1];
	b[10] = d.idProduct[0];
	b[11] = d.idProduct[1];
	b[12] = d.bcdDev[0];
	b[13] = d.bcdDev[1];
	b[14] = d.iManufacturer;
	b[15] = d.iProduct;
	b[16] = d.iSerialNumber;
	b[17] = d.bNumConfigurations;
	return b;
}

DConf.populate(d: self ref DConf, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.wTotalLength = array[2] of byte;
	d.wTotalLength[0] = b[2];
	d.wTotalLength[1] = b[3];
	d.bNumInterfaces = b[4];
	d.bConfigurationValue = b[5];
	d.iConfiguration = b[6];
	d.bmAttributes = b[7];
	d.MaxPower = b[8];
}

DConf.serialize(d: self ref DConf): array of byte
{
	b := array[Dconflen] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	b[2] = d.wTotalLength[0];
	b[3] = d.wTotalLength[1];
	b[4] = d.bNumInterfaces;
	b[5] = d.bConfigurationValue;
	b[6] = d.iConfiguration;
	b[7] = d.bmAttributes;
	b[8] = d.MaxPower;
	return b;
}

DIface.populate(d: self ref DIface, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.bInterfaceNumber = b[2];
	d.bAlternateSetting = b[3];
	d.bNumEndpoints = b[4];
	d.bInterfaceClass = b[5];
	d.bInterfaceSubClass = b[6];
	d.bInterfaceProtocol = b[7];
	d.iInterface = b[8];
}

DIface.serialize(d: self ref DIface): array of byte
{
	b := array[Difacelen] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	b[2] = d.bInterfaceNumber;
	b[3] = d.bAlternateSetting;
	b[4] = d.bNumEndpoints;
	b[5] = d.bInterfaceClass;
	b[6] = d.bInterfaceSubClass;
	b[7] = d.bInterfaceProtocol;
	b[8] = d.iInterface;
	return b;
}

DEp.populate(d: self ref DEp, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.bEndpointAddress = b[2];
	d.bmAttributes = b[3];
	d.wMaxPacketSize = array[2] of byte;
	d.wMaxPacketSize[0] = b[4];
	d.wMaxPacketSize[1] = b[5];
	d.bInterval = b[6];
}

DEp.serialize(d: self ref DEp): array of byte
{
	b := array[Deplen] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	b[2] = d.bEndpointAddress;
	b[3] = d.bmAttributes;
	b[4] = d.wMaxPacketSize[0];
	b[5] = d.wMaxPacketSize[1];
	b[6] = d.bInterval;
	return b;
}

strtol(s: string, base: int): (int, string)
{
	if (str == nil)
		str = load String String->PATH;
	if (base != 0)
		return str->toint(s, base);
	if (len s >= 2 && (s[0:2] == "0X" || s[0:2] == "0x"))
		return str->toint(s[2:], 16);
	if (len s > 0 && s[0:1] == "0")
		return str->toint(s[1:], 8);
	return str->toint(s, 10);
}

