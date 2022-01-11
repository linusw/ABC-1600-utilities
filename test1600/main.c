#include "h/tests.h"

int pgm;
int prexist,pron;

struct {
    char active;
    int loops;
    char circ;
    char comm;
    char pr;
    char win;
    char flop;
    char mem;
    char key;
    char mus;
    char term;
    char mov;
    unsigned int count;
    unsigned int par;
    unsigned int memsz;
} prog;




/*************************************************************************\
* 									  *
* ABC1600 Test Program Ver 5.0						  *
* 									  *
* 850225  Created  Peter AA                                               *
* 850410  Fixed memtest and Swedish daylight savings time                 *
* 850703  Added lprintf printer logging                                   *
* 850718  Added winchestertest						  *
* 850725  Added floppytest						  *
* 850815  Changed menu selection					  *
* 851023  Changed version number to 5.0					  *
* 									  *
\*************************************************************************/
main()
{
    char alt;
    short i;
    char z[80];

    prog.active=0;
    prog.circ=prog.comm=prog.pr=prog.win=prog.flop=prog.mem=0;
    prog.key=prog.mus=prog.term=prog.mov=1;
    prog.loops=1;
    
    do {
	cls();
	printf("\n");
	printf("****** LUXOR DATORER -- ABC1600 TESTPROGRAM   Ver. 5.0 -- LUXOR DATORER ******\n");
	for (i=0;i<8;i++) printf("\n");
	printf("                    ****** ABC1600  TESTPROGRAM ******");
	for (i=0;i<4;i++) printf("\n");
	fflush(stdout);
	printf("                           V{lj testrutin:\n\n");
	printf("                           PF1   Kretstest\n");
	printf("                                 (Automatiskt)\n\n");
	printf("                           PF2   Kommunikationstest\n\n");
	printf("                           PF3   Skrivarkanalstest\n");
	printf("                                 (Kr{ver skrivare)\n\n");
	printf("                           PF4   Terminaltest\n");
	printf("                                 (Kr{ver terminalanslutning)\n\n");
	printf("                           PF5   Minnestest\n");
	printf("                                 (Automatiskt)\n\n");
	printf("                           PF6   Tangentbordstest\n\n");
	printf("                           PF7   Mustest\n\n");
	printf("                           PF8   Winchestertest\n\n");
	printf("                           PF9   Floppytest\n\n");
	printf("                           PF10  Sk{rmtest\n\n");
	printf("                           PF14  Programmera sekvens\n\n");
	printf("                           PF15  Exekvera sekvens\n");
	printf("                                 (F|rprogrammerad eller egen)\n\n");
	printf("                           DEL   Avbryt\n\n");
	printf("\n");
	printf("   STOP     avbryter en p}g}ende testsekvens och minnestestet\n");
	printf("   PRINT    p}b|rjar/avbryter loggning p} skrivare (om test 3 genomf|rts)\n");

	keyb=0x05;
	printf("\n\n\n");
menu:
        alt= -1;
        printf("                           V{lj alternativ: ");
	fflush(stdout);
	while (alt<0 || alt>16) {
	    while (!(status & 1)) kick;
	    alt=keyb;
	    if (alt == 0x7f) alt=0;
	    else if (alt == 0x82) alt=16;
	    else alt -= 0xbf;
	}

	if (alt==0) {
	    cls();
	    exit(0);
	}

	if (alt==16) {
	    if (prexist && !pron) {
		pron=1;
		printf(" Loggning p}\n");
	    }
	    else {
		pron=0;
		printf(" Loggning av\n");
	    }
	    goto menu;
	}
	
	select(alt);

	if (prog.active) {
	    i=0;
	    while (i++ < prog.loops) {
		lprintf("\n\n\nStart testvarv: %d\n",i);
		if (prog.circ) if (select(1)) break;
		if (prog.comm) if (select(2)) break;
		if (prog.pr) if (select(3)) break;
		if (prog.term) if (select(4)) break;
		if (prog.mem) if (select(5)) break;
		if (prog.key) if (select(6)) break;
		if (prog.mus) if (select(7)) break;
		if (prog.win) if (select(8)) break;
		if (prog.flop) if (select(9)) break;
		if (prog.mov) if (select(10)) break;
		lprintf("\nKlar testvarv: %d\n",i);
	    }
	    prog.active=0;
	}
    } while(1);
}




