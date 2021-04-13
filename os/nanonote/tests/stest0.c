#include "mem.h"

extern void fbprint(unsigned int v, unsigned int l);

char*
strchr(char *s, int c)
{
	char c1;

	if(c == 0) {
		while(*s++)
			;
		return s-1;
	}

	while(c1 = *s++)
		if(c1 == c)
			return s-1;
	return 0;
}

long
strlen(char *s)
{

	return strchr(s, 0) - s;
}

void main(void)
{
    fbprint(strlen("test"), 0);
    fbprint(strlen("tst"), 1);
    fbprint(strlen("testy"), 2);
    fbprint(strlen("inferno"), 3);
    for (;;) {}
}
