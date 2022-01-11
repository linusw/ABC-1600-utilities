/************************************************************************/
/*      fesub.c                                                         */
/************************************************************************/

#include <errno.h>
#include <sgtty.h>
#include "fedef.h"

#define STDIN 0
#define STDOUT 1

extern int x,y;
extern char edarea[];
extern int eraflag;
extern char inbuf[256];
struct sgttyb ttysave;
extern int errno;
extern char speed[];
extern int sptr;
extern char str[];
extern int echoflag;
extern int silentflag;
extern int intflag;
extern int edsiz_x,edsiz_y;
extern int cnt;        /* # of characters left in keybuf[]*/

/************************************************************************/
setraw()        /* set raw mode                                */
{

        ioctl(0,TIOCGETP,&ttysave);
        ttysave.sg_flags |= RAW;
        ttysave.sg_flags &= ~ECHO;
        ioctl(0,TIOCSETP,&ttysave);
}

/************************************************************************/
setcooked()        /* set terminal cooked mode                  */
{

        ttysave.sg_flags &= ~RAW;
        ttysave.sg_flags |= ECHO;
        ioctl(0,TIOCSETP,&ttysave);
}


/************************************************************************/
getone()        /* get one character with echo                  */
{
        static char     keybuf[256];     /* tty read buffer      */
        static char     *inbufp;        /* pointer to keybuf[]   */

        if( echoflag == 1) sendfill("\033[?35l");    /* cursor on */
        if( sptr>0 ) send();            /* flush buffer         */
        if( cnt == 0 ) {


        /* software interrupt can occur */
        while( cnt <= 0 ) {                                       
              if((cnt = read(STDIN,&keybuf[0],256)) <= 0 ) {      
/*                    if( (cnt == -1) && (errno == EINTR)  ) {          */
/*                        intflag = 1;                                  */
/*                        continue;                                     */
/*                    }                                                 */
/*                  if( (cnt == -1) && (errno != EINTR) ) exit();     */
              }                                                   
        }                                                         

  
            inbufp = &keybuf[0];
        }
        if( cnt <= 0 ) return( 10 );     /* interrupt,return a 'lf' */
        cnt--;
        if( echoflag == 1) write(STDOUT,inbufp,1);         /* echo char   */
        return(*inbufp++ & 255);
}


/************************************************************************/
point(x,y,op,colnr)
int x,y,op,colnr;
{
        if( op == 1 ) op = 0;
        else op = 1;
        sendfill("\033:");
        numout(x);
        sendfill(";");
        numout(y);
        sendfill(";");
        numout(op);
        sendfill(";");
        numout(colnr);
        sendfill("p");
}
/************************************************************************/
fill(x,y,op)
int x,y,op;
{
        sendfill("\033:");
        numout(x);
        sendfill(";");
        numout(y);
        sendfill(";;");
        numout(op);
        sendfill("f");
}

/************************************************************************/
fillpattn(x,y,pattn,op)
int x,y,pattn,op;
{
        sendfill("\033:");
        numout(x);
        sendfill(";");
        numout(y);
        sendfill(";");
        numout(pattn);
        sendfill(";");
        numout(op);
        sendfill("f");
}

/************************************************************************/
draw(x,y,pattrn,colnr)
int x,y,pattrn,colnr;
{
        sendfill("\033:");
        numout(x);
        sendfill(";");
        numout(y);
        sendfill(";");
        numout(pattrn);
        sendfill(";");
        numout(colnr);
        sendfill("d");
} 

/************************************************************************/
bline(x1,y1,x2,y2, colnr)        /* draw line from x1,y1 to x2,y2        */
int x1,y1,x2,y2, colnr;
{
        point(x1,y1,0,0);
        draw(x2,y2,0,colnr);
}       /* bline */



