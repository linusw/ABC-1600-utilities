#define X35 X35
#define	NULL	0
#include "h/ports.h"
#include "h/tests.h"

#define	INPUT	(inb(BCLK) & 0x02)	/* Get nvram data */
#define SELECT		outb(BCLK,0x0c);
#define DESELECT	outb(BCLK,0x04);
#define CLKLO		outb(BCLK,0xe0);
#define CLKHI		outb(BCLK,0xe1);
#define HIZ		outb(BCLK,0xd2);
#define	OUTPUT(x)	{ register v;\
			v = (x) ? 0x02 : 0 ; \
			outb(BCLK,0xd0 | v); \
			TOGGLE \
			}
#define TOGGLE		{ dlymicr(1); \
			CLKHI \
			dlymicr(1); \
			CLKLO \
			}
#define SERROR 0
#define SREAD 1
#define SCHECK 2


nvram()
{
    int errtype=0;

    lprintf("\n");
    errtype=shownv();
    lprintf("NVRAM: ");
    if (errtype) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        G}r ej att l{sa\n");
	if (errtype & 2) lprintf("        Lagrade data ej korrekta\n");
	if (errtype & 4) lprintf("        G}r ej att skriva\n");
	if (errtype & 8) lprintf("        CIO ej initierad\n");
    }
    else {
	lprintf("OK\n");
    }
}




/********************************************************************\
* 								     *
*  RD returns the data at the CIO register adressed by the argument  *
* 								     *
\********************************************************************/
rdnv(adr)
int adr;
{
    byte dump,data;

    dump = cio;
    cio = adr;
    data = cio;
    return(data & 255);
}




/*******************************************************************\
* 								    *
* Read nvram                                                        *
* 								    *
\*******************************************************************/
unsigned short *
rnvram(ptr)
unsigned short *ptr;
{
	register int i,j,cmd;
	register unsigned short *p;
	register state = SERROR ;
	int errcnt = 0 ;
	for (;;) {
		p = ptr;
		if ( ++state > SCHECK ) { break ; }
		for ( cmd = 0x80 ; cmd < 0x90 ; cmd++ ) {
			command(cmd);
			HIZ;
			j = 0;
			for ( i = 0; i < 17 ; i++ ) {
				CLKLO;
				dlymicr(1);
				j <<=1 ;
				if ( INPUT ) { j |= 1 ; }
				CLKHI;
				dlymicr(1);
			}
			j &= 0xffff;
			if ( state==SREAD ) {
				*p++ = j ;
			} else if ((state == SCHECK && *p++ != j)) {
				if ( ++errcnt == 5 ) {
					DESELECT;
					return(0) ;
				} else {
					state = SERROR ;
				}
			}
		}
	}
	DESELECT;
	return(ptr);
}

wnvram(ptr)
register unsigned short *ptr;
{
	register int i,j,cmd;
	register unsigned short *p;

	command(0x30);		/* Set write */
	command(0x20);		/* Erase all */
	progpulse();
	p = ptr;
	for(cmd = 0x40 ; cmd < 0x50 ; cmd++){
		command(cmd);
		j = *p++;
		for(i = 0 ; i < 16 ; i++){
			OUTPUT(j & 0x8000);
			j <<= 1;
		}
		progpulse();
	}
	command(0);	/* Erase/write disable */
	DESELECT;
}

/*
 *	Write instruction to nvram
 */
command(cmd)
register int cmd;
{
	register int i ;

	DESELECT;
	TOGGLE;
	SELECT;
	cmd &= 255;
	cmd |= 0x100;
	for(i = 10 ; i ; i--){
		OUTPUT(cmd & 0x200);
		cmd <<=1;
	}
}

progpulse()
{
	int i;
	DESELECT;
	i=kick;
	for(i = 0 ; i < 300 ; i++){
		TOGGLE;
	}
}





/*************************************************************************\
* 									  *
* SHOW performs a read and write test of the NVRAM and puts it's present  *
* contents on screen.							  *
* 									  *
\*************************************************************************/
shownv()
{
    unsigned short nvram[16],back[16];
    int i;
    byte error=0;
    
    if (rdnv(0x01)!=0xb0) {
	error |= 8;
	return(error);
    }

    for (i=0;i<16;i++) nvram[i]=0;
    if (!rnvram(nvram)) {
	error |= 1;
	return(error);
    }
    
    lprintf("NVRAM data:\n");
    lprintf("Console baud: %d",nvram[1]/256);
    if (0x0d != nvram[1]/256) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Boot niv}: %d",nvram[1]&255);
    if (0x03 != (nvram[1] & 255)) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Boot device: %d,%d",nvram[2]/256,nvram[2]&255);
    if (0x0528!=nvram[2] && 0x0902!=nvram[2]) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Root device: %d,%d",nvram[3]/256,nvram[3]&255);
    if (0x0528!=nvram[3] && 0x0902!=nvram[3]) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Swap device: %d,%d",nvram[4]/256,nvram[4]&255);
    if (0x0528!=nvram[4] && 0x0902!=nvram[4]) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Pipe device: %d,%d",nvram[5]/256,nvram[5]&255);
    if (0x0528!=nvram[5] && 0x0902!=nvram[5]) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("Tidszon: %dmin fr}n GMT",nvram[6]);
    if (nvram[6]!=60 && nvram[6]!=120) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	if (nvram[6]==120) lprintf("     Sommartid");
	lprintf("\n");
    }
    if (nvram[15] & 0x0100) {
	lprintf("U.S. sommartid g{ller   **** FEL\n");
	error |= 2;
    }
    lprintf("CPU klocka: %dHz",nvram[8]*65536+nvram[9]);
    if (0x007a*65536+0x1200 != nvram[8]*65536+nvram[9]) {
	lprintf("   **** FEL\n");
	error |= 2;
    }
    else {
	lprintf("\n");
    }
    lprintf("\n");;

    for (i=0;i<16;i++) {
	back[i]=0x8001;
    }

    wnvram(back);
    for (i=0;i<16;i++) back[i]=0;

    rnvram(back);
    for (i=0;i<16;i++) if (back[i] != 0x8001) error |= 4;

    wnvram(nvram);
    return(error);
}


