/***********************************************************
This file contains routines to delete characters. I have
tried to be as clear as possible, but they can probably cause
severe braindamage even to people with nerves of steel. I
think the best way to understand them is to read the code
and execute it simultaneously. DON'T TRY TO UNDERSTAND IT
IF YOU HAVEN'T READ THE NOTES IN THE FILE <global.h>!!!!!
***********************************************************/
        
#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           col       ;
extern int           row       ;
extern int           tcols     ;
extern int           wdw_size  ;
extern int           trows     ;
extern int	     row_len   ;
extern int           tot_len   ;
extern int	     scr_len   ;
extern int	     buf_inx   ;
extern int           eoln      ;
extern char          *row_buf  ;
extern char          *space    ;
extern char          prev_cmnd ;
extern struct row_p  *prev_row ;
extern struct row_p  kill_buf  ;
extern int	     b_row     ;
extern int 	     mark_b_row;
extern int	     mark_b_inx;
extern FLAG	     last_eoln ;

/***********************************************************
Delete() : deletes the character to the left of the cursor 
and shifts the rightmost characters one step left. If the 
cursor is at the left margin and the previous row has an 
eoln, the lines are appended. 
***********************************************************/

delete()
{
    if (back(1)) return;
    test_redraw();
    ctrl_D();
}

/***********************************************************
Ctrl_D() : deletes the character at the cursor and shifts
the rightmost characters one step left. If the character
is a newline the next line is appended. ctrl_D_action is
the special case of ctrl_D when col != scr_len without
printing extracted for the benefit of q_rplace and esc-D.
***********************************************************/

ctrl_D()
{
    int i , n , c ;
    FLAG splt ;

    /* Is the character a newline ???                     */
    if( col == scr_len ) {

        /* Do nothing if last row                         */
        if( last_row() ) return( FALSE ) ;

        /* Append the next line                       */
	i = tot_len;
	splt = append_to_row_buf(prev_row->next);	    
	pr_down();
	if (splt)
	    pr_msg(L_SPLT, NULL);
	del_adjust_mark(splt, i);
	return ;
        }

    /* Delete the char and shift the leftmost     */
    if( (b_row == mark_b_row) && (buf_inx <= mark_b_inx) ) mark_b_inx--;
    n = buf_rows();
    tot_len-- ; 
    for( i = buf_inx ; i < tot_len ; i++ ) {
         *(row_buf + i) = *(row_buf + i + 1) ;
         }
    c = scr_len ;
    scr_len = abs_col(row_buf , tot_len);
    c -= scr_len ;
    /* Rewrite rest of screen or current line depending on situation. 'n' */
    /* contains the number of rows that the two rows occupyed on the      */
    /* screen before appended into buf_rows. 'c' is the number of         */
    /* characters to be cleared. */
    if (n != buf_rows())
	pr_down();
    else {
	cur(row , col);
	pr_row(row_buf,buf_inx,tot_len,col);
	cur( row  , scr_len );
	pr_row( space , 0 , c, scr_len );
	}
}

ctrl_D_action()
{
    register int i ;

    if( col == scr_len ) return FALSE ;

    /* Delete the char and shift the leftmost     */
    if( (b_row == mark_b_row) && (buf_inx <= mark_b_inx) ) mark_b_inx--;
    tot_len-- ; 
    for( i = buf_inx ; i < tot_len ; i++ ) {
         *(row_buf + i) = *(row_buf + i + 1) ;
         }
    scr_len = abs_col(row_buf , tot_len);
    return TRUE ;
}

/***********************************************************
Ctrl_K() : Works a little different depending on the situa-
tion. If the cursor is at the end of a line i.e. at the new-
line, the newline is deleted and the next line is appended
to the current row. However, if the cursor is not at the end
of the line, the rest of the characters in the line are del-
eted, leaving only the newline at the cursor-position. All
killed rows/lines are unlinked from the first_r-list and put
at the end of the kill-buffer.
***********************************************************/