/************************************************************************/
rectang(x1,y1,x2,y2)            /* draw a rectanle with its corners in  */
register int x1,y1,x2,y2;       /* x1,y1 and x2,y2                      */
{
        bline(x1,y1,x2,y1,1);
        bline(x2,y1,x2,y2,1);
        bline(x2,y2,x1,y2,1);
        bline(x1,y2,x1,y1,  1);
}       /* rectang */


/************************************************************************/
cls()
{
        sendfill("\033[;H\033[2J");         /* Clear Screen (VT100      */
}

/************************************************************************/
fillpnt(x0,y0,op)       /* x0,y0=pixeladdress, op=0 clear; op=1 turn on */
int x0,y0,op;
{
        extern int pixsizx,pixsizy,x_start,y_start;
        int tmp1,tmp2,xp,yp;

        tmp1 = ( x0 * pixsizx ) + x_start;
        tmp2 = ( y0 * pixsizy ) + y_start;
        point(tmp1 + 1,tmp2 + 1,0,0 ) ;
        fill(tmp1 + pixsizx - 1, tmp2 + pixsizy - 1, op ) ;
        xp = x_start + 124 + x0;
        yp = y_start + 32*pixsizy + 10 + y0;
        point(xp,yp,op,0);
}

/************************************************************************/
eraseraster()
{
        extern int pixsizx,pixsizy,x_start,y_start;
        extern int edsiz_x,edsiz_y;
        extern char oldarea[];
        int i;

        /* clear raster */
        point(x_start,y_start,0,0) ;
        fill(x_start+(32*pixsizx),y_start+(32*pixsizy),0);

        /* clear normal size area */
        point(x_start+124,y_start+32*pixsizy+10,0,0 ) ;
        fill(x_start+124+32,y_start+(32*pixsizy)+10+32,0);

        /* reset compare area  */
        for( i=0; i < 32*32; i++ ) {
                oldarea[i] = 0;
        }

}


/************************************************************************/
cursor(op)
int op;
{

        extern int pixsizx,pixsizy,x_start,y_start;
        int tmp,tmp1,tmp2,pattrn,colnr,x0,y0;

        pattrn = 0;                                     /* default pattrn 0 */
        if( tmp=setbit(&edarea[(y*32)+(x/8)],7-(x%8),3) == 1){ /* test box bit  */
                if( op == 0 ) colnr=1;                  /* reset or set cur.*/
                else colnr=0;
        }
        else {
                if( op == 0 ) colnr=0;
                else colnr=1;
        }
        y0=32-y-1;
        x0 = x;


        tmp1 = ( x0 * pixsizx ) + x_start;
        tmp2 = ( y0 * pixsizy ) + y_start;
        point(tmp1 + 1,tmp2 + 1, 0, colnr ) ;
        draw(tmp1 + pixsizx - 1, tmp2 + pixsizy - 1, pattrn, colnr) ;

}


/************************************************************************/
int mask[8] ={ 
        1, 2, 4, 8, 16, 32, 64, 128 };

display()       /* show edit area on screen.Only changed bits compared  */
{               /* with oldarea[] is sent to screen                     */
        extern char edarea[],oldarea[];
        extern int edsiz_x,edsiz_y;
        int bitar,x0,y0,op;
        int i,j,bitcnt;

        if( silentflag == 1 ) return(0);
        for( i = 0 ; i < edsiz_y  ; i++ ){     /* box heigth  */
                y0 = 32-i-1;                   /* top start   */
                bitcnt = 0;
                for( j = 0 ; j < 4 ; j++ ){    /* max 32 bits */
                        for( bitar = 7 ; bitar >= 0 ; bitar-- ) { /* 8 bits */
                                op = ( edarea[(i * 32) + j] & mask[bitar] ) ;
                                if( op != (oldarea[(i*32)+j] & mask[bitar]) ){
                                        if( op != 0 ) op = 1;
                                        x0 = j * 8 + ( 7 - bitar );
                                        fillpnt(x0,y0,op);  /* draw a pixel */
                                }
                                bitcnt++;
                                if( bitcnt == edsiz_x ) break;
                        }
                        if( bitcnt == edsiz_x ) break;
                }
        }

        /* copy edarea to compare area  */
        for( i=0; i < 32*32; i++ ) {
                oldarea[i] = edarea[i];
        }
        send();                 /* send buffer  */
}


