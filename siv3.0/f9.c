/***********************************************************
This file contains the CTRL_X commands, i.e. the load-/save-
from file routines.
***********************************************************/

#include             "define.h"
#include             "struct.h"
#include             <stdio.h>
#include             <errno.h>
#include             <sys/types.h>
#include             <sys/stat.h>
#include	     <sys/dir.h>
#include	     <fcntl.h>

extern int           wdw_size  ;
extern int           tot_len   ;
extern int 	     buf_inx   ;
extern int	     row_len   ;
extern int           tcols     ;
extern int           trows     ;
extern int           col       ;
extern int           row       ;
extern int           errno     ;
extern int	     language  ;
extern FLAG	     restricted;
extern FLAG	     siv_file  ;
extern FLAG	     no_ctrl_Q_S;
extern char          *row_buf  ;
extern char          f_name[]  ;
extern char	     cwd[]     ;
extern char	     buf[]     ;
extern char	     hom_dir[] ;
extern int	     b_	       ;
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;
extern int	     b_row     ;
extern int	     mark_b_row;
extern int	     mark_b_inx;
extern char	     m_arr[][MACRLEN];
extern char	     c_arr[]   ;
extern char	     stop_c[]  ;

/***********************************************************
Newfile() : read the name of a file to be edit.
***********************************************************/

newfile()
{
    char buf[MAXLEN], chr ;
    FLAG file_legal() ;

    pr_msg(READ_F, NULL);
    while ((chr = read_char()) == ' ');
    if (chr == CTRL_J) {
	newread();
	return ;
	}
    if (input(chr, buf, file_legal, f_name, 11) == ABORTED) return ;
    if (strlen(buf) > 0)
	strcpy(f_name, buf);
    newread();
}

/***********************************************************
Newread() : loads the editor with a new file. Anything
previously in the textbuffer is scratched.
***********************************************************/

newread()
{
    FILE *fopen(), *fp ;
    FLAG splt ;

    /* Scratch old contents in textbuffers                */
    scratch( &first_r ) ;
    prev_row = &first_r ;
    mark_b_row = NOTSET ;
    col = buf_inx = row = 0 ;
    b_row = 1 ;

    if (NOT readable(f_name)) return ;

    /* open file */
    if( ( fp = fopen( f_name, "r" ) ) == NULL ) {
        /* File does not exist. Buffer is empty. */
	tot_len = 0 ;
        ctrl_L() ;
        return ;
    }

    /* File exists. Load into the empty buffer            */
    splt = readfile( fp ) ;
    fclose( fp ) ;

    /* Set global variables and move first row to row_buf */
    prev_row = &first_r ;
    if( prev_row->next != NULL )
        /* File was not empty */
	get_current();
    else
	tot_len = 0 ;

    ctrl_L() ;
    if( splt ) pr_msg(L_SPLT, NULL);
}

/***********************************************************
wr_exit() :
***********************************************************/

wr_exit()
{
    if( FAILED save( FALSE ) )
	return ;
    else {
	save_fname();
        pr_msg(0, NULL) ;
        cursor( trows - 1, 0 ) ;
	dump();
        sterm(OLD | XON) ;
        exit(0) ;
	}
}

/***********************************************************
Save() : saves the whole textbuffer in the file f_name
***********************************************************/

save( ask_flag )
FLAG ask_flag;
{
    FILE *fopen(), *fp ;
    char buf[MAXLEN], chr ;
    FLAG file_legal() ;

    pr_msg(0, NULL); dump();

    if ( f_name[0]=='\0' ) ask_flag=TRUE;
    if ( ask_flag ) {
        pr_msg( WRIT_F, NULL ) ;
        while( ( chr = read_char() ) == ' ' ) ;
        if(input(chr, buf, file_legal, f_name, 12) == ABORTED)
	    return ( FALSE );
        strcpy( f_name, buf );
        print_ewd();
	}
    /* Check if file is writeable                         */
    if( NOT writeable( f_name ) ) return( FALSE ) ;

    /* Try to open file                                   */
    if( ( fp = fopen( f_name, "w" ) ) == NULL ) {
        pr_msg( NO_SAV, f_name ) ;
        return( FALSE ) ;
    }

    savefile( fp, FALSE ) ;
    fclose( fp ) ;
    pr_msg( WROT_F, f_name ) ;
    return( TRUE ) ;
}

