/*
 *  LEDs device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#include "devices/stm32f405.h"

enum{
    Qdir,
    Qdata,
};

static
Dirtab ledstab[]={
    ".",    {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "leds", {Qdata, 0},       0, 0666,
};

static void
ledsinit(void)
{
    setup_led();
}

static Chan*
ledsattach(char* spec)
{
    return devattach('L', spec);
}

static Walkqid*
ledswalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, ledstab, nelem(ledstab), devgen);
}

static int
ledsstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, ledstab, nelem(ledstab), devgen);
}

static Chan*
ledsopen(Chan* c, int omode)
{
    return devopen(c, omode, ledstab, nelem(ledstab), devgen);
}

static void
ledsclose(Chan* c)
{
    USED(c);
}

static long
ledsread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[2];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, ledstab, nelem(ledstab), devgen);
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
ledswrite(Chan* c, void* a, long n, vlong offset)
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

Dev ledsdevtab = {     /* defaults in dev.c */
    'L',
    "leds",

    devreset,
    ledsinit,
    devshutdown,
    ledsattach,
    ledswalk,
    ledsstat,
    ledsopen,
    devcreate,
    ledsclose,
    ledsread,
    devbread,
    ledswrite,
    devbwrite,
    devremove,
    devwstat,
};
