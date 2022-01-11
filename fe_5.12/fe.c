/************************************************************************/
/*      fe.c

        Routines for font definition

        by Rolf Loh
*/
/************************************************************************/

#include <win/w_macros.h>
#undef FIOCLEX
#undef FIONCLEX
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>
#include "fedef.h"                      /* Include definitions          */
#define  STDIN     0
#define  STDOUT    1
#define  STDERROUT 2

/************************************************************************/
/*                         D A T A      A R E A                         */

struct cgenblk ref_hdr;         /* Allocate font information block      */
struct cgenblk wrk_hdr;         /*                                      */
char refarea[ 0x7fff ];         /* reference font area                  */
char wrkarea[ 0x7fff ];         /* work fontfile area                   */
char *langbuff,*getenv();

/* TEXT DEFINITION : 0-27 English 28-55 Swedish */
char *texts[2][31] = {
	{
	"Error: Font editor must be started by window menu selection.\n",
	"The work area is edited. 'QUIT' again if you want to exit without saving.",
	"Ascii code (in hex) for char (",
	"Error: Ascii code too low",
	"Error: Ascii code too high",
	"File:               ",
	"Opening ",
	"Error: Can't open ",
	" bytes read from ",
	"Type (hex):",
	"     p/l-flag:",
	"Baseline:",
	"  First (hex):",
	"  Last (hex):",
	"  Width:",
	"Default:",
	"File to write to: ",
	"Error: Can't create ",
	"All written.                   ",
	"File: ",
	"The work area is edited. Overwrite? (y/n):",
	"Copied      ",
	"   p/l  x-size  y-size  first-char (hex)  last-char (hex)",
	"Defined     ",
	"Give matrix parameters:",
	"baseline left_margin matsiz_x matsiz_y",
	"Give font number and style name (6 chars):",
	"f_numb style_name"
	},
	{
	"Fel: Fonteditorn m}ste startas fr}n f|nstermenyn.\n",
	"Arbetsarean {r modifierad. 'AVSLUTA' igen om Du vill sluta utan att spara.",
	"Ascii kod (i hex) f|r tecknet (",
	"Fel: Ascii kod f|r l}g",
	"Fel: Ascii kod f|r h|g",
	"Fil:                ",
	"\\ppnar ",
	"Fel: Kan ej |ppna ",
	" bytes inl{sta fr}n ",
	"Typ (hex):",
	"     p/l-flagga:",
	"Baslinje:",
	"  F|rsta (hex):",
	"  Sista (hex):",
	"  Bredd:",
	"Standardv{rde:",
	"Filnamn att skriva till: ",
	"Fel: Kan ej skapa ",
	"Allt skrivet.                  ",
	"Fil: ",
	"Arbetsarean {r modifierad. Skriv |ver {nd}? (j/n):",
	"Kopierad      ",
	"   p/l  x-stlk  y-stlk  f|rsta-tkn (hex)  sista-tkn (hex)",
	"Definierad    ",
	"Ge matris parametrarna:",
	"baslinje v{nstermarginal matrstlk_x matrstlk_y",
	"Ge fontnummer och stilnamn (6 tecken):",
	"f_num stilnamn"
	}
};
int lang;

/* EDIT AREA */
char edarea[ BMAX_X * BMAX_Y  ];/* character edit area                  */
char dum1[4];
char oldarea[ BMAX_X * BMAX_Y ];/* compare edit area                    */
char dum2[4];
char old[ BMAX_X * BMAX_Y ];    /* used for zoom,turn                   */
char dum3[4];

