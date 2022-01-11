#include "h/tests.h"

/*************************************************************************\
* 									  *
* PRINTP performs a test of the printer channel, verifying both computer  *
* and printer function.							  *
* 									  *
\*************************************************************************/
printp()
{
    int errtype;

    prinit();			/* Try to set up CIO */
    printf("\n\nDART register: ");fflush(stdout);
    if ((errtype=ciochk())==0) {
	printf("OK\n\n");
	initpr();		/* Try to set up DART */
	printf("DART signaler:\n");fflush(stdout);
	if ((errtype=check())==0) {
	    printf("DART signaler: OK\n\n");
	    printf("DART kommunikation:\n");fflush(stdout);
	    if (errtype=test()) {
		printf("DART kommunikation: FEL ****\n");
		prexist=0;
		if (errtype & 1) printf("        Fel vid s{ndning till skrivaren\n");
		if (errtype & 2) printf("        Felaktig utskrift p} skrivaren\n");
	    }
	    else {
		prexist=1;
		lprintf("DART kommunikation: OK\n");
		lprintf("Skrivarkanalen initialiserad och fungerande\n");
	    }
	    fflush(stdout);
	}
	else {
	    printf("DART signaler: FEL ****\n");
	    if (errtype & 1) printf("        Skrivaren {r inte ansluten eller fel i DART-kretsen\n");
	}
	fflush(stdout);
    }
    else {
	printf("FEL ****\n");
	if (errtype & 1) printf("        CIO ej initierad\n");
    }
    fflush(stdout);
}




/**********************************************************\
* 							   *
*  INIT sets up the DART channel A                         *
* 							   *
\**********************************************************/
initpr()
{
    static int setup[] = {
	0x00,0x00,0x00,0x41,0x07,0xa2
    };

    int i;

    set=0x18;			/* Reset channel */
    delay(1);
    for (i=0;i<6;i++) {
	if (i==2) i++;
	set=i;
	set=setup[i];
    }
    set=5;
    set=0xaa;
}
	



/********************************************************\
* 							 *
* WR sends it's argument to the printer port (DART CHA)	 *
* 1 is returned if the operation was unsuccessful	 *
* 							 *
\********************************************************/
prwr(data)
byte data;
{
    int cnt;

    cnt=0;
    while ((set & 4)==0) {
	kick;
	cnt++;
	if (cnt>30000) {
	    return(1);
	}
    }
    print=data&127;
    return(0);
}




/*********************************************************************\
* 								      *
*  PR prints a string to the printer port                             *
* 								      *
\*********************************************************************/
pr(s)
char *s;
{
    int i;

    while (*s) {
	prwr(*s++);
    }
}




/************************************************************\
* 							     *
* CIOCHK checks that the CIO has been correctly initialized  *
* 							     *
\************************************************************/
ciochk()
{
    byte dump,data;

    dump = cio;
    cio = 0x0b;
    if (!(cio & 1)) {
	return(1);
    }
    return(0);
}




/*******************************************************************\
* 								    *
*  TEST performs a test printout of the printable ASCII characters  *
* 								    *
\*******************************************************************/
test()
{
    int i,j,dump,error;

    error=0;    
    if (prwr(13)) {
err:	error |= 1;
	return(error);
    }
    for (i=0;i<5;i++) {
	for (j=32;j<128;j++) {
	    if (prwr(j)) goto err;
	    printf("%c",j);fflush(stdout);
	    if (j == 32+79) {
		printf("\n");fflush(stdout);
		prwr(13);
		prwr(10);
	    }
	}
	dump=kick;
	prwr(13);
	prwr(10);
	printf("\n");
    }
    pr("\n\nPrinter test......\n");
    printf("\n\nPrinter test......\n");fflush(stdout);
    for (i=0;i<10;i++) {
	pr("Testing.................................\n");
	printf("Testing.................................\n");fflush(stdout);
    }
    pr("Printer OK!          Printer OK!\n");

    prwr(12);

    delay(40);
    delay(40);
    delay(40);
    delay(40);
    delay(40);
    delay(40);
    if (!query("Fungerade skrivaren ok?")) {
	error |= 2;
    }

    return(error);
}




/***********************************************************************\
* 								        *
* CHECK performs a check of the DART RR0 to verify that the printer is  *
* connected							        *
* 								        *
\***********************************************************************/
check()
{
    byte error=0;
    
    if (set & 0x20) {
	return(0);
    }
    else {
	printf("** Skrivaren ej ansluten eller ej ON LINE\n");
	query("** S{tt skrivaren ON LINE och tryck RETURN");

	set=0x10;
	delay(20);

	if (set & 0x20) {
	    return(0);
	}
	else {
	    error |= 1;
	}
	return(error);
    }
}
