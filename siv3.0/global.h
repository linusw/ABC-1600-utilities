/***********************************************************

  * * *  S T A N D A R D S  &  C O N V E N T I O N S * * *

  <1> A ROW is a string of characters of maxsize (tcols-1).
      I.e. one row on the CRT.
  <2> A LINE is the string between two newlines.
  <3> The row that holds the cursor is called the CURRENT
      ROW, and the cursor position is called the CURRENT
      POSITION. 
  <4> The text is stored in a number of buffers called
      TEXT-BUFFERS. These are created by calls to malloc().
  <5> Pointers to every row are stored in areas called
      "ROW_P"-BUFFERS. See fig in <struct.c>.
  <6> Pointers to nonused areas in the text-buffer are
      stored in the "FREE_P"-BUFFERS. See fig in <struct.c>.
      A free_p is defined as a row_p in define.h
  <7> What you see on the screen abowe the final two rows
      is called the WINDOW.

***********************************************************/


/* Terminal parameters                                    */

int   tcols ;      /* no of columns supported by tty      */
int   trows ;      /*  -"-  rows         -"-              */
int   row_len  ;   /* length of a row - i e tcols - 1     */
int   wdw_size ;   /* size of window, i.e. trows - 2      */

/* window parameters                                      */

int   row ;        /* cursors row position                */
int   col ;        /* cursors column position             */

/* various pointers to the "free_p"-block                 */

struct free_p first_f ;     /* pointer to the first "free"
                            area in the text-buffer       */
/* the list of "free" free_p:s, i.e. free_p's that are
   not currently used is the same as the list of "free"
   row_p's (*free_r) */
struct free_p dummy ;       /* dummy-object ending the
                            first_f-list.                 */

/* various pointers to the "row_p"-block                  */

struct row_p first_r ;     /* pointer to the row_p that
                            points to the first row.      */
struct row_p *prev_row ;   /* points to the row before the
                           current row.                   */
struct row_p *free_r ;     /* points to a list of "free"
                            row_p:s, i.e. row_p's that can
                            be used for new rows.         */

/* various pointers to row-buffers                        */

char *row_buf ;            /* The row that holds the 
                            cursor is put in this buffer  */
char *chr_p ;               /* Points to the current
                            position in the current row   */
int tot_len ;               /* total length of the row in
                            the row-buffer.               */
int scr_len ;		    /* screen length of the row in
			    the row buffer ( how many screen
			    posisions the line occupies when
			    printed).			  */
int buf_inx; 		    /* buffer index		  */

/* pointers etc to the kill buffer.                       */

struct row_p kill_buf ;     /* pointer to kill-buffer.    */

/* Misc.                                                  */

char *space ;               /* Blank line                 */

char hom_dir[MAXLEN] ;	    /* home directory		  */
char f_name[MAXLEN] ;       /* filename                   */
char cwd[MAXLEN] ;	    /* working directory          */
char search[MAXLEN] ;       /* string to search for       */
char replce[MAXLEN] ;       /* string to replace with     */
char prev_cmnd  ;           /* the previous command       */
char cmnd       ;           /* current command            */
FLAG vt100      ;	    /* terminal vt100 compatible  */

int  language   ;           /* 0 = english, 1 = swedish   */

FLAG restricted ;           /* no subshells               */
FLAG siv_file ;		    /* Save filename in .siv_file */
FLAG no_ctrl_Q_S ;	    /* Don't take CTRL_Q OR CTRL_S*/

FLAG ctrl_Q_flag ;	    /* a ctrl_Q is going on       */

char buf[SCREENBUF + 128] ; /* screen output buffer	  */
			    /*+128 is owerwrite protection*/
int  b_ ;		    /* screen buffer index	  */

int  b_row ;		    /* current row in the buffer
			      (cosmetica)		  */
int  u_arg ;		    /* ctrl u argument	 	  */

int  mark_b_row	;	    /* row where the mark is set  */
int  mark_b_inx ;	    /* the index on that row	  */

FLAG last_eoln ;	    /* tells ctrl_Y if ctrl_K or 
			       ctrl_W last kill line ended
			       with an eoln		  */

char stop_c[MAXLEN];	    /* chars to stop esc-F esc-B
			       and esc-D.		  */

/* any ctrl character is replaced by its corresponding
   definition in .siv_pro (read into c_arr).

   Any esc sequence is replaced by a macro sequence, if
 the esc sequence is found in m_arr.

   m_arr format: (\033 not stored)
	<esc sequence END_M replacement sequence END_M>
	------------ max length MACRLEN ---------------
*/

char c_arr[CTRLNUM];	    /* ctrl char replacment vector*/
char m_arr[MACRNUM][MACRLEN];/* esc sequence replacement macros    */

char macr[MACRLEN + 2] ;     /* executing macro		   */
int  m_ ;		     /* macr index			   */
int cmnd_start;		     /* Start of command in macro string */
int m_arg;		     /* Argument to macro 		*/

FLAG in_macro;		     /* A macro is going on		*/

