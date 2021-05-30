/*
 *  System information device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

void get_cpu_config(char *lbuf, int n);

enum{
    Qdir,
    Qdata,
};

static
Dirtab sysinfotab[]={
    ".",       {Qdir, 0, QTDIR}, 0, 0555, /* entry for "." must be first if devgen used */
    "sysinfo", {Qdata, 0},       0, 0666,
};

static void
sysinfoinit(void)
{
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
    char lbuf[41];

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, sysinfotab, nelem(sysinfotab), devgen);
    case Qdata:
        get_cpu_config(lbuf, 41);
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

/*
 * Show CPU configuration
 */
void get_cpu_config(char *lbuf, int n)
{
    char cpu[7];
    char arch[7];
    char addr[7];
    char rel[2];

    switch (getprid()) {
    case 0x0ad0024f:
        snprint(cpu, 7, "JZ4720"); break;
    case 0x02d0024f:
        snprint(cpu, 7, "JZ4730"); break;
    default:
        snprint(cpu, 7, "?     "); break;
    }

    /* 0x80000483 -> MIPS32r2 (1), TLB (1), cacheable noncoherent (3) */
    ulong v = getconfig();

    switch ((v & CFG_AT) >> 13) {
    case 0:
        snprint(arch, 7, "MIPS32");
        snprint(addr, 7, "32-bit"); break;
    case 1:
        snprint(arch, 7, "MIPS64");
        snprint(addr, 7, "32-bit"); break;
    case 2:
        snprint(arch, 7, "MIPS64");
        snprint(addr, 7, "64-bit"); break;
    default:
        snprint(arch, 7, "MIPS  ");
        snprint(addr, 7, "?     "); break;
    }

    switch ((v & CFG_AR) >> 10) {
    case 0:
        snprint(rel, 2, "1"); break;
    case 1:
        snprint(rel, 2, "2"); break;
    default:
        snprint(rel, 2, "?"); break;
    }

    snprint(lbuf, n, "cpu %s\narch %s\naddr %s\nrel %s", cpu, arch, addr, rel);
}
