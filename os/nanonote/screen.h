typedef struct Cursor Cursor;
typedef struct Cursorinfo Cursorinfo;

#define CURSWID 16
#define CURSHGT 16

struct Cursor {
	Point   offset;
	uchar   clr[CURSWID/BI2BY*CURSHGT];
	uchar   set[CURSWID/BI2BY*CURSHGT];
};

struct Cursorinfo {
	Cursor;
	Lock;
};

/* Required by port/devdraw.c */
void     blankscreen(int);
void     flushmemscreen(Rectangle);
uchar*   attachscreen(Rectangle*, ulong*, int*, int*, int*);
void     getcolor(ulong p, ulong *pr, ulong *pg, ulong *pb);
int      setcolor(ulong p, ulong r, ulong g, ulong b);
