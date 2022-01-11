#define X35 X35
#define	NULL	0

#include "h/ports.h"
#include "h/tests.h"
#include <mac.h>

#define SERROR 0
#define SREAD 1
#define SCHECK 2

/*
 *      Battery clock definitions
 */

struct  batbuf {
        unsigned char   bat_hour,bat_min,bat_date,bat_month,
                        bat_year,bat_wday,bat_sec,bat_swtch
        };



batclk()
{
    int errtype;

    lprintf("\n");
    errtype=show();
    lprintf("BATTERIKLOCKA: ");
    if (errtype) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        Felaktig tidsdata lagrad i klockan\n");
	if (errtype & 2) lprintf("        Skrivning/l{sning fungerar inte\n");
	if (errtype & 4) lprintf("        Tidsuppr{kning fungerar inte\n");
	if (errtype & 8) lprintf("        CIO ej initierad\n");
    }
    else {
	lprintf("OK\n");
    }
    fflush(stdout);
}




/********************************************************************\
* 								     *
*  RD returns the data at the CIO register adressed by the argument  *
* 								     *
\********************************************************************/
rdcio(adr)
int adr;
{
    byte dump,data;

    dump = cio;
    cio = adr;
    data = cio;
    return(data & 255);
}




/*********************************************************************\
* 								      *
*  SHOW reads the batclk and prints the present time on screen        *
* 								      *
\*********************************************************************/
show()
{
    unsigned char map[12],back[12];
    int i;
    byte error;

    error=0;
    for (i=0;i<12;i++) map[i]=0;
    rwbclk(map,0);
    if (rdcio(0x01)!=0xb0) {
	error |= 8;
	return(error);
    }
    kick;

    for (i=0;i<12;i++) back[i]=map[i];
    lprintf("Datorns tid: 19");
    lprintf("%d%d ",map[0]-48,map[1]-48);
    switch((map[2]-48)*10+map[3]-48) {
	case 1: lprintf("Jan "); break;
	case 2: lprintf("Feb "); break;
	case 3: lprintf("Mar "); break;
	case 4: lprintf("Apr "); break;
	case 5: lprintf("Maj "); break;
	case 6: lprintf("Jun "); break;
	case 7: lprintf("Jul "); break;
	case 8: lprintf("Aug "); break;
	case 9: lprintf("Sep "); break;
	case 10: lprintf("Okt "); break;
	case 11: lprintf("Nov "); break;
	case 12: lprintf("Dec "); break;
	default:
	    lprintf("FEL ");
	    error |= 1;
	    break;
    }
    lprintf("%d%d   ",map[4]-48,map[5]-48);
    lprintf("%d%d:%d%d:%d%d\n",map[6]-48,map[7]-48,map[8]-48,map[9]-48,
    				map[10]-48,map[11]-48);
				
    fflush(stdout);

    back[0]=48+6;
    back[1]=48+2;
    back[2]=48;
    back[3]=48+4;
    back[4]=48+1;
    back[5]=48+5;
    rwbclk(back,1);
    for (i=0;i<12;i++) back[i]=0;
    rwbclk(back,0);
    if(back[0]==48+6 && back[1]==48+2 && back[2]==48 && back[3]==48+4 &&
    			back[4]==48+1 && back[5]==48+5) {
    }
    else {
	error |= 2;
    }
    fflush(stdout);

    map[11]=48;
	
    rwbclk(map,1);

    delay(40);
    delay(40);
    delay(40);
    delay(40);
    rwbclk(back,0);
        
    if (back[11]!=48+5) {
	lprintf("**** FEL: Tidsuppr{kning ");
	switch(back[11]-48) {
	    case 0: lprintf("fungerar inte alls\n"); break;
	    case 1:
	    case 2:
	    case 3:
	    case 4: lprintf("f|r l}ngsam\n"); break;
	    case 6:
	    case 7:
	    case 8:
	    case 9: lprintf("f|r snabb\n"); break;
	    default: lprintf("helt felaktig\n"); break;
	}
	error |= 4;
    }
    fflush(stdout);
    return(error);
}





/******************************************************************\
* 								   *
*  RWBLK handles all the bit manipulation concerning the clock     *
* 								   *
\******************************************************************/
rwbclk(tarea,dowrit)
unsigned char *tarea;
int dowrit;
BEGIN
	struct batbuf batbuf ;
	unsigned char *adrtab[8];
	register int i,j ;
	int state;
	int errcnt;
	register unsigned char *cp ;
	register unsigned char **cpp ;
	register unsigned char *p;

	state = SERROR;
	errcnt = 0;
	adrtab[0] = &batbuf.bat_year;
	adrtab[1] = &batbuf.bat_month;
	adrtab[2] = &batbuf.bat_date;
	adrtab[3] = &batbuf.bat_hour;
	adrtab[4] = &batbuf.bat_min;
	adrtab[5] = &batbuf.bat_sec;
	adrtab[6] = NULL;
	if(dowrit){
		goto wbclk;
	}
	/* read from b-clock */
	LOOP
		BATCCSH;		/* cs high clk high */
		IF ++state > SCHECK THEN break FI
		sbclock(15);			/* Set address */
		j = 0 ; cp = (unsigned char *) &batbuf ;
		i=7*8 + 1 ; /* bits we can read from bat ram */
		REP
			BATCCLL;		/* Clock low */
			dlymicr(4);
			j >>=1 ;
			IF BATCRD THEN j |= 0x80 FI;
			BATCCLH;	/* Clock high */
			i-- ;
			IF i < 56 && (i & 7) == 0 THEN
				IF state==SREAD THEN
					*cp++ = j ;
				ELIF state == SCHECK && *cp++ != j THEN
					IF ++errcnt == 5 THEN
						return ;
					ELSE
						state = SERROR ;
					FI
				FI
			FI
		PER i DONE
	POOL
	cp = tarea ;
	FOR cpp = adrtab ; *cpp ; cpp++ DO
		IF *cp=='.' THEN cp++ FI
		j = **cpp ;
		*cp++ = (j >> 4) + '0' ;
		*cp++ = (j & 15) + '0' ;
	OD
	return;

wbclk:
	/* write to b-clock */
	cp = tarea;
	for(cpp = adrtab; *cpp; cpp++){
		if( *cp == '.'){
			cp++;
		}
		**cpp = (((*cp++) & 15) << 4) + ((*cp++) & 15);
	}
	sbclock(14);		/* Set write */
	j = 0; p = (unsigned char *) &batbuf;
	i = 7*8;
	while(i){
		if((i & 7) == 0){
			j = *p++;
		}
		BATCWR(j & 0x80);
		j <<=1;
		i--;
	}
	return;
}

/*
 *	Set up address and read/write in battery clock
 */
sbclock(addr)
register int addr;
{
	register int i ;

	i = 4;
	BATCCSH;		/* Set cs high clock high */
	dlymicr(4);		/* Wait some micros */
	BATCCLH;		/* Set cs low clock high */
	while(i--){
		BATCWR(addr & 8);
		addr <<=1;
	}
}

