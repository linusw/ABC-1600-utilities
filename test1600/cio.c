#define porta	 *((char *) 0x7F704)
#include "h/tests.h"

byte initdata[] = {
    0x02,0x10,0x40,0x50,0x60,0xf0,0xf0,0xf2,0x08,0x08,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,
    0x9c,0x40,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

ciop()
{
    int errtype;
    
    lprintf("\nCIO register: ");
    delay(1);
    initialize();
    if (errtype=compare()) {
	initialize();
	prinit();
	delay(1);
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        Skrivning/l{sning fungerar inte\n");
    }
    else {
	initialize();
	prinit();
	delay(1);
	lprintf("OK\n");
    }

    lprintf("CIO timers: ");
    delay(1);
    initialize();
    if (errtype=timers()) {
	initialize();
	prinit();
	delay(1);
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        Fungerar inte\n");
	if (errtype & 2) lprintf("        Tidtagning felaktig\n");
    }
    else {
	initialize();
	prinit();
	delay(1);
	lprintf("OK\n");
    }
}
    



/********************************************************************\
* 								     *
*  RD returns the data at the CIO register adressed by the argument  *
* 								     *
\********************************************************************/
rd(adr)
int adr;
{
    byte dump,data;

    dump = cio;
    cio = adr;
    data = cio;
    return(data & 255);
}




/********************************************************************\
* 								     *
*  WR writes data to the CIO register adressed by the argument       *
* 								     *
\********************************************************************/
wr(adr,data)
int adr;
byte data;
{
    byte dump;

    dump=cio;
    cio=adr;
    cio=data & 255;
}




/****************************************************************************\
* 									     *
* Compares each CIO register against Initdata. Any differences are reported  *
* to the user and back to the caller.					     *
* 									     *
\****************************************************************************/
compare()
{
    int i;
    byte error=0;

    for (i=0;i<0x30;i++) {
	kick;
	if (i<0x0d || i>0x16) {
	    if (initdata[i] != rd(i) && !(i==0x0b && rd(i)==0x20)) {
		error=1;
	    }
	}
    }
    return(error);
}




/**********************************************************\
* 							   *
*  INITIALIZE sets up the CIO registers as they should be  *
* 							   *
\**********************************************************/
initialize()
{
    int i;

    for (i=0;i<0x30;i++) {
	kick;
	if (i<0x0d || i>0x16) {
	    wr(i,initdata[i]);
	}
    }
}




/*********************************************************\
* 							  *
*  TIMERS performs a test run of the CIO internal timers  *
* 							  *
\*********************************************************/
timers()
{
    int i,j;
    byte error;

    error=0;
    wr(0x01,rd(0x01) & ~0x70);	/* Disable counters */

    for (j=0;j<3;j++) {
	wr(0x0a+j,0x00);	/* Command and status */
	wr(0x1c+j,0x00);	/* Mode spec. */
	wr(0x16+j*2,0xff);	/* Count MSB */
	wr(0x17+j*2,0xff);	/* Count LSB */
	wr(0x01,rd(0x01) | 0x0040>>j);/* Enable counter */

	wr(0x0a+j,0x06);	/* Start counting */
	for (i=0;rd(0x0a+j) & 1;i++) {
	    kick;		/* Keep dog quiet */
	    if (i>1000) {
		error |= 1;
		break;
	    }
	}

	wr(0x01,rd(0x01+j) & ~(0x0040>>j));/* Disable counter */
	wr(0x0a+j,0x20);	/* Reset counter */

	if (i>384 || i<379) {
	    error |= 0x02;
	}
    }
    return(error);
}





/**********************************************************\
* 							   *
*  PRINIT sets up the printer baud rate clock through PB0  *
* 							   *
\**********************************************************/
prinit()
{
    wr(0x28,0x00);		/* Port mode */
    wr(0x09,0x20);		/* Port command */
    wr(0x2a,0x00);		/* Data path */
    wr(0x2b,0xfe);		/* Data direction */
    wr(0x2c,0x00);		/* Special i/o */
    wr(0x1d,0xc2);		/* Counter mode */
    wr(0x18,0x00);		/* MSB time */
    wr(0x19,0x68);		/* LSB time */
    wr(0x01,0xb0);		/* Master configuration */
    wr(0x0b,0x26);		/* Counter command */

    				/* Counter now started */
}