int loadcode;                   /* current edit character               */
int inscode;
int dirty;                      /* Flag for dirty edit area             */
int pixsizx;                    /* pixelsize in pixels on edit raster x */
int pixsizy;                    /* pixelsize in pixels on edit raster y */
int baslin;                     /* basline of current character box     */
int edsiz_x ;                   /* Current box size x                   */
int edsiz_y ;                   /* Current box size y                   */
int x_start,y_start;            /* edit raster position                 */
int pl;                         /* global portrait/landscape flag       */
int x,y;                        /* pixel-cursor address                 */
int eraflag;                    /* flag for erase command info          */
int tmp;
int echoflag;                   /* flag for command input echo          */
int silentflag;
int intflag;                    /* flag for moved window (redraw)       */
int cnt=0;                      /* chars left in keybuf                 */
char *work,*dst,*src ;          /* string pointers                      */
char infnam[128];               /* infilename at 'load ref.area         */
char utfnam[128];               /* outfilename at 'save wrk.area'       */
char inbuf[256];                /* input buffer                         */
char speed[4096];               /* speeding up buffer of screen output  */
int sptr;                       /* pointer into speed[]                 */
char string[256];                  /* temp string buffer                */
/*                                                                      */ 
/************************************************************************/


main(argc,argv)         /* If argc >1, assume VT100 text terminal,      */
int argc;               /* otherwise ABC1600-screen is assumed          */
char *argv[];           /* If used with widow-handler,mouse is used     */
{
        char c ;
        int i;

	lang=0;			     /* Determine what language to use */
	if ((langbuff=getenv("LANGUAGE"))!=NULL)
	    if (strcmp(langbuff,"swedish")==0) lang=1;
	    
        cls();                         /* clear screen                 */

        pixsizx=12;                    /* set default values           */
        pixsizy=12;
        x_start=20;
        y_start=20;
        edsiz_x=32;
        edsiz_y=32;

	work=" ";

        inithdr(&ref_hdr);              /* set default header           */
        inithdr(&wrk_hdr);              /* set default header           */

        /*
         * Initialize the automatic redrawing of the window.
         */
 
        if( w_reini() != 0 ) {
                sendfill(texts[lang][0]);
		send();
		exit();
		/* Must be started from window menu */
        }


	if (lang) sendfill("\33(H");   /* set swedish mode	       */
        tmp = winini();         /* initialize window menu       */


        if( tmp != 0 ) {
                Rmicons(0);             /* remove icons         */
                exit();
        }

        sendfill("\033[1v");    /* cursor off                   */

        setraw();                       /* set raw mode                 */

        update(1);                      /* draw edit raster             */

        /* reset compare area  */
        for( i=0; i < 32*32; i++ ) {
                oldarea[i] = 0;
        }

        /* reset pixel-cursor address */
        x = 0;
        y = 0;
        cursor(1);              /* display pixel-cursor */
        loadcode=inscode = '\101';   /* set box to 0x41 (A) for now  */

        if( ldfnt(&ref_hdr,refarea) == 0 ) { /* load fontfile to refarea */
                tmp = copy();           /* copy it to work area also    */
                loadchr(&wrk_hdr,wrkarea); /* get box          */
        }
	print_chcode();

/****** MAIN COMMAND LOOP ***********************************************/
resume:
        for ( ; ; ) {                  /* start loop                    */
                sendfill("\033:8M");    /* set mouse mode: report on button */
		if (lang) sendfill("\033(H");/* set swedish mode       */
		else sendfill("\033(I");
                if( eraflag != 0 ) {
                        cur(4,3);
                        multout(74,' ');
                        cur(4,3);
                }
                send();                 /* send to terminal     */
                cur(4,0);
                sendfill("ok>");
                c = getone();           /* get a character      */
                intflag = 0;            /* reset int flag.(used by strin) */
                cur(4,0);
                sendfill("  ");
                if( eraflag != 0 ) {
                        cur(4,3);
                        multout(74,' ');
                        cur(5,0);
                        multout(77,' ');
                        cur(6,0);
                        multout(77,' ');
                        cur(7,0);
                        multout(77,' ');
                        cur(4,3);

                        eraflag = 0;
                }
                send();                 /* send to terminal     */

		if (lang) sendfill("\033(H");/* set swedish mode       */
		else sendfill("\033(I");

                switch ( c ) {

                case 'a':
			/* get ascii codes */
			code_in(&inscode);
                        break;
                case 'q':
		case 'A':
                        goto exit1;             /* exit from program            */
                        break;

                case 'l':
                        tmp=ldfnt(&ref_hdr,refarea); /* load fontfile into ref area  */
                        break;

                case 's':
                        savfnt(&wrk_hdr,wrkarea);/* save fontfile from work area */
                        break;

                case 'c':
		case 'k':
                        tmp = copy();           /* copy ref area into work area */
                        break;


                case 'r':
                        code_in(&loadcode);
                        loadchr(&ref_hdr,refarea);
						   /* load char fr.ref area */
                        break;                     /* to edit area          */

                case 'w':
                        code_in(&loadcode);
						   /* get ascii code       */
                        loadchr(&wrk_hdr,wrkarea);
						  /* load char fr. work area*/
                        break;                  /* to edit area           */

                case 'i':
			code_in(&inscode);
                        insert(&wrk_hdr,wrkarea);/* save character into      */
                        break;                   /*      work area           */

                case 'b':
                        boxdef(&wrk_hdr);       /* define work area box-size */
                        break;

                case 'B':
                        boxdef2(&wrk_hdr);       /* define other param.  */
                        break;

                case 't':               /* turn character clockwise     */
		case 'v':
                        turn();
                        break;

                case 'T':               /* turn all characters clockwise */
		case 'V':
                        TURN();
                        break;                        

                case 'p':               /* print document               */
                        setcooked();
                        fntprint(dst,"/dev/lp"); /* dst=name of loaded file */
                        eraflag=1;
                        setraw();
                        break;

                case 'x':               /* show work area on screen     */
			show();
                        break;

                case '7':
                        cursor(0);      /* erase cursor         */
                        y--; if (y<0) y=0;
                        x--; if (x<0) x=0;
                        cursor(1);      /* paint cursor         */
                        break;
                case '8':
                        cursor(0);      /* erase cursor         */
                        y--; if (y<0) y=0;
                        cursor(1);      /* paint cursor         */
                        break;
                case '9':
                        cursor(0);      /* erase cursor         */
                        y--; if (y<0) y=0;
                        x++; if (x>edsiz_x-1) x=edsiz_x-1;
                        cursor(1);      /* paint cursor         */
                        break;
                case '4':
                        cursor(0);
                        x--; if (x<0) x=0;
                        cursor(1);
                        break;
                case '5':
                        dispbit(2);  /* toggle current bit   */
                        cursor(1);
                        break;
                case '6':
                        cursor(0);
                        x++; if (x>edsiz_x-1) x=edsiz_x-1;
                        cursor(1);
                        break;
                case '1':
                        cursor(0);      /* erase cursor         */
                        x--; if (x<0) x=0;
                        y++; if (y>edsiz_y-1) y=edsiz_y-1;
                        cursor(1);      /* paint cursor         */
                        break;
                case '2':
                        cursor(0);
                        y++; if (y>edsiz_y-1) y=edsiz_y-1;
                        cursor(1);
                        break;
                case '3':
                        cursor(0);      /* erase cursor         */
                        y++; if (y>edsiz_y-1) y=edsiz_y-1;
                        x++; if (x>edsiz_x-1) x=edsiz_x-1;
                        cursor(1);      /* paint cursor         */
                        break;
                case 'u':
                        movup();
                        break;
                case 'n':
                        movdown();
                        break;
                case 'h':
                        movleft();
                        break;
                case 'j':
                        movright();
                        break;
                case 'z':
                        getzpar();
                        zoom();
                        break;

                case 'Z':
                        ZOOM();         /* Automatic zoom loop  */
                        break;

                case '\033':
                        cnt=0;          /* clear keybuf         */
                        mousepix();     /* read mouse pos       */
                        break;

                }       /* end of switch        */

        }

exit1:;
        if( dirty == 1 ) {
                cur(5,0);
                sendfill(texts[lang][1]);
	 	/* Work area edited, exit anyway? */
                if((c=getone()) != 'q' ) {
		        if (c=='\r' && cnt>0) c=getone();
			if (c!='q') {
			        cnt=0;	        /* Clear keyboard buffer */
				cur(5,0);
				multout(77,' ');
				goto resume;
			}
			/* save workarea on file */
		}
        }
        Rmicons(0);                             /* remove icons         */
        setcooked();                            /* remove raw mode      */
        sendfill("\033[0v");                    /* cursor on            */
        cls();                                  /* clear screen         */

        send();                                 /* flush buffer         */
}       /* main */

