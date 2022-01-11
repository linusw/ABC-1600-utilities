#include "h/tests.h"

dma()
{
    int errtype;
    
    lprintf("\nDMA0 kontroll: ");
    if (errtype=setup(BUS0DMA,BUS0HI)) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        **** FEL **** -> Programfel....\n");
	if (errtype & 2) lprintf("        Fungerar inte\n");
    }
    else {
	lprintf("OK\n");
    }
    
     
    lprintf("DMA1 kontroll: ");
    if (errtype=setup(BUS1DMA,BUS1HI)) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        **** FEL **** -> Programfel....\n");
	if (errtype & 2) lprintf("        Fungerar inte\n");
    }
    else {
	lprintf("OK\n");
    }
    
    
    lprintf("DMA2 kontroll: ");
    if (errtype=setup(BUS2DMA,BUS2HI)) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        **** FEL **** -> Programfel....\n");
	if (errtype & 2) lprintf("        Fungerar inte\n");
    }
    else {
	lprintf("OK\n");
    }
    
}




/***************************************************************************\
* 									    *
* SETUP initializes the DMA specified by it's argument and then performs a  *
* test move of about 30000 bytes to see if it works.			    *
* 									    *
\***************************************************************************/
setup(dmaport,dmahigh)
register unsigned char *dmaport,*dmahigh;
{
    static unsigned char dmatab[] = {
	0xa3,0x83,
        0xc3,0xc3,0xc3,0xc3,0xc3,
	0x79,0x00,0x00,0x00,0x00,
	0x54,0x0d,
	0x50,0x0d,
	0x80,
	0xcd,0x00,0x00,
	0x92,
	0xc7,0xcb,0xcf,0x8b,0xbb,0x7f,0xa7,0xb3
    };

    register unsigned int i;
    unsigned char *from,*to,*fill,*orig;
    unsigned int pval,dump,diff;
    unsigned short dmamap,macto,macfrom,error=0;

    orig=to=(unsigned char *) calloc(64*1024,sizeof(unsigned char));
    while ((unsigned int)to & 0x8000) to++;/* Skip until A15 is high */
    dump=kick;
    macto = (*((unsigned short *)(0x80000+((unsigned int)to & 0xff800))) & 0x03ff)>>5;
    
    from=to;
    while (!((unsigned int)from & 0x8000)) from++; /* Skip until A15 is low */
    dump=kick;
    macfrom = (*((unsigned short *)(0x80000+((unsigned int)from & 0xff800))) & 0x03ff)>>5;

    diff=(unsigned int)from - (unsigned int)to - 1;
    
    pval=(unsigned int) from;
    if (pval+diff > (unsigned int)orig + 65535) return(1);
    dmatab[18]=pval & 255;
    dmatab[19]=pval>>8 & 255;

    pval=(unsigned int) to;
    dmatab[8]=pval & 255;
    dmatab[9]=pval>>8 & 255;
    
    dmatab[10]=diff & 255;
    dmatab[11]=diff>>8 & 255;

    dmamap=(macfrom | 0x80) | macto<<8;
    out2b(dmahigh,dmamap);
    
    for (i=0;i<29;i++) {
	outb(dmaport,dmatab[i]);
    }
    dump=kick;

    spcr=0x0d;
    outb(dmaport,0x87);
    dump=kick;
    outb(dmaport,0xa7);

    pval=(unsigned int) to;
    fill=to;
    for (i=pval;i<pval+diff;i++) {
	if (*fill++ != 0xff) error |= 2;
	dump=kick;
    }
    
    cfree(orig);
    dump=kick;
    return(error);
}
