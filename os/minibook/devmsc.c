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
static ulong *msc_read_scratch;
static ulong *msc_write_scratch;
/* Use locks for the buffers */
static Lock msc_read_lock;
static Lock msc_write_lock;

static long sdread_blocks(void* a, long n, vlong offset)
{
    lock(&msc_read_lock);

    ulong addr = (ulong)a;
    ulong blocklen = mmc_sd->csd.block_len;
    vlong within = offset % blocklen;
    long bytes_read = 0;

    if (within != 0) {
        /* Read the block containing the start of the data. */
        if (msc_read((ulong)(offset / blocklen), msc_read_scratch, 1) != 0) {
            unlock(&msc_read_lock);
            return -1;
        }

        /* Copy at most n bytes. */
        bytes_read = blocklen - within;
        if (n < bytes_read)
            bytes_read = n;

        /* Copy the initial data into the buffer. */
        memcpy((void *)addr, (uchar *)msc_read_scratch + within, bytes_read);
        n -= bytes_read;
        addr += bytes_read;
        offset += bytes_read;
    }

    if (n == 0) {
        unlock(&msc_read_lock);
        return bytes_read;
    }

    /* The offset should be at the start of a block. */
    vlong first_block = offset / blocklen;
    /* The last block contains the last byte to read. */
    vlong last_block = (offset + n - 1) / blocklen;
    /* Calculate the blocks to read, ignoring the last one unless the last byte
       is at the end of a block. */
    uvlong blocks = last_block - first_block;
    if ((n % blocklen) == 0)
        blocks++;

    /* Copy data to non-aligned addresses via a scratch buffer */
    ulong dest;
    if (addr & 0x3)
        dest = (ulong)msc_read_scratch;
    else
        dest = addr;

    /* Copy a whole number of blocks. */
    while (blocks > 0)
    {
        /* Read each block into word-aligned memory */
        if (msc_read(first_block++, (ulong *)dest, 1) != 0) {
            unlock(&msc_read_lock);
            return -1;
        }
        if (addr & 0x3) {
            /* Copy non-aligned data from the buffer */
            memcpy((void *)addr, (void*)dest, blocklen);
        } else
            dest += blocklen;

        addr += blocklen;
        n -= blocklen;
        bytes_read += blocklen;
        blocks--;
    }

    if (n > 0) {
        /* Read a whole block. */
        if (msc_read(last_block, msc_read_scratch, 1) != 0) {
            unlock(&msc_read_lock);
            return -1;
        }

        /* Copy n bytes from the buffer. */
        memcpy((void *)addr, msc_read_scratch, n);
        bytes_read += n;
    }

    unlock(&msc_read_lock);
    return bytes_read;
}

static long sdwrite_blocks(void* a, long n, vlong offset)
{
    lock(&msc_write_lock);

    ulong addr = (ulong)a;
    ulong blocklen = mmc_sd->csd.block_len;
    vlong within = offset % blocklen;
    long bytes_written = 0;

    if (within != 0) {
        /* Read the block containing the start of the data */
        if (msc_read((ulong)(offset / blocklen), msc_write_scratch, 1) != 0) {
            unlock(&msc_write_lock);
            return -1;
        }

        /* Copy at most n bytes */
        bytes_written = blocklen - within;
        if (n < bytes_written)
            bytes_written = n;

        /* Copy the initial data into the buffer then write the buffer back to
           the card */
        memcpy((uchar *)msc_write_scratch + within, (void *)addr, bytes_written);

        if (msc_write((ulong)(offset / blocklen), msc_write_scratch, 1) != 0) {
            unlock(&msc_write_lock);
            return -1;
        }

        n -= bytes_written;
        addr += bytes_written;
        offset += bytes_written;
    }

    if (n == 0) {
        unlock(&msc_write_lock);
        return bytes_written;
    }

    /* The offset should be at the start of a block */
    vlong first_block = offset / blocklen;
    /* The last block contains the last byte to read */
    vlong last_block = (offset + n - 1) / blocklen;
    /* Calculate the blocks to write, ignoring the last one unless the last
       byte is at the end of a block */
    uvlong blocks = last_block - first_block;
    if ((n % blocklen) == 0)
        blocks++;

    /* Copy a whole number of blocks */
    while (blocks > 0)
    {
        ulong dest;
        if (addr & 0x3) {
            /* Copy data from non-aligned addresses via a scratch buffer */
            memcpy(msc_write_scratch, (void *)addr, blocklen);
            dest = (ulong)msc_write_scratch;
        } else
            dest = addr;

        /* Write word-aligned data to the card */
        if (msc_write(first_block++, (ulong *)dest, 1) != 0) {
            unlock(&msc_write_lock);
            return -1;
        }

        addr += blocklen;
        n -= blocklen;
        bytes_written += blocklen;
        blocks--;
    }

    if (n > 0) {
        /* Read the block containing the start of the data */
        if (msc_read(last_block, msc_write_scratch, 1) != 0) {
            unlock(&msc_write_lock);
            return -1;
        }

        /* Copy n bytes into the buffer */
        memcpy(msc_write_scratch, (void *)addr, n);
        bytes_written += n;

        /* Write the whole block back to the card */
        if (msc_write(last_block, msc_write_scratch, 1) != 0) {
            unlock(&msc_write_lock);
            return -1;
        }
    }

    unlock(&msc_write_lock);
    return bytes_written;
}

static void
sdinit(void)
{
    msc_init();
    if (mmc_sd->rca != 0) {
        msc_read_scratch = malloc(mmc_sd->csd.block_len);
        msc_write_scratch = malloc(mmc_sd->csd.block_len);
    }
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
        if (bytes_read < 0)
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
    long bytes_written;

    switch((ulong)c->qid.path)
    {
    case Qdata:
        bytes_written = sdwrite_blocks(a, n, offset);
        if (bytes_written < 0)
            error(Eio);
        break;
    default:
        error(Eperm);
    }
    return bytes_written;
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
