#include "h/tests.h"
#define BUSY	4
#define NO	0
#define YES	4
#define REQUEST	0x0f
#define OK	0x0d
#define READ	0x07
#define ERROR   0x05

unsigned char newbuff[512],oldbuff[512];



winch()
{
    short errtype;
    lprintf("\nKontroller minnestest: ");
    if (errtype=ramtest()) {
	lprintf("FEL ****\n");
	errprintf(errtype);
    }
    else {
	lprintf("OK\n");
	errprintf(0);
    }

    lprintf("\nKontroller sj{lvtest: ");
    if (errtype=controltest()) {
	lprintf("FEL ****\n");
	errprintf(errtype);
    }
    else {
	lprintf("OK\n");
	errprintf(0);
    }

    lprintf("\nDrivetest:\n");
    if (errtype=drivetest()) {
	lprintf("Drivetest: FEL ****\n");
	errprintf(errtype);
    }
    else {
	lprintf("Drivetest: OK\n");
	errprintf(0);
    }

    lprintf("\nL{stest:\n");
    if (errtype=(readtest(0) | readtest(20000) | readtest(10000))) {
	lprintf("L{stest: FEL ****\n");
	errprintf(errtype);
    }
    else {
	lprintf("L{stest: OK\n");
	errprintf(0);
    }

    lprintf("\nDMA-L{stest:\n");
    if (errtype=(dmatest(0) | dmatest(20000) | dmatest(10000))) {
	lprintf("DMA-L{stest: FEL ****\n");
	errprintf(errtype);
    }
    else {
	lprintf("DMA-L{stest: OK\n");
	errprintf(0);
    }
}




errprintf(errtype)
short errtype;
{
    static unsigned char errseq[] = {
	0x03,0x00,0x00,0x00,0x00,0x00
    };
    register short i;
    unsigned short dummy;
    
    while (!sasistat(REQUEST,ERROR)) dummy=inb(SASIDATA);
    kick;
    while (!sasistat(REQUEST,READ)) dummy=inb(SASIDATA);
    kick;
    
    if (errtype==1) {
	lprintf("Kontrollern kunde inte selekteras\n");
	return;
    }
    
    if (errtype==256) {
	lprintf("Felaktig data vid j{mf|relseprov\n");
	return;
    }

    if (errtype==512) {
	lprintf("Felaktig data vid j{mf|relseprov med DMA\n");
	return;
    }

    if (activate()) {
	lprintf("Kan inte selektera kontrollern\n");
	return;
    }

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) {
	    lprintf("Kan inte s{nda till kontrollern\n");
	    return;
	}
	outb(SASIDATA,errseq[i]);
    }

    if (sasiwait(REQUEST,READ)) {
	lprintf("Kontrollern svarar inte\n");
	return;
    }
    
    errtype=inb(SASIDATA);
    for (i=0;i<3;i++) {
	sasiwait(REQUEST,READ);
	dummy=inb(SASIDATA);
    }
    
    switch (errtype>>4 & 0x03) {
	case 0:
	    switch(errtype & 0x0f) {
		case 0:
		    lprintf("Inget fel uppt{ckt\n");
		    break;
		case 1:
		    lprintf("Ingen indexpuls\n");
		    break;
		case 2:
		    lprintf("Ingen seek-complete\n");
		    break;
		case 3:
		    lprintf("Skrivfel\n");
		    break;
		case 4:
		    lprintf("Ingen READY-signal\n");
		    break;
		case 6:
		    lprintf("Hittar ej sp}r 00\n");
		    break;
		default:
		    lprintf("Odefinierat fel\n");
		    break;
	    }
	    break;
	case 1:
	    switch (errtype & 0x0f) {
		case 0:
		    lprintf("ID L{sfel\n");
		    break;
		case 1:
		    lprintf("Datafel\n");
		    break;
		case 2:
		    lprintf("Adressm{rke saknas\n");
		    break;
		case 4:
		    lprintf("Hittar ej sektorn\n");
		    break;
		case 5:
		    lprintf("S|kfel\n");
		    break;
		case 8:
		    lprintf("Korrigerbart datafel\n");
		    break;
		case 9:
		    lprintf("D}ligt sp}r\n");
		    break;
		case 10:
		    lprintf("Formatteringsfel\n");
		    break;
		default:
		    lprintf("Odefinierat fel\n");
		    break;
	    }
	    break;
	case 2:
	    switch (errtype & 0x0f) {
		case 0:
		    lprintf("Felaktigt kommando\n");
		    break;
		case 1:
		    lprintf("Felaktig diskadress\n");
		    break;
		default:
		    lprintf("Odefinierat fel\n");
		    break;
	    }
	    break;
	case 3:
	    switch (errtype & 0x0f) {
		case 0:
		    lprintf("Kontroller RAM-fel\n");
		    break;
		case 1:
		    lprintf("Checksummefel i kontrollern\n");
		    break;
		case 2:
		    lprintf("ECC generatorn fungerar ej\n");
		    break;
		default:
		    lprintf("Odefinierat fel\n");
		    break;
	    }
	    break;
	default:
	    lprintf("Odefinerad felmod\n");
	    break;
    }
    kick;
    sasiwait(REQUEST,ERROR);
    dummy=inb(SASIDATA);
    return;
}    




