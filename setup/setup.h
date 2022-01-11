/*---------------------------------------------------------------------------|
|									     |
|	setup.h								     |
|									     |
|       structure for loading / reading of user defined keyboard keys        |
|                                                                            |
|       by Kjell Sandberg LUXOR DATORER MOTALA 841221                        |
|---------------------------------------------------------------------------*/


#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#undef  FIOCLEX
#undef  FIONCLEX
#include <dnix/fcodes.h>

#define PFNKLD    (('p'<<8)|4)
#define PFNKRD    (('p'<<8)|5)
#define PMODE 0644
#define ENGLISH  0
#define SWEDISH  1
#define MAXLEN   60
#define Ioctlr(fd,subfc,bp,bc) dnix(F_IOCR,fd,bp,bc,subfc,0,0,0)

char filarr[7680];
char fnkarr[128][60];
char fnklen[128];
char name[255];
int language;
strmbuf[BUFSIZ];

struct fnkst {
    union {
	struct {
	    unsigned char code;		/* keycode, usually > 0x80	*/
	    unsigned char len;		/* length of string		*/
	} st;
    } u;
    unsigned char string[MAXLEN];       /* the functionkey string       */
};

struct sgttyb iocb;

struct keytr {
    unsigned char nr;
    unsigned char code;
    unsigned char l;
    unsigned char r;
    char *string;
    };

struct keytr keytb[] =
  {
    1,0x80,3,10,"1 HELP :",
    1,0x80,3,10,"1 HELP :",
    2,0x81,4,10,"2 STOP :",
    2,0x81,4,10,"2 STOP :",
    3,0x82,5,10,"3 PRINT:",
    3,0x82,5,10,"3 PRINT:",
    4,0x83,6,10,"4 ALT  :",
    4,0x83,6,10,"4 ALT  :",
    5,0x84,7,10,"5 INS  :",
    5,0x84,7,10,"5 INS  :",
    6,0x89,8,10,"6 -->! :",
    6,0x89,8,10,"6 -->! :",
    7,0x88,9,10,"7 !<-- :",
    7,0x88,9,10,"7 !<-- :",
    8,0xa1,13,17," 8 UP         :",
    8,0xa1,13,17," 8 UPP        :",
    9,0xa5,14,17," 9 UP-RIGHT   :",
    9,0xa5,14,17," 9 OPP-H\\GER  :",
   10,0xa4,15,17,"10 RIGHT      :",
   10,0xa4,15,17,"10 H\\GER      :",
   11,0xa7,16,17,"11 DOWN-RIGHT :",
   11,0xa7,16,17,"11 NER-H\\GER  :",
   12,0xa3,17,17,"12 DOWN       :",
   12,0xa3,17,17,"12 NER        :",
   13,0xaf,18,17,"13 DOWN-LEFT  :",
   13,0xaf,18,17,"13 NER-V[NSTER:",
   14,0xac,19,17,"14 LEFT       :",
   14,0xac,19,17,"14 V[NSTER    :",
   15,0xad,20,17,"15 UP-LEFT    :",
   15,0xad,20,17,"15 UPP-V[NSTER:",
   16,0xb1,13,57,"16 UP         :",
   16,0xb1,13,57,"16 UPP        :",
   17,0xb5,14,57,"17 UP-RIGHT   :",
   17,0xb5,14,57,"17 UPP-H\\GER  :",
   18,0xb4,15,57,"18 RIGHT      :",
   18,0xb4,15,57,"18 H\\GER      :",
   19,0xb7,16,57,"19 DOWN-RIGHT :",
   19,0xb7,16,57,"19 NER-H\\GER  :",
   20,0xb3,17,57,"20 DOWN       :",
   20,0xb3,17,57,"20 NER        :",
   21,0xbf,18,57,"21 DOWN-LEFT  :",
   21,0xbf,18,57,"21 NER-V[NSTER:",
   22,0xbc,19,57,"22 LEFT       :",
   22,0xbc,19,57,"22 V[NSTER    :",
   23,0xbd,20,57,"23 UP-LEFT    :",
   23,0xbd,20,57,"23 UPP-V[NSTER:"
  };


/* Messages and error-codes */

