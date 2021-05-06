/*
 *  MMC/SD (MSC) peripheral device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

enum{
    Qdir,
    Qctl,
    Qdata,
    Qinfo,
};

static
Dirtab msctab[]={
    ".",        {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "ctl",      {Qctl, 0},  0, 0666,
    "data",     {Qdata, 0}, 0, 0666,
    "info",     {Qinfo, 0}, 0, 0444,
};

static void
sdinit(void)
{
    msc_init();
}

static Chan*
sdattach(char* spec)
{
    return devattach('S', spec);
}

static Walkqid*
sdwalk(Chan *c, Chan *nc, char **name, int nname)
{
    return devwalk(c, nc, name, nname, msctab, nelem(msctab), devgen);
}

static int
sdstat(Chan* c, uchar *db, int n)
{
    return devstat(c, db, n, msctab, nelem(msctab), devgen);
}

static Chan*
sdopen(Chan* c, int omode)
{
    return devopen(c, omode, msctab, nelem(msctab), devgen);
}

static void
sdclose(Chan* c)
{
    USED(c);
}

static long
sdread(Chan* c, void* a, long n, vlong offset)
{
    char lbuf[94];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, msctab, nelem(msctab), devgen);
    case Qinfo:
/*        snprint(lbuf, 12, "version %ux\n", (unsigned int)sdhc_host_version(&sdhc0) + 0x10);
        snprint(lbuf + 11, 38, "cid %8.8ux%8.8ux%8.8ux%8.8ux\n",
            sdhc0.cid[3], sdhc0.cid[2], sdhc0.cid[1], sdhc0.cid[0]);
        snprint(lbuf + 48, 38, "csd %8.8ux%8.8ux%8.8ux%8.8ux\n",
            sdhc0.csd[3], sdhc0.csd[2], sdhc0.csd[1], sdhc0.csd[0]);
        snprint(lbuf + 85, 9, "rca %4.4ux", sdhc0.rca);

        return readstr(offset, a, n, lbuf);*/
        error(Eperm);
    case Qctl:
        error(Eperm);
    case Qdata:
        error(Eperm);
    default:
        n=0;
        break;
    }
    return n;
}

static long
sdwrite(Chan* c, void* a, long n, vlong offset)
{
    USED(c, a, n, offset);
    error(Eperm);
    return 0;
}

/* The root (msc) of this structure's name must match the device entry in the
   conf file */
Dev mscdevtab = {     /* defaults in dev.c */
    'S',
    "sd",

    devreset,
    sdinit,
    devshutdown,
    sdattach,
    sdwalk,
    sdstat,
    sdopen,
    devcreate,
    sdclose,
    sdread,
    devbread,
    sdwrite,
    devbwrite,
    devremove,
    devwstat,
};
