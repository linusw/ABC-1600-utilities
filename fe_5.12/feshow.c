#include "fedef.h"
#define STDIN 0

extern char inbuf[256];
extern struct cgenblk wrk_hdr;
extern char wrkarea[];

show()          /* Routine to show work area on screen */
{
        int line,x1,y1,xt,x2,yt,op,dax,day;

        dax = 440;      /* 700x700 pixel 'screen'         */
        day = 404;

        setcooked();    /* make possible to break       */
        for(line=0; line<wrk_hdr.cg_dumpl/wrk_hdr.cg_bwidth; line++) {
                cur(6,0);numout(line);
                x1=0;
                y1=day-line;
                yt=line*32;
                op = pixel(&wrkarea[yt],x1,0,2);
                point(dax+x1,y1,1,op);
                while(x1<256 && y1>=0 ) {
                        xt=dax+x1;
                        op = pixel(&wrkarea[yt],x1,0,2);
                        x2=x1+getxline(&wrkarea[yt],x1,0,255);
                        draw(dax+x2,y1,0,op);
                        x1=x2;
                }
                if( check_quit() ) break;
        }
        setraw();
}

/************************************************************************/
check_quit()
{
        int c;

        return(0);                  /*   ok yet... */
/*      if( c=rdchk(STDIN) != 0 ) {
                read(STDIN,&inbuf[0],1);
                if( inbuf[0] == 'q' ) return(1);
                else return(0);
        }
        else return(0);
  */
}