readtest(adr)
int adr;
{
    register short i,j;
    short errtype;

    if (errtype=winchread(adr,1)) return(errtype);
    for (i=0;i<512;i++) oldbuff[i]=newbuff[i];

    printf("L{sning......\n");
    for (j=0;j<1000;j++) {
	printf(" Kontrollomg}ng: %d\r",j);
	if (errtype=winchread(adr,1)) return(errtype);
	for (i=0;i<512;i++) {
	    if (newbuff[i]!=oldbuff[i]) return(256);
	}
    }
    printf("\nPositionering.....\n");
    for (j=0;j<100;j++) {
	if (recal()) return(2);
	printf(" Kontrollomg}ng: %d\r",j);
	if (errtype=winchread(adr,1)) return(errtype);
	for (i=0;i<512;i++) {
	    if (newbuff[i]!=oldbuff[i]) return(256);
	}
    }
    printf("\n");
    return(0);
}

    


winchread(address,sect_cnt)
int address;
int sect_cnt;
{
    static unsigned char readseq[6] = {
	0x08,0x00,0x00,0x00,0x00,0x04
    };
    register short i;

    readseq[1]=address>>16 & 0x1f;
    readseq[2]=address>>8 & 0xff;
    readseq[3]=address & 0xff;
    readseq[4]=sect_cnt & 0xff;

    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	outb(SASIDATA,readseq[i]);
    }
    
    if (sasiwait(REQUEST,READ)) return(2);

    for (i=0;i<512;i++) {
	newbuff[i]=inb(SASIDATA);
    }
    kick;

    if (sasiwait(REQUEST,ERROR)) return(2);

    return(inb(SASIDATA));
}




recal()
{
    static unsigned char recalseq[] = {
	0x0b,0x00,0x00,0x00,0x00,0x04
    };

    register short i;
    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	else outb(SASIDATA,recalseq[i]);
    }

    while (sasiwait(REQUEST,ERROR));

    return(inb(SASIDATA));
}





drivetest()
{
    static unsigned char driveseq[] = {
	0xe3,0x00,0x00,0x00,0x00,0x04
    };

    register short i;
    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	else outb(SASIDATA,driveseq[i]);
    }

    printf("V{ntar p} att provet ska bli klart...\n");
    while (sasiwait(REQUEST,ERROR)) {
	printf("V{ntar...\n");
    }

    return(inb(SASIDATA));
}



controltest()
{
    static unsigned char contrseq[] = {
	0xe4,0x00,0x00,0x00,0x00,0x00
    };

    register short i;
    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	else outb(SASIDATA,contrseq[i]);
    }

    while (sasiwait(REQUEST,ERROR)) {
	printf("V{ntar...\n");
    }

    return(inb(SASIDATA));
}




ramtest()
{
    static unsigned char ramseq[] = {
	0xe0,0x00,0x00,0x00,0x00,0x00
    };
    register short i;
    int data;
    
    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	else outb(SASIDATA,ramseq[i]);
    }

    if (sasiwait(REQUEST,ERROR)) return(2);

    return(inb(SASIDATA));
}




unsigned char *orig,*to,*dmaadr,*mapadr;
unsigned int pval;

