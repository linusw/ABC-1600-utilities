#include "h/tests.h"

extern readled();
extern byte crash();

int ledok=1;

mouse()
{
    int errtype;

    lprintf("\nMUS: \n");fflush(stdout);
    if (errtype=musinit()) {
	lprintf("MUS: FEL ****\n");
	if (errtype & 1) lprintf("        Musen svarar inte\n");
	if (errtype & 2) lprintf("        Musen fungerar inte\n");
	if (errtype & 4) lprintf("        Felaktig positionsangivelse\n");
	if (errtype & 8) lprintf("        Musen fungerar inte korrekt\n");
	if (errtype & 16) lprintf("        'Poll'-mod fungerar inte\n");
	if (errtype & 32) lprintf("        V{nstra musknappen fungerar inte\n");
	if (errtype & 64) lprintf("        Mitten knappen fungerar inte\n");
	if (errtype & 128) lprintf("        H|gra knappen fungerar inte\n");
    }
    else {
	lprintf("MUS: OK\n");
    }
}




/*******************************************************************\
* 								    *
*  INIT sets up the keyboard mouse CPU                              *
* 								    *
\*******************************************************************/
musinit()
{
    int i,j,cnt,cnt2,dump,xold,yold,xpos,ypos;
    byte port[6];
    byte error,key,old;

    static int limits[] = {
	0x3b,0x20,0x30,0x21,0x20,0x30,0x20,0x30,0x50
    };

    static int scale[] = {
	0x3a,0x20,0x24,0x20,0x24,0x20,0x21,0x20,0x21
    };

    static int pos[] = {
	0x39,0x20,0x50,0x30,0x30
    };

    error=0;
    keyb=0x3f;			/* Disable Auto mode */
    delay(1);
    key=0;
    keyb=7*16;
    delay(1);
    keyb=0;
    delay(20);

    for (i=0;i<9;i++) {
	keyb=limits[i];
	delay(1);
    }
    delay(1);

    dump=kick;
    for (i=0;i<9;i++) {
	keyb=scale[i];
	delay(1);
    }
    delay(1);

    dump=kick;
    for (i=0;i<5;i++) {
	keyb=pos[i];
	delay(1);
    }
    delay(1);

    key=cnt=0;
    keyb=0x3c;
    for (i=0;i<6;) {
	if (i==0) {
	    if (status & 1) if ((port[0]=(keyb&255))==0x90) i++;
	}
	else {
	    if (status & 1) port[i++]=(keyb&255);
	}
	dump=kick;
	cnt++;
	if (cnt>10000) {
	    error |= 1;
	    break;
	}
    }

    error |= crash(2);
    
    for (i=1;i<5;i++) if (port[i]!=pos[i]) error |= 4;

    lprintf("\n");
    printf("Prova att styra omkring med musen och se att den inte\n");
    printf("underskrider x:16 och y:1024 eller |verskrider x:64 och y:1072\n");
    printf("Avbryt med STOP n{r du {r klar\n\n");fflush(stdout);
    while (port[0]!=3 && port[0]!=0x81) {
	dump=kick;
	delay(1);
        keyb=0x3c;
        for (i=0;i<5;) {
	    if (i==0) {
		if (status & 1) if ((port[0]=(keyb&255))==0x90) i++;
	    }
	    else {
		if (status & 1) port[i++]=(keyb&255);
	    }
	    dump=kick;
	    cnt++;
	    if (cnt>300000) {
		error |= 8;
		break;
	    }
	    if (port[0]==3 || port[0]==0x81) break;
	}
        if (error & 8) break;
	xpos=(port[1]-32)*64+port[2]-32;
	ypos=(port[3]-32)*64+port[4]-32;
	if (xpos!=xold || ypos!=yold) {
	    xold=xpos; yold=ypos;
	    printf(" x:%4d  y:%4d\r",xpos,ypos);fflush(stdout);
	    cnt=0;
	}
	if (xpos>64 || xpos<16 || ypos>1072 || ypos<1024) error |= 8;
    }
    keyb=0x3f;
    error |= crash(16);
    printf("\n");
    
    printf("\nOK, prova knapparna p} musen\n");
    printf("Avbryt med STOP n{r du {r klar\n\n");fflush(stdout);
    delay(20);
    error |= 32|64|128;		/* Assume they don't work */

    cnt2=0;
    port[0]=0;
    error &= 0xfe;			/* Invalid error condition */
    while(port[0]!=3 && port[0]!=0x81) {
        cnt=key=0;
	keyb=0x3c;
	for (i=0;i<6;) {
	    if (i==0) {
		if (status & 1) if ((port[0]=(keyb&255))==0x90) i++;
	    }
	    else {
		if (status & 1) port[i++]=(keyb&255);
	    }
	    dump=kick;
	    cnt++;
	    if (cnt>30000) {
		error |= 1;
		break;
	    }
	    if (port[0]==3 || port[0]==0x81) break;
	}

	if (error & 1) break;
	cnt2++;
	if (cnt2>1000) break;
	if (old!=port[5]) {
	    cnt2=0;
	    old=port[5];
	    printf("\r Knapp: ");
	    switch(port[5]-32) {
	    case 1:
	        printf("V[NSTER");
		error &= 0xdf;
		break;
	    case 2:
	        printf("MITTEN");
		error &= 0xbf;
		break;
	    case 4:
	        printf("H\\GER");
		error &= 0x7f;
		break;
            case 5:
	        printf("H\\GER & V[NSTER");
		break;
	    case 6:
	        printf("H\\GER & MITTEN");
		break;
	    case 3:
	        printf("V[NSTER & MITTEN");
		break;
	    case 7:
	        printf("H\\GER, V[NSTER & MITTEN");
		break;
	    default:
	        printf("ingen");
		break;
	}
	printf(" nertryckt                           \r");fflush(stdout);
	}
    }
    printf("\n");
    return(error);
}


	
