/***********************************************************
This file contains the META- or ESCAPE-commands, i.e. 
commands created by pressing the esc-key and thereafter
another key. It also contains the search-commands, i.e.
CTRL_S and _R which are related to the query replace.
***********************************************************/

#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           col       ;
extern int           row       ;
extern int           tcols     ;
extern int           wdw_size  ;
extern int           trows     ;
extern int           tot_len   ;
extern int 	     scr_len   ;
extern int	     buf_inx   ;
extern int	     row_len   ;
extern char          *row_buf  ;
extern char          f_name[]  ;
extern char          search[]  ;
extern char          replce[]  ; 
extern char	     *space    ;
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;
extern int	     b_row     ;
extern char	     stop_c[]  ;
extern FLAG	     ctrl_Q_flag;

/***********************************************************
stop_char returns TRUE if c is a word stopchar 
***********************************************************/

stop_char(c)
register char c ;
{
    short register int a ;
    register char h ;

    if( c < 32 ) return TRUE;
    for(a = 0 ; (h = stop_c[a++]) ; )
	if( c == h ) return TRUE ;
    return FALSE ;
}

/***********************************************************
esc_F() moves cursor one word forward.
***********************************************************/

esc_F()
{
    int old_b_row;

    while (stop_char( *(row_buf + buf_inx) ) )
	if ( forward(1) ) break;
    old_b_row=b_row;
    while ( (!stop_char( *(row_buf + buf_inx) )) && old_b_row==b_row )
	if ( forward(1) ) break;
    test_redraw();
}

/***********************************************************
esc_B() moves cursor one word forward.
***********************************************************/

esc_B()
{
    int old_b_row;

    if ( back(1) ) goto tof ;
    while (stop_char( *(row_buf + buf_inx) ) )
	if ( back(1) ) goto tof;
    old_b_row=b_row;
    while ( (!stop_char( *(row_buf + buf_inx) )) && old_b_row==b_row )
	if ( back(1) ) goto tof;
    forward(1);
tof:
    test_redraw();
}

/***********************************************************
esc_D() deletes the word after the cursor
***********************************************************/

esc_D()
{
    int tmp_len, tmp_n_rows;

    tmp_len = scr_len ;
    tmp_n_rows = buf_rows();

    while (stop_char( *(row_buf + buf_inx) ) )
	   if( !ctrl_D_action() ) goto done ;
    while (!stop_char( *(row_buf + buf_inx) ) )
	   if( !ctrl_D_action() ) break ;

done:
    cur(row, 0);
    pr_row(row_buf, 0, tot_len, 0);

    if (scr_len < tmp_len) pr_row(space, 0, tmp_len - scr_len, 0);
    if (tmp_n_rows != buf_rows()) pr_down() ;
}

/***********************************************************
Top_file() : moves the cursor to the first row in the 
buffer.
***********************************************************/

top_file()
{
    FLAG f_ctrl_L ;
    int old_row ;
    register struct row_p *rp ;
    short register int n ;

    old_row = row ; row = 0 ; n = b_row ; f_ctrl_L = TRUE;

    if (b_row < wdw_size) {
        for(rp = &first_r ; --n ; ) {
            rp = rp->next ;
            row += scr_rows(rp);
            }
	f_ctrl_L = (old_row != row);
	}

    col = row = buf_inx = 0 ; b_row = 1 ;

    /* Check if already at top row                        */
    if( prev_row->previous == NULL )
	return( TRUE ) ;
    
    /* Save the current row                               */
    insert( row_buf, prev_row, tot_len) ;

    /* Set the first row to current                       */
    prev_row = &first_r ;
    get_current();

    /* Rewrite the screen                                 */
    if( f_ctrl_L ) ctrl_L() ;

    return FALSE ;
}

/***********************************************************
End_file() : moves the cursor to the last row of the file.
***********************************************************/

end_file()
{
    /* Already at last row ???                            */
    if( last_row() ) {
        col = scr_len ;
	buf_inx = tot_len ;
        return ;
    }
    
    /* Save current row                                   */
    insert( row_buf, prev_row, tot_len) ;
    prev_row = prev_row->next ;

    /* Find the last row                                  */
    while( !last_row() ) {
	b_row++;
	if( row < wdw_size ) row += scr_rows(prev_row) ;
        prev_row = prev_row->next ;
        }
    get_prev();
    
    /* The last row was on a later window ???             */
    col = scr_len ;
    buf_inx = tot_len ;
    test_redraw();
}

/***********************************************************
Bck_wdw() : moves the cursor to the previous window unless
it was at the top of the file.
***********************************************************/

bck_wdw()
{
    up_scr_rows( row );
    ctrl_L();

    /* adj_col adjusts buf_inx and col automatically 	  */
    adj_col();
}

