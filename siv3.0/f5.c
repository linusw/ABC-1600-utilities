/***********************************************************
This file contains the actual commands, i.e. the actions
taken when the user presses one or a combination of control-
characters.
It contains the following functions:
	set_mark()	ctrl_A()	ctrl_B()	back()
	ctrl_E()	ctrl_F()	forward()	ctrl_L()
	ctrl_N()	ctrl_P()	ctrl_O()	ctrl_Q()
	ctrl_T()	ctrl_U()	ctrl_Z()	ctrl_V()
	ctrl_X_X()	ctrl_Y()	ctrl_W()	find_row()
	down_scr_rows()	up_scr_rows()	switch_mark()	this_row()
	new_line()	new_l_action()		to_killbuf()
	fill_row()	kill_blk()	down()		up()
***********************************************************/

#include           <stdio.h>
#include           "define.h"
#include           "struct.h"

extern int          col       ;
extern int          row       ;
extern int          tcols     ;
extern int          trows     ;
extern int	    row_len   ;
extern int          wdw_size  ;
extern int          tot_len   ;
extern int	    scr_len   ;
extern int	    buf_inx   ;
extern char         *space    ;
extern char         prev_cmnd ;
extern char         cmnd      ;
extern char         *row_buf  ;
extern struct row_p kill_buf  ;
extern struct row_p *prev_row ;
extern struct row_p first_r   ;
extern int	    b_row     ;
extern int	    mark_b_row;
extern int	    mark_b_inx;
extern int	    u_arg     ;
extern FLAG 	    last_eoln ;
extern FLAG	    ctrl_Q_flag;
extern FLAG	    in_macro  ;
extern int	    m_	      ;

/***********************************************************
Set_mark() :
***********************************************************/

set_mark()
{
    mark_b_row = b_row ;
    mark_b_inx = buf_inx ;
    pr_msg(M_SET, NULL);
}

/***********************************************************
Ctrl_A() : moves the cursor to the beginning of the line.
I.e. to the position after the latest newline. 
***********************************************************/

ctrl_A()
{
    buf_inx = col = 0 ;
    test_redraw();
}

/***********************************************************
Ctrl_B() moves the cursor u_arg steps backwards. If the cursor 
is at the left margin it moves to the previous row (if 
there is one) . If the cursor is in the upper leftmost
corner you also have to scroll the screen. FALSE is returned
if beginning of text or line reached.
***********************************************************/

ctrl_B()
{
    if ( u_arg==0 ) u_arg = 1;
    back( u_arg );
    test_redraw();
    u_arg = 0;
}

/***********************************************************
Back() moves the cursor n steps backwards. If the cursor 
is at the left margin it moves to the previous row (if 
there is one) . If it reaches the beginning of the file it
returns TRUE.
***********************************************************/

back( n )
int n;
{
    int i;

    for ( i=0; i<n; i++ )
        if( col > 0 )
            dec_col() ;
        else
            if ( prev_row->previous == NULL ) {
                /* At the first row                            */
                pr_msg( TOP_FL, NULL ) ;
		return ( TRUE );
		}
            else {
                insert( row_buf, prev_row, tot_len ) ;

                /* And make the previous row current           */
                get_prev() ;
	        col = scr_len ;
	        buf_inx = tot_len ;
	        b_row--;
                row -= buf_rows() ;
                }
    return (FALSE);
}

/***********************************************************
Ctrl_E() : moves the cursor to the end of the line i.e. to 
the next newline.
***********************************************************/
            
ctrl_E()
{
    col = scr_len ;
    buf_inx = tot_len ;
    test_redraw();
}

/***********************************************************
Ctrl_F() : moves the cursor u_arg steps right. An up-scroll
is performed if it reaches the end of the last row in a window.
***********************************************************/

ctrl_F()
{
    if ( u_arg==0 ) u_arg = 1;
    forward( u_arg );
    test_redraw();
    u_arg = 0;
}

/***********************************************************
Forward() : moves the cursor n steps right. If it reaches
the end of the window it returns TRUE.
***********************************************************/

