implement Usbd;

include "sys.m";
	sys: Sys;
include "draw.m";
include "string.m";
	str: String;
include "arg.m";
	arg: Arg;

include "usb.m";
	usb: Usb;
	Conf, Ep: import Usb;
	
include "bufio.m";
	bufio: Bufio;
	Iobuf: import bufio;
include "readdir.m";
	readdir: Readdir;

Usbd: module
{
	init: fn(nil: ref Draw->Context, args: list of string);
};

Hub: adt {
	pwrmode: byte;
	compound: int;
	pwrms: int;		# time to wait in ms
	maxcurrent: byte;	# 	after powering port
	leds: int;			# has port indicators?
	maxpkt: int;
	nport: int;
	port: cyclic array of ref Port;
	failed: int;
	isroot: int;
	dev: ref Usb->Dev;
};

Port: adt {
	state: int;		# state of the device
	sts: int;		# old port status
	removable: int;
	pwrctl: int;
	dev: ref Usb->Dev;		# attached device (if non-nil)
	hub: cyclic ref Hub;		# non-nil if hub attached
};

DHub: adt {
	bLength: byte;
	bDescriptorType: byte;
	bNbrPorts: byte;
	wHubCharacteristics: array of byte;
	bPwrOn2PwrGood: byte;
	bHubContrCurrent: byte;
	DeviceRemovable: array of byte;
	populate: fn(d: self ref DHub, b: array of byte);
	serialize: fn(d: self ref DHub): array of byte;
};

Devtab: adt {
	name: string;
	csps: array of int;
	vid: int;
	did: int;
	args: string;
};

Dhub: con 16r29;		# hub descriptor type
Dhublen: con 9;		# hub descriptor length

Fportconnection: con 0;
Fportenable: con 1;
Fportsuspend: con 2;
Fportovercurrent: con 3;
Fportreset: con 4;
Fportpower: con 8;
Fportlowspeed: con 9;
Fcportconnection: con 16;
Fcportenable: con 17;
Fcportsuspend: con 18;
Fcportovercurrent: con 19;
Fcportreset: con 20;
Fportindicator: con 22;

Rclearfeature: con 1;
Rsetfeature: con 3;

PSpresent: con 16r0001;
PSenable: con 16r0002;
PSsuspend: con 16r0004;
PSovercurrent: con 16r0008;
PSreset: con 16r0010;
PSpower: con 16r0100;
PSslow: con 16r0200;
PShigh: con 16r0400;

PSstatuschg: con 16r10000;		# PSpresent changed
PSchange: con 16r20000;			# PSenable changed

Pdisabled, Pattached, Pconfiged: con iota;

# Delays, timeouts (ms)
Spawndelay: con 1000;		# how often may we re-spawn a driver
Connectdelay: con 1000;		# how much to wait after a connect
Resetdelay: con 20;		# how much to wait after a reset
Enabledelay: con 20;		# how much to wait after an enable
Powerdelay: con 100;		# after powering up ports
Pollms: con 250; 		# port poll interval
Chgdelay: con 100;		# waiting for port become stable
Chgtmout: con 1000;		# ...but at most this much

#
# device tab for embedded usb drivers.
#
DCL: con 16r01000000;		# csp identifies just class
DSC: con 16r02000000;		# csp identifies just subclass
DPT: con 16r04000000;		# csp identifies just proto

Line: adt {
	level: int;
	command: string;
	value: int;
	svalue: string;
};

verbose: int;
pollms: int;
hubs: list of ref Hub;
nhubs: int;

mustdump: int;

