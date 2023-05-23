#define ROM_START       0x4000
#define VEC_SIZE        0x264

#define HZ      	(100)       /*! clock frequency */
#define MS2HZ       (1000/HZ)   /*! millisec per clock tick */
#define TK2SEC(t)   ((t)/HZ)    /*! ticks to seconds */
#define MS2TK(t)    ((t)/MS2HZ) /*! milliseconds to ticks */

#define KiB             1024u       /*! Kibi 0x0000000000000400 */
#define MiB             1048576u    /*! Mebi 0x0000000000100000 */
#define GiB             1073741824u /*! Gibi 000000000040000000 */

#define KZERO           0x4000              /*! kernel address space */
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

#define KSTKSIZE	2048
#define KSTACK		KSTKSIZE

/* Memory map: SRAM at 0x20000000-0x20040000 (256KB)
               data | alloc | mach | stack */

#define SRAM_BASE       0x20000000
#define SRAM_TOP        0x20040000
#define STACK_TOP       (SRAM_TOP - 4)
/*! Address of Mach structure, assuming a maximum size of 128 bytes. */
#define MACHADDR        (SRAM_TOP - KSTKSIZE - 128)
#define MEMORY_TOP      MACHADDR      /* End of memory Inferno can use */

#define MAXRPC (IOHDRSZ+512)
