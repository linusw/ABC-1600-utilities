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
extern int	     buf_inx   ;
extern int	     scr_len   ;
extern char          prev_cmnd ;
extern char          cmnd      ;
extern char          *row_buf  ;
extern char          *space    ;
extern struct row_p  *prev_row ;
extern char	     buf[]     ;
extern int	     b_	       ;
extern int	     b_row     ;
extern int 	     u_arg     ;
extern int	     m_arg     ;
extern int	     mark_b_row;
extern int	     mark_b_inx;
extern char          f_name[]  ;
extern char	     hom_dir[] ;
extern char	     stop_c[]  ;
extern char	     m_arr[][MACRLEN];
extern int	     cmnd_start;
extern char	     c_arr[]   ;
extern char	     macr[]    ;
extern int	     m_	       ;
extern FLAG	     ctrl_Q_flag;
extern FLAG	     no_ctrl_Q_S;
extern FLAG	     in_macro  ;
/***********************************************************
Edit() is the main command loop
***********************************************************/

edit( file )
char *file ;
{
    init() ;
    if( file ) {
        strcpy( f_name, file);
        newread() ;
        }
    while( TRUE ) {
        prev_cmnd = cmnd ;
	pr_b_row();
        cur( row, col ) ;
	cmnd_scan() ;
	}
} 

/***********************************************************
Init() : initializes most global variables             
***********************************************************/

static init()
{
    char *calloc() ;
    int i ;
    
    Buf_init() ;
    prev_cmnd = MARK ;
    row_buf = calloc( MAXROW , 1 ) ;
    space   = calloc( tcols , 1 ) ;
    for( i = 0 ; i < tcols ; i++ ) *(space + i) = ' ' ;
    col = row = buf_inx = scr_len = 0 ;
    wdw_size = trows - 2 ;
    buf[0] = '\0' ; b_ = 0 ;
    b_row = 1 ;
    mark_b_row = NOTSET ;
    mark_b_inx = 0 ;
    u_arg = 0 ;
    m_arg=0;
    macr[0] = END_M ;
    m_ = 0 ;
    cmnd_start=0;
    ctrl_Q_flag=FALSE;
    in_macro=FALSE;

    /* initialize window					*/
    clrscr() ;
    print_ewd();

    get_macrofile();

    if ( no_ctrl_Q_S ) {
	c_arr[CTRL_AA]=CTRL_S;
	c_arr[CTRL_OE]=CTRL_Q;
	}
}

/***********************************************************
get_macrofile() tries to read ".siv". If it can't it reads
"/etc/.siv". If it can't read this one either it creates a
standard profile and tries to save it.
***********************************************************/

get_macrofile()
{
    FLAG f ;
    char file[80];

    strcpy(file, hom_dir);
    strcat(file, "/.siv" );

    f = read_profile( file );
    if( !f ) f = read_profile( "/etc/.siv" );
    if( !f ) standard_profile();
}

/***********************************************************
standard_profile initializes standard stop_c, c_arr, m_arr 
***********************************************************/

standard_profile()
{
    register int i ;

    strcpy( stop_c, STOP_C );

    for(i = 0 ; i < CTRLNUM ; i++) c_arr[i] = i ;
    strcpy( m_arr[0], "w\377\027\031\377" );
    strcpy( m_arr[1], "W\377\027\031\377" );
    strcpy( m_arr[2], "l\377Text \377" );
    strcpy( m_arr[3], "L\377En rad\n[nnu en rad\n\377" );
    for (i = 4 ; i < MACRNUM ; i++) {
	m_arr[i][0] = END_M ;
	m_arr[i][1] = END_M ;
	}

    save_profile();
}

/***********************************************************
Cmnd_scan() is the main command scan.
***********************************************************/