init(nil: ref Draw->Context, args: list of string)
{
	sys = load Sys Sys->PATH;
	str = load String String->PATH;
	arg = load Arg Arg->PATH;
	usb = load Usb Usb->PATH;
	bufio = load Bufio Bufio->PATH;
	readdir = load Readdir Readdir->PATH;

	i: int;

	usb->init();
	mnt := "/dev";
	pollms = 250;
	arg->init(args);
	usb->argv0 = arg->progname();
	arg->setusage("usbd [-d] [-s srv] [-m mnt] [dev...]");
	while ((c := arg->opt()) != 0)
		case c {
		'd' => usb->usbdebug++;
		'i' => pollms = int arg->earg();
		'm' => mnt = arg->earg();
		* => arg->usage();
		}
	args = arg->argv();

	portc := chan of string;

	loaddriverdatabase();

	spawn work(portc);

	if(len args == 0){
		(d,nd) := readdir->init("/dev/usb", Readdir->NONE);
		if(nd < 2) {
			sys->print("/dev/usb: no hubs\n");
			exit;
		}
		for(i = 0; i < nd; i++)
			if(d[i].name != "ctl")
				portc <-= sys->sprint("/dev/usb/%s", d[i].name);
	}
	else
		while(args != nil) {
			portc <-= hd args;
			args = tl args;
		}
	portc <-= "";
	<- portc;
	portc = nil;
}

work(portc: chan of string)
{
	hubs = nil;
	#
	# Receive requests for root hubs
	#
	while((fname := <- portc) != ""){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s starting\n", fname));
		h := newhub(fname, nil);
		if(h == nil)
			sys->fprint(sys->fildes(2), "%s: %s: %r\n", usb->argv0, fname);
	}
	#
	# Enumerate (and acknowledge after first enumeration).
	# Do NOT perform enumeration concurrently for the same
	# controller. new devices attached respond to a default
	# address (0) after reset, thus enumeration has to work
	# one device at a time at least before addresses have been
	# assigned.
	# Do not use hub interrupt endpoint because we
	# have to poll the root hub(s) in any case.
	#
	Again:
	for(;;){
		for(hl := hubs; hl != nil; hl = tl hl)
			for(i := 1; i <= (hd hl).nport; i++)
				if(enumhub(hd hl, i) < 0){
					# changes in hub list; repeat
					continue Again;
				}
		if(portc != nil) {
			portc <-= "";
			portc = nil;
		}
		sys->sleep(pollms);
		if(mustdump)
			dump();
	}
}

DHub.populate(d: self ref DHub, b: array of byte)
{
	d.bLength = b[0];
	d.bDescriptorType = b[1];
	d.bNbrPorts = b[2];
	d.wHubCharacteristics = array[2] of byte;
	d.wHubCharacteristics[0] = b[3];
	d.wHubCharacteristics[1] = b[4];
	d.bPwrOn2PwrGood = b[5];
	d.bHubContrCurrent = b[6];
	d.DeviceRemovable = array[int b[0] - 7] of byte;
	for(i := 0; i < int b[0] - 7; ++i)
		d.DeviceRemovable[i] = b[i+7];
}

DHub.serialize(d: self ref DHub): array of byte
{
	b := array[int d.bLength] of byte;
	b[0] = d.bLength;
	b[1] = d.bDescriptorType;
	b[2] = d.bNbrPorts;
	b[3] = d.wHubCharacteristics[0];
	b[4] = d.wHubCharacteristics[1];
	b[5] = d.bPwrOn2PwrGood;
	b[6] = d.bHubContrCurrent;
	for(i := 0; i < int b[0] - 7; ++i)
		b[i+7] = d.DeviceRemovable[i];
	return b;
}

