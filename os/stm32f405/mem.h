#define ROM_START       0x08000000
#define VEC_SIZE        0x188

#define HZ      	(100)       /*! clock frequency */
#define MS2HZ       (1000/HZ)   /*! millisec per clock tick */
#define TK2SEC(t)   ((t)/HZ)    /*! ticks to seconds */
#define MS2TK(t)    ((t)/MS2HZ) /*! milliseconds to ticks */

#define KiB             1024u       /*! Kibi 0x0000000000000400 */
#define MiB             1048576u    /*! Mebi 0x0000000000100000 */
#define GiB             1073741824u /*! Gibi 000000000040000000 */

#define KZERO           0x08000000          /*! kernel address space */
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

/* Memory map: banks at 0x10000000-0x10010000 (64KB) and
                        0x20000000-0x20020000 (128KB)
Bank 0: Stack | Mach | free
Bank 1: Static data | dynamic data | free
*/

#define CCM_BASE        0x10000000
#define CCM_MEMORY_TOP  0x10010000
#define MEMORY_TOP      0x20020000      /* End of memory Inferno can use */
#define STACK_TOP       (CCM_BASE + KSTKSIZE - 4)
#define MACHADDR        (STACK_TOP + 4) /*! Mach structure */