/***************************************************************************/
code_in(code)           /* get ascii code to 'code'           */
int *code;
{
                        cur(5,0);
                        sendfill(texts[lang][2]);
			/* ascii code for char( */
                        hexout(*code);
                        sendfill("):");
                        eraflag=1;
                        tmp=strin(&inbuf[0]);
                        if(tmp) sscanf(&inbuf[0],"%x", code);

                        /* check input  */
                        if( *code < wrk_hdr.cg_firstc ){
                                cur(6,0);
                                sendfill(texts[lang][3]);
				/* Too low ascii */
                                *code=wrk_hdr.cg_firstc;
                        }
                        if( *code > wrk_hdr.cg_lastc ) {
                                cur(6,0);
                                sendfill(texts[lang][4]);
				/* Too high ascii */
                                *code=wrk_hdr.cg_lastc;
                        }
                        print_chcode();
}

/************************************************************************/
print_chcode()
{
                        /* print current character      */
                        cur(3,15);
                        hexout(inscode);sendfill(" (");chrout(inscode);
                        sendfill(")");
}
/************************************************************************/
TURN()                  /* Turn all boxes from ref.area to work area    */
{
        int i;

        setcooked();    /* make possible to break                       */
        for( i=0 ; i<sizeof(wrkarea) ; i++ ) {
                wrkarea[i] = 0;
        }

                        for( loadcode = wrk_hdr.cg_firstc; 
                                loadcode <= wrk_hdr.cg_lastc; loadcode++) {
                                cur(6,0);hexout(loadcode);   /* trace     */
                                send();
                                silentflag=1;
                                loadchr(&ref_hdr,refarea);
							   /* load box  */
                                silentflag=0;
                                turn();                    /* turn it   */
                                insert(&wrk_hdr,wrkarea);  /* put back  */
                                if( check_quit() ) break;
                        }
        setraw();
}
/************************************************************************/
ZOOM()                  /* zoom all boxes from ref.area to work area    */
{
        int i;

        setcooked();    /* make possible to break                       */
        for( i=0 ; i<sizeof(wrkarea) ; i++ ) {
                wrkarea[i] = 0;
        }

                        for( loadcode = wrk_hdr.cg_firstc;
                                loadcode <= wrk_hdr.cg_lastc; loadcode++) {
                                cur(6,0);hexout(loadcode);   /* trace     */
                                send();
                                silentflag=1;
                                loadchr(&ref_hdr,refarea);
							   /* load box  */
                                silentflag=0;
                                zoom();                    /* process it*/
                                insert(&wrk_hdr,wrkarea);  /* put back  */
                                if( check_quit() ) break;
                        }
        setraw();
}

