#include "h/tests.h"

int ledok;
int keymap[96];

keyboard()
{
    int i,errtype;
    
    lprintf("TANGENTBORD: lysdioder:\n");
    ledok=1;
    for (i=0;i<95;i++) keymap[i]= -1;
    keyb=5;
    delay(1);
    if (errtype=ledtest()) {
	lprintf("TANGENTBORD lysdioder: FEL ****\n");
	if (errtype = -1) {
	    lprintf("***** Provet avbrutet *****\n");
	    return;
	}
	if (errtype & 1) lprintf("        Tangentbordet svarar inte\n");
	if (errtype & 2) lprintf("        Felaktigt tangentbordssvar\n");
	if (errtype & 4) lprintf("        Alla lysdioderna fungerar inte\n");
    }
    else {
	lprintf("TANGENTBORD lysdioder: OK\n");
    }
    
	lprintf("\nTANGENTBORD ljudgenerator:\n");
	if (errtype=soundtest()) {
	    lprintf("TANGENTBORD ljudgenerator: FEL ****\n");
	    if (errtype & 1) lprintf("        Alarmsignalen fungerar inte\n");
	    if (errtype & 2) lprintf("        Kort klick-ljudmod fungerar ej\n");
	    if (errtype & 4) lprintf("        L}ngt klick-ljudmod fungerar ej\n");
	    if (errtype & 8) lprintf("        Inget-klick-ljudmod fungerar ej\n");
	    if (errtype & 16) lprintf("        Felaktigt tangentbordsklick-ljud\n");
	}
	else {
	    lprintf("TANGENTBORD ljudgenerator: OK\n");
	}

	lprintf("\nTANGENTBORD funktioner:\n");
	if (errtype=functest()) {
	    lprintf("TANGENTBORD funktioner: FEL ****\n");
	    if (errtype & 1) lprintf("        Auto-repeteringsmod fungerar ej\n");
	    if (errtype & 2) lprintf("        Auto-repeteringsavst{ngning fungerar ej\n");
	    if (errtype & 4) lprintf("        Auto-repetering fungerar ej\n");
	    if (errtype & 8) lprintf("        T{ndning av lysdioderna i knapparna fungerar ej\n");
	    if (errtype & 16) lprintf("        Fel p} lysdioderna i knapparna\n");
	    if (errtype & 32) lprintf("        UP/DOWN mod fungerar ej\n");
	    if (errtype & 64) lprintf("        Musinitieringen fungerar inte korrekt\n");
	    if (errtype & 128) lprintf("        Mussvar ej korrekt\n");
	}
	else {
	    lprintf("TANGENTBORD funktioner: OK\n");
	}

	lprintf("\nTANGENTBORD mark|rplacerare:\n");
	if (errtype=cursor()) {
	    lprintf("TANGENTBORD mark|rplacerare: FEL ****\n");
	}
	else {
	    lprintf("TANGENTBORD mark|rplacerare: OK\n");
	}

	lprintf("\n\n\n");
	query("Tryck RETURN f|r att forts{tta....");
	cls();
	printf("\n");
	title("ABC1600 Testprogram TANGENTBORDSTEST del2");
	errtype=keytest();
	printf("\n");
	if (errtype>=0) {
	    lprintf("Resultat av testet: Tangenterna numrerade enligt schemat.\n\n");
	    lprintf("\nTestade moder: ");
	    keyprint(keymap[95]);
	    lprintf("\n");
	    for (i=0;i<95;i++) {
		if (i<10) lprintf(" ");
	        lprintf("%d",i+1);
	        keyprint(keymap[i]);
	        lprintf("         ");
	        if ((i+1)%5==0) lprintf("\n");
	    }
	    lprintf("\n");
	}
	else {
	    lprintf("Tangenter: ej testade\n");
	}
	
}




/***************************************************************************\
* 									    *
*  KEYPRINT prints test results
* 									    *
\***************************************************************************/
keyprint(code)
int code;
{
    if (code & 1) lprintf("N"); else lprintf(" ");
    if (code & 2) lprintf("S"); else lprintf(" ");
    if (code & 4) lprintf("C"); else lprintf(" ");
    if (code & 8) lprintf("Z"); else lprintf(" ");
    if (code & 16) lprintf("U"); else lprintf(" ");
}




