/* cu call-Unix utility, with some feutures            */
/* by HS 1985-03-14                                     */
/* vers. 1.0                                            */
/* compile as cc cu.c -o cu -n                        */


/* split speed handling added 85-09-11, HS		*/


#include <sgtty.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#define TRUE    1
#define FALSE   0

#define EVEN    1
#define ODD     0  /* used for parity  */
#define NOPAR   2

#define ERROR   -(1)
#define SUCCESS 1

#define PUT 1           /* used in filecopy command     */
#define TAKE 2

#define TMASK   ~(RAW + ECHO)  /* used in ioctl's */
#define TTY_OPTS  RAW
#define LMASK   ~(RAW + ECHO + EVENP + ODDP + CRMOD)
#define LINE_OPTS RAW + TANDEM

#define R       0       /* read mode */
#define W       1       /* write mode */
#define RW      2       /* read-write mode */

#define PMODE   0644    /* protection mode on new files */

#define NAMESIZE  20   /* input string maxsize */

/*      #define TELNOSIZE 10     no of digits in telno */

#define BUFSIZE 256    /* use same buffer size as tty driver for best effiency */

#define CMASK   0177   /* used to remove parity bits at rx  */

/* tables used for generation of parity bits in tx  */

static char  even[128] = {
     0x00, 0x81, 0x82, 0x03, 0x84, 0x05, 0x06, 0x87, 0x88, 0x09,
     0x0a, 0x8b, 0x0c, 0x8d, 0x8e, 0x0f, 0x90, 0x11, 0x12, 0x93,
     0x14, 0x95, 0x96, 0x17, 0x18, 0x99, 0x9a, 0x1b, 0x9c, 0x1d,
     0x1e, 0x9f, 0xa0, 0x21, 0x22, 0xa3, 0x24, 0xa5, 0xa6, 0x27,
     0x28, 0xa9, 0xaa, 0x2b, 0xac, 0x2d, 0x2e, 0xaf, 0x30, 0xb1,
     0xb2, 0x33, 0xb4, 0x35, 0x36, 0xb7, 0xb8, 0x39, 0x3a, 0xbb,
     0x3c, 0xbd, 0xbe, 0x3f, 0xc0, 0x41, 0x42, 0xc3, 0x44, 0xc5,
     0xc6, 0x47, 0x48, 0xc9, 0xca, 0x4b, 0xcc, 0x4d, 0x4e, 0xcf,
     0x50, 0xd1, 0xd2, 0x53, 0xd4, 0x55, 0x56, 0xd7, 0xd8, 0x59,
     0x5a, 0xdb, 0x5c, 0xdd, 0xde, 0x5f, 0x60, 0xe1, 0xe2, 0x63,
     0xe4, 0x65, 0x66, 0xe7, 0xe8, 0x69, 0x6a, 0xeb, 0x6c, 0xed,
     0xee, 0x6f, 0xf0, 0x71, 0x72, 0xf3, 0x74, 0xf5, 0xf6, 0x77,
     0x78, 0xf9, 0xfa, 0x7b, 0xfc, 0x7d, 0x7e, 0xff     };


static char  odd[128] = {
     0x80, 0x01, 0x02, 0x83, 0x04, 0x85, 0x86, 0x07, 0x08, 0x89,
     0x8a, 0x0b, 0x8c, 0x0d, 0x0e, 0x8f, 0x10, 0x91, 0x92, 0x13,
     0x94, 0x15, 0x16, 0x97, 0x98, 0x19, 0x1a, 0x9b, 0x1c, 0x9d,
     0x9e, 0x1f, 0x20, 0xa1, 0xa2, 0x23, 0xa4, 0x25, 0x26, 0xa7,
     0xa8, 0x29, 0x2a, 0xab, 0x2c, 0xad, 0xae, 0x2f, 0xb0, 0x31,
     0x32, 0xb3, 0x34, 0xb5, 0xb6, 0x37, 0x38, 0xb9, 0xba, 0x3b,
     0xbc, 0x3d, 0x3e, 0xbf, 0x40, 0xc1, 0xc2, 0x43, 0xc4, 0x45,
     0x46, 0xc7, 0xc8, 0x49, 0x4a, 0xcb, 0x4c, 0xcd, 0xce, 0x4f,
     0xd0, 0x51, 0x52, 0xd3, 0x54, 0xd5, 0xd6, 0x57, 0x58, 0xd9,
     0xda, 0x5b, 0xdc, 0x5d, 0x5e, 0xdf, 0xe0, 0x61, 0x62, 0xe3,
     0x64, 0xe5, 0xe6, 0x67, 0x68, 0xe9, 0xea, 0x6b, 0xec, 0x6d,
     0x6e, 0xef, 0x70, 0xf1, 0xf2, 0x73, 0xf4, 0x75, 0x76, 0xf7,
     0xf8, 0x79, 0x7a, 0xfb, 0x7c, 0xfd, 0xfe, 0x7f     };


