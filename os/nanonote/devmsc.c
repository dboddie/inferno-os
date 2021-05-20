/*
 *  MMC/SD (MSC) peripheral device file
 */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"
#include    "peripherals/mmc.h"

extern void* memcpy(void*, void*, ulong);

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

extern MMC *mmc_sd;
static char *msc_scratch;

static long sdread_blocks(void* a, long n, vlong offset)
{
    ulong blocklen = mmc_sd->csd.block_len;
    vlong within = offset % blocklen;
    long bytes_read = 0;

    if (within != 0) {
        /* Read the block containing the start of the data. */
        if (msc_read((ulong)(offset / blocklen), (ulong *)msc_scratch, 1) != 0)
            return -1;

        /* Copy at most n bytes. */
        bytes_read = blocklen - within;
        if (n < bytes_read)
            bytes_read = n;

        /* Copy the initial data into the buffer. */
        memcpy(a, msc_scratch + within, bytes_read);
        n -= bytes_read;
        a = (void *)((ulong)a + bytes_read);
        offset += bytes_read;
    }

    if (n == 0)
        return bytes_read;

    /* The offset should be at the start of a block. */
    vlong first_block = offset / blocklen;
    /* The last block contains the last byte to read. */
    vlong last_block = (offset + n - 1) / blocklen;
    /* Calculate the blocks to read, ignoring the last one unless the last byte
       is at the end of a block. */
    uvlong blocks = last_block - first_block;
    if ((n % blocklen) == 0)
        blocks++;

    /* Copy a whole number of blocks. */
    while (blocks > 0) {
        if (msc_read(first_block++, a, 1) != 0)
            return -1;
        a = (void *)((uint)a + blocklen);
        n -= blocklen;
        bytes_read += blocklen;
        blocks--;
    }

    if (n > 0) {
        /* Read a whole block. */
        if (msc_read(last_block, (ulong *)msc_scratch, 1) != 0)
            return -1;

        /* Copy n bytes into the buffer. */
        memcpy(a, msc_scratch, n);
        bytes_read += n;
    }

    return bytes_read;
}

static void
sdinit(void)
{
    msc_init();
    if (mmc_sd->rca != 0)
        msc_scratch = malloc(mmc_sd->csd.block_len);
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
    long bytes_read;

    switch((ulong)c->qid.path){
    case Qdir:
        return devdirread(c, a, n, msctab, nelem(msctab), devgen);
    case Qinfo:
        snprint(lbuf, 9, "rca %4.4ux", mmc_sd->rca);
        return readstr(offset, a, n, lbuf);
    case Qctl:
        error(Eperm);
    case Qdata:
        bytes_read = sdread_blocks(a, n, offset);
        if (bytes_read == -1)
            error(Eio);
        return bytes_read;
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
