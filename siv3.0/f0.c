#include <stdio.h>
#include "define.h"         /* constants etc                     */
#include "struct.h"
#include "global.h"         /* global definitions                */


main( argc, argv )
int argc ;
char * argv[] ;
{
    char file[MAXLEN];
    FILE *fopen(), *fp;
    FLAG cont=FALSE;
    int i, j;

    for (i=1; i<argc && argv[i][0]=='-'; i++ ) {
	for ( j=1; argv[i][j] != '\0'; j++ )
	    switch ( argv[i][j] ) {
	    case 'c':
		cont=TRUE;
		break;
	    case 's':
		siv_file=TRUE;
		break;
	    case 'r':
		restricted=TRUE;
		break;
	    case 'x':
		no_ctrl_Q_S=TRUE;
		break;
		}
	}
    initterm();
    set_sig(ON);
    clrscr();
    getcwd( cwd, MAXLEN );
    if ( i<argc )
	edit( argv[i] );
    else if ( cont ) {
	strcpy( file, cwd );
	strcat( file, "/.siv_file" );
	if ( ( fp=fopen( file, "r" ) ) != NULL) {
	    fscanf( fp, "%s", file );
	    fclose( fp );
	    edit( file );
	    }
	else
	    edit( NULL );
	}
    else
	edit( NULL );
}
