#include         <stdio.h>
#include         <signal.h>
#include	 <sys/types.h>
#include	 <sys/ioctl.h>
#include	 <sys/termio.h>
#include         "define.h"
#include         "struct.h"
#include         "term.c"

extern int           tcols    ;
extern int           trows    ;
extern int           col      ;
extern int           row      ;
extern int           wdw_size ;
extern struct row_p  first_r  ;
extern struct row_p  kill_buf ;
extern FLAG	     no_ctrl_Q_S;

/***********************************************************
Initterm() checks if the terminal type in TERM has an entry
in the termcap file and sets the program to support that
terminal. The program exits if error occures.
***********************************************************/

initterm()
{
    int n ;
    
    switch(n = prepterm()) {
        case OK: sterm( ON | (no_ctrl_Q_S ? XON : NO_XON) );
                 return ;
        default:  error(n) ;
                  exit(1) ;
    }
}



static char *err_msg[] = {
    " " ,
    "out of memory" ,
    "TERM not set" ,
    "can't find termcap file" ,
    "no entry in termcap file" ,
    "terminaltype is not supported"
} ;

static error(n)
{
    outstr("terminal error: ") ;
    outstr(err_msg[n]) ;
    outstr("\n\nexiting\n") ;
    dump();
}

/***********************************************************
Set_sig() sets quit and interrupt to be ignored (So that siv
doesn't stop when these keys are depressed in a subshell)
***********************************************************/
set_sig(n)
int n;
{
    static int (*sigint)(), (*sigquit)();

    if (n == ON) {
	sigint = signal(SIGINT, SIG_IGN);
	sigquit = signal(SIGQUIT, SIG_IGN);
	}
    else {
	signal(SIGINT, sigint);
	signal(SIGQUIT, sigquit);
	}
}

/***********************************************************
Term(ON) resets ICANON, resets ECHO, sets quit and intr char's
Term(OLD) sets ICANON, sets ECHO, resets quit and intr char's
Term(CRNL) sets ICRNL (map CR to NL)
Term(NO_CRNL) resets ICRNL.
Term(XON) sets IXON
Term(NO_XON) resets IXON
***********************************************************/

sterm(option)
int option;
{
    static struct termio tty;
    static char intr, quit;

    ioctl(0,TCGETA  ,&tty  ) ;
    if (option & ON) {
	intr = tty.c_cc[VINTR];
	quit = tty.c_cc[VQUIT];
	tty.c_cc[VINTR] = tty.c_cc[VQUIT] = 255;
	/* I use the above assignments and not ISIG in tty.c_lflag */
	/* because I want to change c_cc[VQUIT] to CTRL_G in a     */
	/* later version. */
	tty.c_lflag &= ~(ICANON | ECHO);
	}
    if (option & OLD) {
	tty.c_cc[VINTR] = intr;
	tty.c_cc[VQUIT] = quit;
	tty.c_lflag |= (ICANON | ECHO);
	}
    if (option & XON)
	tty.c_iflag |= IXON;
    if (option & NO_XON)
        tty.c_iflag &= ~IXON;
    if (option & CRNL)
        tty.c_iflag |= ICRNL;
    if (option & NO_CRNL)
        tty.c_iflag &= ~ICRNL;
    ioctl(0,TCSETA  ,&tty  ) ;
}

/***********************************************************
Help() gives help
***********************************************************/

help()
{
    pr_msg( HELP, NULL );
}

/***********************************************************
Quit siv
***********************************************************/

quit() 
{
    static FLAG here = FALSE ;

    if ( here ) return ;
    here = TRUE ;
    pr_msg( XIT_ED, NULL ) ;
    loop :
    switch( upper_case( read_char() ) ) {

        case 'N' : here = FALSE ;
                   return ;

        case 'Y' :                   
        case 'J' : save_fname();
		   pr_msg(0, NULL);
		   cursor(trows - 1, 0);
		   dump();
                   sterm(OLD | XON) ;
                   exit(0) ; 

         default : goto loop ;

    }
}