cmnd_scan()
{
    if ( in_macro ) {
	if ( u_arg > 1 && prev_cmnd != CTRL_U ) {
	    u_arg--;
	    m_ = cmnd_start;
	    }
	else if ( u_arg==1 ) u_arg=0;
	if ( macr[m_] == END_M )
	    if ( m_arg > 1 ) {
		m_arg--;
		m_ = 0;
		}
	    else {
		in_macro = FALSE;
		goto no_more_macros;
		}
	cmnd_start = m_;
	cmnd = macr[m_++];
	}
    else {
no_more_macros:
	m_ = 0;
	if ( u_arg > 1 && prev_cmnd != CTRL_U ) {
	    cmnd = macr[m_++];
	    u_arg--;
	    }
	else {
	    if ( u_arg==1 ) u_arg=0;
	    macr[m_++] = cmnd = get_char();
	    macr[m_] = END_M;
	    }
	}
    if ( printable( cmnd ) )
	add_char( cmnd );
    else
	switch ( cmnd ) {
            case DEL    :            /* Delete and        */
            case CTRL_H :            /* Backspace         */
                 delete() ;
                 break ;

            case CTRL_M :            /* Carriage ret      */
            case CTRL_J :            /* & Line Feed       */
                 new_line() ;
                 break ;

            case MARK   :
                 set_mark() ;
                 break ;

            case CTRL_A :            /* Start of line     */
                 ctrl_A() ;
                 break ;

            case CTRL_B :            /* Cursor backwards  */
                 ctrl_B() ;
                 break ;

            case CTRL_C :            /* Quit siv          */
                 quit() ;
                 break  ;

            case CTRL_D :            /* Right delete      */
                 ctrl_D() ;
                 break ;

            case CTRL_E :	     /* End of line       */
                 ctrl_E() ;
                 break ;

            case CTRL_F :            /* Cursor forward    */
                 ctrl_F() ;
                 break ;

            case CTRL_G :            /* Get out           */
                 u_arg = 0 ;
		 m_arg = 0 ;
                 break ;

            case TAB    :            /* Tab               */
                 add_char(cmnd);
                 break ;

            case CTRL_K :            /* Line delete       */
                 ctrl_K() ;
                 break ;

            case CTRL_L :            /* Rewrite window    */
                 ctrl_L() ;
                 break ;

            case CTRL_N :            /* Cursor down       */
                 ctrl_N();
                 break ;

            case CTRL_O :            /* Insert blank line */
                 ctrl_O() ;
                 break ;

            case CTRL_P :            /* Cursor up         */
                 ctrl_P() ;
                 break ;

            case CTRL_Q :            /* Quote character   */
                 ctrl_Q() ;
                 break ;

            case CTRL_R :            /* Reverse search    */
                 ctrl_R() ;
                 break ;

            case CTRL_S :           /* Forward search    */
                 ctrl_S() ;
                 break ;

            case CTRL_T :            /* Transpose chars   */
                 ctrl_T() ;
                 break ;

            case CTRL_U :            /* Repeat command    */
                 ctrl_U() ;
                 break ;

            case CTRL_V :            /* Next window       */
                 ctrl_V() ;
                 break ;

            case CTRL_W :            /* Kill region       */
                 ctrl_W() ;
                 break ;

            case CTRL_X :            /* Alter             */
                 ctrl_X() ;
                 break ;

            case CTRL_Y :            /* Yank kill buffer  */
                 ctrl_Y() ;
                 break ;

            case CTRL_Z :            /* Scroll one line up*/
                 ctrl_Z() ;
                 break ;

            case CTRL_SH_O :         /* Execute subshell  */
                 subshell( INTERACTIVE );
                 break;

            case ESC    :            /* Escape / Meta     */
                 escape() ;
                 break ;

            default  :               /* Not implemented   */
                 pr_msg( NO_IMP, NULL );
                 break ;
        }
}

/***********************************************************
Escape() is the escape command and macro command scan
***********************************************************/