/***********************************************************
Input() : inputs a string to search for or a filename. If it
is a filename, file_legal() is called, which diables spaces.
***********************************************************/

input( chr, str, legal, old_str, column )
register char chr, *str, *old_str;
FLAG (*legal) () ;
int column;
{
    register int i, j, len, n, k=column;

    *str = '\0';
    i = len = 0 ;
    while( chr != '\n' ) {
	switch(chr) {
	    case CTRL_G:
		return ABORTED;
	    case DEL:
	    case CTRL_H:
	    case CTRL_B:
back:		if (i>0) {
		    i--; k--;
		    if (NOT printable(str[i]))
			k -= 3;
		    if (chr != CTRL_B && chr != 'D') {
del:			for (j=i; j<len; j++)
			    str[j] = str[j+1];
			len--;
			goto rewrite;
			}
		    cursor(trows - 1, k);
		    }
		break;
	    case CTRL_F:
forward:	if (i<len) {
		    k++;
		    if (NOT printable(str[i]))
			k += 3;
		    i++;
		    cursor(trows - 1, k);
		    }
		break;
	    case CTRL_D:
		if (i<len)
		    goto del;
		break;
	    case CTRL_A:
		i = 0;
		k = column;
		cursor(trows - 1, k);
		break;
	    case CTRL_E:
		i = len;
		for (j=0, k=column; j<len; j++) {
		    k++;
		    if (NOT printable(str[j]))
			k += 3;
		    }
		cursor(trows - 1, k);
		break;
	    case CTRL_K:
		len = i;
		str[len] = '\0';
	    case CTRL_L:
rewrite:	cursor(trows - 1, column);
		for (j=column; j<row_len; j++)
		    pr_char(' ');
		cursor(trows - 1, column);
		for (j=0; j<len; j++)
		    pr_all(str[j]);
		cursor(trows - 1, k);
		break;
	    case CTRL_Y:
		n = strlen(old_str);
		if (len+n < MAXLEN) {
		    for (j=len; j>=i; j--)
			str[j+n] = str[j];
		    for (j=0; j<n; j++)
			str[i+j] = old_str[j];
		    len += n;
		    }
		goto rewrite;
	    case ESC:
		if (read_char() == 'O')
		    if ((chr=read_char()) == 'C')
			goto forward;
		    else if (chr == 'D')
			goto back;
		break;
	    case CTRL_Q:
		ctrl_Q_flag = TRUE;
		chr = read_char();
	    default:
		if (printable(chr) || chr==TAB || ctrl_Q_flag) 
		    if ( (*legal)(chr) && len+1<MAXLEN ) {
			for (j=len; j>=i; j--)
			    str[j+1] = str[j];
                	str[i] = chr ;
                	i++; len++; k++;
			if (NOT printable(chr))
			    k += 3;
			ctrl_Q_flag = FALSE;
			if (i == len)
			    pr_all(chr);
			else
			    goto rewrite;
			}
		ctrl_Q_flag = FALSE;
	    }
	chr=read_char();
	}
    return( CONT ) ;
}

pr_all(c)
char c;
{
    if (printable(c) || c == TAB)
	pr_char(c);
    else {
	pr_char('\\');
	pr_char( ( (c & 192)/64 ) + '0');
	pr_char( ( (c &  56)/8  ) + '0');
	pr_char(   (c &   7)      + '0');
	}
}

/***********************************************************
File_legal() : checks that chr is a legal character in a
filename. I.e.  ascii 32 < chr < ascii 127.
***********************************************************/

file_legal( chr )
char chr ;
{
    return (chr == ' ') ? FALSE : printable( chr );
}

/***********************************************************
Str_legal() : checks that chr is a legal character to search
for. I.e. chr > '\0'
***********************************************************/

str_legal( chr ) 
char chr ;
{
    return (chr > '\0');
}

/***********************************************************
Ctrl_S() : does a forward search, i.e. tries to find the
the string stored in search[].
***********************************************************/

ctrl_S()
{
    FLAG str_legal() ;
    char str[MAXLEN], chr ;

    /* Input the string to search for                     */
    pr_msg( SEARCH, NULL ) ;

    /* Two consecutive ctrl-s's or a Return searches the
       same string                                        */
    chr = read_char() ;
    if( ( chr != CTRL_S ) && ( chr != CTRL_J ) ) {
          if(input(chr, str, str_legal, search, 12) == ABORTED) {
              pr_msg( ABORTD, NULL ) ;
              return ;
          } 
          strcpy( search, str ) ;
    }
    cursor(trows - 1, 0); prints("%s", search, row_len - 1); dump();

    if( FAILED f_search( buf_inx + 1 ) ) {

        /* String was not found                               */
        pr_msg( NO_FND, NULL ) ;
	} 
    else {
	pr_msg(0, NULL);
	test_redraw();
	}
}

