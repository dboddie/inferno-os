/*
 *  USB peripheral device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"
#include    "hardware.h"

enum{
    Qdir,
    Qin,
    Qout,
    Qinfo
};

static
Dirtab usbtab[]={
    ".",        {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "in",       {Qin, 0},         0, 0222,
    "out",      {Qout, 0},        0, 0444,
    "info",     {Qinfo, 0},       0, 0444,
};

static void
usbinit(void)
{
    usb_init();
}

static Chan*
usbattach(char* spec)
{
    return devattach('u', spec);
}

static Walkqid*
usbwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, usbtab, nelem(usbtab), devgen);
}

static int
usbstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, usbtab, nelem(usbtab), devgen);
}

static Chan*
usbopen(Chan* c, int omode)
{
    return devopen(c, omode, usbtab, nelem(usbtab), devgen);
}

static void
usbclose(Chan* c)
{
    USED(c);
}

static long
usbread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[44];
    long bytes_read;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, usbtab, nelem(usbtab), devgen);
    case Qin:
        error(Eperm);
        break;
    case Qout:
        /* Data from the host */
        bytes_read = usb_read(a, n, offset);
        if (bytes_read == -1)
            error(Eio);
        return bytes_read;
    case Qinfo:
	usb_info(lbuf, 44);
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
usbwrite(Chan* c, void* a, long n, vlong offset)
{
    switch((ulong)c->qid.path){
    case Qin:
        /* Data to the host */
        return usb_write(a, n, offset);
    default:
        error(Eperm);
        break;
    }
    return n;
}

Dev usbdevtab = {     /* defaults in dev.c */
    'u',
    "usb",

    devreset,
    usbinit,
    devshutdown,
    usbattach,
    usbwalk,
    usbstat,
    usbopen,
    devcreate,
    usbclose,
    usbread,
    devbread,
    usbwrite,
    devbwrite,
    devremove,
    devwstat,
};