escape()
{
    int t;
    char c;

    if ( in_macro ) {
	if ( macr[m_] == END_M ) {
	    pr_msg( MACR_E, NULL );
	    return;
	    }
	}
    else if ( prev_cmnd==CTRL_U || u_arg<1 ) {
	do {
	    macr[m_++] = get_char();
	    macr[m_] = END_M;
	    t= test_if_macr( macr+1, m_-1 );
	    if ( t == FALSE ) {
		m_ = 1;
		break;
		}
	    if ( t == TRUE ) {
		m_arg = u_arg;
		u_arg = 0;
		in_macro = TRUE;
		m_ = 0;
		return;
		}
	    } while ( t==MAYBEE && m_<MACRLEN-1 );
	if ( t==MAYBEE ) {
	    pr_msg( MACR_E, NULL );
	    u_arg = 0;
	    return;
	    }
	}
    switch( c = upper_case( macr[m_++] ) ) {
	case TOP_F :             /* To top of file        */
            top_file() ;
            break ;
        case END_F :             /* To end of file        */
            end_file() ;
            break ;
        case BCK_W :             /* Previous screen       */
            bck_wdw() ;
            break ;
	case DEL :
	    add_char('\377');
	    break;
        case QUERY :             /* Query replace         */
            q_rplce() ;
            break ;
        case 'O'   :
        case '['   :             /* vt100 cursor          */
            vt100_keys( c );
            break ;
        case 'F'   :             /* forward word          */
            esc_F();
            break ;
        case 'B'   :             /* back word             */
            esc_B();
            break ;
        case 'D'   :             /* delete word           */
            esc_D();
            break ;
        case CTRL_G :            /* abort                 */
            pr_msg( ABORTD, NULL );
            break ;
        default    :
            pr_msg( NO_IMP, NULL );
            break ;
        }
}

upper_case( c )
char c ;
{
    return( (c < 'a' || c > '}') ? c : c - 32 );
}

/***********************************************************
Ctrl_X() is the CTRL_X command scan.
***********************************************************/

ctrl_X()
{
    if ( in_macro ) {
	if ( macr[m_] == END_M ) {
	    pr_msg( MACR_E, NULL );
	    return;
	    }
	}
    else if ( prev_cmnd==CTRL_U || u_arg<1 ) {
	macr[m_] = get_char();
	macr[m_+1] = END_M;
	}

    switch( macr[m_++] ) {

        case CTRL_V :          /* Read file               */
        case CTRL_R :
             newfile() ;
             break ;

        case CTRL_F :          /* Save file and exit      */
             wr_exit() ;
             break ;

        case CTRL_I :          /* Insert file             */
             ins_file() ;
             break ;

        case CTRL_L :          /* Load siv_profile        */
             get_macrofile();
             break ;

        case CTRL_S :          /* Save file               */
             save( FALSE ) ;
             break ;

        case CTRL_T :          /* save as text            */
             ctrl_X_T();
             break ;

        case CTRL_W :          /* Write file              */
             save( TRUE ) ;
             break ;

        case CTRL_D :          /* Change directory        */
             change_dir();
             break;

        case SH_COMM :         /* Execute shell command   */
             subshell( ONE_COMMAND );
             break;

        case CTRL_X :          /* switch mark and cursor  */
             if( ctrl_X_X() ) ctrl_L();
             break ;

        case CTRL_G :          /* abort                   */
             pr_msg( ABORTD, NULL );
             break ;

        default     :          /* Not implemented         */
             pr_msg( NO_IMP, NULL );
             break ;
        }
}

/***********************************************************
Vt100_keys() : vt100 cursor arrows and other keys.
c is the first character after the escape, eg 'O' in
'ESC O A' for the up arrow.
***********************************************************/

vt100_keys( c )
char c;
{
    static FLAG ord=FALSE, rad=FALSE;

    if ( in_macro ) {
	if ( macr[m_] == END_M ) {
	    pr_msg( MACR_E, NULL );
	    return;
	    }
	}
    else if ( prev_cmnd==CTRL_U || u_arg<1 ) {
	macr[m_] = get_char();
	macr[m_+1] = END_M;
	}
    if (c == 'O') switch( macr[m_++] ) {
	case 'A':		/* Shift up pad */
	    bck_wdw();
	    break;
	case 'B':		/* Shift down pad */
	    ctrl_V();
	    break;
	case 'I' :		/* HELP */
	    help();
	    break;
	case 'q' :		/* SHIFT PF2 */
	    wr_exit();
	    break;
	case 'Q' :		/* PF2 */
	    save( FALSE );
	    break;
	default :
	    pr_msg( NO_IMP, NULL );
	    break ;
        }
    else switch ( macr[m_++] ) {	/* ESC [ ... */
	case 'A' :		/* arrow up	*/
	    ctrl_P();
	    break ;
	case 'B' :		/* arrow down	*/
	    ctrl_N();
	    break ;
	case 'C' :		/* arrow right	*/
	    ctrl_F() ;
	    break ;
	case 'D' :		/* arrow left   */
	    ctrl_B() ;
	    break ;
	default:
	    pr_msg( NO_IMP, NULL);
	    break;
	}
}
