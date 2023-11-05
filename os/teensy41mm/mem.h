#define ROM_START       0x60001080
#define VEC_SIZE        0x2c0

#define HZ      	(100)       /*! clock frequency */
#define MS2HZ       (1000/HZ)   /*! millisec per clock tick */
#define TK2SEC(t)   ((t)/HZ)    /*! ticks to seconds */
#define MS2TK(t)    ((t)/MS2HZ) /*! milliseconds to ticks */

#define KiB             1024u       /*! Kibi 0x0000000000000400 */
#define MiB             1048576u    /*! Mebi 0x0000000000100000 */
#define GiB             1073741824u /*! Gibi 000000000040000000 */

/* #define KZERO           0x10000       */ /*! kernel address space */
#define BY2PG           256                 /*! bytes per page */
#define BI2BY           8                   /*! bits per byte */
#define BI2WD           32                  /* bits per word */
#define BY2WD           4                   /* bytes per word */
#define BY2V            8                   /*! only used in xalloc.c and allocb.c */

#define KTZERO          (ROM_START + VEC_SIZE)  /* kernel text start */

#define ROUND(s,sz)     (((s)+(sz-1))&~(sz-1))
#define PGROUND(s)      ROUND(s, BY2PG)

#define MAXMACH         1

#define CACHELINESZ     32
#define BLOCKALIGN	32

#define KSTKSIZE	4096
#define KSTACK		KSTKSIZE

#define SRAM_BASE       0x20000000
#define SRAM_TOP        0x20080000
#define STACK_TOP       (SRAM_TOP - 4)
/*! Address of Mach structure, assuming a maximum size of 128 bytes. */
#define MACHADDR        (SRAM_TOP - KSTKSIZE - 128)
#define OCRAM2          0x20200000
#define OCRAM2_TOP      0x20280000
#define MEMORY_TOP      OCRAM2_TOP    /* End of memory Inferno can use */

/* Memory map: banks at 0x20000000-0x2007ffff (512KB, DTCM) and
                        0x20200000-0x2027ffff (512KB, OCRAM2)
Bank 0: Stack | Mach | free
Bank 1: Static data | dynamic data | free */

//#define DTCM_BASE       0x20000000
//#define DTCM_MEMORY_TOP 0x20080000
//#define MEMORY_TOP      0x20280000      /* End of memory Inferno can use */
//#define STACK_TOP       (DTCM_BASE + KSTKSIZE - 4)
//#define MACHADDR        (STACK_TOP + 4) /*! Mach structure */

#define MAXRPC (IOHDRSZ+512)