forward( n )
int n;
{
    int i;

    for ( i=0; i<n; i++ )
	if( col < scr_len ) {
	    inc_col() ;		/* move right */
	    }
	else
            /* The cursor is at the end of a row. Change  */
            /* to next row if not at eof                  */
            if ( last_row() ) {
		pr_msg( END_FL, NULL ) ;
		return ( TRUE );
		}
            else {
                /* Save the current row                   */
                insert( row_buf, prev_row, tot_len ) ;

                /* Make next row current                  */
		b_row++;
                row += buf_rows();
                buf_inx = col = 0 ;
                get_nxt() ;
	        }
    return ( FALSE );
}

/***********************************************************
Ctrl_L() : rewrites the whole window - current line is
centered.
***********************************************************/

ctrl_L()
{
    short register int i , c , p , b , hs , tmp_row ;
    register struct row_p *rp , *r ;
    
    clrscr();

    /* find line to print at top of screen			*/
    rp  = prev_row ;
    hs  = wdw_size / 2 ;
    for( i = hs - row_offset() ; (i > 0) && (rp->previous != NULL) ; ) {
	i -= scr_rows(rp);
	rp = rp->previous;
	}
    
    rp = rp->next ;

    /* center row if there was enough rows above the cursor
      otherwise recalculate the row from top of file		*/
    if( i <= 0 ) row = hs - row_offset();
    else {
	for(r = &first_r , b = b_row , row = 0 ; --b ; ) {
	    r = r->next ;
	    row += scr_rows(r);
            }
	}
    tmp_row=row;
    row=0;

    if( (i < 0) && (rp !=NULL) ) {

	/* print just the last visible lines of a row		*/
	i = - i ;
	for(b = c = 0 , p = i ; p ; b++) {
	    c += clen( *(rp->row + b), c);
	    if( c > row_len ) { c = clen( *(rp->row + b), 0) ; p-- ; }
	    }
	pr_with_eoln( rp->row , b - 1 , rp->length , 0) ;
	row = scr_rows(rp) - i ;	
	rp = rp->next ;
	}

    /* Print first row if it was the current row		*/
    if( rp == prev_row->next ) {
	pr_with_eoln( row_buf, 0, tot_len , 0);
	row += buf_rows();
	}

    /* print rest of the rows					*/
    for( ; ( row < wdw_size) && (rp != NULL) ; ) {
         pr_with_eoln( rp->row, 0, rp->length , 0) ;
	 row += scr_rows(rp);
         if( rp == prev_row ) {

	     /* print the current row 				*/
             pr_with_eoln( row_buf, 0, tot_len , 0);
             row += buf_rows() ;
             }
         rp = rp->next ;
         }
    row=tmp_row;
    print_ewd();
}


/***********************************************************
Ctrl_N() : moves the cursor down u_arg rows or 1 row unless
it reaches the last row in the buffer. 
***********************************************************/

ctrl_N()
{
    int err;

    if ( u_arg == 0 )
	err=down( 1 );
    else {
	err=down( u_arg );
	u_arg=0;
	}
    test_redraw();
/* Print error message if there was one */
    pr_msg( err, NULL );
}

/***********************************************************
Ctrl_P() : same as ctrl_N but moves the cursor up.
***********************************************************/

ctrl_P()
{
    int err;

    if ( u_arg == 0 )
	err=up( 1 );
    else {
	err=up( u_arg );
	u_arg=0;
	}
    test_redraw();
/* Print error message if there was one */
    pr_msg( err, NULL );
}
        
/***********************************************************
Ctrl_O() : acts like a new_line() but the cursor stays on
the same position.
***********************************************************/

ctrl_O()
{
    new_line() ;
    up(1) ;

    col	    = scr_len ;
    buf_inx = tot_len ;
}


/***********************************************************
Ctrl_Q() : makes it possible to write a control character. 
***********************************************************/

FLAG ctrl_Q()
{
    char c;

    ctrl_Q_flag=TRUE;
    if ( (c=read_char()) == CTRL_J )
	new_line();
    else
        add_char( c );
    ctrl_Q_flag=FALSE;
}

