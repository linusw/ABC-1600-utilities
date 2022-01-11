/************************************************************************/
/*      feprint.c            print fontfile document to a file

        by Rolf Loh
*/
/************************************************************************/

#include <stdio.h>
#include <termio.h>
#include <sys/ioctl.h>
#include "fedef.h"                      /* Include definitions          */


/************************************************************************/
/*                         D A T A      A R E A                         */

extern char refarea[ 0x7fff ];         /* work fontfile area            */
extern int eraflag;
extern int lang;


typedef unsigned char   uchar_t;


struct termio term;
struct termio termsav;
struct cgenblk ref_hdr;

char utbuf[1024];
char c;
int pixwidth,i,x;
char str[64];

static char *ptexts[2][1] = {
	{
	"fntprint: Can't create "
	},
	{
	"fntprint: Kan inte skapa "
	}
};


/************************************************************************/
fntprint(ifnam,ofnam)  /* Print a document to file whos name ofnam points to */
char ifnam[],ofnam[];  /* ifnam,ofnam are pointers to strings holding names  */
{
        int fp2;

        if(( fp2 = creat(ofnam, 0644)) == -1) {  /* open printer channel */
                cur(5,0);
                sendfill(ptexts[lang][0]);
                sendfill(ofnam);
                eraflag=1;
                return(1);
        }

        prcr(fp2);
        prlf(fp2);
        cstr();
        sprintf( &str[0],"          ABC1600 FONTFILE PRINTOUT\n\n\n");
        write(fp2, &str[0], 64 ); 
        cstr();
        sprintf( &str[0],"          Fontfile:        %s\n\n",ifnam);
        write(fp2, &str[0], 64 ); 
        cstr();
        sprintf( &str[0],"          Port(0),Land(1): %d\n\n",ref_hdr.cg_stat);
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Gen.number:      %d\n\n",ref_hdr.cg_fnumb);
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Font size:       %dx%d\n\n",ref_hdr.cg_fxsiz,ref_hdr.cg_fysiz);
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          First/last char: %x/%x (hex)\n\n",ref_hdr.cg_firstc,ref_hdr.cg_lastc);
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Baseline:        %d\n\n",ref_hdr.cg_baslin );
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Left margin:     %d\n\n",ref_hdr.cg_leftmg );
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Matrix size x:   %d\n\n",ref_hdr.cg_matx );
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Matrix size y:   %d\n\n",ref_hdr.cg_maty );
        write(fp2,&str[0],64);
        cstr();
        sprintf( &str[0],"          Style name:      %s\n\n",ref_hdr.cg_style );
        write(fp2,&str[0],64);

        fontprint( fp2);                /* convert and print    */

        close(fp2);                     /* close outfile        */

}

/************************************************************************/
fontprint(fp2)
int fp2;
{

        rawmode(fp2);               /* prepare tty-driver for graphics.. */

        pixwidth = 8 * ref_hdr.cg_bwidth  ;
        if( ref_hdr.cg_stat == 0 ) x=ref_hdr.cg_fxsiz;
        else x=ref_hdr.cg_fysiz;

        i=0;
        for( i=0; i<1024; i++ ){
                if( (i % x) == 0 ) c='\076'; /*set box-border */
                else c=1;               /* make border  */
                utbuf[i]=c;
        }
        prcr(fp2);
        prlf(fp2);
        prout( fp2,&utbuf[0], pixwidth );
        prcr(fp2);
        prlf(fp2);

        i=0;
        while( i < ref_hdr.cg_dumpl ) {
		prconv( &refarea[i], ref_hdr.cg_bwidth,pixwidth);
                prout( fp2,&utbuf[0], pixwidth );
                prnull(fp2);
                prcr(fp2);
                prlf(fp2);
                i += 7 * ref_hdr.cg_bwidth ;
        }
        prcr(fp2);
        prlf(fp2);
        prff(fp2);
        cookedmode(fp2);             /* re-set tty-driver    */
        close( fp2 );
}