/****************************************************************\
* 								 *
* READLED returns the present LED status or it's entry argument	 *
* unchanged if there has been an error condition		 *
* 								 *
\****************************************************************/
readled(notwant)
int notwant;
{
    int cnt,i,j;
    byte port[7];
    
    cnt=0;
    delay(1);
    keyb=24;			/* Send ID-request to keyboard */
    for (i=0;i<7;) {
	if (status & 1) {	/* See if there is a byte to read */
	    port[i++]=keyb;
	}
	j=kick;			/* Kick the dog! */
	cnt++;
	if (cnt>10000) {	/* If no complete reply has been received */
            return(notwant);	/* Return error condition to caller */
	}
    }
    if (notwant<0) return(port[5]);
    if (port[0]==27) {		/* Mask out LEDs if sequence seems ok */
	return((port[3]<<4 & 0xF0) | port[4] & 0x0F);
    }
    else {
	return(notwant);
    }
}




/***************************************************************************\
* 									    *
* Crash checks the LEDs 0 and 7 on the keyboard to see if they are lit. If  *
* not, the keyboard is assumed to have crashed due to the last operation.   *
* 									    *
\***************************************************************************/
byte crash(bit)
byte bit;
{
    byte error;

    error=0;
    if (!ledok) return(0);
    if (readled(0)!=129) {	/* Read the LEDs and see if #0 & #7 are lit */
	error |= bit;		/* If not, set error bit */
	keyb=0;			/* And relight the LEDs */
	delay(1);
	keyb=7*16;
	delay(1);
	keyb=5;
	delay(1);
    }
    return(error);
}




/***********************************************************\
* 							    *
* LEDTEST tests the LEDs on the keyboard		    *
* User interaction is necessary for function verification   *
* 							    *
\***********************************************************/
ledtest()
{
    int i,j,cnt;
    byte port[7];
    byte error,key;

    printf("Kontrollera att samtliga lysdioder t{nds nu:\n");
    fflush(stdout);

    error=0;
    for (i=0;i<16*8;i += 16) {
        keyb=i+128;		/* Clear all LEDs */
        delay(1);
    }

    delay(1);
    for (i=0;i<16*8;i += 16) {	/* Light all LEDs */
        keyb=i;
        delay(1);
    }

    delay(40);
    key=cnt=0;
    keyb=24;			/* Send ID-request to keyboard */
    while(key != 27 && cnt<10000) {
	if (status & 1) key=keyb;
	cnt++;
    }
    cnt=0;
    if (key==27) {
    for (i=1;i<7;) {
	if (status & 1) {	/* See if there is a byte to read */
	    port[i++]=keyb;
	}
	j=kick;			/* Kick the dog! */
	cnt++;
	if (cnt>10000) {	/* If no complete reply has been received */
	    error=1;		/* skip out and report error */
	    break;
	}
    }
    }

    if ((port[3] & 15)!=15 || (port[4] & 15)!=15) {
	error |= 2;		/* Check that all LEDs are reported on */
    }

    if (error & 3) {
	ledok=0;
	printf("\n**** VARNING: Tangentbordssvar ej korrekt!\n");
	printf("****          Detta inneb{r att tangentbordsprovet\n");
	printf("****          ej blir utf|rt lika noggrant som\n");
	printf("****          det annars skulle bli.\n");fflush(stdout);
	if (query("Vill du avbryta provet?")) return(-1);
    }				/* If not, something's wrong */

    delay(40);
    delay(20);

    key=cnt=0;
    keyb=24;			/* Request keyboard ID */
    while(key != 27 & cnt<10000) {
	if (status & 1) key=keyb;
	cnt++;
    }
    cnt=0;
    if (key==27) {
    for (i=1;i<7;) {
	if (status & 1) {	/* Read byte if there is one */
	    port[i++]=keyb;
	}
	j=kick;			/* Kick dog! */
	cnt++;
	if (cnt>10000) {	/* If nothing has happened, exit and error */
	    error |= 1;
	    break;
	}
    }
    }
    
    if ((port[3] & 15)!=15 || (port[4] & 15)!=15) {
	error |= 2;		/* Check if response OK */
    }
    if ((error & 3)==0) {
	    lprintf("\nLandsvariant: ");
	    switch (port[1] & 31) {
		case 1:
		    lprintf("Svensk\n");
		    break;
		case 2:
		    lprintf("Norsk\n");
		    break;
		case 3:
		    lprintf("Dansk\n");
		    break;
		case 4:
		    lprintf("USA\n");
		    break;
		case 5:
		    lprintf("Brittisk\n");
		    break;
		case 6:
		    lprintf("Spansk\n");
		    break;
		case 7:
		    lprintf("Fransk\n");
		    break;
		case 8:
		    lprintf("Tysk\n");
		    break;
		case 9:
		    lprintf("Isl{ndsk\n");
		    break;
		default:
		    error |= 2;
		    lprintf("Felaktig kod\n");
		    break;
	    }
	    lprintf("Tangentbordstyp: ");

	    switch (port[2] & 7) {
		case 1:
		    lprintf("ABC 55\n");
		    break;
		case 2:
		    lprintf("ABC 77\n");
		    break;
		case 3:
		    lprintf("ABC 99\n");
		    break;
		default:
		    error |= 2;
		    lprintf("Felaktig kod\n");
		    break;
	    }
	    
	    lprintf("Mod: ");
	    switch (port[2] & 8) {
		case 0:
		    lprintf("normal\n");
		    break;
		case 8:
		    lprintf("UP/DOWN\n");
		    keyb=6;
		    delay(1);
		    break;
		default:
		    error |= 2;
		    lprintf("Felaktig kod\n");
		    break;
	    }
	    
	    lprintf("Musen: ");
	    switch (port[2] & 16) {
		case 0:
		    lprintf("ej ansluten\n");
		    break;
		case 16:
		    lprintf("ansluten\n");
		    break;
		default:
		    error |= 2;
		    lprintf("Felaktig kod\n");
		    break;
	    }
    }
    
    delay(40);
    printf("\nNu utf|rs individuellt styrningsprov p} lysdioderna.\n");
    printf("Kontrollera att alla kan t{ndas och sl{ckas\n");fflush(stdout);
    delay(40);
    delay(40);
    for (i=0;i<16*8;i += 16) {
        keyb=i+128;		/* Clear LED */
        delay(1);
        if (i>0) {
            keyb=i-16;		/* Restore old LED */
        }
        else {
            keyb=i+16*7;
        }
        delay(9);
    }

    for (i=0;i<16*8;i += 16) {
        keyb=i+128;		/* Clear all LEDs */
        delay(1);
    }

    for (i=0;i<16*8;i += 16) {
        if (i==4*16) i += 16;
        keyb=i;			/* Keep two LEDs lit, running towards */
        delay(1);		/* each other */
        keyb=16*7-i;
        delay(1);
        if (i>0) {
            keyb=i-16+128;
            delay(1);
            keyb=16*7-i+16+128;	/* Take care of old LEDs */
            delay(1);
        }
        delay(9);
    }

    for (i=0;i<16*8;i += 16) {
        keyb=i+128;		/* Clear all LEDs */
        delay(1);
    }

    for (i=0;i<16*8;i += 16) {
        keyb=i;			/* Light LED */
        delay(9);
	if (i>0) {
	    keyb=i-16+128;	/* Clear old LED */
	    delay(1);
	}
	else {
	    keyb=16*7+128;
	    delay(1);
	}
	delay(9);
    }
    keyb=16*7+128;
    if (!query("Fungerade lysdioderna korrekt?")) error |= 4;

    delay(1);
    return(error);
}
/*************************************************************************\
* 									  *
*  End of LED test                                                        *
* 									  *
\*************************************************************************/