newhub(fname: string, d: ref Usb->Dev): ref Hub
{
	h := ref Hub;
	i: int;

	h.pwrmode = byte 0;
	h.compound = 0;
	h.pwrms = 0;
	h.maxcurrent = byte 0;
	h.leds = 0;
	h.maxpkt = 0;
	h.nport = 0;
	h.port = nil;
	h.isroot = (d == nil);
	h.failed = 0;
	if(h.isroot){
		h.dev = usb->opendev(fname);
		if(h.dev == nil){
			sys->fprint(sys->fildes(2), "%s: opendev: %s: %r", usb->argv0, fname);
			newhubfail(d, h);
			return nil;
		}
		if(usb->opendevdata(h.dev, Sys->ORDWR) == nil){
			sys->fprint(sys->fildes(2), "%s: opendevdata: %s: %r\n",
				usb->argv0, fname);
			newhubfail(d, h);
			return nil;
		}
		configroothub(h);	# never fails
	}
	else{
		h.dev = d;
		if(confighub(h) < 0){
			sys->fprint(sys->fildes(2), "%s: %s: config: %r\n", usb->argv0, fname);
			newhubfail(d, h);
			return nil;
		}
	}
	if(h.dev == nil){
		sys->fprint(sys->fildes(2), "%s: opendev: %s: %r\n", usb->argv0, fname);
		newhubfail(d, h);
		return nil;
	}
	usb->devctl(h.dev, "hub");
	ud := h.dev.usb;
	if(h.isroot)
		usb->devctl(h.dev, sys->sprint("info roothub csp %#08ux ports %d",
			16r000009, h.nport));
	else{
		usb->devctl(h.dev, sys->sprint("info hub csp %#08ubx ports %d %q %q",
			ud.csp, h.nport, ud.vendor, ud.product));
		for(i = 1; i <= h.nport; i++)
			if(hubfeature(h, i, Fportpower, 1) < 0)
				sys->fprint(sys->fildes(2), "%s: %s: power: %r\n",
					usb->argv0, fname);
		sys->sleep(h.pwrms);
		for(i = 1; i <= h.nport; i++)
			if(h.leds != 0)
				hubfeature(h, i, Fportindicator, 1);
	}
	hubs = h :: hubs;
	nhubs++;
	usb->dprint(2, "hub allocated:");
	if(usb->usbdebug)
		sys->fprint(sys->fildes(2),
			" ports %d pwrms %d max curr %d pwrm %d cmp %d leds %d\n",
			h.nport, h.pwrms, int h.maxcurrent,
			int h.pwrmode, h.compound, h.leds);
	return h;
}

newhubfail(d: ref Usb->Dev, h: ref Hub)
{
	if(d != nil)
		usb->devctl(d, "detach");
	h.port = nil;
	h = nil;
	usb->dprint(2, "hub failed to start:");
}

hubfeature(h: ref Hub, port: int, f: int, on: int): int
{
	cmd: int;

	if(on)
		cmd = Rsetfeature;
	else
		cmd = Rclearfeature;
	return usb->usbcmd(h.dev, Usb->Rh2d|Usb->Rclass|Usb->Rother,
		cmd, f, port, nil, 0);
}

#
# This may be used to detect overcurrent on the hub
#
checkhubstatus(h: ref Hub)
{
	buf := array[4] of byte;

	if(h.isroot)	# not for root hubs
		return;
	if(usb->usbcmd(h.dev, Usb->Rd2h|Usb->Rclass|Usb->Rdev,
			Usb->Rgetstatus, 0, 0, buf, 4) < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: get hub status: %r\n", h.dev.dir));
		return;
	}
	sts := usb->get2(buf);
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("hub %s: status %#ux\n", h.dev.dir, sts));
}

