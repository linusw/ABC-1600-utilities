#include "h/tests.h"
#include <mac.h>

#define K	(1024)
#define M	(128 * K)

#define MAP(lpn,fpn)	((*(unsigned short *)(0x80000 +((lpn)<<8))) = ((fpn) | 0x4000))
#define MAXERR	5
#define ABORT	lprintf("Sub-test aborted!\n"); return(0);
#define putlong(ad,x)  MAP(512,((ad)>>11)); (*(unsigned int *)(M + ((ad) & 2047))) = x 
#define getlong(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned int *)(M + ((ad) & 2047))) 
#define putshort(ad,x)  MAP(512,((ad)>>11)); (*(unsigned short *)(M + ((ad) & 2047))) = x 
#define getshort(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned short *)(M + ((ad) & 2047))) 
#define putbyte(ad,x)  MAP(512,((ad)>>11)); (*(unsigned char *)(M + ((ad) & 2047))) = x 
#define getbyte(ad,x)  MAP(512,((ad)>>11)); x = (*(unsigned char *)(M + ((ad) & 2047))) 

#define HALT()	((status & 1) && (keyb==0x81 || keyb==0x03))

unsigned int sadr,size,value,count,dump;

memtest(adr,sz,val,cnt,par)
unsigned int adr,sz,val,cnt,par;

BEGIN
        sadr=adr;
	size=sz;
	value=val;
        count=cnt;
        if (!par) {
	    spcr=0x0c;		/* Kill parity check */
	    lprintf("NO PARITY CHECK\n\n");
	}
	else {
	    spcr=0x04;
	    lprintf("PARITY CHECK\n\n");
	}
        
	
	size = size & -4;
	lprintf("Adr = 0x%X, Size=0x%X, Value=0x%X, Count=0x%X\n",
		sadr,size,value,count);
        
	WHILE count-- DO
		lprintf("Address test...\n");
		if (adrtest()) break;
		lprintf("Bit test a...\n");
		if (bittest(1,0)) break;
		lprintf("Bit test b...\n");
		if (bittest(0xfffe,0xffff)) break;
		lprintf("Byte test...\n");
		if (bytetest()) break;
	OD
END





adrtest()
BEGIN
	register unsigned int adr,sz,val,vval;
	int errcnt;

	sz = size; adr = sadr; val = value;
        lprintf("Skriver m|nster...\n");
	WHILE sz DO
		putlong(adr,val);
		dump=kick;
		adr += 4; sz -= 4; val++;
	OD
	sz = 64 * K;
	if (HALT()) return(1);
        lprintf("F|rdr|jning....\n");
	WHILE sz-- DO dump=kick; OD
	errcnt = 0; sz = size; adr = sadr; val = value;
        lprintf("Kontrollerar m|nster...\n");
	WHILE sz DO
		dump=kick;
		getlong(adr,vval);
		IF val != vval THEN
			err(0,adr,val,vval);
			IF ++errcnt > MAXERR THEN ABORT FI
		FI
		adr += 4; sz -= 4; val++;
	OD
	return(0);
END





bittest(spat,epat)
unsigned short epat,spat;
BEGIN
	register unsigned int adr,sz;
	register unsigned short pat,ppat;
	int errcnt;

	pat = spat;	errcnt = 0;
	WHILE pat != epat DO
		if (HALT()) return(1);
                lprintf("Bitm|nster: %x\n",pat); 
	        dump=kick;
		sz = size; adr = sadr;
		WHILE sz DO
			dump=kick;
			putshort(adr,pat);
			adr += 2; sz -= 2;
		OD
		sz = size; adr = sadr;
		WHILE sz DO
			dump=kick;
			getshort(adr,ppat);
			IF pat != ppat THEN
				err(1,adr,pat,ppat);
				IF errcnt++ > MAXERR THEN ABORT FI
			FI
			adr += 2; sz -= 2;
		OD
		pat <<= 1;
		IF epat == 0xffff THEN pat |= 1; FI
	OD
	return(0);
END





bytetest()
BEGIN
	register unsigned int adr,sz;
	register unsigned short val;
	int errcnt;

	errcnt = 0; adr = sadr; sz = size;
	WHILE sz DO
		if (HALT()) return(1);
		dump=kick;
		putshort(adr,0);
		putbyte(adr,0xff);
		getshort(adr,val);
		IF val != 0xff00 THEN
			err(2,adr,0xff00,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		putshort(adr,0);
		putbyte(adr+1,0xff);
		getshort(adr,val);
		IF val != 0xff THEN
			err(2,adr,0xff,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		dump=kick;
		putshort(adr,0xffff);
		putbyte(adr,0);
		getshort(adr,val);
		IF val != 0xff THEN
			err(2,adr,0xff,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		putshort(adr,0xffff);
		putbyte(adr+1,0);
		getshort(adr,val);
		IF val != 0xff00 THEN
			err(2,adr,0xff00,val);
			IF errcnt++ > MAXERR THEN ABORT FI
		FI
		sz -= 2; adr += 2;
	OD
	return(0);
END




err(type,adr,okval,errval)
unsigned int okval,errval,type,adr;
BEGIN
        unsigned int i,size;
	
	SWITCH type IN
	CASE 0:
		lprintf("Address test error, ");
		size=32;
		break;
	CASE 1:
		lprintf("Bit test error, ");
		size=16;
		break;
	CASE 2:
		lprintf("Byte test error, ");
		size=16;
		break;
	END

	lprintf("at phys. adr=%X, read %X, should be %X\n",adr,errval,okval);

	for (i=0;i<size;i++) {
	    if ((errval & 1<<i) != (okval & 1<<i)) chip(adr,i);
	}

	
END



chip(adr,bit)
unsigned int adr,bit;
{
    lprintf("Pos %d",28-bit%8);

    switch((adr+bit/8)>>16) {
	case 0:
	case 1:
	case 2:
	case 3:
	    lprintf("D\n");
	    break;
	case 4:
	case 5:
	case 6:
	case 7:
	    lprintf("C\n");
	    break;
	case 8:
	case 9:
	case 10:
	case 11:
	    lprintf("B\n");
	    break;
	case 12:
	case 13:
	case 14:
	case 15:
	    lprintf("A\n");
	    break;
	default:
	    lprintf("FEL\n");
	    break;
    }

}
