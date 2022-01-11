/*---------------------------------------------------------------------------
 *
 *
 *     setupinv.c
 *
 *     Program to define special keys on ABC-99 keyboard
 *
 *     By Kjell Sandberg LUXOR DATORER AB 841221
 *---------------------------------------------------------------------------
 *
 * Modified by K. Sandberg 85 11 14 in line
 *
 */


#include <stdio.h>
#include <sgtty.h>
#undef   FIOCLEX
#undef   FIONCLEX
#include <dnix/fcodes.h>

#define PFNKLD    (('p'<<8)|4)
#define PFNKRD    (('p'<<8)|5)
#define PMODE 0644
#define Ioctlr(fd,subfc,bp,bc) dnix(F_IOCR,fd,bp,bc,subfc,0,0,0)
#define MAXLEN  60
#define FILE0   ".fnkeys"
#define FILE1   "/etc/.fnkeys"

char filarr [7680];
char fnkarr [128][60];
char fnklen [128];

struct fnkst {
    union {
        struct {
            unsigned char code;    /* keycode, usually > 0x80           */
            unsigned char len;     /* length of string                  */
        } st;
    } u;
    unsigned char string[MAXLEN];      /* the functionkey string            */
};

struct sgttyb iocb;

struct keytr
{
    unsigned char nr;
    unsigned char code;
    unsigned char l;
    unsigned char r;
    char *string;
};

struct keytr keytb[] =
{
    1,0x80,7,10,"1 HELP :",
    2,0x81,9,10,"2 STOP :",
    3,0x82,11,10,"3 PRINT:",
    4,0x83,13,10,"4 ALT  :",
    5,0x84,15,10,"5 INS  :",
    6,0x89,17,10,"6 -->! :",
    7,0x88,19,10,"7 !<-- :",
    8,0xa1,23,17," 8 UP         :",
    9,0xa5,25,17," 9 UP-RIGHT   :",
   10,0xa4,27,17,"10 RIGHT      :",
   11,0xa7,29,17,"11 DOWN-RIGHT :",
   12,0xa3,31,17,"12 DOWN       :",
   13,0xaf,33,17,"13 DOWN-LEFT  :",
   14,0xac,35,17,"14 LEFT       :",
   15,0xad,37,17,"15 UP-LEFT    :",
   16,0xb1,23,57,"16 UP         :",
   17,0xb5,25,57,"17 UP-RIGHT   :",
   18,0xb4,27,57,"18 RIGHT      :",
   19,0xb7,29,57,"19 DOWN-RIGHT :",
   20,0xb3,31,57,"20 DOWN       :",
   21,0xbf,33,57,"21 DOWN-LEFT  :",
   22,0xbc,35,57,"22 LEFT       :",
   23,0xbd,37,57,"23 UP-LEFT    :",
  };

/*      MESSAGES AND ERRORCODES         */

#define NO_ERR          0
#define L_KEY_ERR       1       /*Error when loading keyboard   */
#define TO_L_FU_STR     2       /*The function string is to long*/
#define N_OPEN          3       /*Can't open file               */
#define R_ERR           4       /*Read error                    */
#define TO_M_ARG        5       /*To many argument              */

static char *messages[] = {

 " ",
 "setupinv: Error when loading keyboard\n",
 "setupinv: The function string is to long\n",
 "setupinv: Can't open that file\n",
 "setupinv: Read error\n",
 "\nsetupinv: To many argument in comand line\n"
};

setty()
{
    gtty(0,&iocb);
    iocb.sg_flags |=(CBREAK);
    iocb.sg_flags &= ~ECHO;
    stty(0,&iocb);
}


restty()
{
    iocb.sg_flags &= ~(CBREAK);
    iocb.sg_flags |= ECHO;
    stty(0,&iocb);
}




putfstr(c)
int c;
{
    register int i, len;
    register unsigned char *from, *to;
    struct fnkst fks;

    fks.u.st.code=c;
    i=fks.u.st.len=fnklen[c&0x7f]; 
    len=i;
    to = &fks.string[0];
    from = (unsigned char*) &fnkarr[c&0x7f][0];
    if (i>MAXLEN)
    {
	write(2,messages[2],strlen(messages[2]));
	restty();
	exit(1);
    }

    while (i-->0)
    {
        *to++ = *from++;
    }

    if ( Ioctlr(0,PFNKLD,&fks,sizeof(fks)) < 0)
    {
	write(2,messages[1],strlen(messages[1]));
	restty();
	exit(1);
    }
}



main(argc,argv)
    int argc;
    char *argv[];
{
    int fk,fd;
    int i,j,k,l;

    setty();
    if (argc > 2){
      write(2,messages[5],strlen(messages[5]));
      restty();
      exit(1);
    }

    if (argc == 2){
      if ((fd = open(argv[1],0)) == -1){
	write(2,messages[3],strlen(messages[3]));
	restty();
	exit(1);
      }
    }
    else if ((fd=open(FILE0,0)) == -1){
	   if ((fd=open(FILE1,0)) == -1){
	     write(2,messages[3],strlen(messages[3]));
	     restty();
	     exit(1);
	   }
	 }

    fk=read(fd,(char*)(&filarr[0]),sizeof(filarr));
    if (fk <= 128){
     write(2,messages[4],strlen(messages[4]));
     restty();
     exit(1);
    }
    close(fd);

    i = 0,
    j = 0;
    k = 0;
    while(k < 128){
      fnklen[k] = filarr[k];
      k++;
    }
    while(i < 128){
      l = filarr[i];
      while(j < l){
	fnkarr[i][j++] = filarr[k++];
      }
      i++;
      j = 0;
    }

    for (fk=1; fk<16;fk++){
    putfstr(0x3f+0x80+fk);
    putfstr(0x4f+0x80+fk);
    putfstr(0x5f+0x80+fk);
    putfstr(0x6f+0x80+fk);
    }

    for(fk=0; fk<23; fk++){
    putfstr(keytb[fk].code);
    }
    restty();
    exit(0);
}