static char nopar[128] = {
     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
     0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
     0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
     0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,
     0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
     0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
     0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
     0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63,
     0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
     0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
     0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f     };


struct options {        /* options from command string collected here */
        char ispeed;
	char ospeed;
        short parity;
        short direct;
        short device;
/*        short telephone;      */
/*        short acudev;         */
/*        short lecho;          */
        char dev[NAMESIZE];
/*        char telno[TELNOSIZE];        */
/*        char acu[NAMESIZE];           */
}  opts;

struct sgttyb  targ,larg;       /* arguments in ioctl   */
short tsave;                    /* where to save line and tty characteristics */
short lsave;
char  lspisave,lsposave;

short  pid = -1;

short  linedesc;        /* file descriptor for serial line  */

char *tabptr;  /* pointer to parity table */

int sighndl();
/*      double sum;
short count,statistic;  */

main(argc,argv)
int argc;
char *argv[];
{

        signal(SIGQUIT,sighndl);
/*      statistic = FALSE;      */

/* default values       */

        opts.ispeed = opts.ospeed = B1200;
        opts.parity = NOPAR;

/* scan input arguments                                         */
        if (scan(argc,argv) != SUCCESS)
        {
                write(2,"usage: cu [-sspeed] [-e|-o] -ldevice dir\n",42);
                exit(0);
        }

        if (opts.parity == EVEN)
                tabptr = even;
        else if (opts.parity == ODD)
                tabptr = odd;
        else
                tabptr = nopar;


/* look for not implemented commands                            
        if (opts.acudev)
        {
                write(2," -a not implemented\n",20);
                exit(0);
        }
        if (opts.lecho)
        {
                write(2," -h not implemented\n",20);
                exit(0);
        }
        if (opts.telephone)
        {
                write(2," telno not implemented\n",23);
                exit(0);
        }
*/

/* initialize own tty                           */
        if (init_tty() != SUCCESS)
        {
                write(2,"fatal error during tty initialization\n",38);
                nae_exit();
        }

/* initialize line to the other system                          */
        if (init_line() != SUCCESS)
        {
                write(2,"fatal error during output line initialization\n",46);
                nae_exit();
        }

/* split in two processes for transmit and receive              */
        if ((pid = fork()) < 0)
        {
                write(2,"fatal error in fork sys-call\n",29);
                nae_exit();
        }

/* if ok to here -> start dancing                               */
        (pid == 0) ? rx() : tx();

}



