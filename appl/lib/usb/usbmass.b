#
# Adapted from earlier usbmass.b
#
implement UsbDriver;

include "sys.m";
	sys: Sys;
include "usb.m";
	usb: Usb;
	Ep, Rd2h, Rh2d: import Usb;

readerpid, watcherpid: int;
setupfd, ctlfd: ref Sys->FD;
infd, outfd: ref Sys->FD;
inep, outep: ref Ep;
iep, oep: ref Usb->Dev;
cbwseq := 0;
capacity: big;
debug := 0;

lun: int;
blocksize: int;
dev: ref Usb->Dev;

kill(pid: int)
{
	fd := sys->open("#p/"+string pid+"/ctl", Sys->OWRITE);
	if (fd != nil)
		sys->fprint(fd, "kill");
}

watcher(pidc: chan of int, watchc: chan of string)
{
	pid := sys->pctl(0, nil);
	pidc <-= pid;
	while(1){
		sys->sleep(5000);
		watchc <-= "poll";
	}
}

reader(pidc: chan of int, watchc: chan of string, fileio: ref Sys->FileIO)
{
	pid := sys->pctl(0, nil);
	pidc <-= pid;
	for(;;) alt{
	s := <- watchc =>
		if(scsireadcapacity(0) < 0) {
			scsirequestsense(0);
			continue;
		}
		setlength("/chan/usbdisk", capacity);
	(offset, count, nil, rc) := <-fileio.read =>
		if (rc != nil) {
			if (offset%blocksize || count%blocksize) {
				rc <- = (nil, "unaligned read");
				continue;
			}
			offset /= blocksize;
			count /= blocksize;
			buf := array [count * blocksize] of byte;
			if (scsiread10(lun, offset, count, buf) < 0) {
				scsirequestsense(lun);
				rc <- = (nil, "read error");
				continue;
			}
			rc <- = (buf, nil);
		}
	(offset, data, nil, wc) := <-fileio.write =>
		if(wc != nil){
			count := len data;
			if(offset%blocksize || count%blocksize){
				wc <-= (0, "unaligned write");
				continue;
			}
			offset /= blocksize;
			count /= blocksize;
			if(scsiwrite10(lun, offset, count, data) < 0){
				scsirequestsense(lun);
				wc <-= (0, "write error");
				continue;
			}
			wc <-= (len data, nil);
		}
	}
	readerpid = -1;
}

massstoragereset(d: ref Usb->Dev): int
{
	if (usb->usbcmd(d, Usb->Rh2d | Usb->Rclass | Usb->Riface, 255, 0, 0, nil, 0)
			 < 0) {
		sys->print("usbmass: storagereset failed: %r\n");
		return -1;
	}
	return 0;
}
	
getmaxlun(d: ref Usb->Dev): int
{
	buf := array[1] of byte;
	if (usb->usbcmd(d, Usb->Rd2h | Usb->Rclass | Usb->Riface, 254, 0, 0, buf, 1)
			< 0) {
		sys->print("usbmass: getmaxlun failed: %r\n");
		return -1;
	}
	return int buf[0];
}

#
# CBW:
#	sig[4]="USBC" tag[4] datalen[4] flags[1] lun[1] len[1] cmd[len]
#
sendcbw(dtl: int, outdir: int, lun: int, cmd: array of byte): int
{
	cbw := array [31] of byte;
	cbw[0] = byte 'U';
	cbw[1] = byte 'S';
	cbw[2] = byte 'B';
	cbw[3] = byte 'C';
	usb->put4(cbw[4:], ++cbwseq);
	usb->put4(cbw[8:], dtl);
	if (outdir)
		cbw[12] = byte Rh2d;
	else
		cbw[12] = byte Rd2h;
	cbw[13] = byte lun;
	cbw[14] = byte len cmd;
	cbw[15:] = cmd;
	rv := sys->write(outfd, cbw, len cbw);
	if (rv < 0) {
		sys->print("sendcbw: failed: %r\n");
		return -1;
	}
	if (rv != len cbw) {
		sys->print("sendcbw: truncated send\n");
		return -1;
	}
	return 0;
}

#
# CSW:
#	sig[4]="USBS" tag[4] residue[4] status[1]
#

recvcsw(tag: int): (int, int)
{
	if(debug)
		sys->print("recvcsw\n");
	buf := array [13] of byte;
	if (sys->read(infd, buf, len buf) != len buf) {
		sys->print("recvcsw: read failed: %r\n");
		return (-1, -1);
	}
	if (usb->get4(buf) != (('S'<<24)|('B'<<16)|('S'<<8)|'U')) {
		sys->print("recvcsw: signature wrong\n");
		return (-1, -1);
	}
	recvtag := usb->get4(buf[4:]);
	if (recvtag != tag) {
		sys->print("recvcsw: tag does not match: sent %d recved %d\n",
			tag, recvtag);
		return (-1, -1);
	}
	residue := usb->get4(buf[8:]);
	status := int buf[12];
	if(debug)
		sys->print("recvcsw: residue %d status %d\n", residue, status);
	return (residue, status);
}

