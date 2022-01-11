/****************************************************************************/
/*        fewin.c         routines for fonteditor,associated with windows   */ 
/*                        and mouse handling                                */
/****************************************************************************/

#include <signal.h>
#include <win/w_const.h>
#include <win/w_types.h>
#include <win/w_structs.h>
#include <win/w_macros.h>

#define STDIN 0
#define STDOUT 1
#define STDERROUT 2

#define F0 I_PMODE+I_LMODE+I_INVERT+I_PRESS+I_RQST+I_LZERO
#define F1 I_PMODE+I_LMODE+I_PRESS+I_RQST+I_LZERO

extern struct cgenblk ref_hdr;
extern struct cgenblk wrk_hdr;
extern char infnam[];
extern int intflag;
extern int lang;
extern char *work;
extern int wrkcode;

short xorig[] = {
/*   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26 */
314,101, 70,141, 30, 70,100,439,340,320,360,340,    314,314,314,433,320,360,320,360,433,433,433,433,180 };
short yorig[] = {
504,540,491,450,410,408,408,407,460,440,440,420,    558,540,522,486,460,460,420,420,558,540,504,522,410 };
short xsize[] = {
 72, 79, 20, 39, 20, 20, 20,259, 20, 20, 20, 20,     72, 72, 72,215, 20, 20, 20, 20,215,215,215,215,100 };
short ysize[] = {
 18, 20, 29, 20,110, 22, 22, 20, 20, 20, 20, 20,     18, 18, 18, 18, 20, 20, 20, 20, 18, 18, 18, 18, 25 };

char *cmdseq[] = {
"\rq","l","c","s","r","w","i","x","u","h","j","n",    "t","z","p","b","uh","uj","nh","nj","T","Z","a","B","\r" };
short flags[] = {
 F0, F0, F0, F0, F0, F0, F0, F0, F0, F0, F0, F0,     F0, F0, F0, F0, F0, F0, F0, F0, F0, F0, F0, F0, F0 };

/* TEXT DEFINITIONS : 0-23 English, 24-47 Swedish */
char *wtexts[2][24] = {
	{
	"QUIT",
	"Ref",
	"Area",
	"Work",
	"Area",
	"DISK",
	"Show Work Area",
	"Turn",
	"Zoom",
	"Print",
	"Define Font Attributes",
	"Set Edit Box Size",
	"Turn Entire Font",
	"Zoom Entire Font",
	"Set Ascii Value",
	"Font Editor \033(C",
	"Ver. 5.12",
	"land,XxY,first/last,baseline",
	"Ref. area:                  ",
	"Work area:                  ",
	"Edit area:                  ",        
	"Winstat not OK\n\r",
	"RSIGNAL not OK\n\r",
	"OK"
	},
	{
	"AVSLUTA",
	"Ref",
	"Area",
	"Arbets",
	"Area",
	"DISK",
	"Visa arbetsarean",
	"Vrid",
	"Zooma",
	"Skriv",
	"Definiera fontdata",
	"S{tt red.rutans storlek",
	"V{nd hela fonten",
	"Zooma hela fonten",
	"S{tt ascii v{rde",
	"Font Editor \33(C",
	"Ver. 5.0",
	"land,XxY,f|rst/sist,baslinje",
	"Ref.area:                  ",
	"Arb.area:                  ",
	"Red.area:                  ",
	"Winstat FEL\n\r",
	"RSIGNAL FEL\n\r",
	"OK"
	}
};


struct winicon bp;
int rsignal;

winini()
{
        int i,c,j;
        char *ptr;


       for( i = 0; i < 25; i++ ) {
                bp.ip_xorig = xorig[i];
                bp.il_xorig = xorig[i];
                bp.ip_yorig = yorig[i];
                bp.il_yorig = yorig[i];
                bp.ip_xsize = xsize[i];
                bp.il_xsize = xsize[i];
                bp.ip_ysize = ysize[i];
                bp.il_ysize = ysize[i];

                ptr = cmdseq[i];                /* set up command-string */

                j=0;
                while(bp.i_cmdseq[j++] = *ptr++);

                bp.i_flags = flags[i];

                if( (c=Winicon(1, &bp)) < 0 ) {
                        return(1);
                }
                if( bp.i_rstat != W_OK ) {
                        return(1);
                }
        }
        wrmenu();                       /* display graphic menu */
        return(0);

}