/***********************************************************
Ins_file() : inserts a file at the current cursor position.
***********************************************************/

ins_file()
{
    struct row_p *tmp_row, *tmp_prev_row ;
    FILE  *fopen(), *fp  ;
    char buf[MAXLEN] ;
    FLAG splt ;

    /* Input filename                             */
    pr_msg( INSERT, NULL ) ;
    if (input(read_char(), buf, file_legal, f_name, 13) == ABORTED)
        return ;

    /* Check if file is readable                  */
    if( NOT readable( buf ) ) return ;
    if( ( fp = fopen( buf, "r" ) ) == NULL ) {
        pr_msg( NO_XST, buf ) ;
        return( FALSE ) ;
	}

    insert(row_buf, prev_row, tot_len);

    /* Insert the file before the current row            */
    tmp_prev_row = prev_row;
    tmp_row = prev_row->next;
    splt = readfile( fp ) ;
    fclose( fp ) ;

/* Adjust the first and last rows of the inserted file:  */
    prev_row = tmp_row->previous;
    get_current();
    /* Cut off the line where the cursor was             */
    insert( row_buf+buf_inx, prev_row, tot_len-buf_inx );
    tot_len = buf_inx;
    /* Append the first line of the inserted file        */
    splt = ( splt || append_to_row_buf( tmp_prev_row->next ) );
    insert( row_buf, tmp_prev_row, tot_len );
    /* Append the last line of the inserted file         */
    get_prev();
    splt = ( splt || append_to_row_buf( prev_row->next ) );
    insert( row_buf, prev_row, tot_len );

/* Get the right row                                     */
    prev_row = tmp_prev_row;
    get_current();
    pr_down();
    if( splt ) pr_msg(L_SPLT, NULL);
    return TRUE ;
}

/***********************************************************
Scratch() : deletes all rows in the textbuffer
***********************************************************/

scratch( rp )
struct row_p *rp ;
{
    while( rp->next != NULL ) {
        get_row( rp->next );
	}
}

/***********************************************************
Readfile() : reads rows from file fp  and inserts them after
the prev_row.
***********************************************************/

readfile( fp )
register FILE *fp ;
{
    short register int chr, i, n , splt ;

    n = 0 ; splt = FALSE ;
    while( ( chr = getc( fp ) ) != EOF ) {
        if( chr != '\n' ) {
	    if( n >= MAXROW-1 ) {
		splt = TRUE ;
		insert(row_buf, prev_row, n);
		prev_row = prev_row->next ;
		if ( mark_b_row > b_row )
		    mark_b_row++;
		n = 0 ;
		}
            *(row_buf + n++ ) = chr ;
	    }
	else {
            /* Insert row                                 */
            insert( row_buf, prev_row, n ) ;
            prev_row = prev_row->next ;
	    if ( mark_b_row > b_row )
		mark_b_row++;
            n = 0 ;
	    }
	}
    insert( row_buf, prev_row, n ) ;
    return splt ;
}

/***********************************************************
Savefile() : saves the whole textbuffer in the file fp.
***********************************************************/

savefile( fp, panic )
register FILE *fp ;
FLAG panic;
{
    register struct row_p *rp ;
    short register int i ;
    char c ;

    rp = &first_r ;

    /* Insert the current row, if it isn't a panic_exit   */
    if ( NOT panic )
	insert( row_buf, prev_row, tot_len ) ;
    while( TRUE ) {
        /* save  one row                                  */
        rp = rp->next ;
        for( i = 0 ; i < rp->length ; i++ ) {
            putc( *(rp->row + i ) , fp ) ;
            }
        /* Check if more rows                         */
        if( rp->next == NULL ) break ;
        putc( '\n', fp ) ;
        }

    if( rp->length > 0 && ( ! panic ) ) {
	pr_msg( ADDELN, NULL ); dump();
	c = upper_case(getchar()); /* don't destroy macro sequence */
	if( (c == 'Y' ) || (c == 'J') ) putc( '\n', fp ) ;
	}

    /* No more rows, fetch the current row                */
    get_current();
}