confighub(h: ref Hub): int
{
	buf := array[128] of byte;	# room for extra descriptors
	dd := ref DHub;
	nr: int;

	if(h.dev == nil){
		sys->fprint(sys->fildes(2), "%s: nil h.dev in confighub\n", usb->argv0);
		return -1;
	}
	d := h.dev.usb;
	skipgetdesc := 0;
	for(i := 0; i < len d.ddesc; ++i) {
		if(d.ddesc[i] == nil)
			break;
		else if(d.ddesc[i].data.bDescriptorType == byte Dhub) {
			b := usb->(d.ddesc[i].data).serialize();
			dd.populate(b);
			nr = Dhublen;
			skipgetdesc = 1;
			break;
		}
	}
	if(skipgetdesc == 0) {
		typ := Usb->Rd2h|Usb->Rclass|Usb->Rdev;
		nr = usb->usbcmd(h.dev, typ, Usb->Rgetdesc, Dhub<<8|0, 0, buf, len buf);
		if(nr < Dhublen){
			if(usb->usbdebug)
				usb->dprint(2, sys->sprint("%s: getdesc hub: %r\n", h.dev.dir));
			return -1;
		}
		dd.populate(buf);
	}
	h.nport = int dd.bNbrPorts;
	nmap := 1 + h.nport/8;
	if(nr < 7 + 2*nmap){
		sys->fprint(sys->fildes(2), "%s: %s: descr. too small\n",
			usb->argv0, h.dev.dir);
		return -1;
	}
	h.port = array[h.nport+1] of ref Port;
	for(i = 0; i < h.nport+1; ++i)
		h.port[i] = ref Port(0, 0, 0, 0, nil, nil);
	h.pwrms = int dd.bPwrOn2PwrGood*2;
	if(h.pwrms < Powerdelay)
		h.pwrms = Powerdelay;
	h.maxcurrent = dd.bHubContrCurrent;
	h.pwrmode = dd.wHubCharacteristics[0] & byte 3;
	h.compound = (int dd.wHubCharacteristics[0] & (1<<2))!=0;
	h.leds = (int dd.wHubCharacteristics[0] & (1<<7)) != 0;
	for(i = 1; i <= h.nport; i++){
		pp := h.port[i];
		offset := i/8;
		mask := 1<<(i%8);
		pp.removable = (int dd.DeviceRemovable[offset] & mask) != 0;
		pp.pwrctl = (int dd.DeviceRemovable[offset+nmap] & mask) != 0;
	}
	return 0;
}

configroothub(h: ref Hub)
{
	buf := array[128] of byte;

	d := h.dev;
	h.nport = 2;
	h.maxpkt = 8;
	sys->seek(d.cfd, big 0, 0);
	nr := sys->read(d.cfd, buf, len buf-1);
	if(nr < 0) {
		h.port = array[h.nport+1] of ref Port;
		for(i := 0; i < h.nport+1; ++i)
			h.port[i] = ref Port(0, 0, 0, 0, nil, nil);
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: ports %d maxpkt %d\n",
				d.dir, h.nport, h.maxpkt));
		return;
	}

	(nil, p) := str->splitstrl(string buf[:nr], "ports ");
	if(p == nil)
		sys->fprint(sys->fildes(2), "%s: %s: no port information\n",
			usb->argv0, d.dir);
	else
		(h.nport, nil) = str->toint(p[6:], 0);
	(nil, p) = str->splitstrl(string buf, "maxpkt ");
	if(p == nil)
		sys->fprint(sys->fildes(2), "%s: %s: no maxpkt information\n",
			usb->argv0, d.dir);
	else
		(h.maxpkt, nil) = str->toint(p[7:], 0);
	h.port = array[h.nport+1] of ref Port;
	for(i := 0; i < h.nport+1; ++i)
		h.port[i] = ref Port(0, 0, 0, 0, nil, nil);
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("%s: ports %d maxpkt %d\n",
			d.dir, h.nport, h.maxpkt));
}

#
# If during enumeration we get an I/O error the hub is gone or
# in pretty bad shape. Because of retries of failed usb commands
# (and the sleeps they include) it can take a while to detach all
# ports for the hub. This detaches all ports and makes the hub void.
# The parent hub will detect a detach (probably right now) and
# close it later.
#
hubfail(h: ref Hub)
{
	for(i := 1; i <= h.nport; i++)
		portdetach(h, i);
	h.failed = 1;
}

closehub(h: ref Hub)
{
	usb->dprint(2, "closing hub\n");
	hubs = delhub(h, hubs);
	nhubs--;
	hubfail(h);		# detach all ports
	h.port = nil;
	usb->devctl(h.dev, "detach");
	usb->closedev(h.dev);
}

delhub(h: ref Hub, hl: list of ref Hub): list of ref Hub
{
	if(hl == nil) {
		sys->fprint(sys->fildes(2), "closehub: no hub");
		exit;
	}
	if(hd hl == h)
		return tl hl;
	return hd hl :: delhub(h, tl hl);
}

