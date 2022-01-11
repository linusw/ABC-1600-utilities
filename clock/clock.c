/***************************************************************************\
* 									    *
*    Ver 1.0 	Clock program by Peter AAberg				    *
*    Ver 5.0	Modified Oct 1, 1985 P.AA.				    *
* 									    *
\***************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sgtty.h>
#undef FIOCLEX
#undef FIONCLEX
#include <win/w_macros.h>
#include <h/cwins.h>
#define STDIN 0
#define STDOUT 1


/***************************************************************************\
*  Global variable definitions						    *
\***************************************************************************/

int timez;			/* Time zone				 */
int fd;				/* File descriptor			 */
short mold,sold,hold;		/* Saved values for min, sec, hrs	 */
short first,interrupt,skip;	/* Flags				 */
char nowin,showsec;		/* Boolean flags			 */
char hline[15],wrbuff[256];	/* Character buffers for quick-draw	 */
struct winstruc wins;		/* Window definitions structure	 	 */


/***************************************************************************\
*  Function definitions							    *
\***************************************************************************/

main(argc,argv)
int argc;
char *argv[];
{
    int i;
    short hlineset=0;
    char temp[15];
    
    nowin=timez=showsec=0;
    while (--argc>0) {		/* Decode arguments			 */
	if ((*++argv)[0]=='-') ++(*argv);
	switch(**argv) {
	    case 't':		/* Time zone				 */
	        if (--argc>0) timez=atoi(*++argv);
		break;
	    case 'h':		/* Header				 */
	        hlineset=1;
		if (--argc>0) strcpy(temp,*(++argv));
		break;
	    case 'n':		/* No window				 */
	        nowin=1;
		break;
	    case 's':
	        showsec=1;	/* Show seconds				 */
		break;
	    default:
		argc= -1;
		break;
	}
    }

    if (!nowin) {		/* Open own window			 */
	if (fork() != 0) {
	    exit(1);
	}
	if (setpgrp()<0) {
	    exit(2);
	}

	for (i=0;i<_NFILE;i++) close(i);
    }

    if (!hlineset) strcpy(hline," ABC1600 ");
    else {			/* Define header			 */
	strcat(temp," ");
	strcat(hline," ");
	strcat(hline,temp);
    }
    if (argc == -1) exit(0);
    else clk();
}




/**********************************************************************\
* 								       *
* CLK sets up the window parameters and signals. It also contains the  *
* main program loop.						       *
* 								       *
\**********************************************************************/
clk()
{
    uflags flgs,bflgs;
    int moved(),halt();
    struct headstruc head;
    struct sgttyb tty;

    if (!nowin) {		/* Window parameters			 */
	flgs=(PMODE | LMODE | NOCURSOR | NOCPIN | SPECIAL);
	bflgs=(BX_MOVE | BX_CLOS);
	fd=clkwinop(&wins,615,860,850,600,100,100,WHITE,DLBORDER,'I',flgs,bflgs);
        dup(fd);

	strcpy(head.h_hdr,hline);
	Winheader(STDOUT,&head);
    }

    tty.sg_flags=RAW;		/* Set terminal to RAW mode		 */
    ioctl(STDOUT,TIOCSETP,&tty);
    
    signal(SIGUSR2,halt);	/* Define kill signal			 */

    timez *= 3600;
    interrupt=1;

    while(1) {			/* Main program loop			 */
	if (interrupt) {
	    interrupt=0;
	    signal(SIGUSR1,moved);
	    redraw();
	    sold=mold=hold=60;
	    skip=first=1;
	}
	else {
	    ticks();
	}
    }
}




/***********************************************************\
* 							    *
*  HALT exits the program				    *
* 							    *
\***********************************************************/
halt()
{
    signal(SIGUSR2,SIG_IGN);
    exit(0);
}




/********************************************************************\
* 								     *
* MOVED handles the redrawing of the screen when the clock has been  *
* moved, etc.							     *
* 								     *
\********************************************************************/
moved()
{
    signal(SIGUSR1,SIG_IGN);
    interrupt=1;
}




/***********************************************************\
* 							    *
*  REDRAW performs the actual drawing of the clock	    *
* 							    *
\***********************************************************/
redraw()
{
    /* Data for hour markings */
    static char starts[2][12] = {
	{ 95,89,73,50,28,11,5,11,28,50,73,89 },
	{ 50,73,89,95,89,73,50,28,11,5,11,28 }
    };
    static char lens[2][12] = {
	{ 90,85,70,50,30,15,10,15,30,50,70,85 },
	{ 50,70,85,90,85,70,50,30,15,10,15,30 }
    };
    
    unsigned char i;
    
    cls();			/* Draw clock				 */
    cur(95,50);
    arc(50,50,360,0,1);

    cur(50,50);
    for (i=0;i<12;i++) {	/* Draw hour markings			 */
	cur(starts[0][i],starts[1][i]);
	line(lens[0][i],lens[1][i],0,1);
    }
    wprint(STDOUT);
}




