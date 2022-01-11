/*	pdriver.c	(C)	Mithril Software 1985

	This program opens /dev/lp as a file and prints a file given
	as it's first argument. The file is assumed to contain correct
	printer sequences.

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for messages.
	
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/termio.h>

main(argc,argv)
int argc;
char *argv[];
{
    int fdlp,fd,bufflen,language;
    char buffer[512];
    struct termio oldterm,newterm;

    char *prname, *filename, *l, *getenv();
    if (!(l = getenv("LANGUAGE")))
	l = "english";

    if (strcmp(l,"english"))
	language = 0;
    else
	language = 1;

    if (argc < 2 || argc > 3) {
	if (language)
	    fprintf(stderr,"usage: pdriver [device] filename\n");
	else
	    fprintf(stderr,"anv{ndning: pdriver [device] filnamn\n");
	exit(1);
	}
    if (argc == 2) {
	prname = "/dev/lp";
	filename = argv[1];
	}
    else {
	prname = argv[1];
	filename = argv[2];
	}

    fdlp = open(prname,O_WRONLY);
    if (fdlp < 0) {
	if (language)
	    fprintf(stderr,"pdriver: can't open device %s\n", prname);
        else
	    fprintf(stderr,"pdriver: kan ej |ppna device %s\n", prname);
	exit(1);
	}

    if ((fd = open(filename,0)) < 0) {
	if (language)
	    fprintf(stderr,"pdriver: can't open %s\n", filename);
	else
	    fprintf(stderr,"pdriver: kan ej |ppna %s\n", filename);
	exit(1);
	}

    ioctl(fdlp, TCGETA, &newterm);
    oldterm = newterm;
    newterm.c_iflag &= ~IXOFF;		/* Turn off XON/XOFF on input */
    newterm.c_oflag &= ~OPOST;		/* Turn off mapping of characters */
    newterm.c_lflag &= ~ECHO;		/* No echoing */
    ioctl(fdlp, TCSETA, &newterm);

    while((bufflen = read(fd, buffer, 512))!= 0) {
	write(fdlp, buffer, bufflen);
	nap(10);
	}

    ioctl(fdlp, TCSETA, &oldterm);

    close(fdlp);
    close(fd);
}