warnfprint(fd: ref Sys->FD, s: string)
{
	if (sys->fprint(fd, "%s", s) != len s)
		sys->print("warning: writing %s failed: %r\n", s);
}

bulkread(lun: int, cmd: array of byte, buf: array of byte, dump: int): int
{
	if(sendcbw(len buf, 0, lun, cmd) < 0)
		return -1;
	got := 0;
	if(buf != nil) {
		while(got < len buf) {
			rv := sys->read(infd, buf[got:], len buf - got);
			if (rv < 0) {
				sys->print("bulkread: read failed: %r\n");
				break;
			}
			if(debug)
				sys->print("read %d\n", rv);
			got += rv;
			break;
		}
		if(dump) {
			for (i := 0; i < got; i++)
				sys->print("%.2ux", int buf[i]);
			sys->print("\n");
		}
		if(got == 0)
			usb->unstall(dev, iep, Usb->Ein);
	}
	(residue, status) := recvcsw(cbwseq);
	if(residue < 0) {
		usb->unstall(dev, iep, Usb->Ein);
		(residue, status) = recvcsw(cbwseq);
		if(residue < 0)
			return -1;
	}
	if(status != 0)
		return -1;
	return got;
}

bulkwrite(lun: int, cmd: array of byte, buf: array of byte): int
{
	if (sendcbw(len buf, 1, lun, cmd) < 0)
		return -1;
	got := 0;
	if (buf != nil) {
		while (got < len buf) {
			rv := sys->write(outfd, buf[got:], len buf - got);
			if (rv < 0) {
				sys->print("bulkwrite: write failed: %r\n");
				break;
			}
			if(debug)
				sys->print("write %d\n", rv);
			got += rv;
			break;
		}
		if (got == 0)
			usb->unstall(dev, oep, Usb->Eout);
	}
	(residue, status) := recvcsw(cbwseq);
	if (residue < 0) {
		usb->unstall(dev, oep, Usb->Eout);
		(residue, status) = recvcsw(cbwseq);
		if (residue < 0)
			return -1;
	}
	if (status != 0)
		return -1;
	return got;
}

scsiinquiry(lun: int): int
{
	buf := array [36] of byte;	# don't use 255, many devices can't cope
	cmd := array [6] of byte;
	cmd[0] = byte 16r12;
	cmd[1] = 	byte (lun << 5);
	cmd[2] = byte 0;
	cmd[3] = byte 0;
	cmd[4] = byte len buf;
	cmd[5]  = byte 0;
	got := bulkread(lun, cmd, buf, 0);
	if (got < 0)
		return -1;
	if (got < 36) {
		sys->print("scsiinquiry: too little data\n");
		return -1;
	}
	t := int buf[0] & 16r1f;
	if(debug)
		sys->print("scsiinquiry: type %d/%s\n", t, string buf[8:35]);
	if (t != 0 && t != 5)
		return -1;
	return t;
}

scsireadcapacity(lun: int): int
{
#	warnfprint(ctlfd, "debug 0 1");
#	warnfprint(ctlfd, "debug 1 1");
#	warnfprint(ctlfd, "debug 2 1");
	buf := array [8] of byte;
	cmd := array [10] of byte;
	cmd[0] = byte 16r25;
	cmd[1] = 	byte (lun << 5);
	cmd[2] = byte 0;
	cmd[3] = byte 0;
	cmd[4] = byte 0;
	cmd[5]  = byte 0;
	cmd[6]  = byte 0;
	cmd[7]  = byte 0;
	cmd[8]  = byte 0;
	cmd[9] = byte 0;
	got := bulkread(lun, cmd, buf, 0);
	if (got < 0){
#		sys->print("scsireadcapacity: buldread failed: %r\n");
		return -1;
	}
	if (got != len buf) {
		sys->print("scsireadcapacity: returned data not right size\n");
		return -1;
	}
	blocksize = usb->bigget4(buf[4:]);
	lba := big usb->bigget4(buf[0:]) & 16rFFFFFFFF;
	capacity = big blocksize * (lba+big 1);
	if(debug)
		sys->print("block size %d lba %bd cap %bd\n", blocksize, lba, capacity);
	return 0;
}

scsirequestsense(lun: int): int
{
#	warnfprint(ctlfd, "debug 0 1");
#	warnfprint(ctlfd, "debug 1 1");
#	warnfprint(ctlfd, "debug 2 1");
	buf := array [18] of byte;
	cmd := array [6] of byte;
	cmd[0] = byte 16r03;
	cmd[1] = 	byte (lun << 5);
	cmd[2] = byte 0;
	cmd[3] = byte 0;
	cmd[4] = byte len buf;
	cmd[5]  = byte 0;
	got := bulkread(lun, cmd, buf, 0);
	if (got < 0)
		return -1;
	return 0;
}

scsiread10(lun: int, offset, count: int, buf: array of byte): int
{
	cmd := array [10] of byte;
	cmd[0] = byte 16r28;
	cmd[1] = byte (lun << 5);
	usb->bigput4(cmd[2:], offset);
	cmd[6] = byte 0;
	usb->bigput2(cmd[7:], count);
	cmd[9] = byte 0;
	got := bulkread(lun, cmd, buf, 0);
	if (got < 0)
		return -1;
	return 0;
}