/***********************************************************
Ctrl_T() : exchanges the two characters before the cursor, 
only characters on the same row can be exchanged .
***********************************************************/

FLAG ctrl_T()
{
    char c1, c2 ;

    if( buf_inx < 2 ) return( FALSE ) ;
    c1 = row_buf[buf_inx - 2];
    c2 = row_buf[buf_inx - 1];
    row_buf[buf_inx - 1] = c1 ;
    row_buf[buf_inx - 2] = c2 ;
    if ( (NOT printable(c1)) OR (NOT printable(c2)) )
	pr_down();
    else {
	cursor(row, 0);
	pr_row(row_buf, 0, tot_len, 0);
	}
    return( TRUE )  ;
}

/***********************************************************
Ctrl_U() : used to give an argument to a command
***********************************************************/

ctrl_U()
{
    int arg, chr, i ;
    FLAG ctrl_U_flag;
    
    arg = 0 ;
    ctrl_U_flag=FALSE;
    pr_msg( ARGMNT, NULL ) ;

    /* Get numeric argument, maxsize MAXARG               */
    chr=read_char();
    while( ( chr >= '0' AND chr <= '9' ) OR chr == CTRL_U ) {
	if ( chr == CTRL_U ) {
	    ctrl_U_flag=TRUE;
	    if ( arg==0 ) {
		arg=16;
		pr_char('4');
		}
	    else
		arg *= 4;
	    prints(" * 4", NULL, 0);
	    }
	else {
	    if ( ctrl_U_flag ) {
		pr_msg(ARGMNT, NULL);
		arg=0;
		ctrl_U_flag=FALSE;
		}
	    arg = chr - '0' + arg * 10 ;
            pr_char(chr);
	    }
        if( arg > MAXARG ) {
            pr_msg( BIGARG, NULL ) ;
            return ;
	    }
	chr=read_char();
	}
    if ( arg==0 ) arg=4;
    u_arg = arg ;
    
    if ( in_macro ) {
        m_--; /* push back read char into macro */
        if ( m_<0 ) m_=0;
	}
    else
	ungetc( chr, stdin );
}

/***********************************************************
Ctrl_Z() : scrolls window one line up.
***********************************************************/

ctrl_Z()
{
    register struct row_p *rp ;
    short register int i , n , offs ;
    int o_row ;

    if( row == 0 ) return ;

    rp		= prev_row ;
    o_row	= row ;
    
    insert(row_buf, prev_row, tot_len);

    for( ; (row + (offs = scr_rows(prev_row->next)) - 1) < wdw_size ; ) {
        if( prev_row->next->next == NULL ) { 
	    row		= o_row ;
	    prev_row	= rp ;
	    get_current();
	    return ;
	    }
	prev_row = prev_row->next ;
	row += offs;
	}

    clearrest(wdw_size, 0);
    cursor(trows - 1, 0);
    pr_char('\n');
    cursor( --row, 0);
    pr_row(prev_row->next->row, 0, prev_row->next->length, 0);
    print_ewd();

    row		= o_row - 1 ;
    prev_row	= rp ;
    get_current();
}

/***********************************************************
Ctrl_V() : moves the cursor to the next window unless it's
at the last row.
***********************************************************/

ctrl_V()
{
  down_scr_rows( wdw_size - row );
  ctrl_L();

  /* adj_col adjusts buf_inx and col automatically 	  */
  adj_col();
}

/***********************************************************
ctrl_X_X switches mark and cursor without redrawing -
row may fall outside the screen.
***********************************************************/

FLAG ctrl_X_X()
{
    int o_b_row, o_buf_inx, t ;

    /* Any mark set ???                                 */
    if( mark_b_row == NOTSET ) {
        pr_msg(NOMSET);
        return FALSE ;
	}

    o_b_row = b_row;
    o_buf_inx = buf_inx;

    if( (t = mark_b_row - b_row) > 0 ) {
	if( t < wdw_size - row ) {
	    down( t );
	    test_redraw();
	    b_row = o_b_row;
	    buf_inx = o_buf_inx;
	    switch_mark();
	    col = abs_col( row_buf, buf_inx );
	    return FALSE ;
	    }
	}
    else
	if( (t = -t) <= row ) {
	    up( t );
	    test_redraw();
	    b_row = o_b_row ;
	    buf_inx = o_buf_inx;
            switch_mark();
	    col = abs_col( row_buf, buf_inx );
	    return FALSE ;
	    }

    switch_mark();

    insert(row_buf, prev_row, tot_len);

    find_row(b_row);
    get_current();
    col = abs_col(row_buf, buf_inx);

    /* caller will have to redraw screen in order for row
    to fall correctly inside the screen			*/
    return TRUE ;
}

