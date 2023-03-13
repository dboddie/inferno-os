/*
 *  Winbond W25Q128 flash device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#include "devices/fns.h"

enum{
    Qdir,
    Qdata,
    Qinfo,
};

typedef struct {
    QLock;
} W25Q128dev;

static W25Q128dev w25q128lock;

static
Dirtab w25q128tab[]={
    ".",    {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "data", {Qdata, 0},       0, 0666,
    "info", {Qinfo, 0},       0, 0444,
};

extern void w25q128_test(void);

static void
w25q128init(void)
{
    W25Q128_init();
}

static Chan*
w25q128attach(char* spec)
{
    return devattach('F', spec);
}

static Walkqid*
w25q128walk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, w25q128tab, nelem(w25q128tab), devgen);
}

static int
w25q128stat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, w25q128tab, nelem(w25q128tab), devgen);
}

static Chan*
w25q128open(Chan* c, int omode)
{
    return devopen(c, omode, w25q128tab, nelem(w25q128tab), devgen);
}

static void
w25q128close(Chan* c)
{
    USED(c);
}

static long
w25q128read(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[26];
    int info;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, w25q128tab, nelem(w25q128tab), devgen);
    case Qdata:
        if ((offset < 0) || ((offset + n) > 16*1024*1024)) error(Eio);
        qlock(&w25q128lock);
        W25Q128_read_data((int)offset, (char *)a, n);
        qunlock(&w25q128lock);
        break;
    case Qinfo:
        qlock(&w25q128lock);
        info = W25Q128_get_info();
        qunlock(&w25q128lock);
	snprint(lbuf, 26, "manufacturer=%02x device=%02x", info & 0xff, (info >> 8) & 0xff);
	return readstr(offset, a, n, lbuf);
        break;
    default:
        n=0;
    }
    return n;
}

static long
w25q128write(Chan* c, void* a, long n, vlong offset)
{
    error(Ebadusefd);
    return 0;
}

Dev w25q128devtab = {     /* defaults in dev.c */
    'F',
    "w25q128",

    devreset,
    w25q128init,
    devshutdown,
    w25q128attach,
    w25q128walk,
    w25q128stat,
    w25q128open,
    devcreate,
    w25q128close,
    w25q128read,
    devbread,
    w25q128write,
    devbwrite,
    devremove,
    devwstat,
};