scan(argc,argv) /* scan input args, put in opts struct, return ERROR or SUCCESS */
int argc;
char *argv[];
{

        register short i,j;

        if (argc == 1)  /* no arguments! */
                return(ERROR);

        while (--argc > 0)
        {
                if ( *(*++argv) != '-') /* arguments not starting with - */
                {
                        if ( strcmp((*argv),"dir") == 0)
                                opts.direct = TRUE;
                        else
                                return(ERROR);
 /*                     {
                                j = 0;
                                while ( (j < TELNOSIZE) && (*argv)[j] != '\0')
                                        opts.telno[j++] = (*argv)[j++];
                                opts.telno[j] = '\0';
                                opts.telephone = TRUE;
                        }       */
                }
                else
                {

                switch ((*argv)[1])     /* arguments starting with - */
                {
                        case  's' :     /* speed */
                                if (strlen(*argv) <= 3) {
                                        ++argv;
                                        --argc;
                                }
                                if (getspeed(*argv) != SUCCESS) return(ERROR);
                                break;

                        case  'l' :     /* device */
                                j = 0;
                                if (strlen(*argv) <= 3) {
                                        ++argv;
                                        --argc;
                                        i = 0;
                                }
                                else
                                        i = 2;
                                while ((j < NAMESIZE) && (*argv)[i] != '\0')
                                        opts.dev[j++] = (*argv)[i++];
                                opts.dev[j] = '\0';
                                opts.device = TRUE;
                                break;

                        case  'o' :     /* odd parity */
                                opts.parity = ODD;
                                break;

                        case  'e' :     /* even parity */
                                opts.parity = EVEN;
                                break;

/*                      case 'a' :       acu device
                                j = 0;
                                if (strlen(*argv) <= 3) {
                                        ++argv;
                                        --argc;
                                        i = 0;
                                }
                                else 
                                        i = 2;
                                while ((j < NAMESIZE) && (*argv)[i] != '\0')
                                        opts.acu[j++] = (*argv)[i++];
                                opts.acu[j] = '\0';
                                opts.acudev = TRUE;
                                break;  */

/*                      case 'h' :       emulate local echo 
                                opts.lecho = TRUE;
                                break;  */

/*                      case 'S' : statistic = TRUE;
                                break;  */
                        default :
                                return(ERROR);
                        }
                }
        }
/*
        if (!(opts.direct) && !(opts.telephone))
                return(ERROR);
        else    */
                return(SUCCESS);


}       /* scan */ 



getspeed(spstr) /* get speed and convert to form used in ioctl */
char spstr[NAMESIZE];
{

register short i,j;
char tmpstr[NAMESIZE];

        j = 0;
        i = 0;
        while ( isdigit(spstr[i]) == NULL)
                ++i;
        while ( (i<NAMESIZE) && (spstr[i] != '\0') )
        {
                if ((isdigit(spstr[i])) == NULL)
                {
                        return(ERROR);
                }
                tmpstr[j++] = spstr[i++];
        }
        tmpstr[j] = '\0';

        i = atoi(tmpstr);
        
        if (i <= 50) {
		opts.ispeed = opts.ospeed = B50;
		return(SUCCESS);
	}

        if (i <= 75) {
		opts.ispeed = opts.ospeed = B75;
		return(SUCCESS);
	}

        if (i <= 110) {
		opts.ispeed = opts.ospeed = B110;
                return(SUCCESS);
	}

        if (i <= 134) {
		opts.ispeed = opts.ospeed = B134;
                return(SUCCESS);
	}

        if (i <= 150) {
		opts.ispeed = opts.ospeed = B150;
                return(SUCCESS);
	}

        if (i <= 200) {
		opts.ispeed = opts.ospeed = B200;
                return(SUCCESS);
	}

        if (i <= 300) {
		opts.ispeed = opts.ospeed = B300;
                return(SUCCESS);
	}

        if (i <= 600) {
		opts.ispeed = opts.ospeed = B600;
                return(SUCCESS);
	}

        if (i <= 1200) {
		opts.ispeed = opts.ospeed = B1200;
                return(SUCCESS);
	}

	if (i <= 1275) {
		opts.ispeed = B75;
		opts.ospeed = B1200;
		return(SUCCESS);
	}

        if (i <= 1800) {
		opts.ispeed = opts.ospeed = B1800;
                return(SUCCESS);
	}

        if (i <= 2400) {
		opts.ispeed = opts.ospeed = B2400;
                return(SUCCESS);
	}

        if (i <= 4800) {
		opts.ispeed = opts.ospeed = B4800;
                return(SUCCESS);
	}

	if (i <= 7512) {
		opts.ispeed = B1200;
		opts.ospeed = B75;
		return(SUCCESS);
	}

        else {
		opts.ispeed = opts.ospeed = B9600;
                return(SUCCESS);
	}


}



