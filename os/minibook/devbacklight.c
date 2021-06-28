/*
 *  Backlight device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

extern void backlight_init(void);
extern ulong backlight_get_brightness(void);
extern void backlight_set_brightness(int brightness);

enum{
    Qdir,
    Qdata,
};

static
Dirtab backlighttab[]={
    ".",         {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "backlight", {Qdata, 0},       0, 0666,
};

static void
backlightinit(void)
{
    backlight_init();
}

static Chan*
backlightattach(char* spec)
{
    return devattach('B', spec);
}

static Walkqid*
backlightwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, backlighttab, nelem(backlighttab), devgen);
}

static int
backlightstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, backlighttab, nelem(backlighttab), devgen);
}

static Chan*
backlightopen(Chan* c, int omode)
{
    return devopen(c, omode, backlighttab, nelem(backlighttab), devgen);
}

static void
backlightclose(Chan* c)
{
    USED(c);
}

static long
backlightread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[4];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, backlighttab, nelem(backlighttab), devgen);
    case Qdata:
	snprint(lbuf, 4, "%lud", backlight_get_brightness());
	return readstr(offset, a, n, lbuf);
    default:
        n=0;
        break;
    }
    return n;
}

static long
backlightwrite(Chan* c, void* a, long n, vlong offset)
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
        if (v < 0 || v > 255)
            error(Ebadarg);

        backlight_set_brightness(v);
	poperror();
        free(cb);
        break;
    default:
        error(Ebadusefd);
    }
    return n;
}

Dev backlightdevtab = {     /* defaults in dev.c */
    'B',
    "backlight",

    devreset,
    backlightinit,
    devshutdown,
    backlightattach,
    backlightwalk,
    backlightstat,
    backlightopen,
    devcreate,
    backlightclose,
    backlightread,
    devbread,
    backlightwrite,
    devbwrite,
    devremove,
    devwstat,
};