/*******************************************************************\
* 								    *
*  TICKS handles the actual timing.				    *
* 								    *
\*******************************************************************/
ticks()
{
    /* Data for clock arm positions */
    static char minpos[122] = {
	  50,54,57,61,64,67,70,73,76,78,80,82,83,84,85,85,85,84,83,82,80,
	  78,76,73,70,67,64,61,57,54,50,46,43,39,36,32,29,26,24,22,20,18,
	  17,16,15,15,15,16,17,18,20,22,24,26,29,32,36,39,43,46,50,
	  85,85,84,83,82,80,78,76,73,70,67,64,61,57,54,50,46,43,39,36,32,
	  29,26,24,22,20,18,17,16,15,15,15,16,17,18,20,22,24,26,29,32,36,
	  39,43,46,50,54,57,61,64,67,70,73,76,78,80,82,83,84,85,50 
    };
    static char hrpos[122]  = {
	  50,52,54,56,58,60,62,63,65,66,67,68,69,69,70,70,70,69,69,68,67,
	  66,65,63,62,60,58,56,54,52,50,48,46,44,42,40,38,37,35,34,33,32,
	  31,30,30,30,30,30,31,32,33,34,35,37,38,40,42,44,46,48,50,
	  70,70,69,69,68,67,66,65,63,62,60,58,56,54,52,50,48,46,44,42,40,
	  38,37,35,34,33,32,31,30,30,30,30,30,31,32,33,34,35,37,38,40,42,
	  44,46,48,50,52,54,56,58,60,62,63,65,66,67,68,69,69,70,50 
    };
    static char secpos[122] = {
	  50,55,59,64,68,72,76,80,83,86,89,91,93,94,95,95,95,94,93,91,89,
	  86,83,80,76,72,68,64,59,55,50,45,41,36,32,27,24,20,17,14,11,9,
	  7,6,5,5,5,6,7,9,11,13,16,20,23,27,31,36,40,45,50,
	  95,95,94,93,91,89,86,83,80,76,72,68,64,59,55,50,45,41,36,32,27,
	  24,20,17,14,11,9,7,6,5,5,5,6,7,9,11,13,16,20,23,27,31,36,40,45,
	  50,55,59,64,68,72,76,80,83,86,89,91,93,94,95,50 
    };

    static struct tm *localtime();
    static struct tm *times;
    long systime;
    char pm;
    
    if (!first && showsec && times->tm_sec) {
				/* Should clock not be synchronized?	 */
	if (++times->tm_sec>=60) times->tm_sec=0;
    }
    else {			/* OK, synchronize clock		 */
	time(&systime);		/* Get system time			 */
	systime += timez;	/* Do some converting			 */
	times=localtime(&systime);

	if (times->tm_hour >= 12) {
	    times->tm_hour -= 12;
	    pm=1;
	}
	else pm=0;

	if (first | !(times->tm_hour | times->tm_min)) {
	    			/* Should pm/am be displayed?		 */
	    if (pm) write(STDOUT,"[7;0Hpm",8);
	    else write(STDOUT,"[7;0Ham",8);
	}

	if (first | !(times->tm_min % 12)) {
	    			/* Time to draw hour arm?		 */
	    cur(50,50);
	    iline(hrpos[hold],hrpos[hold+61]);
	    cur(50,50);
	    hold=times->tm_hour*5+times->tm_min/12;
	    iline(hrpos[hold],hrpos[hold+61]);
	}

	cur(50,50);		/* Draw minute arm!			 */
	iline(minpos[mold],minpos[mold+61]);
	cur(50,50);
	iline(minpos[mold=times->tm_min],minpos[times->tm_min+61]);
    }

    /* This is always done, weither the clock is synchronized or not	 */
    if (showsec) {		/* Should seconds' arm be shown?	 */
	cur(50,50);
	iline(secpos[sold],secpos[sold+61]);
	cur(50,50);
	iline(secpos[sold=times->tm_sec],secpos[times->tm_sec+61]);
    }

    first=0;
    wprint(STDOUT);
    if (!interrupt)		/* If we don't have to redraw, sleep!	 */
        if (!showsec) sleep(60);
	else sleep(1);
}



/***********************************************************************\
* 								        *
* CLKWINOP opens a window on screen. The size and shape of the window are  *
* based on the given parameters.				        *
* 								        *
\***********************************************************************/
clkwinop(bp,xp0,yp0,xl0,yl0,xs,ys,clr,brd,fnt,flags,bflags)
struct winstruc *bp;
pix_d xp0,yp0,xl0,yl0,xs,ys;
short clr;
sint brd;
char fnt;
uflags flags,bflags;
{
    int fd;
    
    bp->wp_xorig=xp0;
    bp->wp_yorig=yp0;
    bp->wl_xorig=xl0;
    bp->wl_yorig=yl0;
    bp->wp_xsize=xs;
    bp->wp_ysize=ys;
    bp->wl_xsize=ys;
    bp->wl_ysize=xs;
    bp->wp_vxorig=0;
    bp->wp_vyorig=0;
    bp->wl_vxorig=0;
    bp->wl_vyorig=0;
    bp->wp_vxsize=xs;
    bp->wp_vysize=ys;
    bp->wl_vxsize=xs;
    bp->wl_vysize=ys;
    bp->w_color=clr;
    bp->w_border=brd;
    bp->wp_font=fnt;
    bp->wl_font=fnt;
    bp->w_rsig=16;
    bp->w_csig=17;
    bp->w_flags=flags;
    bp->w_boxes=bflags;

    if ((fd=open("/win",2))<0) exit(1);
    Wincreat(fd,bp);

    if (bp->w_rstat==W_OK) return(fd);
    else exit(1);
}