/***********************************************************
Readable() : checks if the file is readable i.e. :

               * That the file exists.
               * That read permission exists.
               
***********************************************************/

readable( name )
char *name ;
{
    struct stat stbuf ;

    if ( strlen(name)==0 ) 
        /* Don't print any message if null filename is given */
        return( FALSE );
    if( stat( name, &stbuf ) == -1 ) {
        pr_msg( NEW_FL, name ) ;
        return( TRUE ) ;
    }
    if( ( stbuf.st_mode & S_IREAD ) == S_IREAD ) {
        return( TRUE ) ;
    }
    pr_msg( NO_REA, NULL ) ;
    return( FALSE ) ;
}

/***********************************************************
Writeable() : checks if name is writeable, i.e. :

       * That the file exists.
       * That the file has the same user ID as the user.
       * That the file has write permission.

***********************************************************/

writeable( name )
char *name ;
{
    int  wfd ;

    if ( strlen(name)==0 ) {
        pr_msg( NO_CRT, name);
        return( FALSE );
        }
    if( access( name, 2 ) < 0 ) {
        if( errno == ENOENT ) {

            /* Create a new file                             */
            if( ( wfd = creat( name, PMODE ) ) == -1 ) {
                pr_msg( NO_CRT, name ) ;
                return( FALSE ) ;
            }
            close( wfd ) ;
            return( TRUE ) ;
        }
        else {
            pr_msg( NO_WRT, NULL ) ;
            return( FALSE ) ;
        }
    }
    if( FAILED bckup_file( name ) ) {
        pr_msg( NO_BAK, name ) ;
/*      allow exit even if backup failed        */
    }
    return( TRUE ) ;
}

/***********************************************************
Bckup_file() : make cp <name> <name>.BAK
***********************************************************/

bckup_file( name )
char *name ;
{
    FILE *fopen(), *inp, *outp ;
    char chr, *strcat(), buf[MAXLEN];
    int i, fd;
    struct stat stbuf;

    if ( strlen(name)>MAXLEN-5 ) return( FALSE ) ;
    if( is_bak( buf ) ) return( TRUE ) ;
    buf[0]='/';
    strcpy( buf+1, name ) ;
    for ( i=strlen( buf ); buf[i] != '/'; i-- );
    strcpy ( buf, buf+1 );
    if ( ( inp = fopen( buf, "r" ) ) == NULL ) return( FALSE ) ;
    stat( buf, &stbuf );
    if ( strlen( buf+i ) > 10 ) *(buf+i+10)='\0';
    if ( ( outp = fopen( strcat( buf, ".BAK" ), "w" ) ) == NULL )
        return( FALSE ) ;
    while( ( chr = getc( inp ) ) != EOF ) {
        putc( chr, outp ) ;
    }
    fclose( inp ) ;
    fclose( outp ) ;
    chmod( buf, stbuf.st_mode &0777 );
    return( TRUE ) ;
}

is_bak( name )
char *name ;
{
    while( *name ) {
        if( *name == '.' ) {
            if( strcmp( ++name, "BAK" ) == 0 ) {
                return( TRUE ) ;
            }
            else continue ;
        }
        name++ ;
    }
    return( FALSE ) ;
}

/***********************************************************
ctrl_X_T() saves buffer as it appears on screen.
***********************************************************/

ctrl_X_T()
{
    char buf[MAXLEN], chr ;
    FLAG file_legal() ;
    int fd ;

    pr_msg( WR_TXT, NULL ) ;
    while( ( chr = read_char() ) == ' ' ) ;
    if (input(chr, buf, file_legal, f_name, 17) == ABORTED)
	return ;

    pr_msg(0, NULL);

    /* Check if file is writeable                         */
    if( NOT writeable( buf ) ) return( FALSE ) ;

    /* Try to open file                                   */
    if( ( fd = creat( buf, PMODE ) ) == NULL ) {
        pr_msg( NO_SAV, buf ) ;
        return( FALSE ) ;
	}

    text_save( fd );
   
    close( fd ) ;
    pr_msg( WROT_F, buf ) ;
    return( TRUE ) ;
}