/******************************************************\
* 						       *
* SOUNDTEST tests the sound generator in the keyboard  *
* 						       *
\******************************************************/
soundtest()
{
    int j,cnt;
    byte error,key;
    
    printf("Test av tangentbordets ljudgenerator\n");
    printf("Kontrollera att alarmsignalen h|rs nu:\n");fflush(stdout);

    error=0;
    delay(40);
    keyb=0;			/* Light LED 0 */
    delay(1);
    keyb=7*16;			/* Light LED 7 */
    delay(1);
    keyb=1+128;			/* Enable alarm */
    delay(1);
    keyb=7;			/* Signal!!!! */
    delay(1);
    keyb=7;
    delay(30);
    keyb=7;
    delay(30);
    keyb=7;
    delay(1);
    keyb=7;
    delay(30);

    error|=crash(1);		/* Check if this has caused a crash */
    if (!(error & 1)) {
	if (!query("Fungerade alarmsignalen OK?"))  error |= 1;
    }

    printf("Kontroll av tangentbordets -klick- funktion\n");
    printf("Testa genom att skriva n}got, tryck RETURN n{r du {r klar\n");
    fflush(stdout);
    keyb=128+2;			/* Enable short click */
    delay(1);
    keyb=128+5; 		/* Enable auto repeat */
    cnt=key=0;
    while(key!=13) {		/* Accept whatever the user types */
	j=kick;
	if (status & 1) {	/* See if there is a character to read */
	    key=keyb;
	    putchar(key);fflush(stdout);	/* Put text on screen */
	}
	cnt++;
	if (cnt>500000) break;	/* If no response has come, skip it! */
    }
    putchar('\n');

    error|=crash(2);		/* See if this has made the keyboard crash */
    keyb=128+5;
    delay(1);

    printf("Nu ska l}ngt -klick- genereras. Testa igen...\n");
    fflush(stdout);
    keyb=2;			/* Disable short clicks */
    delay(1);
    keyb=128+4;			/* Enable long clicks */
    cnt=key=0;
    while(key!=13) {		/* Let the user type away... */
	j=kick;
	if (status & 1) {	/* Read character if there is one */
	    key=keyb;
	    putchar(key);fflush(stdout);	/* Put character on screen */
	}
	cnt++;
	if (cnt>500000) break;	/* Skip out if no response */
    }
    putchar('\n');

    error|=crash(4);		/* Check if keyboard has crashed */
    keyb=128+5;			/* Keep auto repeat enabled */
    delay(1);

    printf("Nu ska allt -klick- vara avst{ngt. Prova igen...\n");
    fflush(stdout);
    keyb=4;			/* Kill long click */
    delay(1);
    keyb=2;			/* Kill short click */
    cnt=key=0;
    while(key!=13) {		/* Let the user type away! */
	j=kick;
	if (status & 1) {
	    key=keyb;
	    putchar(key);fflush(stdout);
	}
	cnt++;
	if (cnt>500000) break;
    }

    error|=crash(8);		/* See if keyboard crashed.. */

    keyb=128+2;			/* Enable short clicks again */
    delay(1);
    keyb=5;			/* Kill auto repeat */
    delay(1);
    if (!query("\nVar alla -klick- OK?")) error |= 16;

    return(error);
}