/***********************************************************
Ctrl_Y() : moves the contents in the kill-buffer to the
position before the cursor. The cursor position after the
move becomes after the block.
***********************************************************/

ctrl_Y()
{
    struct row_p *kill_p, *rp ;
    FLAG f_pr_down , splt , f ;
    int n, r ;
	
    /* Anything in the kill-buffer ???                     */
    if( kill_buf.next == NULL ) return ;

    /* Split the current row in two. Save 2nd half.        */
    insert( row_buf + buf_inx, prev_row, tot_len - buf_inx ) ;
    n       = buf_rows();
    scr_len = col ;
    tot_len = buf_inx ;
    rp      = prev_row ;
    splt    = FALSE ;

    /* Only one line ?					   */
    kill_p  = kill_buf.next ;
    if( kill_p->next == NULL ) f_pr_down = FALSE ; else f_pr_down = TRUE ;

    while( TRUE ) {

        /* Insert from kill-buffer                         */
        f = fill_row( kill_p, tot_len ) ;
	if( !splt ) splt = f ;
        if( kill_p->next == NULL ) break ;
        kill_p = kill_p->next ;
        }

    if( !last_eoln ) {

        /* The last row in the kill-buffer didn't end with */
        /* a newline. I.e. add the second half of the      */
        /* split row above to it.                          */
	get_prev();
	f = append_to_row_buf( prev_row->next );
	if( !splt ) splt = f ;
	insert(row_buf, prev_row, tot_len);
        }

    prev_row = rp ;
    get_current();

    if( splt ) { 
	pr_down();
	pr_msg(L_SPLT, NULL);
	return ;
	}

    /* Reprint screen ?                                    */
    if( (f_pr_down) || (n != buf_rows()) || (last_eoln) ) pr_down();
    else {
	cursor(row, 0);
	pr_row(row_buf, 0, tot_len, 0);
	}
}

/***********************************************************
Ctrl_W() : Moves the rows between mark and cursor (region)
to the kill-buffer.
***********************************************************/

ctrl_W()
{
    struct row_p *rp ;
    FLAG f_ctrl_L , splt ;

    /* Any mark set ???                                 */
    if( mark_b_row == NOTSET ) {
        pr_msg(NOMSET);
        return ;
	}

    /* Scratch kill-buffer                              */
    scratch( &kill_buf ) ;

    /* mark on current row				*/
    if( b_row == mark_b_row ) {
	this_row();
	return ;
	}

    f_ctrl_L = (b_row > mark_b_row) ? ctrl_X_X() : FALSE ;

    insert(row_buf, prev_row, buf_inx);
    prev_row = prev_row->next ;
    insert(row_buf + buf_inx, prev_row, tot_len - buf_inx);
    rp = prev_row ;
    find_row(mark_b_row);
    if (rp->next == prev_row->next)
	to_killbuf(rp);
    else
	kill_blk(rp, prev_row->next );
    prev_row = rp ;

    if( mark_b_inx > 0) {
	get_current();
	insert(row_buf, prev_row, mark_b_inx);
	to_killbuf(prev_row);
	insert(row_buf + mark_b_inx, prev_row, tot_len - mark_b_inx);
	get_prev();
	splt = append_to_row_buf( prev_row->next );
	last_eoln = FALSE ;
	}
    else {
	last_eoln = TRUE ;
	get_prev();
	splt = append_to_row_buf( prev_row->next );
	}

    if( f_ctrl_L )
	ctrl_L();
    else
	pr_down();
    mark_b_row = b_row   ;
    mark_b_inx = buf_inx ;

    if( splt ) pr_msg(L_SPLT, NULL);
}