portstatus(h: ref Hub, p: int): int
{
	d: ref Usb->Dev;
	buf := array[4] of byte;
	t: int;
	sts: int;
	dbg: int;

	dbg = usb->usbdebug;
	if(dbg != 0 && dbg < 4)
		usb->usbdebug = 1;	# do not be too chatty
	d = h.dev;
	t = Usb->Rd2h|Usb->Rclass|Usb->Rother;
	if(usb->usbcmd(d, t, Usb->Rgetstatus, 0, p, buf, len buf) < 0)
		sts = -1;
	else
		sts = usb->get2(buf);
	usb->usbdebug = dbg;
	return sts;
}

stsstr(sts: int): string
{
	i := 0;
	e := "";
	if(sts & PSsuspend)
		e[i++] = 'z';
	if(sts & PSreset)
		e[i++] = 'r';
	if(sts & PSslow)
		e[i++] = 'l';
	if(sts & PShigh)
		e[i++] = 'h';
	if(sts & PSchange)
		e[i++] = 'c';
	if(sts & PSenable)
		e[i++] = 'e';
	if(sts & PSstatuschg)
		e[i++] = 's';
	if(sts & PSpresent)
		e[i++] = 'p';
	if(e == "")
		e = "-";
	return e;
}

getmaxpkt(d: ref Usb->Dev, islow: int): int
{
	buf := array[64] of {* => byte 0};

	dd := ref Usb->DDev;
	dd.bcdUSB = array[2] of byte;
	dd.idVendor = array[2] of byte;
	dd.idProduct = array[2] of byte;
	dd.bcdDev = array[2] of byte;
	if(islow)
		dd.bMaxPacketSize0 = byte 8;
	else
		dd.bMaxPacketSize0 = byte 64;
	buf2 := usb->dd.serialize();
	for(i := 0; i < len buf2; ++i)
		buf[i] = buf2[i];
	if(usb->usbcmd(d, Usb->Rd2h|Usb->Rstd|Usb->Rdev, Usb->Rgetdesc,
			Usb->Ddev<<8|0, 0, buf, len buf) < 0)
		return -1;
	usb->dd.populate(buf);
	return int dd.bMaxPacketSize0;
}