#define NO_ERR          0       /*No error                              */
#define R_KEY_ERR       1       /*Error when reading keyboard           */
#define L_KEY_ERR       2       /*Error when loading keyboard           */
#define TO_L_FU_STR     3       /*The function string is to long\n      */
#define TO_M_ARG        4       /*Too many arguments in command line    */
#define NO_OPEN         5       /*Can't open file                       */
#define RE_ERR          6       /*Read error                            */
#define WR_ERR          7       /*Write error                           */
#define F_NAME          8       /*Filename                              */
#define NO_CREAT        9       /*Can't create file                     */
#define SETUP           10      /*SETUP                                 */
#define PF1             11      /*PF-keys                               */
#define PF2             12      /*PF-keys (SHIFT)                       */
#define PF3             13      /*PF-keys (CRTL)                        */
#define PF4             14      /*PF-keys (SHIFT+CTRL)                  */
#define OTHER           15      /*Other keys                            */
#define R_FILE          16      /*Read from file                        */
#define S_FILE          17      /*Save on file                          */
#define CL_KEYS         18      /*Clear keyboard keys                   */
#define EXIT            19      /*Exit (text)                           */
#define SELECT          20      /*Select                                */
#define SETUP1          21      /*Setup                                 */
#define SETUP2          22      /*Setup SHIFT                           */
#define SETUP3          23      /*Setup CTRL                            */
#define SETUP4          24      /*Setup CTRL+SHIFT                      */
#define KEYNR           25      /*Key nr:                               */
#define ED              26
#define INPUT           27      /*Input                                 */
#define SETUP_OK        28      /*Setup other keys                      */
#define ARR_KEY         29      /*Arrow keys                            */
#define ARR_KEY_SH      30      /*Arrow keys shifted                    */

static char *messages[] = {

 " ",
 " ",
 "Error when reading keyboard",
 "Fel vid l{sning av tangentbordet",
 "Error when loading keyboard",
 "Fel vid laddning av tangentbordet",
 "The function string is to long\n",
 "Funktions str{ngen {r f|r l}ng\n",
 "\nToo many arguments in command line\n",
 "\nF|r m}nga argument i kommandot\n",
 "Can't open file: %s",
 "Kan inte |ppna filen: %s",
 "Read error",
 "L{sfel",
 "Write error",
 "Skrivfel",
 "Filename: ",
 "Filnamn: ",
 "Can't create file %s",
 "Kan inte skapa filen %s ",
 " SETUP ",
 " UPPS[TTNING ",
 "1  PF-keys",
 "1  PF-tangent",
 "2  PF-keys (SHIFT)",
 "2  PF-tangent (SHIFT)",
 "3  PF-keys (CTRL)",
 "3  PF-tangent (CTRL)",
 "4  PF-keys (SHIFT+CTRL)",
 "4  PF-tangent (SHIFT+CTRL)",
 "5  Other keys",
 "5  Andra funktionstangenter",
 "6  Read from file",
 "6  L{sa fr}n fil",
 "7  Save on file",
 "7  Spara p} fil",
 "8  Clear keyboard keys",
 "8  Nollst{ll PF-tangenterna",
 "0  Exit",
 "0  Slut",
 " Select ",
 " V{lj ",
 " SETUP PF-keys ",
 " DEFINIERA PF-tangenter ",
 " SETUP PF-keys (SHIFT) ",
 " DEFINIERA PF-tangenter (SHIFT) ",
 " SETUP PF-keys (CTRL) ",
 " DEFINIERA PF-tangenter (CTRL) ",
 " SETUP PF-keys (CTRL+SHIFT) ",
 " DEFINIERA PF-tangenter (CTRL+SHIFT) ",
 "Key nr:",
 "Ange nummer:",
 "Erase: CTRL-H (Backspace), End: CR, In string CR: \\ CR.",
 "Sudda: CTRL-H (Backspace), Slut: CR, Inuti str{ng CR: \\ CR.",
 "Input :",
 "Input :",
 " SETUP other keys ",
 " DEFINIERA andra funktionstangenter ",
 " ARROW KEYS ",
 " MARK\\RPLACERARE ",
 " ARROW KEYS (SHIFTED) ",
 " MARK\\RPLACERARE (SHIFTED) "
};



