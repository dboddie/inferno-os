/*
 *  Power device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

enum{
    Qdir,
    Qdata,
};

static
Dirtab powertab[]={
    ".",     {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "power", {Qdata, 0},       0, 0666,
};

static void
powerinit(void)
{
    power_init();
}

static Chan*
powerattach(char* spec)
{
    return devattach(0x2193, spec);
}

static Walkqid*
powerwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, powertab, nelem(powertab), devgen);
}

static int
powerstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, powertab, nelem(powertab), devgen);
}

static Chan*
poweropen(Chan* c, int omode)
{
    return devopen(c, omode, powertab, nelem(powertab), devgen);
}

static void
powerclose(Chan* c)
{
    USED(c);
}

static long
powerread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[2];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, powertab, nelem(powertab), devgen);
    case Qdata:
	snprint(lbuf, 2, "%1d", power_button_pressed());
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
powerwrite(Chan* c, void* a, long n, vlong offset)
{
    USED(c, a, n, offset);
    error(Eperm);
    return 0;
}

Dev powerdevtab = {     /* defaults in dev.c */
    0x2193,
    "power",

    devreset,
    powerinit,
    devshutdown,
    powerattach,
    powerwalk,
    powerstat,
    poweropen,
    devcreate,
    powerclose,
    powerread,
    devbread,
    powerwrite,
    devbwrite,
    devremove,
    devwstat,
};