/***********************************************************
text_save saves the text buffern into 'fd'.
OBS text_save depends on the fact that pr_row dosn't empty
buf earlier than (worst case) 512 + MAXROW + 1.
***********************************************************/

text_save(fd)
int fd ;
{
    register struct row_p *rp ;
    short register int i ;

    /* make sure nothing left in the screen buffer	*/
    dump();

    rp = &first_r ;

    /* Insert the current row                           */
    insert( row_buf, prev_row, tot_len ) ;

    while( TRUE ) {

        /* save  one row                                */
        rp = rp->next ;
	pr_row(rp->row, 0, rp->length, 0);

        /* Check if more rows                         	*/
        if( rp->next == NULL ) break ;
	pr_char('\n');
	if( b_ > 512 ) { write(fd, buf, b_); b_ = 0 ; }
        }

    write(fd, buf, b_); b_ = 0 ; 

    /* No more rows, fetch the current row              */
    get_current();
}    

/***********************************************************
save_macros saves all current macro definitions in the 
file .siv in the home directory.
***********************************************************/

save_profile()
{
    int  fd ;
    short register int i , r , a , b ;
    register char c ;
    char file[80] ;

    pr_msg(0, NULL); dump();

    strcpy(file, hom_dir);
    strcat(file, "/.siv" );

    /* Check if file is writeable                         */
    if( NOT writeable( file ) ) return( FALSE ) ;

    /* Try to open file                                   */
    if( ( fd = creat( file, PMODE ) ) == -1 ) {
        pr_msg( NO_SAV, file ) ;
        return( FALSE ) ;
	}

    /* save stop_c					  */
    for(i = b = 0 ; i < MAXLEN ; )
	if( (buf[b++] = stop_c[i++]) == '\0' ) break ;
    buf[b++] = '\n' ;
    write(fd, buf, b); b = 0 ;

    /* save macros					*/
    for(i = 0 ; i < MACRNUM ; i++) {
	for(a = r = 0 ; a < MACRLEN ; ) {
	    buf[b++] = (c = m_arr[i][a++] );
	    if( c == END_M ) r++; 
	    if( r > 1 ) break ;
	    }
	buf[b++] = '\n' ;
	write(fd, buf, b); b = 0 ;
	}

    /* save ctrl replacements				*/
    for(i = 0 ; i < CTRLNUM ; i++) {
	buf[b++] = i ;
	buf[b++] = TAB ;
	buf[b++] = c_arr[i] ;
	buf[b++] = '\n' ;
	}

    write(fd, buf, b); b = 0 ;

    close( fd ) ;
    pr_msg( WROT_F, file ) ;
    return( TRUE ) ;
}

/***********************************************************
read_macros: reads macros from a .siv format file.
Return FALSE if error.
***********************************************************/

FLAG read_profile( file )
char *file ;
{
    short register int a, c , i , r , rw ;
    register FILE *fp ;
    char *int_to_str();

    /* open file                                        */
    if( ( fp = fopen( file , "r" ) ) == NULL ) return FALSE ;

    rw = 1 ;

    for(i = 0 ; (c = getc( fp )) != '\n' ; i++) {
	if( c == EOF ) goto exit ;
	if( i < MAXLEN ) stop_c[i] = c ;
	}

    rw++;

    for(i = 0 ; i < MACRNUM ; i++) {
	for(a = r = 0 ; r < 2 ; ) {
	    c = getc( fp );
	    if( c == EOF ) goto exit ;
	    if( a < MACRLEN ) m_arr[i][a++] = c ;
	    if( c == END_M ) r++;
	    if( r > 1 ) if( getc( fp ) != '\n' ) goto exit ;
	    }
	rw++;
	}

    for(i = 0 ; i < CTRLNUM ; i++) {
	if( getc( fp ) == EOF ) goto exit ;
	if( getc( fp ) != TAB ) goto exit ;
	if( (c = getc( fp )) == EOF ) goto exit ;
	c_arr[i] = c ;
	if( getc( fp ) != '\n' ) goto exit ;
	rw++;
	}

    fclose( fp );
    pr_msg(PRO_RD, file);
    return TRUE ;

exit:
    fclose( fp );
    pr_msg(0, NULL);
    if( language == SWEDISH )
	prints("Fel vid l{sning av %s (rad ", file, 0);
    else prints("Error reading %s (line ", file, 0);
    prints( "%s)", int_to_str(rw), 0 );
    return FALSE ;
}