/************************************************************************/
inithdr(hdrptr)
struct cgenblk *hdrptr;
{
        hdrptr->cg_magic = 0xffff;      /* "this is a font-file"*/
        hdrptr->cg_stat = 0;            /* portrait font        */
        hdrptr->cg_fnumb = 0;           /* font number(not used)*/
        hdrptr->cg_fxsiz = 32;          /* box size horizontal  */
        hdrptr->cg_fysiz = 32;          /* box size vertical    */
        hdrptr->cg_firstc = ' ';        /* ascii of first char  */
        hdrptr->cg_lastc = 0x7f;        /* ascii of last char   */
        hdrptr->cg_bwidth = 32;         /* # of bytes/line      */
        hdrptr->cg_dumpl = 32*((((0x7f-0x20)/8)+1)*32);
        hdrptr->cg_baslin = 0;
        hdrptr->cg_leftmg = 0;
        hdrptr->cg_matx = 32;
        hdrptr->cg_maty = 32;
}


/************************************************************************/
ldfnt( hdrptr,bufptr )          /* load font into memory                */
struct cgenblk *hdrptr;
char *bufptr;
{
        int fp ;
        int bytcnt;

        eraflag=1;
        cur(5,0);
        sendfill(texts[lang][5]);
	/* File: */

        cur(5,6);
        tmp = strin(&infnam[0]);                /* input filename       */
        if( tmp == 0 ){
                cur(5,0);
                sendfill("     ");
                cur(1,30);
                show_par(hdrptr);                /* show parameters      */
                return(1);
        }
        
        src = &infnam[0];
        dst = &utfnam[0];
        utfnam[0] = 0;                          /* clear string         */
        strcat(dst,src);                        /* dst=basename of infile */
        cur(5,0);
        sendfill(texts[lang][6]);
	/* Opening */
        sendfill(dst);
        send();
        if( (fp = open( dst, 0)) == -1) {       /* try own directory    */
                utfnam[0] = 0;
                src = "/usr/lib/abcfont/port/";
                strcat(dst,src);
                src = &infnam[0];
                strcat(dst,src);
                cur(5,0);
                sendfill(texts[lang][6]);
		/* Opening */
                sendfill(dst);
                send();
                if( (fp = open( dst, 0)) == -1) {   /* port dir.*/
                   utfnam[0] = 0;
                   src = "/usr/lib/abcfont/land/";
                   strcat(dst,src);
                   src = &infnam[0];
                   strcat(dst,src);
                   cur(5,0);
                   sendfill(texts[lang][6]);
		   /* Opening */
                   sendfill(dst);
                   send();
                   if( (fp = open( dst, 0)) == -1) { /* land dir.*/
                      cur(6,0);
                      sendfill(texts[lang][7]);
		      /* Can't open */
                      sendfill(infnam);
                      infnam[0] = 0;
                      utfnam[0] = 0;  
                      return(1);
                   }
                }
        } 
        if ( fp >0 ) {
                cur(1,11);
                sendfill(infnam);
                sendfill("                    ");

                /* Read header          */
                bytcnt = read( fp,(char*)hdrptr,sizeof(ref_hdr));
                /* Read data            */
                bytcnt = read( fp,bufptr,0x7fff);      /* Read data     */

                cur(5,0);
                numout(bytcnt);
                sendfill(texts[lang][8]);
		/* Bytes read from */				
                sendfill(dst);
                sendfill("                 ");
                cur(6,0);
                sendfill(texts[lang][9]);
		/* Type: */
                hexout(hdrptr->cg_magic/256);
		hexout(hdrptr->cg_magic%256);
                sendfill(texts[lang][10]);
		/* p/l flag */
		if (hdrptr->cg_stat) sendfill("l");
		else sendfill("p");
                sendfill(" X:");
                numout(hdrptr->cg_fxsiz);
                sendfill(" Y:");
                numout(hdrptr->cg_fysiz);
                cur(7,0);
                sendfill(texts[lang][11]);
		/* Baseline */
                numout(hdrptr->cg_baslin);
                sendfill(texts[lang][12]);
		/* First */
                hexout(hdrptr->cg_firstc);
                sendfill(texts[lang][13]);
		/* Last */
                hexout(hdrptr->cg_lastc);
                sendfill(texts[lang][14]);
		/* Width */
                numout(hdrptr->cg_bwidth);

                if( hdrptr->cg_magic != 0xffff ) {
                        inithdr(hdrptr);                /* set default if */
                }                                       /* not fontfile   */

                eraflag=1;
                cur(1,30);
                show_par(hdrptr);

                return(0);
        }

}

