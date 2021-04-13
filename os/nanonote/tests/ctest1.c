#include "mem.h"

extern void fbdraw(unsigned int v);
extern void fbprint(unsigned int v, unsigned int l);

typedef unsigned int Rune;
#define       nelem(x)        (sizeof(x)/sizeof((x)[0]))
#define nil         ((void*)0)

typedef struct {
	Rune	r;
	char	*m;
	void	(*f)(Rune);
	int	i;	/* function called at interrupt time */
} Dbgkey;

static struct {
//	Rendez;
	Dbgkey	*work;
	Dbgkey	keys[50];
	int	nkeys;
	int	on;
} dbg;

static Dbgkey *
finddbgkey(Rune r)
{
	int i;
	Dbgkey *dp;

	for(dp = dbg.keys, i = 0; i < dbg.nkeys; i++, dp++)
		if(dp->r == r)
			return dp;
	return nil;
}

int
strcmp(char *s1, char *s2)
{
        unsigned c1, c2;

        for(;;) {
                c1 = *s1++;
                c2 = *s2++;
                if(c1 != c2) {
                        if(c1 > c2)
                                return 1;
                        return -1;
                }
                if(c1 == 0)
                        return 0;
        }
}

void
debugkey(Rune r, char *msg, void (*fcn)(), int iflag)
{
	Dbgkey *dp;

	if(dbg.nkeys >= nelem(dbg.keys))
		return;
	if(finddbgkey(r) != nil)
		return;
	for(dp = &dbg.keys[dbg.nkeys++] - 1; dp >= dbg.keys; dp--) {
		if(strcmp(dp->m, msg) < 0)
			break;
		dp[1] = dp[0];
	}
	dp++;
	dp->r = r;
	dp->m = msg;
	dp->f = fcn;
	dp->i = iflag;
}

void main(void)
{
    fbdraw(0xff0000);
    fbprint(0x12345678, 0);
}