/****************************************************************/
rawmode(fp)              /* set tty-driver in raw mode           */
int fp;
{
        ioctl(fp,TCGETA,&term);
	termsav = term;
        term.c_iflag &= ~IXOFF;		/* disable start/stop input control */
        term.c_lflag &= ~ECHO;		/* disable echoing		*/
	term.c_oflag &= ~OPOST;		/* disable post-processing	*/
        ioctl(fp,TCSETA,&term);
    
}

/****************************************************************/
cookedmode(fp)           /* set tty driver in 'normal' mode      */
int fp;
{
	termsav.c_lflag |= ECHO;
        ioctl(fp,TCSETA,&termsav);
}


/**************************************************************/
char msk[8] = { 
        '\200','\100','\040','\020','\010','\04','\02','\01' } 
;


prconv( buf,size,cnt )  /* convert data beginning at 'buf'    */
char *buf;              /* have byte size 'size',and          */
int size,cnt;           /* produce a pixelwidth of cnt        */
{                       /* 7 needles are used in the printer  */
        int i,j,k,l,m,p,q,h;

        p=0;
        h=0;
        for( i=0; i<cnt ; i++ ) {
                q=64;
                k = 0;   /* clear data   */
                for( j=0; j<7 ; j++ ) {
                        /* build needle data    */
                        l = buf[ h+(j*size) ];
                        m = msk[ p ];
                        if( ( l & m ) != 0 ) k += q;
                        q=q/2;
                }
                utbuf[i] = k ;
                p++;
                if( p==8 ) {
                        p=0;
                        h++;
                }
        }
}

/**************************************************************/
cstr()  /* clear string str     */
{
int i;

        for( i=0; i<sizeof(str); i++){
                str[i] = 0;
        }
}

char space[10] = { 
        ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };

/**************************************************************/
prout(fp,buf,cnt)       /* print out graphic data in 'buf'    */
int fp;                 /* count='cnt to file 'fp'            */
char *buf[];
int cnt;
{
        char tmp[4];

        write( fp, &space[0], 10 );      /* 10 spaces     */
        tmp[0]= 27;     /* esc  */
        tmp[1]= 75;     /* k    */
        tmp[2]= ( cnt & 0xff ); /* low part of cnt      */
        tmp[3]= ( ( cnt >> 8 ) & 0xff ) ;       /* high part of cnt */
        write( fp, &tmp[0], 4 );        /* set graphic mode     */
        write( fp, &buf[0], cnt );      /* write needle data    */

}

/**************************************************************/
prcr(fp)                /* print cr                           */
int fp;
{
        char tmp[4];

        tmp[0]= 13;     /* cr  */
        tmp[1]= 0;
        write( fp, &tmp[0], 2 );        /* cr   */

}



prlf(fp)
int fp;
{
        char tmp[6];

        tmp[0]= 27;     /* cr  */
        tmp[1]= 65;
        tmp[2]= 7;      /* mata 7 n}lar */
        tmp[3]= 10;     /* linefeed     */
        tmp[4]= 0;
        tmp[5]= 0;
        write( fp, &tmp[0], 6 );        /* lf   */

}


/**************************************************************/
prnull(fp)              /* print some nulls                   */
int fp;
{
        char tmp[6];

        tmp[0]= 0; 
        tmp[1]= 0;
        tmp[2]= 0;
        tmp[3]= 0;
        tmp[4]= 0;
        tmp[5]= 0;
        write( fp, &tmp[0], 6 );

}

/****************************************************************/
prff(fp)        /* new page,reset to 8 lines/inch               */
int fp;
{
        char tmp[4];

        tmp[0]= 12;     /* cr  */
        tmp[1]= 0;
        tmp[2]=27;
        tmp[3]=48;
        write( fp, &tmp[0], 4 );        /* cr   */

}
