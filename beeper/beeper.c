/* REV. 860529 THOMAS SVENSSON adderade auto-repeat och bell	*/
/* Sten Dahlgren gjorde f|rsta versionen	*/

#include	<stdio.h>
#include	<dnix/fcodes.h>
#include	<signal.h>

#define		K_ONSC		0x82	/* Turn on  short click sound */
#define		K_OFSC		0x02	/* Turn off short click sound */
#define		K_ONLC		0x84	/* Turn on  long click sound  */
#define		K_OFLC		0x04	/* Turn off  long click sound */
#define         K_ONAR          0x85    /* Turn on auto-repeat        */
#define         K_OFAR          0x05    /* Turn off auto-repeat       */
#define         K_ONBE          0x81    /* Turn on bell sound         */
#define         K_OFBE          0x01    /* Turn off bell sound        */
#define		PTOKBD		(('p'<<8) | 6)
#define		ptokbrd(fd, bp)	dnix(F_IOCW,fd,bp,1,PTOKBD,0,0,0)

typedef unsigned char	byte;

char *progname;

Usage() {
	fprintf(stderr,
"Usage: %s -[absl] | +[absl]\n", progname);
        printf("-a st{nger av auto-repeat\n");
        printf("-b st{nger av bell-sign.\n");
        printf("-s st{nger av kort klick\n");
        printf("-l st{nger av l}ngt klick\n");
        printf("+a s{tter p} auto-repeat\n");
        printf("+b s{tter p} bell-sign.\n");
        printf("+s s{tter p} kort-klick\n");
        printf("+l s{tter p} l}ngt klick\n");
	exit(1);
}

main(argc, argv)
int argc;
char **argv;
{
	register char	*cp1;
	byte	a;
	int	i;
	int	enable_s_flag = 0;
	int	enable_l_flag = 0;
        int     enable_a_flag = 0;
        int     enable_b_flag = 0;
	int	disable_s_flag = 0;
	int	disable_l_flag = 0;
        int     disable_a_flag = 0;
        int     disable_b_flag = 0;

	progname = *argv;

	/* Argument Processing */
	for (argc--, argv++; argc > 0; argc--, argv++) {
	    if (**argv == '+') {	/* A flag argument */
		while (*++(*argv)) {	/* Process all flags in this arg */
		    switch (**argv) {
			case 's':
			    enable_s_flag = 1;
			    break;
			case 'l':
			    enable_l_flag = 1;
			    break;
                        case 'a':
			    enable_a_flag = 1;
   			    break;
                        case 'b':
			    enable_b_flag = 1;
                            break;
                	default:
			    Usage();
		    }
		}
	    }
	    else {
		if (**argv == '-') {	/* A flag argument */
		    while (*++(*argv)) { /* Process all flags in this arg */
		        switch (**argv) {
			case 's':
			    disable_s_flag = 1;
			    break;
			case 'l':
			    disable_l_flag = 1;
			    break;
			case 'a':
			    disable_a_flag = 1;
			    break;
                        case 'b':
			    disable_b_flag = 1;
			    break; 
                	default:
		    	    Usage();
		        }
		    }
	        }
	        else {
		    Usage();
	        }
	    }
	}

	for (i = 1 ; i < NSIG ; i++) {
	    signal(i, SIG_DFL);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	for (i = 1; i < _NFILE ; i++) {		/* close files	*/
	    close(i);
	}
	if (open("/dev/console", 2) != 1) {
	   fprintf(stderr, "%s: Can't open /dev/console.\n", progname);
	   exit(1);
	}
	if (disable_s_flag) {
	   a = K_OFSC;
	   (byte *)cp1 = &a;
	   while (ptokbrd(1, (byte *)cp1) < 0);
	}
	if (disable_l_flag) {
	   a = K_OFLC;
	   (byte *)cp1 = &a;
	   while (ptokbrd(1, (byte *)cp1) < 0);
	}
	if (disable_a_flag) {
	   a = K_OFAR;
	   (byte *)cp1 = &a;
	   while (ptokbrd(1, (byte *)cp1) < 0);
	}
        if (disable_b_flag) {
	   a = K_OFBE;
           (byte *)cp1 = &a;
           while (ptokbrd(1, (byte *)cp1) < 0);
        }
	if (enable_s_flag) {
	   a = K_ONSC;
	   (byte *)cp1 = &a;
	   while (ptokbrd(1, (byte *)cp1) < 0);
	}
	if (enable_l_flag) {
	   a = K_ONLC;
	   (byte *)cp1 = &a;
	   while (ptokbrd(1, (byte *)cp1) < 0);
	}
	if (enable_a_flag) {
	   a = K_ONAR;
           (byte *)cp1 = &a;
           while (ptokbrd(1, (byte *)cp1) < 0);
	}   
        if (enable_b_flag) {
           a = K_ONBE;
           (byte *)cp1 = &a;
           while (ptokbrd(1, (byte *)cp1) < 0);
        }
        exit(0);
}