#
# BUG: does not consider max. power avail.
#
portattach(h: ref Hub, p: int, sts: int): ref Usb->Dev
{
	nd: ref Usb->Dev;
	buf := array[40] of byte;

	d := h.dev;
	pp := h.port[p];
	nd = nil;
	pp.state = Pattached;
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("%s: port %d attach sts %#ux\n", d.dir, p, sts));
	sys->sleep(Connectdelay);
	if(hubfeature(h, p, Fportenable, 1) < 0 && usb->usbdebug)
		usb->dprint(2, sys->sprint("%s: port %d: enable: %r\n", d.dir, p));
	sys->sleep(Enabledelay);
	if(hubfeature(h, p, Fportreset, 1) < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: reset: %r\n", d.dir, p));
		portattachfail(pp, h, p, nd);
		return nil;
	}
	sys->sleep(Resetdelay);
	sts = portstatus(h, p);
	if(sts < 0){
		portattachfail(pp, h, p, nd);
		return nil;
	}
	if((sts & PSenable) == 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: not enabled?\n", d.dir, p));
		hubfeature(h, p, Fportenable, 1);
		sts = portstatus(h, p);
		if((sts & PSenable) == 0){
			portattachfail(pp, h, p, nd);
			return nil;
		}
	}
	sp := "full";
	if(sts & PSslow)
		sp = "low";
	if(sts & PShigh)
		sp = "high";
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("%s: port %d: attached status %#ux\n",
			d.dir, p, sts));

	if(usb->devctl(d, sys->sprint("newdev %s %d", sp, p)) < 0){
		sys->fprint(sys->fildes(2), "%s: %s: port %d: newdev: %r\n",
			usb->argv0, d.dir, p);
		portattachfail(pp, h, p, nd);
		return nil;
	}
	sys->seek(d.cfd, big 0, 0);
	nr := sys->read(d.cfd, buf, len buf-1);
	if(nr == 0){
		sys->fprint(sys->fildes(2), "%s: %s: port %d: newdev: eof\n",
			usb->argv0, d.dir, p);
		portattachfail(pp, h, p, nd);
		return nil;
	}
	if(nr < 0){
		sys->fprint(sys->fildes(2), "%s: %s: port %d: newdev: %r\n",
			usb->argv0, d.dir, p);
		portattachfail(pp, h, p, nd);
		return nil;
	}
	fname := sys->sprint("/dev/usb/%s", string buf[:nr]);
	nd = usb->opendev(fname);
	if(nd == nil){
		sys->fprint(sys->fildes(2), "%s: %s: port %d: opendev: %r\n",
			usb->argv0, d.dir, p);
		portattachfail(pp, h, p, nd);
		return nil;
	}
	if(usb->usbdebug > 2)
		usb->devctl(nd, "debug 1");
	if(usb->opendevdata(nd, Sys->ORDWR) == nil){
		sys->fprint(sys->fildes(2), "%s: %s: opendevdata: %r\n",
			usb->argv0, nd.dir);
		portattachfail(pp, h, p, nd);
		return nil;
	}
	if(usb->usbcmd(nd, Usb->Rh2d|Usb->Rstd|Usb->Rdev,
			Usb->Rsetaddress, nd.id, 0, nil, 0) < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: setaddress: %r\n",
				d.dir, p));
		portattachfail(pp, h, p, nd);
		return nil;
	}
	if(usb->devctl(nd, "address") < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: set address: %r\n",
				d.dir, p));
		portattachfail(pp, h, p, nd);
		return nil;
	}

	mp := getmaxpkt(nd, sp == "low");
	if(mp < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: getmaxpkt: %r\n",
				d.dir, p));
		portattachfail(pp, h, p, nd);
		return nil;
	}
	else{
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: maxpkt %d\n",
				d.dir, p, mp));
		usb->devctl(nd, sys->sprint("maxpkt %d", mp));
	}
	if((sts & PSslow) != 0 && sp == "full" && usb->usbdebug){
		usb->dprint(2, sys->sprint("%s: port %d: %s is full speed when port is low\n",
			d.dir, p, nd.dir));
	}
	if(usb->configdev(nd) < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: configdev: %r\n", d.dir, p));
		portattachfail(pp, h, p, nd);
		return nil;
	}
	#
	# We always set conf #1. BUG.
	#
	if(usb->usbcmd(nd, Usb->Rh2d|Usb->Rstd|Usb->Rdev,
			Usb->Rsetconf, 1, 0, nil, 0) < 0){
		if(usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: setconf: %r\n", d.dir, p));
		usb->unstall(nd, nd, Usb->Eout);
		if(usb->usbcmd(nd, Usb->Rh2d|Usb->Rstd|Usb->Rdev,
				Usb->Rsetconf, 1, 0, nil, 0) < 0) {
			portattachfail(pp, h, p, nd);
			return nil;
		}
	}
	if(usb->usbdebug)
		usb->dprint(2, usb->Ufmt(nd));
	pp.state = Pconfiged;
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("%s: port %d: configed: %s\n",
			d.dir, p, nd.dir));
	return pp.dev = nd;
}

portattachfail(pp: ref Port, h: ref Hub, p: int, nd: ref Usb->Dev)
{
	pp.state = Pdisabled;
	pp.sts = 0;
	if(pp.hub != nil)
		pp.hub = nil;	# hub closed by enumhub
	hubfeature(h, p, Fportenable, 0);
	if(nd != nil)
		usb->devctl(nd, "detach");
	usb->closedev(nd);
}

portdetach(h: ref Hub, p: int)
{
	pp := h.port[p];

	#
	# Clear present, so that we detect an attach on reconnects.
	#
	pp.sts &= ~(PSpresent|PSenable);

	if(pp.state == Pdisabled)
		return;
	pp.state = Pdisabled;
	if(usb->usbdebug && pp.dev != nil)
		usb->dprint(2, sys->sprint("%s: port %d: detached\n", pp.dev.dir, p));

	if(pp.hub != nil){
		closehub(pp.hub);
		pp.hub = nil;
	}
	if(pp.dev != nil){
		usb->devctl(pp.dev, "detach");
		usb->closedev(pp.dev);
		pp.dev = nil;
	}
}

