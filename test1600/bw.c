#include "h/tests.h"

bw()
{
    bwp();
    query("");
}



bwp()
{
    register unsigned short *scr;
    int dump;

    scr = ((unsigned short *) SCREEN);

    wait();
    *scr=0xffff;
    flags=0x30;
    outw(0x0c00|384,SIZE);
    outw(1024,SIZE+2);
    outw(0,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;

    wait();
    scr += 24;
    *scr=0x0000;
    flags=0x30;
    outw(0x0c00|384,SIZE);
    outw(1024,SIZE+2);
    outw(384,TO);
    outw(0,TO+2);
    outw(384,FROM);
    outw(0,FROM+2);
    dump=kick;
}