/***********************************************************
F_search() : searches in the textbuffer for a sequence of
characters equal to the one in the string search[]. If an
occurrence is found, that position becomes the current
position.
***********************************************************/

f_search(n)
short register int n ;
{
    register struct row_p *rp ;
    int tmp_row , tmp_b_row ;
    FLAG comp;
    
    insert( row_buf, prev_row, tot_len) ;
    rp		= prev_row ;
    tmp_row 	= row ;
    tmp_b_row	= b_row ;
    prev_row	= prev_row->next ;

    /* Search until found or no more rows to search       */
    while( prev_row != NULL ) {
        if( n > tot_len ) {

            /* Try in next row                            */
	    if( row < wdw_size ) row += scr_rows(prev_row) ;
            prev_row = prev_row->next ;
            if( prev_row == NULL ) break ;  /* NOT FOUND! */
	    b_row++;
            tot_len = prev_row->length ;
            n = 0 ;
	    }
        else {
            if ( (comp=compare( prev_row, tot_len, n )) == EQUAL ) {
                /* FOUND!!!                               */
                /* Reprint window if necessary            */
		get_prev();
		buf_inx = n ;
                col = abs_col(row_buf, n) ;
                return( TRUE ) ;
		}

            /* Step one character                         */
            else if ( comp == NOTEQ )
		n++ ;
	    else
		break;
	    }
	}

    /* String was not found                               */
    prev_row	= rp ;
    row		= tmp_row ;
    b_row 	= tmp_b_row ;
    get_current() ;
    return( FALSE ) ;
}

/***********************************************************
Compare() : compares 'search[] ' with the sequence of
characters starting at rp->row + n . Len is used to see if 
the string continues in the next row.
***********************************************************/

compare( rp, len, n )
register struct row_p *rp ;
short register int len, n ;
{
    short register int i ;
    register char c ;

    i = 0 ;
    while( TRUE ) {
        if ( (c = search[i++]) == '\0' ) return( EQUAL ) ;
        if ( n >= len )
	    if ( c==CTRL_J ) {
		rp = rp->next;
		if (rp == NULL) return (EOF);
                len = rp->length ;
	        n=0;
	        }
	    else
		return ( NOTEQ );
	else if ( c != *(rp->row + n++) )
	    return( NOTEQ ) ;
	}
}

/***********************************************************
Ctrl_R() : does a reverse search and is very similar to 
ctrl_S.
***********************************************************/

ctrl_R()
{
    int n, tmp_row , tmp_b_row ;
    FLAG str_legal() ;
    char str[MAXLEN], chr ;
    struct row_p *rp ;

    /* Input the string to search for                     */
    pr_msg( SEARCH, NULL ) ;

    /* Two consecutive ctrl-r's or a Return  searches the
       same string                                        */
    /* as last time                                       */
    chr = read_char();
    if( ( chr != CTRL_R ) && ( chr != CTRL_J ) ) {
        if(input(chr, str, str_legal, search, 12) == ABORTED ) {
            pr_msg( ABORTD, NULL ) ;
            return ;
        }
        strcpy( search, str ) ;
    }
    cursor(trows - 1, 0); prints("%s", search, row_len - 1); dump();

    /* Copy current row to textbuffer and save some vars  */
    insert( row_buf, prev_row, tot_len) ;
    rp		= prev_row ;
    tmp_row 	= row ;
    tmp_b_row	= b_row ;
    prev_row	= prev_row->next ;
    n		= buf_inx - 1 ;

    /* Search until found or no more rows to search       */
    while( prev_row->previous != NULL ) {
        if( n < 0 ) {

            /* Try in previous row                            */
            prev_row = prev_row->previous ;
            tot_len = prev_row->length ;
	    b_row--;
	    if( row >= 0 ) row -= scr_rows(prev_row) ;
            n = tot_len ;
        }
        else {
            if( compare( prev_row, tot_len, n ) == EQUAL ) {

                /* FOUND!!!                               */
                tot_len  = prev_row->length ;
                prev_row = prev_row->previous ;

                /* Reprint window if necessary            */
		get_current();
		test_redraw();
		buf_inx = n ;
		col = abs_col(row_buf, n) ;
	        pr_msg(0, NULL);
                return ;
            }

            /* Step one character                         */
            else n-- ;
        }
    }

    /* String was not found                               */
    pr_msg( NO_FND, NULL ) ;
    prev_row	= rp ;
    row 	= tmp_row ;
    b_row	= tmp_b_row ;
    get_current();
}

