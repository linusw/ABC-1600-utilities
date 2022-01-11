/*
This program forms part of the font editor fe

The source text is named fezoom.c and the object code fezoom.o

It contains the code for zooming i.e. enlarging and shrinking a character
and also the code for rotating a character

*/
#include "fedef.h"

#define READ 2

extern char old[],edarea[];
extern int edsiz_x,edsiz_y;
extern char inbuf[];
extern int silentflag;
extern int eraflag;
extern int lang;

/* TEXT DEFINITION - 0-4 English  5-9 Swedish */
char *ztexts[2][5] = {
	{
	"Enter scale factors: ",
	"Factor >1 means enlarge, ",
	"factor <1 means shrink",
	"Horizontal factor?               ",
	"Vertical factor?                 "
	},
	{
	"Ange skalfaktorerna: ",
	"Faktor >1 betyder f|rstoring, ",
	"faktor <1 betyder krympning",
	"Horizontal faktor?               ",
	"Vertikal faktor?                 "
	}
};


float hscale = 1,vscale = 1,curoutx,curouty,newlength;
int curinx,curiny,length;

/*************************************************************************

        The routine zoom is called from the central switch in fe
        and will perform zooming
*/

getzpar()               /* input zooming parameters     */
{
        int i;
        /*
                first the dialogue part to get scale factors
                in x and y directions
        */

	eraflag = 1;            /* set flag for command erase   */
        cur(4,1);
        sendfill(ztexts[lang][0]);/* Enter scale factors */
        cur(5,0);
        sendfill(ztexts[lang][1]);/* Factor >1 means enlarge, */
        sendfill(ztexts[lang][2]);/* factor <1 means shrink */
        cur(6,0);
        sendfill(ztexts[lang][3]);/* Horizontal factor? */
        cur(6,20);
        strin(&inbuf[0]);
        sscanf(&inbuf[0],"%f",&hscale);
        cur(7,0);
        sendfill(ztexts[lang][4]);/* Vertical factor? */
        cur(7,20);
        strin(&inbuf[0]);
        sscanf(&inbuf[0],"%f",&vscale);
}


zoom()
{
        /*
        
                Now it is time to work horizontally
        
        */
	ed_to_old();
        if (hscale != 1)
                for (curiny=0;curiny<BMAX_Y;curiny++)
                {
                        for (curinx=0,curoutx=0;curinx<BMAX_X;curinx+=length)
                        {
				length=getxline(&old[0],curinx,curiny
                                    ,BMAX_X-1);
                                newlength=hscale*length;
                                if (curoutx+newlength>=BMAX_X)
                                        newlength=BMAX_X-curoutx;
				drawxline(&edarea[0],(int) (curoutx+.5)
                                    ,(int) (curoutx+newlength-.5)
                                    ,curiny
				    ,pixel(&old[0],curinx,curiny,READ));
                                curoutx+=newlength;
                        }
                        if (curoutx<BMAX_X-1)
				drawxline(&edarea[0],(int) curoutx
                                    ,BMAX_X-1,curiny,0);
			display();
			ed_to_old();                    /* copy area */
                }
        /*
        
                Now it is time to work vertically
        
        */
        if (vscale != 1)
                for (curinx=0;curinx<BMAX_X;curinx++)
                {
                        for (curiny=0,curouty=0;curiny<BMAX_Y;curiny+=length)
                        {
				length=getyline(&old[0],curinx,curiny
                                    ,BMAX_Y-1);
                                newlength=vscale*length;
                                if (curouty+newlength>=BMAX_Y)
                                        newlength=BMAX_Y-curouty;
				drawyline(&edarea[0],curinx,(int) (curouty+.5),
                                (int) (curouty+newlength-.5)
				    ,pixel(&old[0],curinx,curiny,READ));
                                curouty+=newlength;
                        }
                        if (curouty<BMAX_Y-1)
				drawyline(&edarea[0],curinx,(int) curouty
                                    ,BMAX_Y-1,0);
			display();
			ed_to_old();
                }
}
/*************************************************************************

        Drawxline a routine that will set a horizontal line of pixels
        to the desired value indicated by the parameter onoff

*/
drawxline(area,x1,x2,y,onoff)
char *area;
int x1,x2,y,onoff;
{
        int i;
        for (;x1<=x2;x1++)
                pixel(area,x1,y,onoff);
}
/*************************************************************************

        Drawyline a routine that will set a vertical line of pixels
        to the desired value indicated by the parameter onoff

*/
drawyline(area,x,y1,y2,onoff)
char *area;
int x,y1,y2,onoff;
{
        int i;
        for (;y1<=y2;y1++)
                pixel(area,x,y1,onoff);
}
/*************************************************************************

         getxline a routine that will return the number of consecutive
         pixels with the same value on a horizontal line

*/
getxline(area,x,y,xmax)
char *area;
int x,y,xmax;
{
        int pix,len;
        pix=pixel(area,x,y,READ);
        len=1;
        while ((x+len)<=xmax && (pix == pixel(area,x+len,y,READ)))
                len++;
        return (len);
}

/*************************************************************************

         getyline a routine that will return the number of consecutive
         pixels with the same value on a vertical line

*/
getyline(area,x,y,ymax)
char *area;
int x,y,ymax;
{
        int pix,len;
        pix=pixel(area,x,y,READ);
        len=1;
        while ((y+len)<=ymax && (pix == pixel(area,x,y+len,READ)))
                len++;
        return (len);
}
/*************************************************************************

        Pixel a routine that can set, reset and read a pixel
        addressed by an x,y coordinate pair
        The function is controlled via the fourth parameter
        which can take the values: 0 for reset
                                   1 for set
                                   2 for read
        all other values are treated as read
*/
pixel(area,x,y,op)
char *area;
int x,y,op;
{
        area+=(y*BMAX_X+x/8);
        switch(op)
        {
        case 0:
                *area = *area & ~(128>>x%8);
                return (0);

        case 1:
                *area = *area | (128>>x%8);
                return (1);

        default:
                return (((128>>x%8) & *area) >>(7-x%8));
        }
}
/************************************************************************

        The routine turn will turn a given font pattern in a box
        90 degrees clockwise

*/
turn()
{
        int x,y;
	ed_to_old();
        for (y=0;y<edsiz_x;y++)
                for (x=0;x<BMAX_X;x++)
			pixel(&edarea[0],edsiz_x-y-1,x,pixel(&old[0],x,y,READ));
	display();
	ed_to_old();            /* follow with 'old[]'  */
}
/*************************************************************************/
ed_to_old()
{
	int i;
	for(i=0; i<BMAX_X*BMAX_Y; i++){
		old[i] = edarea[i];
	}
}