/************************************************************************/
dispbit(op)             /* set reset or toggle bit x,y in edarea        */
int op;
{
        int tmp,bytenr,bitnr;

        bytenr = (x/8)+y*32;
        bitnr = 7-(x%8);

        fillpnt( x ,32-y-1,                     /* set and show it */
        tmp=setbit(&edarea[bytenr],bitnr,op)  );

        tmp=setbit(&oldarea[bytenr],bitnr,op)  ;   /* follow with image */
}



/************************************************************************/

int vmsk[9] ={ 
        0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };
int hmsk[9] ={ 
        0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };

movein(from,to,step,width) /* move width bits starting at step bits in  */
char from[];               /* into first byte from[0]. step=7-bitpos    */
char to[];
int step,width;
{
        unsigned int f,t,boxd1,
        boxd2,
        boxd3,
        i,w;

        w=0;
        f=0;
        t=0;               /* reset pointers       */

        while( w < width ) {
                boxd1 = from[f] ;


                boxd1 = boxd1 << step;/* left align valid data bits */

                boxd1 = boxd1 & 0xff ; /* mask off                */

                boxd2 = from[f+1];   /* get next 8 data bits         */

                boxd2=boxd2>>((8-step)%8);/* right align valid bits  */
                /* mask off invalid part */
                boxd2 = boxd2 & vmsk[ 8-step ];
                if( step == 0 ) boxd2 = 0; /* dont mix if already aligned */


                boxd3 = boxd1 | boxd2; /* assembled byte       */

                w = w+8;                /* accumulated bit width        */

                /* Now take care of right part of the box-line  */
                if( w <= width ) 
                        i = 0 ;
                else i =  w - width  ;

                boxd3 = boxd3 & hmsk[i];    /* mask right part      */


                to[t] = boxd3 ;

                f +=1;
                t +=1;
        }
}



/************************************************************************/
moveout(from,to,step,width)  /* move with bits from from[] area starting */
/* at step bits in into first byte of from[] */
char from[];
char to[];
int step,width;
{
        unsigned int f,t,boxd1,
        boxd2,
        boxd3,
        boxd4,
        w;

        w=  8 - step ;
        f=0;
        t=0;
        boxd1 = from[f] & 0xff;         /* get first byte       */

        boxd1 >>= step;                 /* right align          */

        boxd2 = to[t] & ~vmsk[step];     /* pick up destination byte     */

        boxd3 = boxd1 | boxd2;          /* assemble             */

        to[t] = boxd3;                  /* write back           */
        t++;
        f++;
        while( w<width ) {

                boxd1 = from[f-1] & 0xff;       /* pick up first byte again     */

                boxd1 <<= (8-step);             /* left align           */

                boxd2 = from[f] & 0xff;         /* pick up next byte    */
                boxd2 &= 0xff;

                boxd2 >>= step;                 /* right align          */
                boxd2 &= 0xff;
                boxd3 = boxd1 | boxd2;          /* assemble             */
                w = w + 8;
                boxd4 = 0;
                if( w > width ) {
                        boxd4 = to[ t ];        /* pick up next dest.byte       */
                        boxd4 &= 0xff;

                        boxd4 &= ~hmsk[w-width];/* leave right part of dest     */
                        boxd3 &= hmsk[w-width]; /* reset invalid bits           */
                }
                to[t] = (boxd3 | boxd4) & 0xff;

                f++;
                t++;
        }
}


/************************************************************************/
cur(row,column)         /* Cursor positioning function          */
int row,column;
{
        sendfill("\033[");
        numout(row);
        sendfill(";");
        numout(column);
        sendfill("H");       /* ESC-sequence for vt100 */
}