/*******************************************************************/
wrmenu()
{
	if (lang) sendfill("\033(H");
	else sendfill("\033(I");
        bline(0,585,699,585,1);
        gcur(319,506);
        sendfill(wtexts[lang][0]);	/* Quit */
        gcur(47,550);		
        sendfill(wtexts[lang][1]);	/* Ref  */
        gcur(42,536);
        sendfill(wtexts[lang][2]);	/* Area */
	if (lang) gcur(73,460);
	else gcur(82,460);
        sendfill(wtexts[lang][3]);	/* Work */
        gcur(82,446);
        sendfill(wtexts[lang][4]);	/* Area */
        gcur(212,503);
        sendfill(wtexts[lang][5]);	/* Disk */
	if (lang) gcur(496,410);
	else gcur(505,410);
        sendfill(wtexts[lang][6]);	/* Show work area */
        gcur(319,560);
        sendfill(wtexts[lang][7]);	/* Turn */
        gcur(319,542);
        sendfill(wtexts[lang][8]);	/* Zoom */
        gcur(319,524);
        sendfill(wtexts[lang][9]);	/* Print */
        gcur(438,524);
        sendfill(wtexts[lang][10]);	/* Set Matpar */
        gcur(438,488);
        sendfill(wtexts[lang][11]);	/* Set size */
        gcur(438,560);
        sendfill(wtexts[lang][12]);	/* Turn seq. */
        gcur(438,542);
        sendfill(wtexts[lang][13]);	/* Zoom seq. */
        gcur(438,506);
        sendfill(wtexts[lang][14]);	/* Set ascii */
	gcur(221,416);
	sendfill(wtexts[lang][23]);	/* -Return- */

        rectang(20,520,100,580);       /* Ref.area              */
        rectang(60,430,140,490);       /* Work area             */
        rectang(180,440,280,580);      /* DISK                  */
        rectang(320,440,380,460);      /* left,right            */
        rectang(340,420,360,480);      /* up,down               */
        rectang(439,20,697,405);       /* File Display          */
        point(440,21,0,0);             /* erase display area    */
        fillpattn(696,404,1,0);        /* fill grey.(pattrn=1)  */
        rectang(105,549,175,551);      /* load file             */
        bline(105,549,110,542,1);
        bline(105,551,110,558,1);
        rectang(79,495,81,515);        /* copy ref to work      */
        bline(79,495,72,500,1);
        bline(81,495,88,500,1);
        rectang(145,459,175,461);      /* save file             */
        bline(175,461,170,468,1);
        bline(175,459,170,452,1);
        rectang(39,415,41,515);        /* ref area char load    */
        bline(39,415,32,420,1);
        bline(41,415,48,420,1);
        rectang(79,415,81,425);      /* work area char load   */
        bline(79,415,72,420,1);
        bline(81,415,88,420,1);
        rectang(109,415,111,425);      /* insert char           */
        bline(109,425,102,420,1);
        bline(111,425,118,420,1);

        bline(380,450,362,458,1);        /* >                     */
        bline(380,450,362,442,1);
        bline(320,450,338,458,1);        /* <                     */
        bline(320,450,338,442,1);
        bline(350,480,342,462,1);        /* up                    */
        bline(350,480,358,462,1);
        bline(350,420,342,438,1);        /* down                  */
        bline(350,420,358,438,1);

        update(1);             /* Update raster        */

        sendfill("\033(L");
        gcur(550,670);
        sendfill(wtexts[lang][15]);	/* Font editor */
        gcur(572,660);
        sendfill(wtexts[lang][16]);	/* ver. 3.3 */

        gcur(260,650);
	if (lang) sendfill("\033(B");
        sendfill(wtexts[lang][17]);	/* land,XxY,first/last,baseline */

	if (lang) sendfill(" \033(H");
	else sendfill(" \033(I");
        cur(1,0);
        sendfill(wtexts[lang][18]);	/* REF.AREA: */
        cur(2,0);
        sendfill(wtexts[lang][19]);	/* WORKAREA: */
        cur(3,0);
        sendfill(wtexts[lang][20]);	/* EDITAREA: */
        

}

/*
 * Initialize things to take care of redrawing the graphics
 *  when the window has changed in some way.
 *  Returns a non zero value if the initialization fails
*/
int
w_reini()
{
        struct winstruc wd;
        struct winstruc *wdp;
        int             w_rstrp();

        wdp = &wd;
        if( Winstat(STDIN,wdp) < 0 || wdp->w_rstat != W_OK ) {
                sendfill(wtexts[lang][21]);/* Winstat not OK */
                return(1);
        }
        if( (rsignal = wdp->w_rsig)==0 ) {
                sendfill(wtexts[lang][22]);/* rsignal not OK */
                return(1);
        }
        signal(rsignal,w_rstrp);
        return(0);
}


w_rstrp()       /* This is the signal interrupt routine          */
{               /*   that redraws the window when it has changed */
	struct winstruc wd;
        int i;
        extern char oldarea[];

        signal(rsignal,SIG_IGN);	/* Ignore signals */

	if (Winstat(STDIN,&wd)<0) {	/* Check if on level 0 */
                sendfill(wtexts[lang][21]);/* Winstat not OK */
                return(1);
        }
	if (wd.w_level != 0) {	/* If not lvl 0, reset and return */
	    signal(rsignal,w_rstrp);
	    return;
	}

        wrmenu();               /* redraw boxes and arrows */
        update(1);               /* redraw raster           */
        /* clear oldarea so display routine displays all   */
        for( i=0; i<32*32; i++ ){
                oldarea[i] = 0;
        }
        
        cur(1,11);
        sendfill(infnam);
	cur(2,11);
	sendfill(work);
        cur(1,30);
        show_par(&ref_hdr);
	if (work[0]!=' ') {
	        cur(2,30);
		show_par(&wrk_hdr);
	}
        print_chcode();

        point(440,21,0,0);                      /* erase display area   */
        fillpattn(696,404,1,0);         /* fill grey.(pattrn=1) */

        display();              /* display edit area       */

        cur(4,0);
        sendfill("ok>");
        sendfill("\033[?35h");    /*cursor off    */
        send();
        intflag=1;              /* tell strin routine to terminate      */

        signal(rsignal,w_rstrp);
  }

/*********************************************************************/
gcur(x,y)       /* set text cursor at pixel x,y */
int x,y;
{
        point(x,y,0);
        sendfill("\033:1H");
}
/* end of fewin.c */
