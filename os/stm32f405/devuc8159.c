/*
 *  UC8159 device file
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
};

static
Dirtab uc8159tab[]={
    ".",    {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "uc8159", {Qdata, 0},       0, 0666,
};

extern void UC8159_test(void);

static void
uc8159init(void)
{
    UC8159_init();
    UC8159_test();
}

static Chan*
uc8159attach(char* spec)
{
    return devattach('u', spec);
}

static Walkqid*
uc8159walk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, uc8159tab, nelem(uc8159tab), devgen);
}

static int
uc8159stat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, uc8159tab, nelem(uc8159tab), devgen);
}

static Chan*
uc8159open(Chan* c, int omode)
{
    return devopen(c, omode, uc8159tab, nelem(uc8159tab), devgen);
}

static void
uc8159close(Chan* c)
{
    USED(c);
}

static long
uc8159read(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[2];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, uc8159tab, nelem(uc8159tab), devgen);
    case Qdata:
	snprint(lbuf, 2, "%1d", get_led());
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
uc8159write(Chan* c, void* a, long n, vlong offset)
{
    USED(a, offset);
    Cmdbuf *cb;

    switch((ulong)c->qid.path){
    case Qdata:
        if(offset != 0)
            error(Ebadarg);

        cb = parsecmd(a, n);
        /* Add an error handling block. */
        if (waserror()) {
            free(cb);
            nexterror();
        }
        if (cb->nf != 1)
            error(Ebadarg); /* Report an error using the error handler. */

        int v = atoi(cb->f[0]);
        if (v < 0 || v > 1)
            error(Ebadarg);

        set_led(v);

	poperror();
        free(cb);
        break;
    default:
        error(Ebadusefd);
    }
    return n;
}

Dev uc8159devtab = {     /* defaults in dev.c */
    'u',
    "uc8159",

    devreset,
    uc8159init,
    devshutdown,
    uc8159attach,
    uc8159walk,
    uc8159stat,
    uc8159open,
    devcreate,
    uc8159close,
    uc8159read,
    devbread,
    uc8159write,
    devbwrite,
    devremove,
    devwstat,
};
