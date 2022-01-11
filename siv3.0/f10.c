
#include <stdio.h>
#include "define.h"
#include "struct.h"

extern int	col ;
extern int	row ;
extern int	tcols ;
extern int	trows ;
extern int	row_len ;
extern int	tot_len ;
extern int	scr_len ;
extern int	buf_inx ;
extern int	wdw_size;
extern char	*row_buf ;

/***********************************************************
Tabsize returns the tabulator length from col 'c'.
***********************************************************/

int tabsize(cl)
short register int cl;
{
  return TABSIZE - (cl % TABSIZE) ;
}

/***********************************************************
clen returns the number of absolute screen positions the 
char c will occupy at column cl compensating for the
right margin.
***********************************************************/

int clen(c,cl)
short register int c , cl ;
{
    short register int l ;

    cl %= row_len ;

    if (c == TAB)
	l = tabsize(cl);
    else
	l = (printable(c) ? CHARSIZE : CTRLSIZE );
    if (cl + l > row_len) {
	if( c == TAB ) return ( row_len - cl + tabsize(0) );
	return ( row_len - cl + l ) ;
	}
    else
	return ( l ) ;
}

/***********************************************************
Abs_col returns an integer that equals the 'inx'
posision on screen of the line pointed to by rp ( namely 
if the line contains a tab at col 0, Abs_col will return
col % 8 (tabsize(col)) if col == 1).
***********************************************************/

int abs_col(rp,inx)
register char *rp ;
short register int inx ;
{
  short register int a , c ;

  for( a = 0 , c = 0 ; a < inx ; )
       c += clen( *(rp + a++) , c ) ;

  return c ;
}

/***********************************************************
Scr_rows returns the number of rows rp occupyes on screen.
***********************************************************/

int scr_rows(rp)
struct row_p *rp ;
{
    return (int) ((abs_col(rp->row , rp->length) - 1) / (tcols - 1) + 1) ;
}

/***********************************************************
Buf_rows returns the number of rows a row of length scr_len
occupies on the screen.
***********************************************************/

int buf_rows()
{
    return (int) ( (scr_len - 1) / (tcols - 1 ) + 1) ;
}

/***********************************************************
row_offset returns the number of rows the cursor is down
from the first line of the row.
***********************************************************/

int row_offset()
{
    return (int) (col / tcols) ;
}

/***********************************************************
Test_redraw calls ctrl_l if cursor is outside window.
***********************************************************/

test_redraw()
{
    if( (row+row_offset() >= wdw_size) || (row < 0) ) ctrl_L();
}

/***********************************************************
Adj_col adjusts the column when moving up or down lines.
***********************************************************/

adj_col()
{
    short register int i ;

    if ( col > scr_len ) {
	col = scr_len;
	buf_inx = tot_len;
	}
    else {
	for( i = buf_inx = 0 ; i < col ; )
	    i += clen( *(row_buf + buf_inx++) , i );
	col = i;
	}
}

/***********************************************************
Inc_col increments col according to the next char in the 
row_buf. Buf_inx is alsow incremented.
***********************************************************/

inc_col()
{
  col += clen( *(row_buf + buf_inx++) , col ) ;
}

/***********************************************************
Dec_col decrements col according to the previous char in the 
row_buf. Buf_inx is alsow decremented.
***********************************************************/

dec_col()
{
    char c ;

    if( (c = *(row_buf + --buf_inx)) != TAB ) col -= clen( c , col );
    else col = abs_col(row_buf, buf_inx);
}