/***********************************************************
this_row handles the ctrl_W case when mark_b_row == b_row
***********************************************************/

this_row()
{
    short register int i , o_b_rows , o_scr_len;

    if( buf_inx > mark_b_inx ) {
	switch_mark();
	col = abs_col(row_buf, buf_inx);
	}
    o_b_rows = buf_rows();
    o_scr_len = scr_len;
    insert(row_buf, prev_row, tot_len);
    prev_row = prev_row->next ;
    insert(row_buf + buf_inx, prev_row, mark_b_inx - buf_inx);
    to_killbuf(prev_row);
    get_prev();
    for(i = 0 ; i < tot_len - mark_b_inx ; i++)
	*(row_buf + buf_inx + i) = *(row_buf + mark_b_inx + i);
    tot_len -= (mark_b_inx - buf_inx);
    scr_len = abs_col(row_buf, tot_len);
    if (o_b_rows == buf_rows()) {
	cur(row, 0);
	pr_row(row_buf, 0, tot_len, 0);
	pr_row(space, 0, o_scr_len - scr_len);
	}
    else
	pr_down();
    mark_b_inx = buf_inx ;
    last_eoln = FALSE ;
}

/***********************************************************
find_row finds a row with row number n.
***********************************************************/

find_row(n)
short register int n  ;
{
    register struct row_p *rp ;

    for(rp = &first_r ; --n ; ) rp = rp->next ;
    prev_row = rp ;
}

/***********************************************************
down_scr_rows : moves the cursor n screen rows down
(if possible). Row and b_row are adjusted accordingly.
***********************************************************/

down_scr_rows( n )
short register int n ;
{
    short register int i ;
    
    insert(row_buf, prev_row, tot_len);

    for( i = row ; (row - i) < n ; ) {
        if( prev_row->next->next == NULL ) {
	    pr_msg( END_FL, NULL ) ;
            break ;
            }
        prev_row = prev_row->next ;
	b_row++;
	row += scr_rows(prev_row) ;
        }
    get_current();
}

/***********************************************************
up_scr_rows() : moves the cursor n screen rows up 
(if possible). Row and b_row is adjusted accordingly.
***********************************************************/
    
up_scr_rows( n )
short register int n ;
{
    short register int i ;

    insert(row_buf, prev_row, tot_len);

    for( i = row ; (i - row) < n ; ) {
        if( prev_row->previous == NULL ) {
	    pr_msg( TOP_FL, NULL ) ;
            break ;
	    }
	b_row--;
	row -= scr_rows(prev_row);
        prev_row = prev_row->previous ;
	}
    get_current();
}

/***********************************************************
switch_mark swaps b_row and buf_inx with mark_ 
***********************************************************/

switch_mark()
{
    int tmp ;

    tmp = mark_b_row ;
    mark_b_row = b_row ;
    b_row = tmp ;

    tmp = mark_b_inx ;
    mark_b_inx = buf_inx ;
    buf_inx = tmp ;
}

/***********************************************************
new_line() inserts a newline and does necessary reprinting
***********************************************************/

new_line()
{
    if ( new_l_action(TRUE) )
	ctrl_L();
    else
	pr_down();
}

/***********************************************************
New_l_action() inserts a newline and prints the current
row. It is extracted for the benefit of Query replace. It
has the following cases:
1 You're at the end of that row. Just insert a
  blank row after it.

2 You're in the middle of the row. Insert a blank
  row and push down the characters to the right of
  the cursor.

If it returns TRUE the calling function will have to do a
ctrl_L() otherwise it just has to do a pr_down().
***********************************************************/

