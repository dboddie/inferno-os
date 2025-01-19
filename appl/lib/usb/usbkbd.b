implement UsbDriver;

include "sys.m";
	sys: Sys;
include "usb.m";

Setproto: con 16r0b;
Bootproto: con 0;
KbdCSP: con big 16r010103;

#
# Borrowed from os/pc/kbd.c
#
Spec: con 16r80;
PF: con Spec | 16r20;
View: con Spec | 16r00;
KF: con 16rf000;
Shift: con Spec | 16r60;
Break: con Spec | 16r61;
Ctrl: con Spec | 16r62;
Latin: con Spec | 16r63;
Caps: con Spec | 16r64;
Num: con Spec | 16r65;
Middle: con Spec | 16r66;

Home: con KF | 13;
Up: con KF | 14;
Pgup: con KF | 15;
Print: con KF | 16;
Left: con KF | 17;
Right: con KF | 18;
End: con '\r';
Down: con View;
Pgdown: con KF | 19;
Ins: con KF | 20;
Del: con 16r7f;
Scroll: con KF | 21;

workpid, reppid, tickpid: int;

init(usb:Usb, d: ref Usb->Dev): int
{
	sys = load Sys Sys->PATH;

	ud := d.usb;
	for(i := 0; i < len ud.ep; ++i)
		if(ud.ep[i] != nil
				 && ud.ep[i].etype == Usb->Eintr
				 && ud.ep[i].dir == Usb->Ein
				 && ud.ep[i].iface.csp == KbdCSP)
			break;
	if(i >= len ud.ep){
		sys->fprint(sys->fildes(2), "failed to find keyboard endpoint\n");
		return -1;
	}
	outfd := sys->open("#c/keyboard", Sys->OWRITE);
	r := Usb->Rh2d|Usb->Rclass|Usb->Riface;
	ret := usb->usbcmd(d, r, Setproto, Bootproto, ud.ep[i].id, nil, 0);
	if(ret >= 0){
		kep := usb->openep(d, ud.ep[i].id);
		if(kep == nil){
			sys->fprint(sys->fildes(2), "kb: %s: openep %d: %r\n",
				d.dir, ud.ep[i].id);
			return -1;
		}
		fd := usb->opendevdata(kep, Sys->OREAD);
		if(fd == nil){
			sys->fprint(sys->fildes(2), "kb: %s: opendevdata: %r\n", kep.dir);
			usb->closedev(kep);
			return -1;
		}
		pidc := chan of int;
		spawn kbdwork(pidc, usb, kep, outfd);
		workpid =<- pidc;
		reppid =<- pidc;
		tickpid =<- pidc;
	}
	else
		sys->fprint(sys->fildes(2), "usbcmd failed: %r\n");
	return ret;
}

kill(pid: int): int
{
	fd := sys->open("/prog/"+string pid+"/ctl", Sys->OWRITE);
	if (fd == nil)
		return -1;
	if (sys->write(fd, array of byte "kill", 4) != 4)
		return -1;
	return 0;
}

shutdown()
{
	if(workpid >= 0)
		kill(workpid);
	if(reppid >= 0)
		kill(reppid);
	if(tickpid >= 0)
		kill(tickpid);
}

usagetab := array[] of {
	0, -1, -1, -1, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
	'3', '4', '5', '6', '7', '8', '9', '0', '\n', 16r1b, '\b', '\t', ' ', '-', '=', '[',
	']', '\\', '~', ';', '\'', '`', ',', '.', '/', Caps, KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,
	KF|7, KF|8, KF|9, KF|10, KF|11, KF|12, Print, Scroll,
		Break, Ins, Home, Pgup, Del, End, Pgdown, Right,
	Left, Down, Up, Num, '/', '*', '-', '+', '\n', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0', '.', '\\', -1, -1, '=', KF|13, KF|14, KF|15, KF|16,
		KF|17, KF|18, KF|19, KF|20,
	KF|21, KF|22, KF|23, KF|24,
};

