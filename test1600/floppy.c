#include "h/tests.h"

#define NOTREADY	0x80
#define BUSY		0x01
#define DATAREQ		0x02
#define LOSTDATA	0x04

#define READADR		0xc4
#define READSECT	0x8c
#define WRITESECT	0xac
#define TRACKSIZE	0x1000
#define SECTSIZE	0x100
#define SIDESIZE	0x50000
#define TRACKS		0x50
#define SECTORS		0x10

#define SEEK		0x1c

#define READ		0x00
#define WRITE		0x01

unsigned char ref[256];
unsigned char *orig,*to,*dmaadr,*mapadr;
unsigned int pval;


floppy()
{
    unsigned short i,j,err=0;
    byte side,track,sector;

    out2b(FLOPCTRL,0x0c1d);	/* Start motor, select floppy */
    for (i=0;i<30000;i++) kick;

    lprintf("\nFormatteringskontroll: ");
    for (i=0;i<TRACKS;i++) {
	if (err=getadr(i)) break;
    }
    if (err) {
	lprintf("FEL ****\n");
	switch(err) {
	    case 1:
	        lprintf("Positionering fungerar ej\n");
		break;
	    case 2:
	        lprintf("Felaktig formattering p} skivan eller l{sfel\n");
		break;
	    case 0x10:
	        lprintf("Hittar ej data\n");
		break;
	    default:
	        printf("ERROR\n");
		break;
	}
    }
    else lprintf("OK\n");

    lprintf("\nSkriv- och l{stest:\n\n");
    for (j=0;j<50;j++) {
	printf(" Provomg}ng: %d\r",j+1);
	for (i=0;i<256;i++) ref[i] = (i+j) & 0xff;
	if (err=writesect(0,0,0x01)) break;
	if (err=readsect(0,0,0x01)) break;

	if (err=writesect(0,TRACKS-1,0x08)) break;
	if (err=readsect(0,TRACKS-1,0x08)) break;

	if (err=writesect(1,TRACKS/2,0x03)) break;
	if (err=readsect(1,TRACKS/2,0x03)) break;
    }
    lprintf("\n\nSkriv- och l{stest: ");

    if (err) {
	lprintf("FEL ****\n");
	switch(err) {
	    case 1:
	        lprintf("Positionering fungerar ej\n");
		break;
	    case 2:
	        lprintf("Kommandon fungerar ej\n");
		break;
	    case 3:
	        lprintf("Felaktig data inl{st\n");
		break;
	    default:
	        if (err & 0x40) lprintf("Skivan skrivskyddad\n");
		if (err & 0x20) lprintf("Skrivfel\n");
		if (err & 0x10) lprintf("Hittar ej data\n");
		if (err & 0x08) lprintf("CRC fel\n");
		if (err & 0x04) lprintf("Data f|rlorad\n");
		break;
	}
    }
    else lprintf("OK\n");

    for (i=0;i<30000;i++) kick;
    out2b(FLOPCTRL,0x00);
}




readsect(side,track,sector)
byte track,sector,side;
{
    register unsigned short i,cnt=0;

    seek(track);
    if (statwait(NOTREADY | BUSY)) return(1);
    
    outb(FLOPSECT,sector);
    kick;
    
    flopdma(READ);		/* Set up floppy DMA 0 read disk */
    spcr=0x0f;
    spcr=0x0d;			/* Turn DMA:s on and pray! */
    
    outb(FLOPCOM,READSECT | side);/* L{skommado till kontrollern */

    if (statwait(NOTREADY | BUSY)) return(2);
    kick;

    for (i=0;i<256;i++) {	/* Verify results */
	if (to[i] != ref[i]) return(3);
    }
    kick;

    spcr=0x05;
    spcr=0x07;
    cfree(orig);
    return(inb(FLOPSTAT) & 0x1c);
}




writesect(side,track,sector)
byte track,sector,side;
{
    register unsigned short i,cnt=0;

    seek(track);
    if (statwait(NOTREADY | BUSY)) return(1);

    outb(FLOPSECT,sector);
    kick;

    flopdma(WRITE);
    for (i=0;i<256;i++) {
	to[i]=ref[i];
    }
    kick;

    spcr=0x0f;			/* Start DMA */
    spcr=0x0d;

    outb(FLOPCOM,WRITESECT | side);

    if (statwait(NOTREADY | BUSY)) return(2);

    spcr=0x05;			/* Turn off DMA */
    spcr=0x07;
    cfree(orig);
    return(inb(FLOPSTAT) & 0x78);
}



seek(track)
byte track;
{
    if (statwait(BUSY | NOTREADY)) return(1);
    outb(FLOPDATA,track);
    outb(FLOPCOM,SEEK);

    if (statwait(BUSY)) return(1);/* Wait until complete */
    if (inb(FLOPSTAT) & 0x10) return(1);
    return(0);
}




getadr(track)
byte track;
{
    register unsigned char i;
    register short cnt;
    byte data[256];
    
    seek(track);
    if (statwait(NOTREADY)) return(1);
    outb(FLOPCOM,READADR);
    
    for (i=0;i<6;) {
	if (inb(FLOPSTAT) & DATAREQ) data[i++]=inb(FLOPDATA);
	if (cnt++ > 10000) break;
    }
    if (data[0] != track) return(2);

    kick;
    return(inb(FLOPSTAT) & 0x10);
}




statwait(bit)
byte bit;
{
    int cnt=0;

    while (inb(FLOPSTAT) & bit) {
	kick;
	if (cnt++ > 200000) return(1);
    }
    kick;
    return(0);
}




flopdma(write)
unsigned short write;
{
    static unsigned char dmatab[] = {
	0xc3,0xc3,0xc3,0xc3,0xc3,/* Reset DMA five times */
	0x79,0x00,0x00,0x00,0x01,/* Transfer port B */
	0x54,0x0d,		/* Port A incr & timing */
	0x68,0x0d,		/* Port B I/O fix & timing */
	0x80,			/* DMA disable */
	0xcd,0x06,0x10,		/* Port B address */
	0x92,			/* Setup RDY etc */
        0xcf
    };
    register unsigned short i;
    unsigned short mapto,dmamap;

    dmaadr=BUS0DMA;
    mapadr=BUS0HI;

    orig = to = (unsigned char *) calloc(512,sizeof(unsigned char));
    while ((unsigned int)to & 0xff) to++;

    mapto = (*((unsigned short *)(0x80000+((unsigned int)to & 0xff800)))
             & 0x03ff)>>5;	/* Convert allocated address to */
	     			/* actual physical address in memory */
				/* Then extract the bits needed for */
				/* the dmamap register */

    pval = (unsigned int) to;
    dmatab[6] = pval & 0xff;
    dmatab[7] = pval>>8 & 0xff;

    kick;
 
    if (write) mapto |= 0x80;
    dmamap = mapto | mapto<<8;	/* Set both high and low part */

    out2b(mapadr,dmamap);

    kick;
    for (i=0;i<20;i++) {
	outb(dmaadr,dmatab[i]);
    }

    if (write) {
	outb(dmaadr,0x05);
	outb(dmaadr,0xcf);
	outb(dmaadr,0x87);
    }
    else {
	outb(dmaadr,0x87);
    }
    
    spcr=0x06; spcr=0x07;	/* Disconnect DMA:s */
}
