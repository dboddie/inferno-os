/*
 *  Keyboard device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#define KBD_DEV_CHAR 0x03ba

enum{
    Qdir,
    Qdata,
};

static
Dirtab kbdtab[]={
    ".",         {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "kbd",       {Qdata, 0},       0, 0666,
};

static void
kbdinit(void)
{
}

static Chan*
kbdattach(char* spec)
{
    return devattach(KBD_DEV_CHAR, spec);
}

static Walkqid*
kbdwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, kbdtab, nelem(kbdtab), devgen);
}

static int
kbdstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, kbdtab, nelem(kbdtab), devgen);
}

static Chan*
kbdopen(Chan* c, int omode)
{
    return devopen(c, omode, kbdtab, nelem(kbdtab), devgen);
}

static void
kbdclose(Chan* c)
{
    USED(c);
}

extern void kbd_read_pressed(void);

static long
kbdread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[2];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, kbdtab, nelem(kbdtab), devgen);
    case Qdata:
        kbd_read_pressed();
	snprint(lbuf, 2, "1");
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
kbdwrite(Chan* c, void* a, long n, vlong offset)
{
    USED(a, offset);

    switch((ulong)c->qid.path){
    case Qdata:
        error(Ebadusefd);   /* TODO: allow the LEDs to be changed */
    default:
        error(Ebadusefd);
    }
    return n;
}

Dev kbddevtab = {     /* defaults in dev.c */
    KBD_DEV_CHAR,
    "kbd",

    devreset,
    kbdinit,
    devshutdown,
    kbdattach,
    kbdwalk,
    kbdstat,
    kbdopen,
    devcreate,
    kbdclose,
    kbdread,
    devbread,
    kbdwrite,
    devbwrite,
    devremove,
    devwstat,
};