portgone(pp: ref Port, sts: int): int
{
	if(sts < 0)
		return 1;
	#
	# If it was enabled and it's not now then it may be reconnect.
	# We pretend it's gone and later we'll see it as attached.
	#
	if((pp.sts & PSenable) != 0 && (sts & PSenable) == 0)
		return 1;
	return (pp.sts & PSpresent) != 0 && (sts & PSpresent) == 0;
}

enumhub(h: ref Hub, p: int): int
{
	if(h.failed)
		return 0;
	d := h.dev;
	if(usb->usbdebug > 3)
		sys->fprint(sys->fildes(2), "%s: %s: port %d enumhub\n",
			usb->argv0, d.dir, p);

	sts := portstatus(h, p);
	if(sts < 0) {
		hubfail(h);		# avoid delays on detachment
		return -1;
	}
	pp := h.port[p];
	onhubs := nhubs;
	if((sts & PSsuspend) != 0) {
		if(hubfeature(h, p, Fportenable, 1) < 0 && usb->usbdebug)
			usb->dprint(2, sys->sprint("%s: port %d: enable: %r\n", d.dir, p));
		sys->sleep(Enabledelay);
		sts = portstatus(h, p);
		sys->fprint(sys->fildes(2), "%s: %s: port %d: resumed (sts %#ux)\n",
			usb->argv0, d.dir, p, sts);
	}
	if((pp.sts & PSpresent) == 0 && (sts & PSpresent) != 0) {
		if(portattach(h, p, sts) != nil)
			if(startdev(pp) < 0)
				portdetach(h, p);
	}
	else if(portgone(pp, sts)){
		if(pp.dev != nil && pp.dev.mod != nil && pp.dev.mod->shutdown != nil)
			pp.dev.mod->shutdown();
		portdetach(h, p);
	}
	else if(pp.sts != sts && usb->usbdebug) {
		usb->dprint(2, sys->sprint("%s port %d: sts %s %#x ->",
			d.dir, p, stsstr(pp.sts), pp.sts));
		sys->fprint(sys->fildes(2), " %s %#x\n",stsstr(sts), sts);
	}
	pp.sts = sts;
	if(onhubs != nhubs)
		return -1;
	return 0;
}

dump()
{
	mustdump = 0;
	for(h := hubs; h != nil; h = tl h)
		for(i := 1; i <= (hd h).nport; i++)
			sys->fprint(sys->fildes(2), "%s: hub %s port %d: %s",
				usb->argv0, (hd h).dev.dir, i, usb->Ufmt((hd h).port[i].dev));
}

writeinfo(d: ref Usb->Dev)
{
	ud := d.usb;
	s := sys->sprint("info %s csp %#08bux", usb->classname(ud.class), ud.csp);
	for(i := 0; i < ud.nconf; i++){
		c := ud.conf[i];
		if(c == nil)
			break;
		for(j := 0; j < len c.iface; j++){
			ifc := c.iface[j];
			if(ifc == nil)
				break;
			if(ifc.csp != ud.csp)
				s += sys->sprint(" csp %#08bux", ifc.csp);
		}
	}
	s += sys->sprint(" vid %06#x did %06#x", ud.vid, ud.did);
	s += sys->sprint(" %q %q", ud.vendor, ud.product);
	usb->devctl(d, sys->sprint("%s", s));
}

