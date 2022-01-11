#include <stdio.h>
#include "define.h"

extern char buf[]	;
extern int  b_		;
extern int  b_row       ;
extern int  col		;
extern int  row		;
extern FLAG vt100	;
extern FLAG ctrl_Q_flag ;
extern FLAG no_ctrl_Q_S ;
extern FLAG in_macro	;
extern int  trows	;
extern int  tcols	;
extern int  row_len 	;
extern int  wdw_size	;
extern int  language	;
extern char f_name[]	;
extern int  mark_b_row  ;
extern int  mark_b_inx	;
extern int  b_row	;
extern int  u_arg	;
extern char m_arr[][MACRLEN];
extern char c_arr[]	;
extern char macr[]	;
extern int  m_		;

/***********************************************************
Prints prints a strings as printf.

Special:
- Any char following a % char that is not s will be
written as it is.
- The first argument is a format string, the second is 
the string to be written and the third tells how many
posisions the string will occupy when written (left
justified).
***********************************************************/

prints(format, st, n)
char *format, *st ;
int n;
{
    register char c ;
    short register int b ;


    for(b = 0 ; (c = *format++) ; b++) {
	if( c != '%' ) { buf[b_++] = c ; continue ; }

	/* Handle format				*/
	c = *format++ ;
	if( c == 's' ) {
	    if( st != NULL ) {
		for( ; *st != '\0' ; b++)
		    if ( printable(*st) )
			buf[b_++]=(*st++);
		    else {
			buf[b_++]='\\';
			buf[b_++]=(char) ( ( (*st & 192)/64 )+'0' );
			buf[b_++]=(char) ( ( (*st &  56)/8  )+'0' );
			buf[b_++]=(char) (   (*st++ & 7)     +'0' );
			b+=3;
			}
	 	}
	    }
	else { buf[b_++] = c ; b++; }
	}
    for(; b <= n + 1 ; b++) buf[b_++] = ' ' ;
}

/***********************************************************
Int_to_str converts an integer to a null-terminated string
***********************************************************/

char *int_to_str( n )
int n;
{
    static char buf[11];

    sprintf( buf, "%d", n );
    return ( buf );
}

/***********************************************************
Read_char reads one character from macro or from input
***********************************************************/

read_char()
{
    if ( in_macro )
	if ( macr[m_]==END_M ) {
	    pr_msg( MACR_E, NULL );
	    return (CTRL_G);
	    }
	else
	    return ( (ctrl_Q_flag OR macr[m_]!=CTRL_M) ? macr[m_++] :
							 CTRL_J );
    else
	return (get_char());
}

/***********************************************************
Get_char reads one character from the input
***********************************************************/

get_char()
{
    char c;

    dump();
    if ( ctrl_Q_flag ) sterm(NO_CRNL);	/* Don't map CR to NL */
    c = getchar();
    while ( no_ctrl_Q_S && ( c==CTRL_Q || c==CTRL_S ) ) c=getchar();
    if ( ctrl_Q_flag ) sterm(CRNL);
    return ( ctrl_Q_flag ? c : c_arr[c] );
}

/***********************************************************
test_if_macr compares mc (a chars) with m_arr, if macro
replacement exists, copy it to macr.
***********************************************************/

int test_if_macr(mc, a)
char *mc ;
int a ;
{
    short register int i , b , p ;

    for(i = 0 ; (i < MACRNUM) && ( m_arr[i][0] != END_M ) ; i++) {
	for(b = 0, p = TRUE ; b < a ; b++ ) 
	    if( *(mc + b) == m_arr[i][b] ) {
		if( (m_arr[i][b + 1] == END_M) && ( p ) ) {
		    copy_macro(i, b + 2 , *(mc + b + 1) );
		    return TRUE ;
		    }
		} else p = FALSE ;
	if( p ) return MAYBEE ;
	}
    return FALSE ;
}

/***********************************************************
copy_macro() copys the macro m starting at index i into macr.
ch is added to macr if an ambigous case occured ( namely 
ESC O matched ESC Oq  (no ESC Oq definition before ESC O 
definition in .siv)).
***********************************************************/

copy_macro(m, i , ch)
short register int m , i ;
char ch ;
{
    short register int a ;
    register char c ;

    for(a = 0 ; i + a < MACRLEN ; ) {
	c = m_arr[m][i + a] ;
	if( c == END_M) break ;
	macr[a++] = c ;
	}
    macr[a++] = ch ; /* may be an END_M char		*/
    macr[a  ] = END_M ;
}

/***********************************************************
dump() prints the screen buffer.
***********************************************************/

dump()
{
   write(1, buf, b_); b_ = 0 ;
}

/***********************************************************
print_ewd prints the end window line.
***********************************************************/

print_ewd()
{
    int tmp ;

    if( (tmp = b_) > ( SCREENBUF - 200 ) ) dump();

    cursor(wdw_size, 0);
    standout();
    outstr("Siv V3.0   ");
    if( language == SWEDISH ) outstr("Fil ") ;  else outstr("File");
    prints(": %s", f_name, MAXLEN-1);
    pr_b_row();

    if( b_ - tmp > row_len ) b_ = tmp + row_len ;
    standend();
} 

/***********************************************************
pr_b_row prints the b_row (line number) variable
***********************************************************/

pr_b_row()
{
    char *int_to_str();

    if( tcols < 78 ) return ;
    cursor(wdw_size, 65);
    standout();
    if( language == SWEDISH ) outstr("Rad  ") ;  else outstr("Line ");
    prints( "%s", int_to_str( b_row ), 9 );
    standend();
}
