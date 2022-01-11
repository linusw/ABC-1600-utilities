#include             <stdio.h>
#include             "define.h"

extern int           trows     ;
extern int           tcols     ;
extern int           row       ;
extern int           col       ;
extern char	     *row_buf  ;
extern int	     row_len   ;
extern int           tot_len   ;
extern int	     scr_len   ;
extern struct row_p  *prev_row ;
extern int           language  ;
extern int           wdw_size  ;
extern char	     *space    ;
extern FLAG	     vt100     ;
extern char	     hom_dir[] ;

char    *clr;
FLAG    f_clr;
char    *poscur;
char    *clrend;
FLAG    f_clrend;
char	*so_str;
char	*se_str;
FLAG	f_standout;

prepterm()
{
    register int err ;
    char          termbp[1024];
    char	  area[128];
    char          *tmp ;
    char          *getenv() ;
    char          *tgetstr() ;
    char          *tgoto() ;
    char          *skipdig() ;

    /* Get home directory	                          */
    if ((tmp = getenv("HOME")) == NULL)
	hom_dir[0] = '\0' ;
    else
	strcpy(hom_dir, tmp);

    /* Check which language                               */
    if ((tmp = getenv("LANGUAGE")) != NULL && strcmp("swedish", tmp) == 0)
        language = SWEDISH;
    else
        language = ENGLISH;

    /* Check which terminal-type                          */
    if ((tmp = getenv("TERM")) == NULL)
        return(NOTERM);
    if ((err = tgetent(termbp, tmp)) != 1)
         return(err ? NOFILE : NOENTRY);

    /* Special case for vt100 compatible terminals        */
    if (strcmp(tmp, "vt100"   ) == 0 ||
	strcmp(tmp, "vt100w"  ) == 0 ||
	strcmp(tmp, "twi72"   ) == 0 ||
	strcmp(tmp, "twist"   ) == 0 ||
        strcmp(tmp, "abc1600p") == 0 ||
        strcmp(tmp, "abc1600l") == 0 ||
        strcmp(tmp, "abc1600w") == 0 ||
        strcmp(tmp, "win"     ) == 0 ||
	strcmp(tmp, "pkwin"   ) == 0 ||
        strcmp(tmp, "dt80"    ) == 0)
        vt100 = TRUE ;
    else
        vt100 = FALSE ;

    if ((tcols = tgetnum("co")) == -1) tcols = 80;
    if ((trows = tgetnum("li")) == -1) trows = 24;
    if (NOT vt100) tcols--;
    row_len = tcols - 1 ;

    tmp = area;
    /* Get clear screen sequence */
    if (f_clr = ((clr = tgetstr("cl", &tmp)) != NULL))
        clr = skipdig(clr);

    /* Get position cursor sequence */
    if ((poscur = tgetstr("cm", &tmp)) != NULL )
        poscur = skipdig( poscur ) ;
    else
        return(NOTSUPPORTED) ;

    /* Get clear screen to end sequence */
    if (f_clrend = (clrend = tgetstr("cd", &tmp)) != NULL)
        clrend = skipdig(clrend);

    /* Get standout (inverted video) sequence */
    if (f_standout = ((so_str = tgetstr("so", &tmp)) != NULL &&
		      (se_str = tgetstr("se", &tmp)) != NULL)) {
	so_str = skipdig(so_str);
	se_str = skipdig(se_str);
	}

    return(OK);
}

char * skipdig( p )
register char * p ;
{
    while (*p >= '0' && *p <= '9')
        ++p ;
    return(p);
}

/***********************************************************
Clrscr() : Clears the screen and "homes" the cursor.
***********************************************************/

clrscr()
{
    register i;

    if( f_clr )
	outstr(clr);
    else {
        cursor(trows - 1, row_len);
        for( i = trows ; i ; --i )
            pr_char( '\n' );
	}
    cursor(0, 0);
}

/***********************************************************
Clearrest( r, c ) : Clears the screen after the cursor.
***********************************************************/
clearrest( r, c )
int r, c ;
{
    register int i;

    cursor( r, c );
    if( f_clrend )
	outstr(clrend);
    else {
        /* the last col on the last row is not cleared, to avoid scroll */
        for( i = row_len - c + (trows - r - 1) * tcols - 1 ; i ; --i )
            pr_char( ' ' );
	}
    cursor( r, c );
}

/***********************************************************
Cur() : Positions the cursor somewhere on the screen i.e.
0--(trows-1) and 0--(row_len) with special compensation to
give cursor correct position when editing.
***********************************************************/

cur(r, c)
register int r , c ;
{
    char *tgoto();

    r += c / row_len ;
    if ((c == scr_len) && (scr_len > 0) && (scr_len % row_len) == 0) {
	c = row_len;
	r--;
	}
    else
	c %= row_len ;
    outstr(tgoto(poscur, c, r));
}

/***********************************************************
Cursor() : Positions the cursor somewhere on the screen i.e.
0--(trows-1) and 0--(row_len) .
***********************************************************/

cursor(r, c)
register int r , c ;
{
    char *tgoto();

    outstr(tgoto(poscur, c , r));
}

/***********************************************************
Standout() sends the sequence for standout text (inverse
video et al)
***********************************************************/

standout()
{
    if (f_standout)
	outstr(so_str);
}

/***********************************************************
Standend() Ends standout mode
***********************************************************/

standend()
{
    if (f_standout)
	outstr(se_str);
}