/************************************************************************/
show_par(hdrptr)        /* display parameters from a header */
struct cgenblk *hdrptr; /* hdrptr is pointer to header area */
{
		if (hdrptr->cg_stat) sendfill("l ");
		else sendfill("p ");
                numout(hdrptr->cg_fxsiz);
                sendfill("x");
                numout(hdrptr->cg_fysiz);
                sendfill(" ");
                hexout(hdrptr->cg_firstc);
                sendfill(" ");
                hexout(hdrptr->cg_lastc);
                sendfill(" ");
                hexout(hdrptr->cg_baslin);
                sendfill("   ");
}



/************************************************************************/
savfnt(hdrptr,bufptr)
struct cgenblk *hdrptr;
char *bufptr;
{
        int fp ;
        int bytcnt;

        eraflag = 1;
        cur(6,0);
        sendfill(texts[lang][15]);
	/* Default */
        dst = &utfnam[0];                       /* use last loaded filename */
        sendfill(dst);
        cur(5,0);
        sendfill(texts[lang][16]);
	/* File to write to */
        eraflag=1;                              /* set erase flag       */
        tmp = strin(&infnam[0]);                /* input filename       */
        if( tmp != 0 ) {        
                dst = &infnam[0];               /* use new name         */
        }

        if( (fp = creat(dst,0644)) == -1 ) {
                cur(5,0);
                sendfill(texts[lang][17]);
		/* Can't create */
                sendfill(dst);
        } 
        else {
                /* write header          */
                bytcnt = write( fp, (char*)hdrptr, sizeof(ref_hdr));
                /* write data            */
                bytcnt = write( fp, bufptr, hdrptr->cg_dumpl);
                dirty = 0;              /* reset dirty flag     */
                if( bytcnt == hdrptr->cg_dumpl ) {
                        cur(5,0);
                        sendfill(texts[lang][18]);
			/* All written */
                        cur(6,0);
                        sendfill(texts[lang][19]);
			/* File: */
			sendfill(dst);
			
                        sendfill("                                   ");
                }
                bytcnt = close( fp );
        }

}