init_tty()      /* initialize tty  parameters */
{

        if (ioctl(0,TIOCGETP,&targ) < 0)
        {
                return(ERROR);
        }

        tsave = targ.sg_flags;
        targ.sg_flags &= TMASK;  
        targ.sg_flags |= TTY_OPTS;
        if (ioctl(0,TIOCSETP,&targ) < 0)
        {
                return(ERROR);
        }

        return(SUCCESS);

}


rest_tty()      /* restore tty parameters to what it was before */
{
        targ.sg_flags = tsave;
        ioctl(0,TIOCSETP,&targ);
}

init_line()     /* initialize serial parameters */
{


        if ((linedesc = open(opts.dev,RW)) <= 0)
        {
                return(ERROR);
        }
        if ( ioctl(linedesc,TIOCGETP,&larg) < 0)
        {
                return(ERROR);
        }
        
        lsave = larg.sg_flags;
        lspisave = larg.sg_ispeed;
        lsposave = larg.sg_ospeed;

        larg.sg_flags &= LMASK; 
        larg.sg_flags = LINE_OPTS;
        if (opts.parity == EVEN)
                larg.sg_flags |= EVENP;
        else if (opts.parity == ODD)
                larg.sg_flags |= ODDP;
        larg.sg_ispeed = opts.ispeed;
        larg.sg_ospeed = opts.ospeed;

        if ( ioctl(linedesc,TIOCSETP,&larg) < 0)
                return(ERROR);
        ioctl(linedesc,TIOCEXCL,&larg);

        return(SUCCESS);        
}


rest_line()     /* restore line parameters to what it was before */
{
        larg.sg_flags = lsave;
        larg.sg_ispeed = lspisave;
        larg.sg_ospeed = lsposave;
        ioctl(linedesc,TIOCSETP,&larg);

}


