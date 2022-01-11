/************************************************************************/
/* fe1.c                                                                */
/************************************************************************/

#include "fedef.h"

extern int eraflag;
extern int lang;
extern int inscode,loadcode;         /* char to edit                 */
extern char *work;

char *texts1[2][2] = {
	{
	"Inserted",
	"Edit box size not set."
	},
	{
	"Insatt",
	"Redigeringsrutans storlek ej satt."
	}
};


/************************************************************************/
loadchr(hdrptr,areaptr)
struct cgenblk *hdrptr;
char areaptr[];
/*    load character box into edit area                                 */
/*              input:pointer to the area to get character from         */
/*              output:edit area loaded with matrix box                 */
{
        extern  int     dirty;          /* flag for dirty edit area     */
        extern  char    edarea[];       /* character edit area          */
        extern  struct  cgenblk wrk_hdr;
        extern  int     edsiz_x;
        extern  int     edsiz_y;
        extern  char    inbuf[];

        int     boxnum,         /* logic box number             */
        boxperrad,      /* box per row count            */
        startbyte,      /* index to starting byte of box*/
        startbit,       /* bit number in first byte     */
        ldsiz_x,        /*     boxsize x       */
        ldsiz_y;        /*     boxsize y       */


        int j;

        if( hdrptr->cg_stat == 0 ) {    /* portrait     */
                ldsiz_x = hdrptr->cg_fxsiz ;
                ldsiz_y = hdrptr->cg_fysiz ;
        }
        else {                          /* landscape    */
                ldsiz_y = hdrptr->cg_fxsiz ;
                ldsiz_x = hdrptr->cg_fysiz ;
        }
        if( hdrptr == &wrk_hdr ) {     /* if work area was loaded,update the */
                edsiz_x = ldsiz_x;      /* edit area size                    */
                edsiz_y = ldsiz_y;
        }


        boxnum = loadcode - hdrptr->cg_firstc;
	if (++loadcode>hdrptr->cg_lastc) loadcode=hdrptr->cg_firstc;

        boxperrad = ( hdrptr->cg_bwidth * 8 ) / ldsiz_x ;
        startbyte = ( boxnum / boxperrad ) * ldsiz_y * hdrptr->cg_bwidth +
            ((( boxnum % boxperrad ) * ldsiz_x ) / 8 );
        startbit = (( boxnum % boxperrad ) * ldsiz_x ) % 8 ;

        /* Copy from memory file to edit area */

        for( j = 0 ; j <= ldsiz_y ; j++ ) {

                movein( &areaptr[ startbyte+(j*hdrptr->cg_bwidth) ],
                &edarea[ j * 32 ],
                startbit,
                ldsiz_x);

        };

        display();                      /*     display on screen      */

} /* loadchr */


/************************************************************************/
insert(hdrptr,areaptr)         /* save char from editarea to work area */
struct cgenblk *hdrptr;         /* hdrptr is a pointer to a structure   */
char *areaptr;

{
        extern  int     dirty;          /* flag for dirty edit area     */
        extern  char    edarea[];       /* character edit area          */
        int     boxnum,         /* logic box number             */
        boxperrad,      /* box per row count            */
        startbyte,      /* index to starting byte of box*/
        startbit,       /* bit number in first byte     */
        savsiz_x,        /*     boxsize x       */
        savsiz_y;        /*     boxsize y       */


        int j;

	if (work[0]==' ') {	/* Check that character box is defined */
	        cur(6,0);
		eraflag=1;
		sendfill(texts1[lang][1]);
		return(1);	/* Abort if it isn't */
	}	    

        dirty = 1;                      /* set dirty flag       */



        /***** pick up box size *****/
        if( hdrptr->cg_stat == 0 ) {
                savsiz_x = hdrptr->cg_fxsiz ;
                savsiz_y = hdrptr->cg_fysiz ;
        }
        else {
                savsiz_y = hdrptr->cg_fxsiz ;
                savsiz_x = hdrptr->cg_fysiz ;
        }

        /***** calculate box position *****/
        boxnum = inscode - hdrptr->cg_firstc;
	if (++inscode>hdrptr->cg_lastc) inscode=hdrptr->cg_firstc;

        boxperrad = ( hdrptr->cg_bwidth * 8 ) / savsiz_x ;
        startbyte = ( boxnum / boxperrad ) * savsiz_y * hdrptr->cg_bwidth +
            ((( boxnum % boxperrad ) * savsiz_x ) / 8 );
        startbit = (( boxnum % boxperrad ) * savsiz_x ) % 8 ;

        for( j=0; j<savsiz_y; j++ ) {
                moveout(&edarea[ j * 32 ],
                &areaptr[ startbyte + ( j * hdrptr->cg_bwidth ) ],
                startbit,
                savsiz_x );
        }
        eraflag = 1;
        cur(6,0);
        sendfill(texts1[lang][0]);
}
/* end of fe1.c */