/************************************************************************/
copy()       /* copy reference area to work area     */
{
        int i;
	char answer;

        if( dirty == 1 ) {
                cur(6,0);
                sendfill(texts[lang][20]);
		/* work area edited, overwrite? */
                eraflag=1;
		answer=getone();
		cur(6,0);
		multout(77,' ');
                if(answer != 'y' && answer != 'j' && answer != '\r')
			return(1);
        }

        point(440,21,0,0);                      /* erase display area   */
        fillpattn(696,404,1,0);         /* fill grey.(pattrn=1) */

        eraseraster();
        wrk_hdr = ref_hdr;
        pl = ref_hdr.cg_stat;         /* portrait/landscape flag: 0=portr. */
        if( pl == 0 ) {
                edsiz_x = ref_hdr.cg_fxsiz;
                edsiz_y = ref_hdr.cg_fysiz;
        }
        else {
                edsiz_y = ref_hdr.cg_fxsiz;
                edsiz_x = ref_hdr.cg_fysiz;
        }
        for( i=0 ; i<sizeof(wrkarea) ; i++ ) {
                wrkarea[i] = refarea[i];
        }
        update(1);                              /* draw new raster      */
        for( i=0; i<BMAX_X*BMAX_Y; i++){        /* be shure display all */
                oldarea[i]=0;
        }
        loadchr(&wrk_hdr,wrkarea);	        /* load current char    */
        display();                              /* show current content */
        cur(2,11);
	work=texts[lang][21];
        sendfill(work);
	/* Copied */
        cur(2,30);
        show_par(&wrk_hdr);
	print_chcode();				/* Show character	*/

        dirty = 0;                              /* reset dirty flag     */
        return(0);
}