/**************************************************************************\
* 									   *
* FUNCTEST tests the auto repeat and keyboard key leds along with	   *
* attempting to do a mouse setup and running the keyboard in UP/DOWN mode  *
* 									   *
\**************************************************************************/
functest()
{
    byte key,error;
    int i,j,cnt;
    byte mouse[6];

    printf("Prova att auto-repeteringen fungerar\n");
    fflush(stdout);
    error=0;
    keyb=0;			/* Light LED 0 */
    delay(1);
    keyb=16*7;			/* Light LED 7 */
    delay(1);
    keyb=128+5;			/* Enable auto repeat */
    cnt=key=0;
    while(key!=13) {		/* Read characters from user until RETURN */
	j=kick;			/* Keep the dog kicked */
	if (status & 1) {
	    key=keyb;
	    putchar(key);fflush(stdout);	/* Dump the characters on screen */
	}
	cnt++;
	if (cnt>500000) break;	/* If no response, skip out */
    }

    error|=crash(1);

    printf("\nSe nu om den {r avst{ngd\n");
    fflush(stdout);
    keyb=5;			/* Disable auto repeat */
    cnt=key=0;
    while(key!=13) {		/* Read from user until RETURN */
	j=kick;			/* Keep dog quiet */
	if (status & 1) {	/* Is there a character to get? */
	    key=keyb;
	    putchar(key);fflush(stdout);	/* Throw it on screen */
	}
	cnt++;
	if (cnt>500000) break;	/* If no response, skip out */
    }
    delay(1);

    error|=crash(2);		/* Check if keyboard has crashed */

    printf("\n");
    if (!query("Fungerade detta OK?")) error |= 4;

    printf("Se nu att lysdioderna p} CAPS LOCK, INS och ALT {r t{nda\n");
    fflush(stdout);
    keyb=0x89;			/* INS on */
    delay(1);
    keyb=0x8a;			/* ALT on */
    delay(1);
    keyb=0x88;			/* CAPS LOCK on */
    delay(1);
    if (!query("Fungerar de?")) error |= 16;

    printf("\nNu ska de slockna\n");
    keyb=9;			/* INS off */
    delay(1);
    keyb=10;			/* ALT off */
    delay(1);
    keyb=8;			/* CAPS LOCK off */
    delay(1);
    error|=crash(8);		/* Check if keyboard crashed */
    if (!query("Gick det bra?")) error |= 16;

    printf("Tangentbordet {r nu i UP/DOWN mod.\n");
    printf("Tryck p} return f|r att testa om det fungerar\n");
    fflush(stdout);
    keyb=128+6;			/* Enable UP/DOWN mode */
    delay(1);
    cnt=key=0;
    while(key!=0x12) {		/* Check if it was code for RETURN */
	j=kick;			/* Kick dog */
	if (status & 1) {
	    key=keyb & 255;	/* Get character if there is one... */
	    if (key!=0x12) {
		printf("Felaktig kod\n");fflush(stdout);
		cnt += 100000;
	    }
	}
	cnt++;
	if (cnt>300000) {	/* If no correct response, error! */
	    error |= 32;
	    break;
	}
    }
    cnt=0;
    if ((error & 32)==0) {	/* If there wasn't an error */
	while (!(status & 1)) {	/* Get upgoing character if there is one */
	    j=kick;
	}
	key=keyb & 255;
	if (key!=128+0x12) {	/* Check if it's 128+RETURN code */
	    error |= 32;
	}
    }

    error|=crash(32);		/* Report error to user */
    fflush(stdout);
    keyb=6;
    delay(1);

    keyb=57;			/* Initiate mouse position */
    delay(1);
    keyb=77;
    delay(1);
    keyb=48;
    delay(1);
    keyb=33;
    delay(1);
    keyb=32;
    delay(1);

    keyb=60;
    for (i=0;i<6;) {
	if (status & 1) {	/* Read byte if there is one */
	    mouse[i++]=(keyb & 255);
	}
	j=kick;			/* Kick dog! */
	cnt++;
	if (cnt>10000) {	/* If nothing has happened, exit and error */
	    error |= 64;
	    break;
	}
    }

    if (!(mouse[0]==144 && mouse[1]==77 && mouse[2]==48 && mouse[3]==33 && mouse[4]==32)) {
	    error |= 128;
	}

    error|=crash(64);
    return(error);
}