/************************************************************************/
update(op)              /* draw raster on screen  op=0 erase  op=1 draw    */
int op;
{
        int i,j;

        /* draw lines vertically */
        for(i = 0 ; i <= edsiz_x ; i++ ) {
                bline( ( pixsizx*i ) + x_start,32*pixsizy + y_start,
                ( pixsizx*i ) + x_start,
                (32*pixsizy)-(edsiz_y*pixsizy) + y_start,op );
        }
        /* draw lines horizontally */
        for( j = 0 ; j <= edsiz_y ; j++ ) {
                bline( x_start,(32*pixsizy)-(pixsizy*j)+y_start,
                (edsiz_x)*pixsizx+x_start,
                (32*pixsizy)-(pixsizy*j)+y_start, op);
        }
}



char ormask[8]  = { 
        '\001','\002','\004','\010','\020','\040','\100','\200' };
char andmask[8] = { 
        '\376','\375','\373','\367','\357','\337','\277','\177' };
/************************************************************************/
setbit(byteptr,bitnr,op)        /* manipulate bit                       */
char *byteptr;
int     bitnr,                  /* bit number 7-0                       */
op;                     /* op: 0=reset 1=set 2=toggle 3=read    */

{

        switch( op ) {
        case 0 :
                *byteptr &= andmask[bitnr];
                return(0);
                break;
        case 1 :
                *byteptr |= ormask[bitnr];
                return(1);
                break;
        case 2 :
                if( *byteptr & ormask[bitnr] ) {
                        *byteptr &= andmask[bitnr];
                        return(0);
                }
                else {
                        *byteptr |= ormask[bitnr];
                        return(1);
                }
                break;
        case 3 :
                if( *byteptr & ormask[bitnr] ) return(1);
                else return(0);
                break;
        }
}


/***********************************************************************/
movup()
{
        extern char edarea[];
        int i,j;

        for( j=0; j<4; j++ ) {

                for( i=0; i<31; i++ ) {
                        edarea[(i*32)+j] = edarea[(i*32)+j+32];
                }
                edarea[(i*32)+j] = 0;

        }
        display(); 
}

/************************************************************************/
movdown()
{

        extern char edarea[];
        int i,j;

        for( j=0; j<4; j++ ) {

                for( i=31; i>0; i-- ) {
                        edarea[(i*32)+j] = edarea[(i*32)+j-32];
                }
                edarea[j] = 0;                  /* topmost bits = 0     */
        }
        display(); 
}

/************************************************************************/
movleft()
{
        extern char edarea[],oldarea[];
        int i;

        for( i=0; i<32; i++ ) {
                /* from oldarea, to edarea , 1 bit shift, 32 bits wide */
                movein(&oldarea[i*32],&edarea[i*32],1,32);
                edarea[ (i*32)+3 ] &= 0xfe;     /* mask rightmost bit  */
        }
        display();
}

/************************************************************************/
movright()
{
        extern char edarea[],oldarea[];
        int i;

        for( i=0; i<32; i++ ) {
                /* from oldarea, to edarea , 1 bit shift, 32 bits wide */
                moveout(&oldarea[i*32],&edarea[i*32],1,32);
                edarea[i*32] &= 0x7f;                   /* leftmost bit = 0 */
        }
        display();
}


/************************************************************************/
int
mousepix()
{
        int button,pixval,bytenr,bitnr,x_old,y_old;

        write(STDOUT,"\033:2n",4);
        read(0,&inbuf[0],256);
        sscanf(&inbuf[0], "\033:%d;%d;%dP",&x,&y,&button);
        x = (x-x_start)/pixsizx;
        y = 31-((y-y_start)/pixsizy);
        if( x<0 | y<0 | x>(edsiz_x-1) | y>(edsiz_y-1) ) return; 
        bytenr = (x/8) + y*32;
        bitnr = 7 - (x%8 );
        pixval = setbit(&edarea[bytenr],bitnr,3); /* read pixel */
        pixval += 1;
        pixval &= 0x01;         /* pixval inverted */        
        while ( button && 0x1 == 1 ) {
                write(STDOUT,"\033:2n",4);
                read(0,&inbuf[0],256);
                sscanf(&inbuf[0], "\033:%d;%d;%dP",&x,&y,&button);
                x = (x-x_start)/pixsizx;
                y = 31-((y-y_start)/pixsizy);
                bytenr = (x/8) + y*32;
                bitnr = 7 - (x%8 );
                if( x<0 | y<0 | x>(edsiz_x-1) | 
                    y>(edsiz_y-1) ) button = 0; /*break*/
                else if( x_old!=x | y_old!=y ) {
                        dispbit(pixval); /* paint pixel */
                        send();         /* clear screenbuffer */
                        x_old=x;
                        y_old=y;
                }
        }

}