/************************************************************************/
boxdef(hdrptr)        /* define header of workfile */
struct cgenblk *hdrptr; /* hdrptr is a pointer to a structure like cgenblk */
{
        int p,x,y,first,last;
	char cp;

        eraflag =1;
        cur(5,0);
        sendfill(texts[lang][22]);
	/* p/l x-size y-size first-char last-char */
        cur(6,0);
        sendfill("ex. p 12 26 20 7f");
        cur(7,5);                    /* input p,x,y,first,last */
        strin(&inbuf[0]);
        sscanf(&inbuf[0],"%c %d %d %x %x",&cp,&x,&y,&first,&last);
	if (cp=='L' || cp=='l') p=1;
	else p=0;

        /* check input parameters */
        if( x < 0 )  x=0;
        if( x > 32)  x=32;
        if( y < 0 )  y=0;
        if( y > 32)  y=32;
        if( first < 0 ) first=0;
        if( first > 0x7f ) first=0x7f;
        if( last < 0 ) last=0;
        if( last > 0x7f ) last=0x7f;


        hdrptr->cg_stat = p;
        hdrptr->cg_firstc = first;
        hdrptr->cg_lastc = last;
        pl = p;
        eraseraster();
        update(0);                      /* erase raster */

        if( pl == 0 ) {                 /* portrait */
                edsiz_x = x;
                edsiz_y = y;
        }

        else {                          /* landscape */
                edsiz_x = y;
                edsiz_y = x;
        }
        hdrptr->cg_fxsiz = x;
        hdrptr->cg_fysiz = y;
        hdrptr->cg_dumpl = 32*( ( ((last-first) / (256/x))+1 ) * y )  ;

        hdrptr->cg_bwidth = 32; /* use all 256 bits in line */
        update(1);
        cur(2,11);
	work=texts[lang][23];
        sendfill(work);
	/* Defined */
        cur(2,30);
        show_par(hdrptr);
        display();
}

/**************************************************************************/
boxdef2(hdrptr)
struct cgenblk *hdrptr;
{
        int i,baslin,fnum,leftmg,matx,maty;
        char style[32];
        cur(5,0);
        sendfill(texts[lang][24]);
	/* Give matrix parameters */
        cur(6,0);
        sendfill(texts[lang][25]);
	/* Baseline leftmg matsiz_x matsiz_y */
        eraflag=1;
        cur(7,0);
        tmp = strin(&inbuf[0]);
        tmp=sscanf(&inbuf[0],"%d %d %d %d",&baslin,&leftmg,&matx,&maty);

        cur(5,0);
        multout(77,' ');
        cur(5,0);
        sendfill(texts[lang][26]);
	/* Give font number and style name */
        cur(6,0);
        multout(77,' ');
        cur(6,0);
        sendfill(texts[lang][27]);
	/* f_numb style_name */
        cur(7,0);
        multout(77,' ');
        cur(7,0);
        tmp = strin(&inbuf[0]);
        tmp = sscanf(&inbuf[0],"%d %s",&fnum,&style[0]);
        if( baslin < 0 )  baslin=0;
        if( baslin > 32)  baslin=0;

        hdrptr->cg_fnumb = fnum;
        hdrptr->cg_baslin = baslin;
        hdrptr->cg_leftmg = leftmg;
        hdrptr->cg_matx = matx;
        hdrptr->cg_maty = maty;

        for( i=0; i<sizeof(hdrptr->cg_style); i++) {
                hdrptr->cg_style[i] = style[i];
        }
        cur(2,30);
        show_par(&wrk_hdr);
}
/* end of fe.c */