ctrl_K()
{
    int n , old_scr_len, old_tot_len ;
    FLAG splt ;

    /* Empty the kill_buf if previous cmnd wasn't CTRL_K  */
    if( prev_cmnd != CTRL_K ) scratch( &kill_buf ) ;

    /* Only one row of the line after the cursor      */

    if( col < scr_len ) {

        /* Delete rest of line/row                     */
	n = buf_rows();
	old_scr_len = scr_len ;
        insert( row_buf + buf_inx, prev_row, tot_len - buf_inx) ;
        to_killbuf( prev_row ) ;
	last_eoln = FALSE ;
        tot_len = buf_inx ;
	scr_len = col ;
	if( n == 1 ) pr_row(space, col, old_scr_len, col);
	else pr_down();

	/* possibly adjust mark_b_inx			*/
	if( b_row==mark_b_row AND mark_b_inx>tot_len )
	    mark_b_inx = tot_len ;
        }
    else {
        if (last_row())
	    return(FALSE);
	if (last_eoln) {
	    insert(row_buf, prev_row, 0); /* blank row */
	    to_killbuf(prev_row);
	    }
	last_eoln = TRUE ;
	old_tot_len = tot_len;
	splt = append_to_row_buf(prev_row->next);
	pr_down();
	del_adjust_mark(splt, old_tot_len);
	if (splt)
	    pr_msg(L_SPLT, NULL);
        }
    return(TRUE);
}

/***********************************************************
Append_to_row_buf appends rp to the row_buf and removes rp.
TRUE is returned if no append took place.
***********************************************************/

FLAG append_to_row_buf(rp)
struct row_p *rp ;
{
    char *get_row() ;
    int n ;

    n = rp->length ;

    if( n + tot_len >= MAXROW )	return TRUE ; /* line split */

    cpystr(get_row(rp), n, row_buf + tot_len );
    tot_len += n ;
    scr_len = abs_col( row_buf , tot_len );

    return FALSE ;
}

/***********************************************************
Del_adjust_mark adjusts the mark when a linefeed is deleted
***********************************************************/
del_adjust_mark(splt, old_tot_len)
FLAG splt; int old_tot_len;
{
    if ( mark_b_row>b_row AND ( NOT splt ) ) {
	if ( mark_b_row == b_row+1 )
	    mark_b_inx = old_tot_len+mark_b_inx;
	mark_b_row--;
	}
}

/***********************************************************
Pr_down() : clears the screen from row down, prints row_buf
followd by the rest of the rows. Finally the dotted line is
printed after the window.
***********************************************************/

pr_down()
{
    register struct row_p *rp ;
    short register int tmp_row ;

    clearrest( row, 0 ) ;
    cursor  ( row, 0 ) ;

    pr_with_eoln( row_buf, 0, tot_len, 0 );

    rp = prev_row->next ;

    tmp_row = row;
    for( row += buf_rows() ; ( row < wdw_size) && (rp != NULL) ; ) {
        pr_with_eoln( rp->row, 0, rp->length, 0 ) ;
	row += scr_rows(rp);
        rp = rp->next ;
	}
    row = tmp_row;
    print_ewd();
}

/***********************************************************
Get_nxt() : makes the next row current.
***********************************************************/

get_nxt()
{
    prev_row = prev_row->next ;
    get_current() ;
}

/***********************************************************
Get_prev() : makes the previous row current.
***********************************************************/

get_prev()
{
    prev_row = prev_row->previous ;
    get_current() ;
}

/***********************************************************
Get_current() : moves the row pointed to by prev_row->next
to the row_buf.
***********************************************************/

get_current()
{
    char *get_row() ;
    
    tot_len  = prev_row->next->length ;
    cpystr(  get_row( prev_row->next ) ,
             tot_len ,
             row_buf ) ;
    scr_len  = abs_col(row_buf,tot_len);
}    