scsiwrite10(lun: int, offset, count: int, buf: array of byte): int
{
	cmd := array [10] of byte;
	cmd[0] = byte 16r2A;
	cmd[1] = byte (lun << 5);
	usb->bigput4(cmd[2:], offset);
	cmd[6] = byte 0;
	usb->bigput2(cmd[7:], count);
	cmd[9] = byte 0;
	got := bulkwrite(lun, cmd, buf);
	if (got < 0)
		return -1;
	return 0;
}

scsistartunit(lun: int, start: int): int
{
#	warnfprint(ctlfd, "debug 0 1");
#	warnfprint(ctlfd, "debug 1 1");
#	warnfprint(ctlfd, "debug 2 1");
	cmd := array [6] of byte;
	cmd[0] = byte 16r1b;
	cmd[1] = byte (lun << 5);
	cmd[2] = byte 0;
	cmd[3] = byte 0;
	cmd[4] = byte (start & 1);
	cmd[5]  = byte 0;
	got := bulkread(lun, cmd, nil, 0);
	if (got < 0)
		return -1;
	return 0;
}

init(usbmod: Usb, d: ref Usb->Dev): int
{
	usb = usbmod;
	dev = d;

	sys = load Sys Sys->PATH;

	rv := massstoragereset(d);
	if (rv < 0)
		return rv;
	maxlun := getmaxlun(d);
	if (maxlun < 0)
		return maxlun;
	lun = 0;
	debug = usb->usbdebug;
	if(debug)
		sys->print("maxlun %d\n", maxlun);
	ud := d.usb;
	inep = outep = nil;
	for(i := 0; i < len ud.ep; ++i)
		if(ud.ep[i] != nil && ud.ep[i].etype == Usb->Ebulk){
			if(ud.ep[i].dir == Usb->Ein){
				if(inep == nil)
					inep = ud.ep[i];
			}else{
				if(outep == nil)
					outep = ud.ep[i];
			}
		}
	if(inep == nil || outep == nil){
		sys->print("can't find endpoints\n");
		return -1;
	}
	isrw := (inep.addr & 16rF) == (outep.addr & 16rF);
	if(!isrw){
		iep = usb->openep(d, inep.id);
		if(iep == nil){
			sys->fprint(sys->fildes(2), "mass: %s: openp %d: %r\n",
				d.dir, inep.id);
			return -1;
		}
		infd = usb->opendevdata(iep, Sys->OREAD);
		if(infd == nil){
			sys->fprint(sys->fildes(2), "mass: %s: opendevdata: %r\n", iep.dir);
			usb->closedev(iep);
			return -1;
		}
		oep = usb->openep(d, outep.id);
		if(oep == nil){
			sys->fprint(sys->fildes(2), "mass: %s: openp %d: %r\n",
				d.dir, outep.id);
			return -1;
		}
		outfd = usb->opendevdata(oep, Sys->OWRITE);
		if(outfd == nil){
			sys->fprint(sys->fildes(2), "mass: %s: opendevdata: %r\n", oep.dir);
			usb->closedev(oep);
			return -1;
		}
	}
	else{
		oep = iep = usb->openep(d, inep.id);
		if(iep == nil){
			sys->fprint(sys->fildes(2), "mass: %s: openp %d: %r\n",
				d.dir, inep.id);
			return -1;
		}
		outfd = infd = usb->opendevdata(iep, Sys->ORDWR);
		if(infd == nil){
			sys->fprint(sys->fildes(2), "mass: %s: opendevdata: %r\n", iep.dir);
			usb->closedev(iep);
			return -1;
		}
	}
	if((t := scsiinquiry(0)) < 0){
		sys->print("usbmass: scsiinquiry failed: %r\n");
		return -1;
	}
	scsistartunit(lun, 1);
	if(scsireadcapacity(0) < 0) {
		scsirequestsense(0);
		if(scsireadcapacity(0) < 0 && t != 5){
			sys->print("usbmass: scsireadcapacity failed: %r\n");
			return -1;
		}
	}
	fileio := sys->file2chan("/chan", "usbdisk");
	if (fileio == nil) {
		sys->print("file2chan failed: %r\n");
		return -1;
	}
	watchc := chan of string;
	if(t == 5){
		wpidc := chan of int;
		spawn watcher(wpidc, watchc);
		watcherpid = <- wpidc;
	}
	setlength("/chan/usbdisk", capacity);
#	warnfprint(ctlfd, "debug 0 1");
#	warnfprint(ctlfd, "debug 1 1");
#	warnfprint(ctlfd, "debug 2 1");
	pidc := chan of int;
	spawn reader(pidc, watchc, fileio);
	readerpid = <- pidc;
	return 0;
}

shutdown()
{
	if(watcherpid >= 0)
		kill(watcherpid);
	if(readerpid >= 0)
		kill(readerpid);
}

setlength(f: string, size: big)
{
	d := sys->nulldir;
	d.length = size;
	sys->wstat(f, d);	# ignore errors since it fails on older kernels
}