/***********************************************************
change_dir() asks for a directory name, then changes to that
directory.
***********************************************************/

change_dir()
{
    char d_name[MAXLEN], chr ;
    FLAG file_legal() ;

    pr_msg( DIRECT, NULL ) ;
    while( ( chr = read_char() ) == ' ' ) ;
    if (input(chr, d_name, file_legal, cwd, 21) == ABORTED)
	return ;
    if ( chdir( d_name ) == -1)
	pr_msg( NO_CHA, d_name );
    else {
	getcwd( cwd, MAXLEN );
	pr_msg( NEWDIR, cwd );
	}
}

/***********************************************************
getcwd() gets the current working directory. Function is
here because it doesn't exist in the standard c library!
***********************************************************/

char pbuf[MAXLEN], nbuf[MAXLEN];

getcwd( d_name, maxlen )
char *d_name;
int maxlen;
{
    int fd;
    struct direct dirbuf;

    fd=open( ".", O_RDONLY );
    read( fd, (char *) &dirbuf, sizeof( dirbuf ) );
    close( fd );
    *nbuf = '\0';
    strcpy( pbuf, ".." );
    scanpath( dirbuf.d_ino );
    strcpy( d_name, nbuf );
    if ( d_name[0]=='\0' )
	strcpy( d_name, "/" );
    return ( strlen( d_name ) );
}

scanpath( pi )
ino_t pi;
{
    int fd;
    struct direct dirbuf;
    ino_t ti;

    fd=open( pbuf, O_RDONLY );
    read( fd, (char *) &dirbuf, sizeof( dirbuf) );
    if ( ( ti = dirbuf.d_ino ) == pi ) return;
    do {
	read( fd, (char *) &dirbuf, sizeof( dirbuf ) );
	} while ( pi != dirbuf.d_ino );
    close( fd );
    strcat( pbuf, "/.." );
    scanpath( ti );
    strcat( strcat( nbuf, "/" ), dirbuf.d_name );
}

/***********************************************************
subshell() executes a sub-shell.
***********************************************************/

subshell( one_command )
FLAG one_command;
{
    int f_nr, status;

    if ( restricted ) {
	pr_msg( NO_SHE, NULL );
	return;
	}
    if ((f_nr=fork())==0) {
	pr_msg(0, NULL);
	set_sig(OFF);
	cursor( trows-1, 0 );
	if ( one_command )
	    prints( geteuid() ? "$ " : "# ", NULL, 0 );
	dump();
	sterm(OLD | XON);
	if ( one_command )
	    execl("/bin/sh", "sh", "-t", NULL );
	else
	    execl("/bin/sh", "sh", NULL);
	exit( 1 );
	}
    else if ( f_nr == -1 )
	pr_msg( NO_SHE, NULL );
    else {
	wait( &status );
	sterm(ON | (no_ctrl_Q_S ? XON : NO_XON) );
	if ( one_command )
	    getchar();
	ctrl_L();
	return;
	}
}

/***********************************************************
save_fname() saves the default file name in ".siv_file"
***********************************************************/

save_fname()
{
    char file[MAXLEN];
    FILE *fopen(), *fp;

    if ( NOT siv_file ) return;
    strcpy( file, cwd );
    strcat( file, "/.siv_file" );
    if ( ( fp=fopen( file, "w" ) ) != NULL ) {
	fprintf( fp, "%s", f_name );
	fclose( fp);
	}
}