tx()    /* transmitter process */
{
        short action;
        char filnam1[NAMESIZE],filnam2[NAMESIZE];
        char c;

        FILE  *fp,*popen();
        static char sh[4] = { 's','h',' ','\0' };
        static char cmd[80] = { 's','h',' ','\0' };
        char line[80];
        char buf[1];
        register short i,j,n,fd;

        write(1,"cu-connected\n\r",15);
	pwrite("\004",1);

        for (;;)
        {
                c = getc(stdin);
                if (c == '\r')  /* new line, could be start of a command */
                {
                        write(linedesc,(tabptr + c),1);
                        c = getc(stdin);
                        while (c == '\r')  /* take care of more than one cr  */
                        {
                                write(linedesc,(tabptr + c),1);
                                c = getc(stdin);
                        }
                        if (c == '~')   /* just a new line  */
                        {
                                c = getc(stdin);
                                switch (c)
                                {
                                        case '.' :      /* end the session */
                                                write(1,"~.",2);
                                                nae_exit();

        /* execute command on own machine       */
                                        case '!' :
                                                write(1,"~!",2);
                                                c = getc(stdin);
                                                write(1,&c,1);
                                                while (c == ' ' || c == '\t')
                                                {
                                                        c = getc(stdin);
                                                        write(1,&c,1);
                                                }
                                                if (c == '\r')  /* only shell */
                                                {
                                                        rest_tty();
                                                        write(1,"\r\n",2);
                                                        system(sh);
                                                        init_tty();
                                                        write(linedesc,(tabptr + '\r'),1);
                                                }
                                                else  /* collect the command */
                                                {
                                                        i = 3;
                                                        cmd[i++] = c;
                                                        while ((c = getc(stdin)) != '\r')
                                                        {
                                                                if (c == '\010')  {
                                                                        if (i > 3) {
                                                                                --i;
                                                                                write(1,"\010 \010",3);
                        
                                                                        }
                                                                }
                                                                else {
                                                                        write(1,&c,1);
                                                                        cmd[i++] = c;
                                                                }
                                                        }
                                                        cmd[i] = '\0';
                                                        rest_tty();
                                                        write(1,"\r\n",2); 
                                                        system(cmd);
                                                        init_tty();
                                                        write(linedesc,(tabptr + '\r'),1);
                                                }
                                                break;

        /* execute command on own machine  send output */
                                        
                                        case '$' :
                                                write(1,"~$",2);
                                                c = getc(stdin);
                                                write(1,&c,1);
                                                while (c == ' ' || c == '\t')
                                                {
                                                        c = getc(stdin);
                                                        write(1,&c,1);
                                                }
                                                if (c == '\r')  /* strange no command */
                                                        ;
                                                else  /* collect the command */
                                                {
                                                        i = 3;
                                                        cmd[i++] = c;
                                                        while ((c = getc(stdin)) != '\r')
                                                        {
                                                                if (c == '\010')  {
                                                                        if (i > 3) {
                                                                                --i;
                                                                                write(1,"\010 \010",3);
                        
                                                                        }
                                                                }
                                                                else {
                                                                        write(1,&c,1);
                                                                        cmd[i++] = c;
                                                                }

                                                        }
                                                        cmd[i] = '\0';
                                                        rest_tty();
                                                        if (( fp = popen(cmd,"r")) == NULL)
                                                        {
                                                                write(1,"\r\ncan't open pipe to",20);
                                                                write(1,cmd,(i+1));
                                                                write(1,"\r\n",2);
                                                        }
                                                        else
                                                                while (fgets(line,80,fp))
                                                                        pwrite(line,strlen(line));
                                                        init_tty();
                                                }
                                                break;

        /* filecopy */
                                        case '%' :
                                                write(1,"~%",2);
                                                i = 0;
                                                while ((c = getc(stdin)) != '\r')
                                                {       
                                                        if (c == '\010')  {
                                                                if (i > 0) {
                                                                        --i;
                                                                        write(1,"\010 \010",3);
                                                                }
                                                        }
                                                        else {
                                                                write(1,&c,1);
                                                                line[i++] = c;
                                                        }
                                                }

                                                line[i] = '\0';
                                                action = (line[0] == 'p') ? PUT : TAKE;

                                                i = 0;
                                                while (line[i] != ' ' && line[i] != '\t')
                                                        ++i;
                                                while (line[i] == ' ' || line[i] == '\t')
                                                        ++i;
                                                j = 0;
                                                while (line[i] != ' ' && line[i] != '\0' && j < NAMESIZE)
                                                        filnam1[j++] = line[i++];
                                                filnam1[j] = '\0';
                                                if (filnam1[0] == NULL)
                                                        write(1,"\r\nno filename in filecopy command\r\n",35);
                                                else
                                                {
                                                        j = 0;
                                                        while (line[i] == ' ' || line[i] == '\t')
                                                                ++i;
                                                        while (line[i] != ' ' && line[i] != '\0' && j < NAMESIZE)
                                                        {
                                                                filnam2[j++] = line[i++];
                                                        }
                                                        filnam2[j] = '\0';
                                                        if (filnam2[0] == '\0')
                                                                strcpy(filnam2,filnam1);


                                                        switch(action)
                                                        {
                                                                case PUT :  /* send file */


                                                                        if ((fd = open(filnam1,R)) < 0)
                                                                        {
                                                                                write(2,"unable to open file ",20);
                                                                                write(2,filnam1,strlen(filnam1));
                                                                                write(2,"\r\n",2);
                                                                        }
                                                                        else
                                                                        {
                                                                                strcpy(line,"stty -echo");
                                                                                strcat(line,";cat >");
                                                                                strcat(line,filnam2);
                                                                                strcat(line,";stty echo\n");

                                                                                pwrite(line,strlen(line));
                                                                                sleep(5);
                                                                                write(1,"\r\n. . .copying. .",17);
                                                                                while ((n = read(fd,buf,1)) > 0)
                                                                                        pwrite(buf,1);
                                                                                c = '\04';
                                                                                write(linedesc,(tabptr + c),1);
                
                                                                                close(fd);
                                                                                write(1,"\r\nok\r\n",6);
                                                                        }


                                                                        break;

                                                                case TAKE :  /* get file */



                                                                        strcpy(line,"echo '\\r~>':");
                                                                        strcat(line,filnam2);
                                                                        strcat(line,";cat ");
                                                                        strcat(line,filnam1);
                                                                        strcat(line,";echo '\\r'~'>\\r'");
                                                                        pwrite(line,strlen(line));

                                                                        sleep(5);
                                                                        pwrite("\r\r",2);
                                                                        write(1,"\r\n. . .copying. .",17);
                                                                        break;


                                                        }
                                                }
                                                        


          /* send line */
                                        
                                        case '~' :
                                                write(1,"~~",2);
                                                i = 0;
                                                line[i++] = '~';
                                                while (( c = getc(stdin)) != '\r')
                                                {
                                                        write(1,&c,1);
                                                        line[i++] = c;
                                                }
                                                line[i++] = '\r';
                                                pwrite(line,i);
                                                break;

        /* could be nostop command */
                                        case 'n' :
                                                write(1,"~n",2);
                                                i = 0;
                                                while (( c = getc(stdin)) != '\r')
                                                {
                                                        write(1,&c,1);
                                                        line[i++] = c;
                                                }
                                                line[i] = '\0';
                                                if (strcmp(line,"nostop"))
                                                {
                                                        larg.sg_flags &=LMASK;
                                                        larg.sg_flags |=RAW;
                                                        if (ioctl(linedesc,TIOCSETP,&larg) < 0)
                                                                write(1,"\r\ncan't set nostop\r\n",20);
                                                        write(linedesc,"\r",1);
                                                }
                                                else  /* it was'nt */
                                                {
                                                        pwrite("~n",2);
                                                        pwrite(line,i);
                                                }
                                                break;



        /* could be stop command */
                                        case 's' :
                                                write(1,"~s",2);
                                                i = 0;
                                                while (( c = getc(stdin)) != '\r')
                                                {
                                                        write(1,&c,1);
                                                        line[i++] = c;
                                                }
                                                line[i] = '\0';
                                                if (strcmp(line,"stop"))
                                                {
                                                        larg.sg_flags &=LMASK;
                                                        larg.sg_flags |=RAW + TANDEM;
                                                        if (ioctl(linedesc,TIOCSETP,&larg) < 0)
                                                                write(1,"\r\ncan't set stop\r\n",18);
                                                        write(linedesc,(tabptr + '\r'),1);
                                                }
                                                else  /* it was'nt */
                                                {
                                                        pwrite("~s",2);
                                                        pwrite(line,i);
                                                }
                                                break;


        /* a line actually starting with ~  well, send it       */
                                        default :
                                                write(linedesc,(tabptr + '~'),1);
                                                write(linedesc,(tabptr + c),1);
                                                break;
                                }

                        }      
                        else
                                write(linedesc,(tabptr + c),1);
                }
                else
                        write(linedesc,(tabptr + c),1);
        }
}



