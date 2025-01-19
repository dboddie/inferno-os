Usb: module
{
	PATH: con "/dis/lib/usb/usb.dis";
	DATABASEPATH: con "/lib/usbdb";

	# fundamental constants
	Nep: con 16;	# max. endpoints per usb device & per interface

	# tunable parameters
	Nconf: con 16;	# max. configurations per usb device
	Nddesc: con 8*Nep; # max. device-specific descriptors per usb device
	Niface: con 16;	# max. interfaces per configuration
	Naltc: con 16;	# max. alt configurations per interface
	Uctries: con 4;	# no. of tries for usbcmd
	Ucdelay: con 50;	# delay before retrying

	Rh2d: con 0<<7;
	Rd2h: con 1<<7;
	Rstd: con 0<<5;
	Rclass: con 1<<5;
	Rvendor: con 2<<5;
	Rdev: con 0;
	Riface: con 1;
	Rep: con 2;
	Rother: con 3;

	# standard requests
	Rgetstatus: con 0;
	Rclearfeature: con 1;
	Rsetfeature: con 3;
	Rsetaddress: con 5;
	Rgetdesc: con 6;
	Rsetdesc: con 7;
	Rgetconf: con 8;
	Rsetconf: con 9;
	Rgetiface: con 10;
	Rsetiface: con 11;
	Rsynchframe: con 12;

	# dev classes
	Clnone: con 0;		# not in usb
	Claudio: con 1;
	Clcomms: con 2;
	Clhid: con 3;
	Clprinter: con 7;
	Clstorage: con 8;
	Clhub: con 9;
	Cldata: con 10;

	# standard descriptor sizes
	Ddevlen: con 18;
	Dconflen: con 9;
	Difacelen: con 9;
	Deplen: con 7;

	# descriptor types
	Ddev: con 1;
	Dconf: con 2;
	Dstr: con 3;
	Diface: con 4;
	Dep: con 5;
	Dreport: con 16r22;
	Dfunction: con 16r24;
	Dphysical: con 16r23;

	# feature selectors
	Fdevremotewakeup: con 1;
	Fhalt: con 0;

	# device state
	Detached,
	Attached,
	Enabled,
	Assigned,
	Configured: con iota;

	# endpoint direction
	Ein, Eout, Eboth: con iota;
	
	GET_STATUS: con 0;
	CLEAR_FEATURE: con 1;
	SET_FEATURE: con 3;
	SET_ADDRESS: con 5;
	GET_DESCRIPTOR: con 6;
	SET_DESCRIPTOR: con 7;
	GET_CONFIGURATION: con 8;
	SET_CONFIGURATION: con 9;
	GET_INTERFACE: con 10;
	SET_INTERFACE: con 11;
	SYNCH_FRAME: con 12;
	
	DEVICE: con 1;
	CONFIGURATION: con 2;
	STRING: con 3;
	INTERFACE: con 4;
	ENDPOINT: con 5;
	HID: con 16r21;
	REPORT: con 16r22;
	PHYSICAL: con 16r23;
	HUB: con 16r29;

	CL_AUDIO: con 1;
	CL_COMMS: con 2;
	CL_HID: con 3;
	CL_PRINTER: con 7;
	CL_MASS: con 8;
	CL_HUB: con 9;
	CL_DATA: con 10;

	PORT_CONNECTION: con 0;
	PORT_ENABLE: con 1;
	PORT_SUSPEND: con 2;
	PORT_OVER_CURRENT: con 3;
	PORT_RESET: con 4;
	PORT_POWER: con 8;
	PORT_LOW_SPEED: con 9;

	Dev: adt {
		dir: string;		# path for the endpoint dir
		id: int;		# USB id for device or ep. number
		dfd: ref Sys->FD;	# descriptor for the data file
		cfd: ref Sys->FD;	# descriptor for the control file
		maxpkt: int;	# cached from usb description
		usb: ref Usbdev;	# USB description
		mod: UsbDriver;
	};

	Usbdev: adt {
		csp: big;		# USB class/subclass/proto
		vid: int;		# vendor id
		did: int;		# product (device id)
		vendor: string;
		product: string;
		serial: string;
		vsid: int;
		psid: int;
		ssid: int;
		class: int;		# from descriptor
		nconf: int;		# from descriptor
		conf: array of ref Conf;	# configurations
		ep: array of ref Ep;	# all endpoints in device
		ddesc: array of ref Desc;	# (raw) device specific descriptors
	};
		
	Ep: adt {
		addr: int;		# endpt address, 0-15 (|16r80 if Ein)
		dir:	int;		# direction Ein/Eout
		etype:	int;	# Econtrol, Eiso, Ebulk, Eintr
		isotype:	int;	# Eunknown, Easync, Eadapt, Esync
		id: int;
		maxpkt: int;	# max. packet size
		ntds: int;		# nb. of Tds per μframe
		conf: cyclic ref Conf;	# the endpoint belongs to
		iface: cyclic ref Iface;	# the endpoint belongs to
	};

	Econtrol, Eiso, Ebulk, Eintr: con iota;	# Endpt.etype
	Eunknown, Easync, Eadapt, Esync: con iota;	# Endpt.isotype
	
	NendPt: con 16;

	Altc: adt {
		attrib: int;
		interval: int;
	};

	Iface: adt {
		id: int;		# interface number
		csp: big;		# USB class/subclass/proto
		altc: array of ref Altc;
		ep: cyclic array of ref Ep;
	};

	Conf: adt {
		cval: int;		# value for set configuration
		attrib: int;
		milliamps: int;	# maximum power in this config
		iface: cyclic array of ref Iface;
	};

	Desc: adt {
		conf: ref Conf;		# where this descriptor was read
		iface: ref Iface;		# last iface before desc in conf.
		ep: ref Ep;			# last endpt before desc in conf.
		altc: ref Altc;		# last alt.c before desc in conf.
		data: ref DDesc;		# unparsed standard USB descriptor
	};

	DDesc: adt {
		bLength: byte;
		bDescriptorType: byte;
		bbytes: array of byte;
		populate: fn(d: self ref DDesc, b: array of byte);
		serialize: fn(d: self ref DDesc): array of byte;
	};

	#
	# layout of standard descriptor types
	#
	DDev: adt {
		bLength: byte;
		bDescriptorType: byte;
		bcdUSB: array of byte;
		bDevClass: byte;
		bDevSubClass: byte;
		bDevProtocol: byte;
		bMaxPacketSize0: byte;
		idVendor: array of byte;
		idProduct: array of byte;
		bcdDev: array of byte;
		iManufacturer: byte;
		iProduct: byte;
		iSerialNumber: byte;
		bNumConfigurations: byte;
		populate: fn(d: self ref DDev, b: array of byte);
		serialize: fn(d: self ref DDev): array of byte;
	};

	DConf: adt {
		bLength: byte;
		bDescriptorType: byte;
		wTotalLength: array of byte;
		bNumInterfaces: byte;
		bConfigurationValue: byte;
		iConfiguration: byte;
		bmAttributes: byte;
		MaxPower: byte;
		populate: fn(d: self ref DConf, b: array of byte);
		serialize: fn(d: self ref DConf): array of byte;
	};
	
	DIface: adt{
		bLength: byte;
		bDescriptorType: byte;
		bInterfaceNumber: byte;
		bAlternateSetting: byte;
		bNumEndpoints: byte;
		bInterfaceClass: byte;
		bInterfaceSubClass: byte;
		bInterfaceProtocol: byte;
		iInterface: byte;
		populate: fn(d: self ref DIface, b: array of byte);
		serialize: fn(d: self ref DIface): array of byte;
	};
	
	DEp: adt {
		bLength: byte;
		bDescriptorType: byte;
		bEndpointAddress: byte;
		bmAttributes: byte;
		wMaxPacketSize: array of byte;
		bInterval: byte;
		populate: fn(d: self ref DEp, b: array of byte);
		serialize: fn(d: self ref DEp): array of byte;
	};
	
		argv0: string;
	usbdebug: int;

	init: fn();
	dprint: fn(n: int, s: string);
	get2: fn(b: array of byte): int;
	put2: fn(buf: array of byte, v: int);
	get4: fn(b: array of byte): int;
	put4: fn(buf: array of byte, v: int);
	bigget2: fn(b: array of byte): int;
	bigput2: fn(buf: array of byte, v: int);
	bigget4: fn(b: array of byte): int;
	bigput4: fn(buf: array of byte, v: int);
	classname: fn(c: int): string;
	closedev: fn(d: ref Dev);
	configdev: fn(d: ref Dev): int;
	devctl: fn(dev: ref Dev, msg: string): int;
	hexstr: fn(a: array of byte, n: int): string;
	loaddevconf: fn(d: ref Dev, n: int): int;
	loaddevdesc: fn(d: ref Dev): int;
	loaddevstr: fn(d: ref Dev, sid: int): string;
	opendev: fn(fname: string): ref Dev;
	opendevdata: fn(d: ref Dev, mode: int): ref Sys->FD;
	openep: fn(d: ref Dev, id: int): ref Dev;
	parseconf: fn(d: ref Usbdev, c: ref Conf, b: array of byte, n: int): int;
	parsedesc: fn(d: ref Usbdev, c: ref Conf, b: array of byte, n: int): int;
	parsedev: fn(xd: ref Dev, b: array of byte, n: int): int;
	unstall: fn(dev: ref Dev, ep: ref Dev, dir: int): int;
	usbcmd: fn(d: ref Dev, typ: int, req: int, value: int, index: int,
		data: array of byte, count: int): int;
	Ufmt: fn(d: ref Dev): string;
	strtol: fn(s:string, base:int): (int, string);
};

UsbDriver: module
{
	init: fn(usb: Usb, dev: ref Usb->Dev): int;
	shutdown: fn();
};
