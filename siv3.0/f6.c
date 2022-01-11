/***********************************************************
                   I M P O R T A N T

The routines in this file either insert or append a
character at the cursor.

  * A LINE denotes the characters between the previous
    carriage return and the next.

  * A ROW denotes the characters you see on one row
    of the CRT. I.e. a row is never longer than a line.

***********************************************************/
 
#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           col       ;
extern int           row       ;
extern int           tcols     ;
extern int           trows     ;
extern int	     row_len   ;
extern int           wdw_size  ;
extern int           tot_len   ;
extern int	     scr_len   ;
extern int	     buf_inx   ;
extern FLAG          eoln      ;
extern char          *row_buf  ;
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;
extern char	     buf[]     ;
extern int	     b_	       ;
extern int	     b_row     ;
extern int	     mark_b_row;
extern int	     mark_b_inx;

/***********************************************************
Add_char() appends or inserts a character into the current
position. Add_char_action is the add_char core (no printing)
extracted for the benefit of q_rplace.
***********************************************************/

add_char( c ) char c ;
{
    int n , tmp_col , tmp_buf_inx ;

    if( tot_len >= MAXROW ) {
	pr_msg(MAX_LN, NULL);
	return ;
	}

    n		= buf_rows();
    tmp_col 	= col ;
    tmp_buf_inx = buf_inx ;

    add_char_action(c);

    /* Print the rightmost characters                     */
    /* cursor not moved since input			  */
    if( row + row_offset() >= wdw_size - 1 ) {
	ctrl_L(); 
	return ;
	}
    if( n == buf_rows() ) pr_row( row_buf , tmp_buf_inx, tot_len , tmp_col) ;
    else pr_down();
}

add_char_action( c )
char c ;
{
    register int i ;

    /* Shift the characters to the right one step right
       and insert c into row_buf. 			  */
    for(i = tot_len ; i >= buf_inx ; i-- )
	*(row_buf + i + 1) = *(row_buf + i) ;
    *(row_buf + buf_inx) = c ;

    tot_len++ ;
    if( col == scr_len ) scr_len += clen(c , col);
    else scr_len = abs_col(row_buf, tot_len);

    if( (b_row == mark_b_row) && (buf_inx < mark_b_inx) ) mark_b_inx++;
    inc_col();
}

/***********************************************************
Pr_row() prints the characters within 'from' to 'to' in the
buffer 'buf'.
***********************************************************/

pr_row( st, from, to , cl) 
register char *st ;
short register int  from, to , cl;
{
    short register int  n , c , offset , r ;

    if( b_ + (to - from) * 4 >= SCREENBUF ) dump();

    for (n = 0 , r = row , cl %= row_len , st += from ; from++ < to ; ) {
        c = ( *st++ ) ;
	if( (cl += (offset = clen(c,cl)) ) > row_len ) {
	    for( cl -= offset ; cl++ < row_len ; ) buf[b_++] = ' ' ;
	    buf[b_++] = '\\' ;
	    buf[b_++] = '\n' ;
	    if( ++r >= wdw_size ) break ;
	    cl = clen(c, 0) ;
	    }
        if ( printable(c) || c == TAB )
	    buf[b_++] = c ;
        else {
       	    buf[b_++] = '\\' ;
            buf[b_++] = (char) ( ((c & 192) / 64) + '0' ) ;
	    buf[b_++] = (char) ( ((c &  56) /  8) + '0' ) ;
	    buf[b_++] = (char) (  (c &   7)       + '0' ) ;
            }
       }
}

/***********************************************************
Pr_with_eoln() calls pr_row and outputs an eoln
***********************************************************/

pr_with_eoln( st, from, to , cl ) 
char *st ;
int  from, to ;
{
    pr_row(st, from, to , cl);
    pr_char('\n');
}

/***********************************************************
Pr_char() outputs a char.
***********************************************************/

pr_char(c)
char c ;
{
    if ( b_ >= SCREENBUF ) dump();
    buf[b_++] = c;
}

/*****************************************************************
Outstr() is used instead of printf() i.e. for printing a string
on the tty.
*****************************************************************/

outstr( string )
char *string ; 
{
    if( b_+strlen(string) > SCREENBUF ) dump();
    for(; (buf[b_++] = *string++) ; ) ;
    b_--; /* skip the \0 char			*/
}