rx()    /* receiver process */
{

        char ci[BUFSIZE];
        char cu[BUFSIZE];
        char filnam[NAMESIZE];
        register short state,fd;
        register char *pi,*pu,*hl;
        char *fi;
        short append,n;
        register short crflg = FALSE;

        fd = 1;         /* stdoutput is default */
        pu = &cu[0];
        state = 0;

/*      sum = 0;
        count = 0;      */

        for (;;)
        {
        
                if ((n = read(linedesc,ci,BUFSIZE)) <= 0)
                        break;
/*              sum += (double) n;
                ++count;        */

                hl = &ci[0] + n;
                pi = &ci[0];
                while (pi < hl)
                {

                        *pi &= CMASK;
                        switch (state)
                        {


                                case 0 :
                                        if (*pi == '\r') {
                                                state = 1;
                                        }
                                        else
                                                *pu++ = *pi;
                                        break;


                                case 1 :
                                        if (*pi == '~')
                                                state = 2;
                                        else if (*pi == '\r') {
                                                *pu++ = '\r';
                                        }
                                        else {
                                                state = 0;
                                                *pu++ = '\r';
                                                *pu++ = *pi;
                                        }
                                        break;

                                case 2 :
                                        if (*pi == '>')
                                                state = 3;
                                        else {
                                                state = 0;
                                                *pu++ = '\r';
                                                *pu++ = '~';
                                                *pu++ = *pi;
                                        }
                                        break;

                                case 3 :
                                        if (*pi == '>')
                                                state = 4;
                                        else if (*pi == ':') {
                                                state = 5;
                                                append = FALSE;
                                        }
                                        else {
                                                state = 0;
                                                *pu++ = '\r';
                                                *pu++ = '~';
                                                *pu++ = '>';
                                                *pu++ = *pi;
                                        }
                                        break;
        
                                case 4 :
                                        if (*pi == ':') {
                                                state = 5;
                                                append = TRUE;
                                        }
                                        else {
                                                state = 0;
                                                *pu++ = '\r';
                                                *pu++ = '~';
                                                *pu++ = '>';
                                                *pu++ = '>';
                                                *pu++ = *pi;
                                        }
                                        break;

                                case 5 :
                                        if (*pi == '\r') {
                                                state = 0;
                                                *pu++ = '\r';
                                                *pu++ = '~';
                                                *pu++ = '>';
                                                if (append)
                                                        *pu++ = '>';
                                                *pu++ = ':';
                                                *pu++ = *pi;
                                        }
                                        else if (*pi == ' ' || *pi == '\t') 
                                                ;
                                        else {
                                                state = 6;
                                                fi = &filnam[0];
                                                *fi++ = *pi;
                                        }
                                        break;

                                case 6 :
                                        if (*pi == '\r') {
                                                crflg = TRUE;
                                                *fi = '\0';
                                                state = 7;
                                                if (pu > &cu[0])
                                                        write(fd,cu,(pu - &cu[0]));
                                                pu = &cu[0];
                                                if (append) {
                                                        fd = open(filnam,W);
                                                        lseek(fd,(long) 0,2);
                                                }
                                                else
                                                        fd = creat(filnam,PMODE);
                                        }
                                        else
                                                *fi++ = *pi;
                                        break;

                                case 7 :
                                        if (*pi == '\r') {
                                                crflg = TRUE;
                                                state = 8;
                                        }
                                        else if (*pi == '\n' && crflg)
                                                crflg = FALSE;
                                        else
                                                *pu++ = *pi;
                                        break;

                                case 8 :
                                        if (*pi == '~')
                                                state = 9;
                                        else if (*pi == '\n' && crflg) {
                                                crflg = FALSE;
                                                *pu++ = '\n';
                                                state = 7;
                                        }
                                        else {
                                                state = 7;
                                                *pu++ = '\r';
                                                *pu++ = *pi;
                                        }
                                        break;

                                case 9 :
                                        if (*pi == '>') {
                                                state = 0;
                                                if (pu > &cu[0])
                                                        write(fd,cu,(pu - &cu[0]));
                                                pu = &cu[0];
                                                close(fd);
                                                write(1,"\r\nok\r\n",6);
                                                fd = 1;
                                        }
                                        else {
                                                *pu++ = '\r';
                                                *pu++ = '~';
                                                *pu++ = *pi;
                                        }
                                        break;
        
                        }
                        ++pi;
                }

                if (pu > &cu[0])
                        write(fd,cu,(pu - &cu[0]));
                pu = &cu[0];

        }
        exit(0);


}



nae_exit()      /* nice and easy exit */
{
	pwrite("\004",1);
        rest_tty();
        rest_line();
        if (pid != -1)                  /* if a child is born   */
        {
                kill(pid,SIGQUIT);            /* kill the child       */
                wait(0);                /* and bury him         */
        }
        ioctl(linedesc,TIOCNXCL,&larg);
        close(linedesc);
        write(1,"\ncu-disconnected\n",18);
        exit(0);

}


pwrite(str,len)         /* write str to lineout with parity */
char str[];
int len;
{

        char *p;
        p = &str[0];
        while (p < (&str[0] + len)) {
                *p = *(tabptr + *p);
                write(linedesc,p,1);
                ++p;
        }

}


sighndl(sig)
int sig;
{
/*      char str[80];

        if (statistic) {
                sprintf(str,"\r\ncu - statistics:\r\n%8.0f bytes received in %5d read call\r\naverage= %5.2f bytes/read\r\n",sum,count, (sum/count) );
                write(1,str,strlen(str));
        }       */

        exit(0);

}
