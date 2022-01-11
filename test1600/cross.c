#include "h/tests.h"
patts()
{
    cls();
    crossp();
    diagonal();
    query("");
}




/*********************************************************************\
* 								      *
*  CROSSP generates a box on the screen                               *
* 								      *
\*********************************************************************/
crossp()
{
    register unsigned short i,j,*scr;
    int dump;

    scr = ((unsigned short *) SCREEN);

    wait();
    *scr=0xffff;
    flags=0x30;
    outw(0x0c00|767,SIZE);
    outw(0,SIZE+2);
    outw(0,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    outw(0,TO);
    outw(512,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    outw(0,TO);
    outw(1023,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    outw(0x0c00|0,SIZE);
    outw(1023,SIZE+2);
    outw(0,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;
    
    wait();
    outw(384,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    outw(767,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

}




/*********************************************************************\
* 								      *
*  DIAGONAL generates a diagonal cross on the screen                  *
* 								      *
\*********************************************************************/
diagonal()
{
    register unsigned short i,j,*scr;
    int dump;
    
    scr = ((unsigned short *) SCREEN);
    wait();
    flags=0x30;
    outw(0x0c00,SIZE);
    outw(0x0000,SIZE+2);
    for (i=0,j=0;i<4095;i+=4,j+=3) {
	wait();
	outw(j>>2,TO);
	outw(i>>2,TO+2);
	outw(0,FROM);
	outw(0,FROM+2);
    }
    dump=kick;

    for (i=0,j=3071;i<4095;i+=4,j-=3) {
	wait();
	outw(j>>2,TO);
	outw(i>>2,TO+2);
	outw(0,FROM);
	outw(0,FROM+2);
    }
    dump=kick;
}
