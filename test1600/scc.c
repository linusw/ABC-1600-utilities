#include "h/tests.h"

scc()
{
    int errtype;
    
    spcr=0x0b;

    lprintf("\nSCC register: ");
    if (errtype=init()) {
	lprintf("FEL ****\n");
	if (errtype & 1) lprintf("        Kanal A initiering fungerar ej\n");
	if (errtype & 2) lprintf("        Kanal B initiering fungerar ej\n");
    }
    else {
	lprintf("OK\n");
    }
    lprintf("\n");
    
    errtype=0;
    
    lprintf("SCC signaler:\n");
    if ((errtype=linetest())==0) {
	lprintf("SCC signaler: OK\n\n");
	lprintf("SCC kommunikation:\n");
	if (errtype=comtest()) {
	lprintf("%x\n",errtype);
	    lprintf("SCC kommunikation: FEL ****\n");
	    if (errtype & 1) lprintf("        S{ndning fr}n kanal B fungerar ej\n");
	    if (errtype & 2) lprintf("        Mottagna tecken felaktiga vid A\n");
	    if (errtype & 4) lprintf("        S{ndning fr}n kanal A fungerar ej\n");
	    if (errtype & 8) lprintf("        Mottagna tecken felaktiga vid B\n");
	}
	else {
	    lprintf("SCC kommunikation: OK\n");
	}
	lprintf("\n");
    }
    else {
	lprintf("SCC signaler: FEL ****\n");
	if (errtype & 1) lprintf("        Kabeln ej korrekt eller signalerna fungerar ej\n");
	if (errtype & 2) lprintf("        Prov ej utf|rt\n");
	if (errtype & 4) lprintf("        Kabeln ej korrekt eller signalerna fungerar ej\n");
	if (errtype & 8) lprintf("        Signalerna A->B fungerar ej\n");
	if (errtype & 16) lprintf("        Signalerna B->A fungerar ej\n");
    }
    
}





/**********************************************************************\
* 								       *
*  TERMINAL performs a test of terminal communications over channel A  *
* 								       *
\**********************************************************************/
terminal()
{
    int errtype;
    
    lprintf("\nSCC terminal kommunikation:\n");
    init();
    
    if (errtype=term()) {
	lprintf("\nSCC terminal kommunikation: FEL ****\n");
	if (errtype & 1) lprintf("        Prov ej utf|rt\n");
	if (errtype & 2) lprintf("        S{ndning till terminal fungerar ej\n");
	if (errtype & 4) lprintf("        Mottagning fr}n terminal fungerar ej\n");
    }
    else {
	lprintf("\nSCC terminal kommunikation: OK\n");
    }
    
}




/**********************************************************\
* 							   *
*  RDA returns the SCC port A register pointed to          *
* 							   *
\**********************************************************/
rda(no)
int no;
{
    scca=no;
    return(scca & 255);
}




/**********************************************************\
* 							   *
*  RDB returns the SCC port B register pointed to          *
* 							   *
\**********************************************************/
rdb(no)
int no;
{
    sccb=no;
    return(sccb & 255);
}




/********************************************************\
* 							 *
*  WRA writes to the specified SCC port A register     	 *
* 							 *
\********************************************************/
wra(no,data)
int no;
byte data;
{
    scca=no;
    scca=data;
}




/********************************************************\
* 							 *
*  WRB writes to the specified SCC port B register     	 *
* 							 *
\********************************************************/
wrb(no,data)
int no;
byte data;
{
    sccb=no;
    sccb=data;
}




/********************************************************\
* 							 *
*  TRANSA sends a data byte to SCC port A              	 *
* 							 *
\********************************************************/
transa(data)
byte data;
{
    int cnt=0;
    int i,dump;
    
    while (!(rda(0) & 4)) {
	cnt++;
	dump=kick;
	if (cnt>10000) return(1);
    }
    for (i=0;i<100;i++) dump=kick;
    dataa=data;
    return(0);
}




/********************************************************\
* 							 *
*  TRANSB sends a data byte to SCC port B              	 *
* 							 *
\********************************************************/
transb(data)
byte data;
{
    int cnt=0;
    int dump,i;

    while (!(rdb(0) & 4)) {
	cnt++;
	dump=kick;
	if (cnt>10000) return(1);
    }
    for (i=0;i<100;i++) dump=kick;
    datab=data;
    return(0);
}




/*********************************************************\
* 							  *
*  RECA reads a byte from the SCC port A                  *
* 							  *
\*********************************************************/
reca()
{
    int cnt=0;
    int dump,i;
    while (!(rda(0) & 1)) {
	cnt++;
	dump=kick;
	if (cnt>10000) return(-1);
    }
    for (i=0;i<100;i++) dump=kick;
    return(dataa & 127);
}




/*****************************************\
* 					  *
*  RECB reads a byte from the SCC port B  *
* 					  *
\*****************************************/
recb()
{
    int cnt=0;
    int dump,i;
    while (!(rdb(0) & 1)) {
	cnt++;
	dump=kick;
	if (cnt>10000) return(-1);
    }
    for (i=0;i<100;i++) dump=kick;
    return(datab & 127);
}




/*******************************************************\
* 						        *
*  OUTSTRA outputs a string to SCC port A               *
* 						        *
\*******************************************************/
outstra(s)
char *s;
{
    while (*s) {
	if (transa(*s++)) {
	    lprintf("ERROR\n");
	    break;
	}
    }
}

  