/***********************************************************
Q_rplce() : searches for the occurrence of a string. When it
has been found, the user is asked if he wants to replace it.
Hitting the space-bar replaces the string, hitting  'N' does
not change the string but searches for the  next occurrence.
Finally Ctrl-G aborts the query replace.
***********************************************************/


q_rplce()
{
    static char buf[MAXLEN] = '\0' ;
    char *int_to_str() , chr, choice ;
    struct row_p *tmp_rp ;
    FLAG ctrl_L_flag; /* A ctrl_L() has to be done when finished */
    FLAG f_ctrl_L; /* A ctrl_L() has to be done after this replacement */
    FLAG replace_rest, splt, f_pr_down;
    int  tmp_col , tmp_row , last_row , tmp_b_row , tmp_buf_inx , dif ;
    int  tmp_n_rows, old_tot_len;
    short register int n , i ;

    ctrl_L_flag = replace_rest = FALSE;
    pr_msg( OLD_ST, NULL ) ;
    chr = read_char();
    if( chr != '\n' )
        if( (input( chr, buf, str_legal, search, 15)) == ABORTED) {
            pr_msg( ABORTD, NULL ) ;
            return ;
            }
    if( strlen(buf) != 0 ) strcpy( search, buf ) ;
    else {
	prints("%s", search, 0 ) ; dump();
	sleep(2);
        }
    pr_msg( NEW_ST, NULL ) ;
    chr = read_char() ;
    if( chr != '\n' ) {
        if ((input(chr, buf, str_legal, search, 12)) == ABORTED) {
            pr_msg( ABORTD, NULL ) ;
            return ;
            }
        strcpy( replce, buf ) ;
        }
    else *replce = '\0' ;

    tmp_rp	= prev_row ;
    tmp_col	= col ;
    tmp_row 	= row ;
    tmp_b_row	= b_row ;
    tmp_buf_inx = buf_inx ;
    last_row	= 0 ;
    i = 0 ;
    dif = strlen(search) - strlen(replce) ;
    pr_msg( Q_HELP, NULL ) ;
    while( TRUE ) {

        /* Any more occurrences??? */
        if( FAILED f_search(buf_inx) ) break ;
	if( row > last_row ) last_row = row ;
	pr_b_row();
	if (NOT replace_rest)
	    test_redraw();

again:  if (NOT replace_rest)
	    cur( row, col ) ;
	choice = replace_rest ? ' ' : read_char();
        switch( choice ) {
	    case '!' :
		ctrl_L_flag = replace_rest = TRUE;
            case ' ' :
                i++ ;
		f_pr_down = f_ctrl_L = splt = FALSE;
                tmp_n_rows = buf_rows();
		for(n = 0 ; search[n++]; ) 
		    if (NOT ctrl_D_action() ) {
			old_tot_len = tot_len;
			splt = append_to_row_buf(prev_row->next);
			if (splt)  {
			    pr_msg( L_SPLT, NULL );
			    sleep(2);
			    i--;
			    goto exit;
			    }
			del_adjust_mark(splt, old_tot_len);
			f_pr_down = TRUE;
			}
		for(n = 0; chr=replce[n++]; )
		    if ( chr!=CTRL_J )
			add_char_action( chr );
		    else {
			if (col == scr_len && NOT replace_rest) {
			    cur( row, 0 );
			    pr_row( row_buf, 0, tot_len, 0 );
			    pr_row( space, 0, row_len - scr_len % row_len,
				     0 );
			    }
			f_ctrl_L = new_l_action(NOT replace_rest);
			f_pr_down = TRUE;
			}
		if ( row+buf_rows() > wdw_size )
		    f_pr_down = TRUE;
		if (NOT replace_rest) {
		    cur( row, 0 );
                    if ( tmp_n_rows!=buf_rows() || f_pr_down ) {
			if ( f_ctrl_L )
			    ctrl_L();
			else
			    pr_down();
			ctrl_L_flag = TRUE;
		 	}
		    else {
			pr_row(row_buf, 0, tot_len, 0);
			pr_row( space, 0, row_len - scr_len % row_len, 0 );
			}
		    }
		dump();
		break ;

            case CTRL_G :
		goto exit ;

	    case CTRL_H :
            case 'n'    :
            case 'N'    :
                buf_inx++;
                break ;

            default     :
                pr_msg( Q_HELP, NULL ) ;
                goto again ;
	    }
	}
exit:
    insert( row_buf, prev_row, tot_len) ;
    if( last_row >= wdw_size ) ctrl_L_flag = TRUE;
    prev_row = tmp_rp  ;
    col      = tmp_col ;
    row	     = tmp_row ;
    b_row    = tmp_b_row ;
    buf_inx  = tmp_buf_inx ;
    get_current();
    if( ctrl_L_flag ) ctrl_L();
    pr_msg( REPLCE, int_to_str( i ) ) ;
}
