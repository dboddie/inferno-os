/*
 *  ILI9341 device file
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

typedef struct {
    QLock;
} ILI9341dev;

static ILI9341dev ili9341lock;

static
Dirtab ili9341tab[]={
    ".",    {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "data", {Qdata, 0},       0, 0222,
};

static void
ili9341init(void)
{
}

static Chan*
ili9341attach(char* spec)
{
    ILI9341_init();
    return devattach('v', spec);
}

static Walkqid*
ili9341walk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, ili9341tab, nelem(ili9341tab), devgen);
}

static int
ili9341stat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, ili9341tab, nelem(ili9341tab), devgen);
}

static Chan*
ili9341open(Chan* c, int omode)
{
    return devopen(c, omode, ili9341tab, nelem(ili9341tab), devgen);
}

static void
ili9341close(Chan* c)
{
    USED(c);
}

static long
ili9341read(Chan* c, void* a, long n, vlong /* offset */)
{
    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, ili9341tab, nelem(ili9341tab), devgen);
    default:
        n=0;
        break;
    }
    return n;
}

static int sent = 0;

static long
ili9341write(Chan* c, void* a, long n, vlong offset)
{
    USED(offset);
    int i = 0;
    uchar *p = (uchar *)a;

    switch((ulong)c->qid.path){
    case Qdata:
        if (sent == 0) {
            ILI9341_start();
            spi_change_cs(0);
        }

        for (; i < n; i++, p++) {
            spi_send_byte(*p);

            sent++;
            if (sent == 153600) {
                sent = 0;
                spi_change_cs(1);
                break;
            }
        }

        break;
    default:
        error(Ebadusefd);
    }
    return n;
}

Dev ili9341devtab = {     /* defaults in dev.c */
    'v',
    "ili9341",

    devreset,
    ili9341init,
    devshutdown,
    ili9341attach,
    ili9341walk,
    ili9341stat,
    ili9341open,
    devcreate,
    ili9341close,
    ili9341read,
    devbread,
    ili9341write,
    devbwrite,
    devremove,
    devwstat,
};