startdev(pp: ref Port): int
{
	ud := pp.dev.usb;

	writeinfo(pp.dev);

	if(ud.class == Usb->Clhub){
		#
		# Hubs are handled directly by this process avoiding
		# concurrent operation so that at most one device
		# has the config address in use.
		# We cancel kernel debug for these eps. too chatty.
		#
		pp.hub = newhub(pp.dev.dir, pp.dev);
		if(pp.hub == nil)
			sys->fprint(sys->fildes(2), "%s: %s: %r\n", usb->argv0, pp.dev.dir);
		else
			sys->fprint(sys->fildes(2), "usb/hub... ");
		if(usb->usbdebug > 1)
			usb->devctl(pp.dev, "debug 0");		# polled hubs are chatty
		if(pp.hub == nil)
			return -1;
		else
			return 0;
	}

	path := searchdriverdatabase(pp.dev);
	if(path != nil) {
		pp.dev.mod = load UsbDriver path;
		if(pp.dev.mod == nil)
			sys->fprint(sys->fildes(2), "%s: failed to load %s\n",
				usb->argv0, path);
		else {
			rv := pp.dev.mod->init(usb, pp.dev);
			if(rv < 0) {
				sys->fprint(sys->fildes(2), "%s: %s: init failed\n", usb->argv0, path);
				pp.dev.mod = nil;
			}
			else
				sys->fprint(sys->fildes(2), "%s running\n", path);
		}
	}
	sys->sleep(Spawndelay);		# in case we re-spawn too fast
	return 0;
}

lines: array of Line;

searchdriverdatabase(d: ref Usb->Dev): string
{
	backtracking := 0;
	level := 0;
	for (i := 0; i < len lines; i++) {
		if(usb->usbdebug > 2)
			sys->fprint(sys->fildes(2),
					"search line %d: lvl %d cmd %s val %d (back %d lvl %d)\n",
					i, lines[i].level, lines[i].command, lines[i].value,
					backtracking, level);
		if (backtracking) {
			if (lines[i].level > level)
				continue;
			backtracking = 0;
		}
		if (lines[i].level != level) {
			level = 0;
			backtracking = 1;
		}
		case lines[i].command {
		"class" =>
			if (d.usb.class != 0) {
				if (lines[i].value != (int d.usb.csp & 16rff))
					backtracking = 1;
			}
			else #if (lines[i].value != (hd conf.iface[0].altiface).class)
				backtracking = 1;
		"subclass" =>
			if (d.usb.class != 0) {
				if (lines[i].value != (int(d.usb.csp >> 8) & 16rff))
					backtracking = 1;
			}
#			else if (lines[i].value != (hd conf.iface[0].altiface).subclass)
#				backtracking = 1;
		"proto" =>
			if (d.usb.class != 0) {
				if (lines[i].value != (int(d.usb.csp >> 16) & 16rff))
					backtracking = 1;
			}
#			else if (lines[i].value != (hd conf.iface[0].altiface).proto)
#				backtracking = 1;
		"vendor" =>
			if (lines[i].value != d.usb.vid)
				backtracking  =1;
		"product" =>
			if (lines[i].value != d.usb.did)
				backtracking  =1;
		"load" =>
			return lines[i].svalue;
		* =>
			continue;
		}
		if (!backtracking)
			level++;
	}
	return nil;
}

loaddriverdatabase()
{
	newlines: array of Line;

	if (bufio == nil)
		bufio = load Bufio Bufio->PATH;

	iob := bufio->open(Usb->DATABASEPATH, Sys->OREAD);
	if (iob == nil) {
		sys->fprint(sys->fildes(2), "usbd: couldn't open %s: %r\n",
			Usb->DATABASEPATH);
		return;
	}
	lines = array[100] of Line;
	lc := 0;
	while ((line := iob.gets('\n')) != nil) {
		if (line[0] == '#')
			continue;
		level := 0;
		while (line[0] == '\t') {
			level++;
			line = line[1:];
		}
		(n, l) := sys->tokenize(line[0: len line - 1], "\t ");
		if (n != 2)
			continue;
		if (lc >= len lines) {
			newlines = array [len lines * 2] of Line;
			newlines[0:] = lines[0: len lines];
			lines = newlines;
		}
		lines[lc].level = level;
		lines[lc].command = hd l;
		case hd l {
		"class" or "subclass" or "proto" or "vendor" or "product" =>
			(lines[lc].value, nil) = usb->strtol(hd tl l, 0);
		"load" =>
			lines[lc].svalue = hd tl l;
		* =>
			continue;
		}
		lc++;
	}
	if(usb->usbdebug)
		usb->dprint(2, sys->sprint("loaded %d lines\n", lc));
	newlines = array [lc] of Line;
	newlines[0:] = lines[0 : lc];
	lines = newlines;
}

