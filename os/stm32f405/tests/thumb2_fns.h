/* Implemented in l.s */
extern int getpc(void);
extern int getsp(void);
extern int getsc(void);
extern int getcpsr(void);
extern int getspsr(void);
extern int getapsr(void);
extern int getfpscr(void);

/* Implemented in sys.c */
extern int getcpacr(void);
extern int getcpuid(void);
extern void enablefpu(void);
