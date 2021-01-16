/*
 * JZ4720 framebuffer via U-Boot
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include <draw.h>
#include <memdraw.h>
#include <cursor.h>

#include "screen.h"

/* Define a cursor bitmap. */

Cursor  arrow = {
    { -1, -1 },
    { 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C,
      0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04,
      0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04,
      0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40,
    },
    { 0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0,
      0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8,
      0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8,
      0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00,
    },
};

/* Define sizes in pixels of various entities. */
enum {
    Tabstop = 4,
    Scroll  = 8,
    Wid     = 320,
    Ht      = 240,
    Depth   = 32,
};

/* The following two structures are defined in include/memdraw.h: */
static Memdata xgdata = 
{
    0,                  /* allocated data pointer */
    0,                  /* pointer to first byte of actual data; word-aligned */
    0,                  /* number of Memimages using this data */
    0,                  
    0                   /* not malloc'd */
};

static Memimage xgscreen =
{
    { 0, 0, 320, 240 },    /* r */
    { 0, 0, 320, 240 },    /* clipr */
    32,                     /* depth */
    4,                      /* nchan */
    RGB24,                  /* chan (8-8-8 constant from draw.h) */
    nil,                    /* cmap */
    &xgdata,                /* data */
    0,                      /* zero */
    0,                      /* width in words of a single scan line */
    0,                      /* layer */
    0,                      /* flags */
};

/* Declare images for the console text and background colours. These will be
   initialised to images containing the appropriate pixel data for these
   colours. */
static Memimage *conscol;
static Memimage *back;

static Memsubfont *memdefont;

Memimage *gscreen;

static Lock screenlock;

static Point	curpos;
static int	h, w;
static Rectangle window;

static void _screenputs(char *s, int n);
static void screenputc(char *buf);
static void screenwin(void);

/* Initialisation function called by main */

void screeninit(void)
{
    serwrite = 0;

    /* Set the pixel format for the channel used to write data to the screen. */
    memsetchan(&xgscreen, RGB24);

    conf.monitor = 1;
    xgdata.bdata = (uchar*)*(unsigned int *)(LCD_SA0 | KSEG1);
    xgdata.ref	= 1;
    gscreen = &xgscreen;
    gscreen->width = wordsperline(gscreen->r, gscreen->depth);

    memimageinit();
    memdefont = getmemdefont();
    screenwin();

    /* Assign the local _screenputs function to the function pointer used by
       devcons. */
    screenputs = _screenputs;
}

void flushmemscreen(Rectangle)
{
}

uchar* attachscreen(Rectangle *r, ulong *chan, int* d, int *width, int *softscreen)
{
    *r = gscreen->r;
    *d = gscreen->depth;
    *chan = gscreen->chan;
    *width = gscreen->width;
    *softscreen = 0;

    return gscreen->data->bdata;
}

void blankscreen(int)
{
}

/* Local functions */

static void screenwin(void)
{
    char *greet;
    Point p, q;
    Rectangle r;

    /* Assign predefined images for black and white pixels to the background
       and text colours. */
    back = memblack;
    conscol = memwhite;

    w = memdefont->info[' '].width;
    h = memdefont->height;

    window = insetrect(gscreen->r, 0);
    memimagedraw(gscreen, window, memblack, ZP, memopaque, ZP, S);

    curpos = window.min;
}

void getcolor(ulong p, ulong *pr, ulong *pg, ulong *pb)
{
    USED(p, pr, pg, pb);
}

int setcolor(ulong p, ulong r, ulong g, ulong b)
{
    USED(p, r, g, b);
    return 0;
}

static void _screenputs(char *s, int n)
{
    int i;
    Rune r;
    char buf[4];

    if(!islo()) {
        /* don't deadlock trying to print in interrupt */
        if(!canlock(&screenlock))
            return;
    }
    else
        lock(&screenlock);

    while(n > 0){
        i = chartorune(&r, s);
        if(i == 0){
            s++;
            --n;
            continue;
        }
        memmove(buf, s, i);
        buf[i] = 0;
        n -= i;
        s += i;
        screenputc(buf);
    }
    unlock(&screenlock);
}

static void scroll(void)
{
    int o;
    Point p;
    Rectangle r;

    o = Scroll*h;
    r = Rpt(window.min, Pt(window.max.x, window.max.y-o));
    p = Pt(window.min.x, window.min.y+o);
    memimagedraw(gscreen, r, gscreen, p, nil, p, S);
    flushmemscreen(r);
    r = Rpt(Pt(window.min.x, window.max.y-o), window.max);
    memimagedraw(gscreen, r, back, ZP, nil, ZP, S);
    flushmemscreen(r);

    curpos.y -= o;
}

static void screenputc(char *buf)
{
    int w;
    uint pos;
    Point p;
    Rectangle r;
    static int *xp;
    static int xbuf[256];

    if (xp < xbuf || xp >= &xbuf[sizeof(xbuf)])
        xp = xbuf;

    switch (buf[0]) {
    case '\n':
        if (curpos.y + h >= window.max.y)
	        scroll();
        curpos.y += h;
        screenputc("\r");
        break;
    case '\r':
        xp = xbuf;
        curpos.x = window.min.x;
        break;
    case '\t':
        p = memsubfontwidth(memdefont, " ");
        w = p.x;
        if (curpos.x >= window.max.x - Tabstop * w)
	        screenputc("\n");

        pos = (curpos.x - window.min.x) / w;
        pos = Tabstop - pos % Tabstop;
        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x + pos * w, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        flushmemscreen(r);
        curpos.x += pos * w;
        break;
    case '\b':
        if (xp <= xbuf)
	        break;
        xp--;
        r = Rect(*xp, curpos.y, curpos.x, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        flushmemscreen(r);
        curpos.x = *xp;
        break;
    case '\0':
        break;
    default:
        p = memsubfontwidth(memdefont, buf);
        w = p.x;

        if (curpos.x >= window.max.x - w)
	        screenputc("\n");

        *xp++ = curpos.x;
        r = Rect(curpos.x, curpos.y, curpos.x + w, curpos.y + h);
        memimagedraw(gscreen, r, back, back->r.min, nil, back->r.min, S);
        memimagestring(gscreen, curpos, conscol, ZP, memdefont, buf);
        flushmemscreen(r);
        curpos.x += w;
        break;
    }
}

void
cursorenable(void)
{
}

void
cursordisable(void)
{
}