usagetabs := array[] of {
	0, -1, -1, -1, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
	'#', '$', '%', '^', '&', '*', '(', ')', '\n', 16r1b, '\b', '\t', ' ', '_', '+', '{',
	'}', '|', '~', ':', '"', '~', '<', '>', '?', Caps, KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,
	KF|7, KF|8, KF|9, KF|10, KF|11, KF|12, Print, Scroll,
		Break, Ins, Home, Pgup, Del, End, Pgdown, Right,
	Left, Down, Up, Num, '/', '*', '-', '+', '\n', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0', '.', '|',  -1, -1, '=', KF|13, KF|14, KF|15, KF|16,
		KF|17, KF|18, KF|19, KF|20,
	KF|21, KF|22, KF|23, KF|24,
};


kbdwork(pidc: chan of int, usb: Usb, d: ref Usb->Dev, fd: ref Sys->FD)
{
	buf := array[d.maxpkt] of byte;
	buf2 := array[d.maxpkt-2] of byte;
	last := array[d.maxpkt-2] of byte;

	pid := sys->pctl(0, nil);
	pidc <-= pid;

	rch := chan of int;
	spawn repeater(fd, rch);
	pid = <- rch;
	pidc <-= pid;
	pid = <- rch;
	pidc <-= pid;

	while(1){
		n := sys->read(d.dfd, buf, d.maxpkt);
		if(n < 3){
			sys->sleep(100);
			continue;
		}
		if(usb->usbdebug){
			usb->dprint(2, "Got from kbd: ");
			for(i := 0; i < n; ++i)
				sys->fprint(sys->fildes(2), "%02x ", int buf[i]);
			sys->fprint(sys->fildes(2), "\n");
		}
		j := 0;
		ctl := 0;
		for(i := 2; i < n; ++i)
			if(int buf[i] == 16r39)
				ctl = 1;
		for(i = 2; i < n; ++i)
			if(int buf[i] > 0 && int buf[i] < len usagetab && int buf[i] != 16r39){
				for(k := 0; k < d.maxpkt-2 && buf[i] != last[k]; ++k);
				if(k < d.maxpkt-2)
					continue;
				if((int buf[0] & 16r22) == 0)
					buf2[j] = byte usagetab[int buf[i]];
				else
					buf2[j] = byte usagetabs[int buf[i]];
				if(ctl || (int buf[0] & 16r11) != 0)
					buf2[j] &= byte 16r1f;
				++j;
			}
		for(i = 2; i < n; ++i)
			last[i-2] = buf[i];
		if(j > 0)
			sys->write(fd, buf2, j);
		j = -1;
		for(k := 0; k < d.maxpkt-2; ++k)
			if(last[k] != byte 0)
				j = k;
		if(j == -1)
			rch <-= 0;
		else{
			if((int buf[0] & 16r22) == 0)
				down := usagetab[int last[j]];
			else
				down = usagetabs[int last[j]];
			if(ctl || (int buf[0] & 16r11) != 0)
				down &= 16r1f;
			rch <-= down ;
		}
	}
}

repeater(fd: ref Sys->FD, ch: chan of int)
{
	key := array[1] of byte;

	pid := sys->pctl(0, nil);
	ch <-= pid;

	tch := chan of int;
	spawn ticker(tch);
	pid = <- tch;
	ch <-= pid;

	state := 0;
	ticks := 0;
	while(1){
		alt{
		n := <- ch =>
			if(n == 0)
				state = 0;
			else if(byte n != key[0]){
				if(state == 0)
					tch <-= 200;
				state = 1;
				ticks = 0;
			}
			key[0] = byte n;
		<- tch =>
			if(state == 1 && ticks < 3){
				++ticks;
				tch <-= 200;
			}
			else if(state != 0){
				sys->write(fd, key, 1);
				tch <-= 200;
			}
		}
	}
}

ticker(ch: chan of int)
{
	pid := sys->pctl(0, nil);
	ch <-= pid;

	while(1){
		n := <- ch;
		sys->sleep(n);
		ch <-= n;
	}
}