/****************************************************************************\
*	 							             *
* CURSOR tests the cursor-positioning key on the keyboard in it's selectable *
* modes.								     *
* 									     *
\****************************************************************************/
cursor()
{
    int i,j,cnt,q,noerr;
    byte key,error,errflag;
    static char *name[] = {
	"h|ger", "ner}t-h|ger", "ner}t", "ner}t-v{nster", "v{nster",
	"upp}t-v{nster", "upp}t", "upp}t-h|ger"
    };

    static char *shifts[] = {
	" ", "SHIFT+", " "
    };
    
    static int code[] = {
	0xa4,0xa7,0xa3,0xaf,0xac,0xad,0xa1,0xa5,
	0xb4,0xb7,0xb3,0xbf,0xbc,0xbd,0xb1,0xb5,
	0x63,0x67,0x61,0x66,0x62,0x64,0x60,0x65
    };

    printf("Tryck mark|rplaceraren genom sina l{gen med b|rjan }t h|ger\n");
    fflush(stdout);
    error=0;
    keyb=5;
    delay(1);
    for (j=0;j<3;j++) {
	/* Run through three times */
	if (j==1) printf("\nUpprepa samma sak med SHIFT nertryckt nu\n");
	if (j==2) printf("\nVill du prova UP/DOWN mod");
	if (j>1 && !query("?")) continue;
	fflush(stdout);
	if (j==2) {
	    printf("OK, en g}ng till utan SHIFT d}...\n");fflush(stdout);
	    keyb=0x86;
	    delay(1);
	}
	cnt=0;
	for (i=0;i<8;) {
	    /* Check all eight possible directions */
	    q=kick;
	    if (status & 1) {
		key=keyb & 255;	/* Get direction and check code */
		if (key==code[i+j*8]) {
		    if (j!=2) i++;
		    if (errflag) {
			printf("\r                                                                             \r");
			errflag=0;
		    }
		    printf("* ");/* Print a * for each accepted direction */
		    fflush(stdout);
		    cnt=0;
		    if (j==2) {
			/* If in UP/DOWN mode */
			while (!(status & 1)) {
			    q=kick;
			    cnt++;	/* Get second code */
			    if (cnt>20000) {
				break;
			    }
			}
			if ((keyb & 255)!=128+code[i+8*j]) {
			    errflag=1; /* Check second code in UP/DOWN mode */
			    printf("\r                                                                             \r");
			    printf(" Fel sl{ppkod- %s%s v{ntat\r",shifts[j],name[i]);fflush(stdout);
			}
			else {
			    i++;
			}
		    }
		}
		else {
		    errflag=1;
		    printf("\r                                                                             \r");
		    printf(" Fel- %s%s v{ntat\r",shifts[j],name[i]);fflush(stdout);
		}
	    }
	    cnt++;
	    if (cnt>300000) {	/* In case of timeout */
	        printf("\n\n");
		keyb=6;
		delay(1);
		if (query("[r detta l{ge felaktigt?")) {
		    printf("\n");
		    noerr=0;
		    switch(j) {
			/* Report error status to user */
			case 0:
			    error |= 1;
		            lprintf("**** FEL: Felaktig kod f|r %s\n",name[i]);
		            break;
		        case 1:
		            error |= 2;
		            lprintf("**** FEL: Felaktig kod f|r SHIFT+%s\n",name[i]);
		            break;
		        case 2:
		            error |= 4;
		            lprintf("**** FEL: Felaktig UP/DOWN kod f|r %s\n",name[i]);
		            break;
		        default:
		            break;
		    }
		}
		else {
		    noerr=1;
		}
		lprintf("\n");fflush(stdout);
		if (query("Vill du forts{tta med provet?")) {
		    if (!noerr) {
			printf("Hoppa |ver detta l{ge och fors{tt...\n");
			i++;
		    }
		    else {
			printf("OK, f|rs|k med det igen d}...\n");
		    }
		    fflush(stdout);
		    cnt=0;
		    if (j==2) {
			keyb=6+128;
			delay(1);
		    }
		}
		else {		/* Exit if he doesn't want to go on */
		    break;
		}
	    }
	}
    }
    delay(1);
    keyb=6;
    printf("\n");
    return(error);
}





