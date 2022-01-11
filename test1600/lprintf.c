#include "h/tests.h"

extern int pron;

lprintf(format,list)
register char *format;
unsigned list;
{
    register c;
    register unsigned int *adrlist;
    char *s;

    adrlist = &list;
    while(1) {
	kick;
	while((c = *format++) != '%') {
	    if (c=='\0') return;
	    putchar(c);
	    if (pron) prwr(c);
	}
	c = *format++;
	if (c=='d') numpr((int)*adrlist,10);
	else if (c=='X' || c=='x') numpr((int)*adrlist,16);
	else if (c=='s') {
	    s=(char *)*adrlist;
	    while(c = *s++) {
		putchar(c);
		if (pron) prwr(c);
	    }
	}
	adrlist++;
    }
}


char *chars="0123456789ABCDEF";

numpr(x,base)
int x,base;
{
    int div;

    if (div=x/base) numpr(div,base);
    putchar(chars[x%base]);
    if (pron) prwr(chars[x%base]);
}

