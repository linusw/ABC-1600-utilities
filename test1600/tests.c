#include "h/tests.h"

/***************************************************************\
* 							        *
* QUERY asks the user a yes/no question and waits for a reply   *
* 1 is returned if the answer was yes, 0 if it was no	        *
* 							        *
\***************************************************************/
query(s)
char *s;
{
    int key,dump,cnt;
    printf("%s ",s);fflush(stdout);	/* Print out query text */
    cnt=key=0;
    while (key!='J' && key!='j' && key!='n' && key!='N' && key!='\r') {
	if (status & 1) {	/* Read byte if there is one */
	    key=keyb;
	}
	dump=kick;		/* Kick dog! */
	cnt++;
	if (cnt>500000) {	/* If there isn't a reply within this time */
	    key='n';		/* automatically send NO to caller */
	    break;
	}
    }
    if (key=='j' || key=='J' || key=='\r') {
	printf("JA\n");fflush(stdout);		/* Was the reply YES? */
	return(1);
    }
    else {
	printf("NEJ\n");fflush(stdout); 	/* It was NO */
	return(0);
    }
}




/***************************************************\
* 						    *
*  LENGTH returns the length of the input string s  *
* 						    *
\***************************************************/
length(s)
char *s;
{
    int n;

    for (n=0; *s;s++,n++) ;	/* Step through until NULL is encountered */
    return(n);
}



/***********************************************************************\
* 								        *
* TITLE is used for generating standardized headers for the test code   *
* sections							        *
* 								        *
\***********************************************************************/
title(s)
char *s;
{
    int len,i,diff;

    lprintf("\n");
    len=length(s);
    diff=(78-len)/2;		/* Calculate number of * to print */
    for (i=0;i<diff;i++) lprintf("*");
    lprintf(" ");
    lprintf("%s",s);		/* Put out string */
    lprintf(" ");
    for (i=0;i<diff;i++) lprintf("*");
    lprintf("\n\n");
}



/********************************************\
* 					     *
* DELAY is used for user-interaction delay.  *
* Minimum delay: 1			     *
* Maximum delay: 50			     *
* 					     *
\********************************************/
delay(t)
int t;
{
    int q,s;
    s = kick;			/* Kick watch-dog */
    for (q=0;q<310*t;q++) ;	/* Delay a while */
    s = kick;
}



cls()
{
    int dump;
    register unsigned short *scr;
    scr = ((unsigned short *) SCREEN);

    wait();
    *scr=0x0000;
    flags=0x30;
    outw(0x0c00 | 767,SIZE);
    outw(1023,SIZE+2);
    outw(0,TO);
    outw(0,TO+2);
    outw(0,FROM);
    outw(0,FROM+2);
    dump=kick;
    wait();
    outw(10,(SCREEN+3*128+96));
    outw(0,(SCREEN+4*128+96));
}
