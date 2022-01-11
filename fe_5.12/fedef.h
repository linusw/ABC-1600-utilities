/************************************************************************/
/*      fedef.h :       Definitions for font-editor for ABC1600         */
#define SCRN_X 699                      /* Screen size x -1             */
#define SCRN_Y 699                      /* Screen size y -1             */
#define BMAX_X 32                       /* maximum boxsize x            */
#define BMAX_Y 32                       /* maximum boxsize y            */


/*
*       font information structure 
*/

struct cgenblk {
  unsigned short cg_magic;      /* type of generator (value: 0xffff)   */
  unsigned short cg_stat;       /* flags i e Portrait/Landscape        */
  unsigned short cg_fnumb;      /* the number of the generator         */
  unsigned short cg_fxsiz;      /* font sixe x, # of pixels            */
  unsigned short cg_fysiz;      /* font size y, # of pixels            */
  unsigned char  cg_firstc;     /* ascii # for first defined char      */
  unsigned char  cg_lastc;      /* ascii # for last defined char       */
  unsigned short cg_bwidth;     /* # of bytes per line in memory dump  */
  unsigned short cg_dumpl;      /* # of bytes in dump after this block */
  unsigned char  cg_baslin;     /* baseline of font (0= bottom of box) */
  unsigned char  cg_leftmg;     /* left margin of matrix in box        */
  unsigned char  cg_matx;       /* matrix size x                       */
  unsigned char  cg_maty;       /* matrix size y                       */
  unsigned char  cg_style[6];   /* name of font style                  */
  unsigned char  pad[13];       /* pad block to 40 bytes. Future use   */
} ;


/*
*       picture load/store structure
*/
struct pictblk {
  unsigned short magic;         /* type of generator (value: 0xfffe)   */
  unsigned short stat;          /* flags                               */
  unsigned short xaddr;         /* from/to x address (aligned pixel)   */
  unsigned short yaddr;         /* from/to y address (pixel)           */
  unsigned short bwidth;        /* # of bytes per line in memory dump  */
  unsigned short bheight;       /* # of bytes in dump after this block */
  unsigned char pad2[38];        /* pad block to 40 bytes. Future use   */
} ;



/*      End of definition file                                          */
/************************************************************************/