mousetrace()
{
        int button;

        write(STDOUT,"\033:2n",4);
        read(0,&inbuf[0],256);
        sscanf(&inbuf[0], "\033:%d;%d;%dP",&x,&y,&button);
        cur(5,0);
        numout(x);
        sendfill(",");
        numout(y);
        sendfill(":");
        numout(button);
        sendfill("  ");
}

/************************************************************************/
int
strin(str)              /* input a string to str        */
char str[];
{
        int i;
        char c;

        echoflag=1;                     /* set echoflag         */
        i=0;

        while( ( c = getone() ) != '\r' ) {
		if (echoflag) {
	        switch(c) {
		    case '\010':
		        if (i==0) sendfill("\033[1C"); /*cur. forwd*/
			if (i>0 ) {
			    i--;            /* check for backspace */
			    str[i] = 0;
			    sendfill(" \010"); /* erase character  */
			}
			break;
		    case '\033':
			echoflag=0;
			str[0]=0;
			break;
		    default:
		        str[i] = c ;
			if( i<42 ) i++;
			else sendfill("\010"); /* stop here ... */
			break;
		}

                if( intflag == 1 ) {     /* check for win-moves  */
                        intflag = 0;
                        str[0] = 0;            
                        break;
                }
		}
        }
	str[i] = 0;             /* end of string mark           */
        echoflag=0;             /* dont echo                    */
        sendfill("\033[?35h");  /* cursor off                   */
        return(i);
}

/************************************************************************/
sendfill(str)   /* fill speed-buffer with characters from str */
char str[];
{
        register i;
        i=0;
        while( str[i] != '\000' )       {
                speed[sptr] = str[i];
                sptr++;
                i++;
                if( sptr>2047 ) send();
        }
}

send()          /* flush buffer to screen */
{
        write(STDOUT,&speed[0],sptr);
        sptr = 0;
}

/************************************************************************/

multout(n,c)    /* This function puts n occurences of 'c' into buffer */
register n;
register char c;
{
        register i;

        for ( i=0; i<n; i++ ) {
                speed[sptr] = c;
                sptr++;
        }
}
 
numout(n)
register n;
{
        char cnvbuf[20];        /* conversion buffer    */
        register char *s;       /* pointer to cnvbuf[]  */

        s = &cnvbuf[20-1];
        *s = '\0';
        do {
                *--s = n % 10 + '0';
        } while ((n /= 10) > 0);
        sendfill(s);

        return;
}       /* end of numout        */

hexout(n)
int n;
{
        char cnvbuf[8];
        register char *s;       /* pointer to cnvbuf    */
        int h,l;

        s = &cnvbuf[8-1];
        *s = '\0';              /* ending zero          */
        h = (n>>4) & 0x0f;      /* high nibble          */
        if( h<10 ) h = h+0x30;
        else h = h+0x37;
        l = n & 0x0f;           /* low nibble          */
        if( l<10 ) l = l+0x30;
        else l = l+0x37;
        *--s = l;
        *--s = h;
        sendfill(s);            /* print                */
}

chrout(n)
int n;
{
        char convbuf[2];

        convbuf[0] = n & 0xff;
        convbuf[1] = '\0';
        sendfill(convbuf);
}