/******************************************************************\
* 								   *
*  ERRPRINT tells the user what key we were expecting him to type  *
* 								   *
\******************************************************************/
errprint(sh,key)
int sh,key;
{
    static char *name[] = {
	"PF1","PF2","PF3","PF4","PF5","PF6","PF7","PF8","PF9","PF10",
	"PF11","PF12","PF13","PF14","PF15","HELP","STOP","PRINT","ALT",
	"ESC","BS","TAB","INS","DEL"
    };

    static char *shifts[] = {
	" ", "SHIFT+", "CTRL+", "SHIFT+CTRL+", " "
    };

    static int code[] = {
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,
	0xcc,0xcd,0xce,0x80,0x81,0x82,0x83,0x1b,0x31,0x32,0x33,0x34,
	0x35,0x36,0x37,0x38,0x39,0x30,0x2b,0x60,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x71,0x77,0x65,0x72,0x74,0x79,0x75,
	0x69,0x6f,0x70,0x7d,0x7e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x61,
	0x73,0x64,0x66,0x67,0x68,0x6a,0x6b,0x6c,0x7c,0x7b,0x27,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x3c,0x7a,0x78,0x63,0x76,0x62,0x6e,
	0x6d,0x2c,0x2e,0x2d,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,
	0xdc,0xdd,0xde,0x80,0x81,0x82,0x83,0x1b,0x21,0x22,0x23,0x24,
	0x25,0x26,0x2f,0x28,0x29,0x3d,0x3f,0x40,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x51,0x57,0x45,0x52,0x54,0x59,0x55,
	0x49,0x4f,0x50,0x5d,0x5e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x41,
	0x53,0x44,0x46,0x47,0x48,0x4a,0x4b,0x4c,0x5c,0x5b,0x2a,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x3e,0x5a,0x58,0x43,0x56,0x42,0x4e,
	0x4d,0x3b,0x3a,0x5f,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,
	0xec,0xed,0xee,0x80,0x81,0x82,0x83,0x1b,0x31,0x32,0x33,0x34,
	0x35,0x36,0x37,0x38,0x39,0x30,0x2b,0x00,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x11,0x17,0x05,0x12,0x14,0x19,0x15,
	0x09,0x0f,0x10,0x1d,0x1e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x01,
	0x13,0x04,0x06,0x07,0x08,0x0a,0x0b,0x0c,0x1c,0x1b,0x27,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x7f,0x1a,0x18,0x03,0x16,0x02,0x0e,
	0x0d,0x2c,0x2e,0x2d,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,
	0xfc,0xfd,0xfe,0x80,0x81,0x82,0x83,0x1b,0x21,0x22,0x23,0x24,
	0x25,0x26,0x2f,0x28,0x29,0x3d,0x3f,0x00,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x11,0x17,0x05,0x12,0x14,0x19,0x15,
	0x09,0x1f,0x10,0x1d,0x1e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x01,
	0x13,0x04,0x06,0x07,0x08,0x0a,0x0b,0x0c,0x1c,0x1b,0x2a,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x7f,0x1a,0x18,0x03,0x16,0x02,0x1e,
	0x1d,0x3b,0x3a,0x5f,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,0x68,0x69,0x6a,0x6b,
	0x6c,0x6d,0x6e,0x6f,0x10,0x13,0x15,0x16,0x41,0x40,0x39,0x38,
	0x31,0x30,0x29,0x28,0x21,0x20,0x19,0x18,0x14,0x17,0x02,0x03,
	0x50,0x51,0x48,0x49,0x05,0x42,0x43,0x3a,0x3b,0x32,0x33,0x2a,
	0x2b,0x22,0x23,0x1a,0x08,0x12,0x52,0x53,0x4a,0x4b,0x04,0x44,
	0x45,0x3c,0x3d,0x34,0x35,0x2c,0x2d,0x24,0x25,0x1c,0x1d,0xff,
	0x54,0x55,0x4c,0x4d,0x07,0x11,0x46,0x47,0x3e,0x3f,0x36,0x37,
	0x2e,0x2f,0x26,0x27,0x06,0x57,0x56,0x4f,0x1e,0x1f,0x4e
    };

    printf("\r                                                                             \r");
    printf(" FEL- %s",shifts[sh]);
    switch(key) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	    printf("%s v{ntat...\r",name[key]);
	    break;
	case 32:
	case 33:
	case 34:
	case 35:
	    printf("%s v{ntat...\r",name[key-12]);
	    break;
	case 39:
	    printf("CE v{ntat...\r");
	    break;
	case 53:
	    printf("RETURN v{ntat...\r");
	    break;
	case 75:
	    printf("RETURN v{ntat...\r");
	    break;
	case 89:
	    printf("LF v{ntat...\r");
	    break;
	case 92:
	    printf("-> v{ntat...\r");
	    break;
	case 93:
	    printf("MELLANSLAG v{ntat...\r");
	    break;
	case 94:
	    printf("<- v{ntat...\r");
	    break;
	case 76:
	case 88:
	    printf("SHIFT v{ntat...\r");
	    break;
	case 40:
	    printf("CTRL v{ntat...\r");
	    break;
	case 58:
	    printf("CAPS LOCK v{ntat...\r");
	    break;
	default:
	    putchar(code[key]);
	    printf(" v{ntat...\r");
	    break;
    }
    fflush(stdout);
}





