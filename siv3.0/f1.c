/***********************************************************
Misc. functions...
***********************************************************/

#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           tcols ;
extern int           trows ;
extern int           row ;
extern int           col ;
extern int	     row_len  ;
extern int           language ;
extern char          *end_wdw ;
extern struct row_p  *prev_row ;
extern char          *row_buf ;
extern int           tot_len ;
extern char          *file_name ;
extern char          *search ;
extern char          *space ;
extern char          *replace ;
extern char	     buf[]	;
extern int	     b_		;

/***********************************************************
Cpystr() copies a string at "from" of length "l" to "to".
***********************************************************/

cpystr( from, l, to ) char *from, *to ; int l ;
{
    while( l-- ) *to++ = *from++ ;
}

/***********************************************************
Last_row() : checks if the current row is the last row.
I.e. if prev_row->next == NULL.
***********************************************************/

FLAG last_row()
{
    return(prev_row->next == NULL) ;
}

/***********************************************************
Firstrow() : checks if the current row is the first one.
***********************************************************/

FLAG firstrow()
{
    return(prev_row->previous == NULL) ;
}

printable( c ) 
int c ;
{
    return( (c >= 32 && c <= 126) ? TRUE : FALSE ) ;
}

static char *messages[] = {
    " " ,
    " " ,
    "Top of file" ,
    "B|rjan av filen" ,
    "End of file" ,
    "Slut p} filen" ,
    "Not implemented" ,
    "Ej implementerad" ,
    "Are you sure you want to quit (y/n) ? " ,
    "Vill du verkligen sluta (j/n) ? " ,
    "Space = replace, N or BS = next, CTRL_G = quit, ! = replace rest" ,
    "Mellanslag = byt, N eller BS = n{sta, CTRL_G = bryt, ! = byt resten" ,
    "Replaced %s occurrences" ,
    "Bytte %s f|rekomster" ,
    "Search for: " ,
    "S|k  efter: " ,
    "Aborted" ,
    "Avbr|t" ,
    "Can't find it" ,
    "Hittar den ej" ,
    "Old   string : " ,
    "Gammal str{ng: " ,
    "New string: " ,
    "Ny  str{ng: " ,
    "Read file: " ,
    "L{s  fil : " ,
    "Insert file: " ,
    "S{tt in fil: " ,
    "Can't save %s" ,
    "Kan inte spara %s" ,
    "Wrote %s" ,
    "Skrev %s" ,
    "Nonexistent file: %s" ,
    "%s existerar ej"  ,
    "Can't insert here" ,
    "Kan inte s{tta in h{r" ,
    "New file: %s" ,
    "Ny fil: %s" ,
    "Can't edit directories" ,
    "Kan inte editera directories" ,
    "Read permission denied" ,
    "Inga l{sr{ttigheter" ,
    "Can't create %s" ,
    "Kan inte skapa %s" ,
    "Write permission denied" ,
    "Skrivr{ttigheter saknas" ,
    "Can't backup %s" ,
    "Kan inte skapa backup-kopia av %s" ,
    "Argument's too large" ,
    "F|r stort argument" ,
    "Argument: " ,
    "Argument: " ,
    "Write file: " ,
    "Skriv  fil: " ,
    "No mark set" ,
    "M{rke ej satt" ,
    "Mark set" ,
    "M{rkt" ,
    "The file does not end with a line feed - should I add one (y/n) ? " ,
    "Filen slutar ej med radskifte - skall jag l{gga till ett (j/n) ? " ,
    "Write text file: " ,
    "Skriv   textfil: " ,
    "To long line - split!" ,
    "F|r l}ng rad - delad!" ,
    "%s profile loaded" ,
    "%s profile inladdad" ,
    "Max line length reached - can't add the character!" ,
    "Max rad l{ngd n}dd - kan inte l{gga till tecknet!" ,
    "Change to directory: " ,
    "Byta till bibliotek: " ,
    "Can't change to directory %s" ,
    "Kan ej byta till biblioteket %s" ,
    "New current working directory: %s",
    "Nytt arbetsbibliotek: %s",
    "Can't execute a shell" ,
    "Kan ej starta en shell",
    "Faulty macro",
    "Felaktigt makro",
    "This is the text editor siv. To quit siv type CTRL_C.",
    "Detta {r texteditorn siv. Tryck CTRL_C f|r att sluta."
} ;

/***********************************************************
Pr_msg prints a message on the message row
***********************************************************/
pr_msg( n, str )
int n ;
char *str ;
{
    char *s ;

    cursor(trows - 1, 0);
    s = messages[ 2 * n + language ] ;
    prints( s, str, row_len - 1 ) ;
    n = strlen(s) ; 
    if( str != NULL ) n += strlen(str);
    cursor(trows - 1, n);
}