/***************************************************************\
* 							        *
*  SELECT branches to the test routine specified by 'alt'       *
* 							        *
\***************************************************************/
select(alt)
char alt;
{
    char c;

    cls();
    printf("\n");
    switch(alt) {
        case 1:
	    title("ABC1600 Testprogram KRETSTEST");
            ciop();
	    batclk();
	    nvram();
	    dma();
	    break;

	case 2:
	    title("ABC1600 Testprogram KOMMUNIKATIONSTEST");
	    scc();
	    break;

	case 3:
	    title("ABC1600 Testprogram PRINTERKANALSTEST");
	    printp();
	    break;

        case 4:
	    title("ABC1600 Testprogram TERMINALTEST");
	    terminal();
	    break;
	    
	case 5:
	    if (prog.mem && prog.active) {
		title("ABC1600 Testprogram MINNESTEST");
		memtest(0x40000,(prog.memsz==1) ? 0x40000:0xc0000,1,prog.count,prog.par);
		break;
	    }
	    memparam();
	    cls();
	    printf("\n");
	    title("ABC1600 Testprogram MINNESTEST");
	    memtest(0x40000,(prog.memsz==1) ? 0x40000:0xc0000,1,prog.count,prog.par);
	    break;

	case 6:
	    title("ABC1600 Testprogram TANGENTBORDSTEST");
	    keyboard();
	    break;

	case 7:
	    title("ABC1600 Testprogram MUSTEST");
	    mouse();
	    break;

	case 8:
	    title("ABC1600 Testprogram WINCHESTERTEST");
	    winch();
	    break;
	    
	case 9:
	    title("ABC1600 Testprogram FLOPPYTEST");
	    floppy();
	    break;
	    
	case 10:
	    mover();
	    patts();
	    bw();
	    break;

	case 14:
	    title("ABC1600 Testprogram sekvensprogrammering");
	    program();
	    break;
	    
	case 15:
	    prog.active=1;
	    break;

	case 16:
	    break;

	default:
	    printf("ERROR\n");
	    break;
    }

    printf("\n\n\n");
    if (status & 1) {
	c=keyb;
	if (c==0x81) return(1);
	if (c==0x82) {
	    if (prexist && !pron) pron=1;
	    else pron=0;
	}
    }

    if (alt!=15 && alt!=10) {
	query("Tryck RETURN f|r att forts{tta....");
    }
    return(0);
}




/**********************************************************************\
* 								       *
*  MEMPARAM sets up the memory test parameters                         *
* 								       *
\**********************************************************************/
memparam()
{
    char memory;

    printf("\n\n\n                    Hur mycket minne finns installerat?\n\n");
    printf("                                    PF1   512K\n\n");
    printf("                                    PF2   1M\n");
    printf("\n                    V{lj alternativ: ");
    fflush(stdout);
    memory= -1;
    while (memory<1 || memory>2) {
	while (!(status & 1)) kick;
	memory=keyb;
	memory -= 0xbf;
    }
    if (memory == 1) printf("1\n");
    else printf("2\n");
    
    prog.memsz=memory;

    printf("\n\n\n                    Hur m}nga g}nger ska minnet testas?\n\n");
    printf("                                    PF1   En g}ng\n\n");
    printf("                                    PF2   Fem g}nger\n\n");
    printf("                                    PF3   Tio g}nger\n\n");
    printf("                                    PF4   100 g}nger\n\n");
    printf("                                    PF5   1000 g}nger (tar L]NG tid)\n\n");
    printf("\n                    V{lj alternativ: ");
    memory= -1;
    while (memory<1 || memory>5) {
	while (!(status & 1)) kick;
	memory=keyb;
	memory -= 0xbf;
    }

    switch(memory) {
	case 1:
	    printf("1\n");
	    prog.count=1;
	    break;
	case 2:
	    printf("2\n");
	    prog.count=5;
	    break;
	case 3:
	    printf("3\n");
	    prog.count=10;
	    break;
	case 4:
	    printf("4\n");
	    prog.count=100;
	    break;
	case 5:
	    printf("5\n");
	    prog.count=1000;
	    break;
    }

    prog.par=0;
    printf("\n\n     Ska minnestestet utf|ras med paritetskontroll?\n");
    printf("     **** Varning: Test utan paritetskontroll ska endast g|ras d} maskinen\n");
    printf("                   'dyker' med paritetskontroll p}slagen!!!\n");
    if (query("\n     Paritetskontroll?")) prog.par=1;
    printf("\n\n");
}




/**********************************************************************\
* 								       *
*  PROGRAM sets up a test sequence                                     *
* 								       *
\**********************************************************************/
program()
{
    int cnt;

    printf("\n");
    printf("           Vilka prov ska genomf|ras?  Svara p} nedanst}ende fr}gor: \n\n");
    if (query("                              Kretstest? ")) prog.circ=1; else prog.circ=0;
    if (query("                              Kommunikationstest? ")) prog.comm=1; else prog.comm=0;
    if (query("                              Minnestest? ")) prog.mem=1; else prog.mem=0;
    if (query("                              Winchester? ")) prog.win=1; else prog.win=0;
    if (query("                              Floppy? ")) prog.flop=1; else prog.flop=0;
    
    printf("\n\n");
    printf("           F|ljande tester {r inte automatiska...\n\n");
    if (query("                              Skrivarkanalstest? ")) prog.pr=1; else prog.pr=0;
    if (query("                              Terminaltest? ")) prog.term=1; else prog.term=0;
    if (query("                              Tangentbord? ")) prog.key=1; else prog.key=0;
    if (query("                              Mustest? ")) prog.mus=1; else prog.mus=0;
    if (query("                              Sk{rmtest? ")) prog.mov=1; else prog.mov=0;

    printf("\n\n");
    cnt= -1;
    while (cnt<1) {
	printf("           Hur m}nga varv ska provet genoml|pa? ");
	fflush(stdout);
	scanf("%d",&cnt);
    }
    prog.loops=cnt;

    if (prog.mem) memparam();

    prog.active=0;
}