/**************************************************************************\
* 									   *
* KEYTEST checks through all possible keyboard codes with the help of the  *
* poor user who actually has to do all the work.			   *
* 									   *
\**************************************************************************/
keytest()
{
    int i,j,cnt,q,okcnt,noerr;
    byte error,key,sq,errflag;

    static int code[] = {
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,
	0xcc,0xcd,0xce,0x80,0x81,0x82,0x83,0x1b,0x31,0x32,0x33,0x34,
	0x35,0x36,0x37,0x38,0x39,0x30,0x2b,0x60,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x71,0x77,0x65,0x72,0x74,0x79,0x75,
	0x69,0x6f,0x70,0x7d,0x7e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x61,
	0x73,0x64,0x66,0x67,0x68,0x6a,0x6b,0x6c,0x7c,0x7b,0x27,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x3c,0x7a,0x78,0x63,0x76,0x62,0x6e,
	0x6d,0x2c,0x2e,0x2d,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,
	0xdc,0xdd,0xde,0x80,0x81,0x82,0x83,0x1b,0x21,0x22,0x23,0x24,
	0x25,0x26,0x2f,0x28,0x29,0x3d,0x3f,0x40,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x51,0x57,0x45,0x52,0x54,0x59,0x55,
	0x49,0x4f,0x50,0x5d,0x5e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x41,
	0x53,0x44,0x46,0x47,0x48,0x4a,0x4b,0x4c,0x5c,0x5b,0x2a,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x3e,0x5a,0x58,0x43,0x56,0x42,0x4e,
	0x4d,0x3b,0x3a,0x5f,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,
	0xec,0xed,0xee,0x80,0x81,0x82,0x83,0x1b,0x31,0x32,0x33,0x34,
	0x35,0x36,0x37,0x38,0x39,0x30,0x2b,0x00,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x11,0x17,0x05,0x12,0x14,0x19,0x15,
	0x09,0x0f,0x10,0x1d,0x1e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x01,
	0x13,0x04,0x06,0x07,0x08,0x0a,0x0b,0x0c,0x1c,0x1b,0x27,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x7f,0x1a,0x18,0x03,0x16,0x02,0x0e,
	0x0d,0x2c,0x2e,0x2d,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,
	0xfc,0xfd,0xfe,0x80,0x81,0x82,0x83,0x1b,0x21,0x22,0x23,0x24,
	0x25,0x26,0x2f,0x28,0x29,0x3d,0x3f,0x00,0x08,0x09,0x84,0x7f,
	0x37,0x38,0x39,0x18,0xff,0x11,0x17,0x05,0x12,0x14,0x19,0x15,
	0x09,0x1f,0x10,0x1d,0x1e,0x0d,0x34,0x35,0x36,0x2d,0xff,0x01,
	0x13,0x04,0x06,0x07,0x08,0x0a,0x0b,0x0c,0x1c,0x1b,0x2a,0xff,
	0x31,0x32,0x33,0x0d,0xff,0x7f,0x1a,0x18,0x03,0x16,0x02,0x1e,
	0x1d,0x3b,0x3a,0x5f,0xff,0x0a,0x30,0x2e,0x89,0x20,0x88,

	0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,0x68,0x69,0x6a,0x6b,
	0x6c,0x6d,0x6e,0x6f,0x10,0x13,0x15,0x16,0x41,0x40,0x39,0x38,
	0x31,0x30,0x29,0x28,0x21,0x20,0x19,0x18,0x14,0x17,0x02,0x03,
	0x50,0x51,0x48,0x49,0x05,0x42,0x43,0x3a,0x3b,0x32,0x33,0x2a,
	0x2b,0x22,0x23,0x1a,0x08,0x12,0x52,0x53,0x4a,0x4b,0x04,0x44,
	0x45,0x3c,0x3d,0x34,0x35,0x2c,0x2d,0x24,0x25,0x1c,0x1d,0xff,
	0x54,0x55,0x4c,0x4d,0x07,0x11,0x46,0x47,0x3e,0x3f,0x36,0x37,
	0x2e,0x2f,0x26,0x27,0x06,0x57,0x56,0x4f,0x1e,0x1f,0x4e
    };

    printf("\n");
    if (!query("Vill du utf|ra ett fullst{ndigt tangenttest?")) return(-1);
    printf("\n");
    printf("Tryck p} alla tangenterna fr}n PF1, }t h|ger, ner}t till <-\n");
    fflush(stdout);
    error=0;
    delay(1);
    keyb=5;			/* The usual setups */
    delay(1);
    keyb=0;
    delay(1);
    keyb=7*16;
    delay(1);
    for (i=0;i<96;i++) keymap[i]=0;
    for (j=0;j<5;j++) {
	for (q=0,sq=1;q<j;q++) sq *= 2;
	if (j==1) printf("\nVill du utf|ra SHIFT prov");
	if (j==2) printf("\nVill du utf|ra CTRL prov");
	if (j==3) printf("\nVill du utf|ra SHIFT+CTRL prov");
	if (j==4) printf("\nVill du utf|ra UP/DOWN prov");
	if (j>0 && !query("?")) continue;
	fflush(stdout);
        keymap[95] |= sq;
	if (j==4) {
	    keyb=128+6;
	    delay(1);
	}
        okcnt=cnt=0;
	for (i=0;i<95;) {	/* There are 95 keys */
	    while (code[i+j*95]==255) i++;
	    q=kick;
	    if (status & 1) {	/* Get key and verify code */
		key=keyb &255;
		if (key==code[i+j*95]) {
		    if (errflag) {
			errflag=0;
			printf("\r                                                                             \r");
		    }
		    printf("* ");fflush(stdout);
		    keymap[i]|=sq;
		    		/* Keep track of keys checked */
		    okcnt++;
		    if (okcnt>30) {
				/* Printout formatting */
			printf("\r                                                                             \r");
			okcnt=0;
		    }
		    cnt=0;
		    if (j==4) {
				/* UP/DOWN mode.... */
			while (!(status & 1)) {
			    q=kick;
			    cnt++;
			    if (cnt>50000) {
				cnt=90000;
				break;
			    }
			}
			if ((keyb & 255)!=128+code[i+95*j]) {
			    cnt += 30000;
			    keymap[i]&= ~sq;
			    i--;/* Try doing this again! */
			    printf("Tryck ej p} tv} tangenter samtidigt!\n");
			    fflush(stdout);
			    	/* Check upgoing code */
			}
		    }
		    i++;
		}
		else {
		    okcnt=0;
		    errflag=1;
		    errprint(j,i);
		}
	    }
	    cnt++;
	    if (cnt>90000) {	/* We probably have an error */
		printf("\n\n");
		keyb=6;
		delay(1);
		if (!query("Fungerar denna tangent?")) {
		    error |= sq;
		    noerr=0;
		    printf("\n**** ");
		    errprint(j,i);
		    printf("\n");
		}
		else {
		    noerr=1;
		}
		if (query("Vill du forts{tta med provet?")) {
		    if (!noerr) {
			printf("\nHoppa |ver denna tangent och fors{tt...\n");
			fflush(stdout);
			i++;
		    }
		    else {
			printf("\nOK, f|rs|k med den igen d}...\n");
			fflush(stdout);
		    }
		    cnt=0;
		    if (j==4) {
			keyb=6+128;
			delay(1);
		    }
		}
		else {
		    break;
		}
	    }
	}
	printf("\n");
	error |= crash(sq);
    }

    keyb=6;
    delay(1);
    return(error);
}