/*******************************************************\
* 						        *
*  OUTSTRB outputs a string to SCC port B               *
* 						        *
\*******************************************************/
outstrb(s)
char *s;
{
    while (*s) {
	if (transb(*s++)) {
	    lprintf("ERROR\n");
	    break;
	}
    }
}




/***************************************************************************\
* 									    *
*  INIT sets up the SCC and performs a small test to see if this succeeded  *
* 									    *
\***************************************************************************/
init()
{
    static byte initdata[] = {
        0x00,0x00,0x00,0x41,0x44,0xaa,0x00,0x00,
	0x00,0x00,0x00,0x52,0x0b,0x00,0x83,0x00
    };
    int i;
    byte error;
    
    error=0;
    
    wra(9,0x80);
    wrb(9,0x40);
    wra(9,0xc0);
    wrb(9,0xc0);
    delay(1);
    
    for (i=0;i<16;i++) {
	wra(i,initdata[i]);
	wrb(i,initdata[i]);
    }
    
    if (rda(12)!=initdata[12]) {
        error |= 1;
    }
    
    if (rdb(12)!=initdata[12]) {
        error |= 2;
    }

    return(error);
}




/*************************************************************************\
* 									  *
* LINETEST performs a test of the SCC communications channels by sending  *
* V24 signals between them. An interconnecting cable is needed for this.  *
* 									  *
\*************************************************************************/
linetest()
{
    int cnt;
    byte error;

    wra(5,0xaa);
    error=cnt=0;
    if (!(rdb(0) & 0x28)) {
	if (!query("Vill du utf|ra signalprov mellan kanalerna?")) {
	    return(2);
	}
	while (!(rdb(0) & 0x28)) {
	    query("Installera kabeln mellan kanalerna och tryck RETURN");
	    if (cnt++ > 2) {
		return(1);
	    }
	}
    }

    wrb(5,0xaa);
    if (!(rda(0) & 0x10)) {
	lprintf("** Kabeln m}ste vara felv{nd.... Justera!\n");
	cnt=0;
	while (!(rda(0) & 0x10)) {
	    query("Tryck RETURN f|r att f|rs|ka igen");
	    if (cnt++ > 2) {
		return(error | 4);
	    }
	}
    }

    wra(5,0x28);
    if (rdb(0) & 0x28) error |= 8;
    wra(5,0x2a);
    if ((rdb(0) & 0x28) != 0x08) error |= 8;
    wra(5,0xa8);
    if ((rdb(0) & 0x28) != 0x20) error |= 8;
    wra(5,0xaa);
    if ((rdb(0) & 0x28) != 0x28) error |= 8;
    
    wrb(5,0x28);
    if (rda(0) & 0x38) error |= 16;
    wrb(5,0x2a);
    if ((rda(0) & 0x38) != 0x18) error |= 16;
    wrb(5,0xa8);
    if ((rda(0) & 0x38) != 0x20) error |= 16;
    wrb(5,0xaa);
    if ((rda(0) & 0x38) != 0x38) error |= 16;

    return(error);
}




/**************************************************************************\
* 									   *
* COMTEST performs a communications test between the two SCC channels. An  *
* interconnecting cable is needed to perform this.			   *
* 									   *
\**************************************************************************/
comtest()
{
    int i,dump;
    byte error=0;
    
    wra(11,0x56);
    wrb(11,0x12);
    wra(5,0xaa);
    wrb(5,0xaa);
    wrb(3,0x41);
    wra(3,0x41);
    wrb(4,0x4c);
    delay(1);
    while (reca() != -1) {
	dump=dataa;
	delay(1);
    }
    for (i=0;i<200;i++) {
	if (transb(65)) error |= 1;
        if ((reca() != 65) && (i>2)) error |= 2;
 	if (error & 3) break; 
        dump=kick;
    }
    delay(3);

    while (recb() != -1) {
	dump=dataa;
	delay(1);
    }
    for (i=0;i<200;i++) {
	if (transa(65)) error |= 4;
        if ((recb() != 65) && (i>2)) error |= 8;
 	if (error & 12) break; 
        dump=kick;
    }
    delay(3);

    wra(11,0x52);
    wrb(11,0x52);
    return(error);
}




/*************************************************************************\
* 									  *
* TERM performs a communications test between SCC channel A and a remote  *
* terminal.								  *
* 									  *
\*************************************************************************/
term()
{
    int cnt,i;
    byte key,error=0;
    
    lprintf("\nKoppla terminalen till kanal A\n");
    cnt=0;
    while (!(query("Tryck RETURN d} detta {r gjort"))) {
	cnt++;
	if (cnt>2) {
	    error=1;
	    break;
	}
    }
    if (error==1) return(1);

    lprintf("\nKontrollera att skrift sker p} terminalen nu...\n");
    

    outstra("\n\r\n\nTestning b|rjar.............\n\r");
    for (i=0;i<10;i++) {
	outstra("Detta {r ett prov\n\r");
	outstra("---------------->    TERMINALKOMMUNIKATION OK!\n\r");
    }
    if (!(query("Fungerade detta OK?"))) error |= 2;

    lprintf("\nSkriv nu p} terminalens tangentbord\n");
    lprintf("Tryck STOP eller CTRL-C n{r du {r klar....\n");

    key=cnt=0;
    while ((key!=3 && key!=0x81) && (cnt<20) && (keyb!=3)) {
	if ((key=reca()) != -1) {
	    lprintf("%c",key);
	    transa(key);
	    cnt=0;
	}
	cnt++;
    }
    if (!(query("\nFungerade detta OK?"))) error |= 4;

    return(error);    
}
