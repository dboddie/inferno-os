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
    Qstatus,
};

typedef struct {
    QLock;
} UC8159dev;

static UC8159dev uc8159lock;

static
Dirtab uc8159tab[]={
    ".",    {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "data", {Qdata, 0},       0, 0222,
    "status", {Qstatus, 0},       0, 0444,
};

extern void UC8159_test(void);

static void
uc8159init(void)
{
    UC8159_init();
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
    case Qstatus:
        qlock(&uc8159lock);
	snprint(lbuf, 2, "%02ux", UC8159_get_status());
        qunlock(&uc8159lock);
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static int sent = 0;

static long
uc8159write(Chan* c, void* a, long n, vlong offset)
{
    USED(offset);
    int i = 0;
    uchar *p = (uchar *)a;

    switch((ulong)c->qid.path){
    case Qdata:
        if (sent == 0)
            UC8159_start();

        for (; i < n; i++, p++) {
            /* Send the two pixels together (highest 4 bits for left pixel). */
            UC8159_send_parameter(*p);

            sent++;
            if (sent == 128000) {
                UC8159_finish();
                sent = 0;
                break;
            }
        }

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
