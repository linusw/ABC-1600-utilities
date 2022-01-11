#include "h/tests.h"

mover()
{
    static unsigned short p[] = {
	0x0000,
	0xfff0,
	0xfff8,
	0x7018,
	0x701c,
	0x301c,
	0x301c,
	0x3018,
	0x7ff8,
	0x7ff0,
	0x3000,
	0x3000,
	0x3000,
	0x7000,
	0x7800,
	0xf800
    };
    static unsigned short dots[] = {
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333,
	0xcccc,
	0x3333
    };
    static unsigned short bricks[] = {
	0x0101,
	0x0101,
	0x01ff,
	0x0101,
	0xff01,
	0x0101,
	0x0101,
	0x01ff,
	0x0101,
	0xff01,
	0x0101,
	0x0101,
	0x01ff,
	0x0101,
	0xff01,
	0x0101
    };
    cls();
    box(0,0,760,1023,bricks);
    box(100,100,200,200,dots);
    box(500,500,200,400,p);
    box(100,800,500,200,dots);
    box(20,400,400,100,p);
    box(400,200,150,150,p);
    box(200,475,500,150,dots);
    query("");
}




/****************************************************************\
* 								 *
*  BOX fills a box on screen with the specified pattern        	 *
* 								 *
\****************************************************************/
box(x,y,xlen,ylen,pattern)
unsigned short x,y,xlen,ylen;
unsigned short *pattern;
{
    register unsigned short i,j,*scr;
    int dump;

    scr = ((unsigned short *) SCREEN);
    wait();
    for (i=0;i<16;i++) {
	*scr = *pattern++;
	scr += 64;
    }
    dump=kick;

    flags=0x20;
    outw(0x0c00|xlen,SIZE);
    outw(16,SIZE+2);
    outw(x,TO);
    outw(y,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    flags=0x00;
    outw(0x0c00|xlen,SIZE);
    outw(ylen-16,SIZE+2);
    outw(x,TO);
    outw(y+16,TO+2);
    outw(x,FROM);
    outw(y,FROM+2);
    dump=kick;
}


    