FLAG new_l_action(print)
FLAG print;
{
    int i, n ;
    struct row_p *rp ;

        if( col == scr_len ) {
            /* See 1 above                           */
	    if (b_row < mark_b_row) mark_b_row++;
	    b_row++;
	    row += buf_rows();
            insert( row_buf, prev_row, tot_len ) ;
            prev_row = prev_row->next ;
            scr_len = tot_len = buf_inx = col = 0 ;
	    return ( row >= wdw_size );
        }
        else {
            /* See 2 above                           */
	    if (b_row <= mark_b_row) {
		mark_b_row++;
		if (b_row == mark_b_row && buf_inx <= mark_b_inx)
		    mark_b_inx -= buf_inx;
		}
	    if (print) {
		cursor( row, 0 );
		pr_row( row_buf, 0, buf_inx, 0 );
		}
            insert( row_buf, prev_row, buf_inx ) ;
            prev_row = prev_row->next ;
            tot_len -= buf_inx ;
            for( i = 0 ; i < tot_len ; i++ )
                 *(row_buf + i) = *(row_buf + buf_inx + i) ;
	    if (print) {
		pr_row(space, col % row_len, row_len, col % row_len);
		cursor(row + row_offset() , row_len); pr_char(' ');
		}
	    buf_inx = col = 0 ;
	    scr_len = abs_col( row_buf , tot_len ) ;
	    b_row++;
	    row += scr_rows(prev_row);
	    return ( row >= wdw_size );
            }
}

/***********************************************************
To_killbuf() : unlinks the row pointed to by rp from the
row_p list and puts it at the end of the kill-buffer.
***********************************************************/

to_killbuf( rp )
struct row_p *rp ;
{
    struct row_p *kill_p ;
    
    kill_p = &kill_buf ;

    /* Find last position                                 */
    while( kill_p->next != NULL ) {
        kill_p = kill_p->next ;
    }
    kill_p->next = rp->next ;
    kill_p->next->previous = kill_p ;
    rp->next = kill_p->next->next ;
    kill_p->next->next = NULL ;
    if( rp->next != NULL ) rp->next->previous = rp ;
}

/***********************************************************
Fill_row() : fills row_buf with the kill-buffer-row
pointed to by rp->row. 
***********************************************************/

fill_row( rp, n )
struct row_p *rp ;
int n ;
{
    int len ;
    FLAG splt ;

    len  = rp->length ;
    splt = FALSE ;

    if( n + len >= MAXROW ) {
	insert( row_buf, prev_row, tot_len ) ;
	prev_row = prev_row->next ;
	n = 0 ;
	splt = TRUE ;
	}

    cpystr( rp->row , len, row_buf + n ) ;
    tot_len = n + len ;
    insert( row_buf, prev_row, tot_len ) ;
    prev_row = prev_row->next ;

    tot_len = 0 ;
    return splt ;
}

/***********************************************************
Kill_blk() : moves the row_p's from p1->next to p2
to the kill-buffer.
***********************************************************/


kill_blk( p1 TO p2 )
struct row_p *p1, *p2 ;
{
    kill_buf.next      = p1->next ;
    p1->next->previous = &kill_buf ;
    p2->next->previous = p1 ;
    p1->next           = p2->next ;
    p2->next	       = NULL ;
}

/***********************************************************
Down() : moves the cursor n rows down (if possible). It 
returns END_FL if it reached the end of the file, other-
wise NO_ERR.
***********************************************************/

down( n )
int n ;
{
    int i, err ;
    
    insert( row_buf, prev_row, tot_len ) ;
    err = NO_ERR ;
    for( i = 0 ; i < n ; i++ ) {
        if( prev_row->next->next == NULL ) {
            err = END_FL ;
            break ;
            }
        prev_row = prev_row->next ;
	b_row++;
	row += scr_rows(prev_row) ;
        }
    get_current() ;
    adj_col();
	
    return (err);
}

/***********************************************************
Up() : moves the cursor n rows up if possible. It returns
TOP_FL if it reached the top of the file.
***********************************************************/
    
up( n )
int n ;
{
    int i, err ;

    insert( row_buf, prev_row, tot_len ) ;
    err = NO_ERR ;
    for( i = 0 ; i < n ; i++ ) {
        if( prev_row->previous == NULL ) {
            err = TOP_FL ;
            break ;
        }
	b_row--;
	row -= scr_rows(prev_row);
        prev_row = prev_row->previous ;
    }
    get_current() ;
    adj_col();

    return (err);
}