dmatest(adr)
{
    int adr;
    register unsigned short i,j;
    short errtype;

    winchread(adr,1);
    printf("DMA aktiverad... L{sning b|rjar....\n");
    for (j=0;j<1000;j++) {
	printf(" Kontrollomg}ng: %d\r",j);
	if (errtype=dmaread(adr,1)) return(errtype);
	for (i=0;i<512;i++) {
	    if (to[i]!=newbuff[i]) return(512);
	}
    }
    printf("\n");
    return(0);
}




dmaread(adr,sect)
unsigned int adr,sect;
{
    register unsigned short i;
    short sastat;
    
    setupdma();
    for (i=0;i<512;i++) to[i]=0;
    spcr=0x0d;			/* Pray.... */
    outb(SASIMODE,0x40);
    if (readcom(adr,sect)) {
	sastat=2;
	goto skip;
    }

    while (sasiwait(REQUEST,ERROR)) {
	printf("V{ntar....\n");
	kick;
    }
    sastat=inb(SASIDATA);
    kick;

skip:
    spcr=0x05;			/* Shut off DMA */
    outb(SASIMODE,0x00);
    cfree(orig);
    return(sastat);
}




setupdma()
{
    static unsigned char dmatab[] = {
	0xc3,0xc3,0xc3,0xc3,0xc3,/* Reset DMA five times */
	0x7a,0x00,0x00,0x00,0x00,/* Search port B */
	0x54,0x0d,		/* Port A incr & timing */
	0x50,0x0d,		/* Port B fix & timing */
	0x80,			/* DMA disable */
	0xcd,0x00,0x00,		/* Port B address */
	0x92,			/* Setup RDY etc */
	0xc7,0xcb,0xcf,0x8b,0xbb,0x7f,0x87
    };
    register unsigned short i;
    unsigned short mapto,dmamap;

    orig = to = (unsigned char *) calloc(1024,sizeof(unsigned char));
    while ((unsigned int)to & 0x1ff) to++;

    mapto = (*((unsigned short *)(0x80000+((unsigned int)to & 0xff800)))
             & 0x03ff)>>5;	/* Convert allocated address to */
	     			/* actual physical address in memory */
				/* Then extract the bits needed for */
				/* the dmamap register */

    pval = (unsigned int) to;
    dmatab[16] = pval & 0xff;
    dmatab[17] = pval>>8 & 0xff;
    dmatab[8] = 0x00;
    dmatab[9] = 0x02;

    kick;
    dmamap = mapto | mapto<<8;	/* Set both high and low part */

    switch (inb(SASICSB) ^ 0xff) {
	case 1:
	    dmaadr=BUS2DMA;
	    mapadr=BUS2HI;
            break;
	case 2:
	    dmaadr=BUS1DMA;
	    mapadr=BUS1HI;
	    break;
	default:
	    dmaadr=BUS0DMA;
	    mapadr=BUS0HI;
	    break;
    }

    out2b(mapadr,dmamap);

    kick;
    for (i=0;i<26;i++) {
	outb(dmaadr,dmatab[i]);
    }
    spcr=0x06; spcr=0x07;	/* Disconnect DMA:s */
}




readcom(address,sect_cnt)
int address;
int sect_cnt;
{
    static unsigned char readseq[6] = {
	0x08,0x00,0x00,0x00,0x00,0x04
    };
    register short i;

    readseq[1]=address>>16 & 0x1f;
    readseq[2]=address>>8 & 0xff;
    readseq[3]=address & 0xff;
    readseq[4]=sect_cnt & 0xff;

    if (activate()) return(1);

    for (i=0;i<6;i++) {
	if (sasiwait(REQUEST,OK)) return(2);
	outb(SASIDATA,readseq[i]);
    }
    
    if (sasiwait(REQUEST,READ)) return(2);
    return(0);
}




sasistat(mask,value)
unsigned short mask,value;
{
    if ((inb(SASISTAT) & mask) != value) {
	kick;
	return(1);
    }
    kick;
    return(0);
}




sasiwait(mask,value)
{
    int cnt=0;
    
    while ((inb(SASISTAT) & mask) != value) {
	kick;
	if (cnt++ > 40000) return(1);
    }
    kick;
    return(0);
}




activate()
{
    if (sasistat(BUSY,NO)) return(1);
    else {
	outb(SASIDATA,1);
	outb(SASISEL,0);
    }
    
    if (sasistat(BUSY,YES)) return(1);
    else return(0);
}
