/*
 *  System information device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"
#include    "devices/teensy41mm.h"

extern char bdata[];

enum{
    Qdir,
    Qdata,
};

static
Dirtab sysinfotab[]={
    ".",       {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "sysinfo", {Qdata, 0},       0, 0666,
};

static void dbgsysinfo(void)
{
    print("etext=%ulx bdata=%ulx edata=%ulx end=%ulx mach=%ulx\n", etext, bdata, edata, end, m);
    print("Total memory available: %ldK\n",conf.npage*BY2PG/1024);
    print("Bank 0: %ld bytes from %ulx to %ulx\n", MACHADDR - conf.base0,
          conf.base0, MACHADDR);
    print("Bank 1: %ld bytes from %ulx to %ulx\n", conf.topofmem - conf.base1,
          conf.base1, conf.topofmem);
}

static void
sysinfoinit(void)
{
    debugkey('s', "system info", dbgsysinfo, 0);
}

static Chan*
sysinfoattach(char* spec)
{
    return devattach('Y', spec);
}

static Walkqid*
sysinfowalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, sysinfotab, nelem(sysinfotab), devgen);
}

static int
sysinfostat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, sysinfotab, nelem(sysinfotab), devgen);
}

static Chan*
sysinfoopen(Chan* c, int omode)
{
    return devopen(c, omode, sysinfotab, nelem(sysinfotab), devgen);
}

static void
sysinfoclose(Chan* c)
{
    USED(c);
}

static long
sysinforead(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[26];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, sysinfotab, nelem(sysinfotab), devgen);
    case Qdata:
        snprint(lbuf, 26, "cpu=%08x fpu=%08x", getcpuid(), getmvfr(0));
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
sysinfowrite(Chan* c, void* a, long n, vlong offset)
{
    USED(c, a, n, offset);
    error(Eperm);
    return 0;
}

Dev sysinfodevtab = {     /* defaults in dev.c */
    'Y',
    "sysinfo",

    devreset,
    sysinfoinit,
    devshutdown,
    sysinfoattach,
    sysinfowalk,
    sysinfostat,
    sysinfoopen,
    devcreate,
    sysinfoclose,
    sysinforead,
    devbread,
    sysinfowrite,
    devbwrite,
    devremove,
    devwstat,
};
