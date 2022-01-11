/***********************************************************

This file contains routines for :

   <1> Inserting a row in the textbuffer.
   <2> Get a row from the textbuffer.
   <3> Initializing the pointers and the global variables.

The rest of the routines are invisible from outside.

The following definitions exist in define.h to simplify
things:
	#define free_p row_p
	#define area_p row
***********************************************************/

#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           tot_len   ;
extern struct free_p first_f   ;
extern struct row_p  first_r   ;
extern struct row_p  kill_buf  ;
extern struct row_p  *free_r   ;
extern struct row_p  *prev_row ;
extern struct free_p dummy     ;
extern char	     f_name[]  ;
extern char          hom_dir[] ;

/***********************************************************
Buf_init() is called once, to initialize the text-, free_p- 
and row_p-buffers plus various global variables.
***********************************************************/

FLAG Buf_init()
{
    /* Put a dummy at the end of the free_p-list          */
    dummy.previous   = &first_f ;
    dummy.area_p     = NULL ;
    dummy.length     = 0 ;
    dummy.next       = NULL ;

    /* Init first_f                                       */
    first_f.previous = NULL ;
    first_f.area_p   = NULL ;
    first_f.length   = 0 ;
    first_f.next     = &dummy ;

    /* Init first_r  & kill_buf                           */
    first_r.previous = kill_buf.previous = NULL ;
    first_r.row      = kill_buf.row      = NULL ;
    first_r.length   = kill_buf.length   = 0 ;
    first_r.next     = kill_buf.next     = NULL ;

    free_r           = NULL ;
    prev_row         = &first_r ;
    tot_len          =  0 ;
}

/***********************************************************
Insert() enters the row_p-pointer returned by ins_text()
into the first_r-list after rp.
***********************************************************/

insert( row, rp, len )
char *row ;
struct row_p *rp ;
int len ;
{
    struct row_p *ins_text(), *p ;

    p = ins_text( row, len ) ;
    /* Insert it after rp                      */
    p->next = rp->next ;
    p->previous = rp ;
    rp->next = p ;
    if( p->next != NULL )
        /* Not last in list */
        p->next->previous = p ;
}

/***********************************************************
Get_row() gets the address of the row pointed to by
rp->row. It also "frees" the row_p-pointer and the text-
buffer-area occupied by the row.
***********************************************************/


char *get_row( rp ) struct row_p *rp ;
{
    if( rp->previous == NULL ) {
        printf("\nSERIOUS ERROR, CAN'T RECOVER!!\n");
        panic_exit() ;
        }
    if( ( rp->previous->next = rp->next ) != NULL ) {
        /* This was not the last row_p pointer           */
        rp->next->previous = rp->previous ;
        }

    if( rp->length > 0 )
        /* Put the unlinked row_p-pointer and text-buffer-area
           in the free_p-list */
        append( rp TO first_f.next ) ;
    else {
	/* Put the unlinked row_p-pointer in the free_r list */
        rp->next = free_r ;
        free_r = rp ;
	}
    return( rp->row ) ;
}

/***********************************************************
Append() : tries to append a free_p-block to another block
in the first_f-list.
***********************************************************/

static append( f1 TO f2 )
struct free_p *f1, *f2 ;
{
    if( f2 == &dummy ) {

        /* At the end of the first_f-list, couldn't       */
        /* append. Insert before dummy                    */
        f1->next = &dummy ;
        f1->previous = dummy.previous ;
        dummy.previous->next = f1 ;
        dummy.previous = f1 ;
        return ;
    }
    else if( f1->area_p + f1->length == f2->area_p ) {

        /* f2 continues after f1. Append blocks!          */
        f2->area_p = f1->area_p ;
        f2->length += f1->length ;
        f1->next = free_r ;
        free_r = f1 ;
        return ;
    }
    else if( f2->area_p + f2->length == f1->area_p ) {

        /* f1 continues after f2. Append blocks!          */
        f2->length += f1->length ;
        f1->next = free_r ;
        free_r = f1 ;
        if( f2->next != &dummy ) {
            if( f2->area_p + f2->length == f2->next->area_p ) {

                /* f1 fits exactly between two blocks!!!  */
                f2->next->area_p = f2->area_p ;
                f2->next->length += f2->length ;
                f2->next->previous = f2->previous ;
                f2->previous->next = f2->next ;
                f2->next = free_r ;
                free_r = f2 ;
            }
        }
        return ;
    }
    else {
        if( f1->area_p < f2->area_p ) {

            /* Couldn't append. Insert before f2          */
            f1->next = f2 ;
            f1->previous = f2->previous ;
            f1->previous->next = f1 ;
            f2->previous = f1 ;
            return ;
        }

        /* Try next free_p-block                          */
        append( f1 TO f2->next ) ;
    }
}
    
/***********************************************************
Ins_text() puts a row in the textbuffer. "Row" is the
address of the row. It returns a pointer to a row_p-
structure.
***********************************************************/

struct row_p *ins_text( row, len ) 
char *row ;
int len ;
{
    char *p, *new() ;
    struct row_p *tmp, *new_l_p() ;

    if( len == 0 ) p = NULL ;
    else {
        p = new( len ) ;
        cpystr( row, len, p ) ;
    }
    tmp = new_l_p() ;
    tmp->row = p ;
    tmp->length = len ;
    return( tmp ) ;         
}


/***********************************************************
New() returns a pointer to a textbuffer-area of size "size". 
If there ain't enough space, a new text-block is allocated. 
***********************************************************/

static char *new( size ) int size ;
{
    struct free_p *fp;
    struct row_p *new_l_p() ;
    char *txt_alloc() ;
    
    fp = first_f.next ;
    while( TRUE ) {
        if( fp == NULL ) {

            /* New block most be created                  */
            fp = new_l_p() ;
            fp->area_p = txt_alloc() ;
            fp->length = 4096 ;
            fp->next = first_f.next ;
            first_f.next->previous = fp ;
            fp->previous = &first_f ;
            first_f.next = fp ;
            return( new( size ) ) ;
        }
        else if( fp->length == size ) {

            /* Just enough space, remove that free_p      */
            fp->previous->next = fp->next ;
            fp->next->previous = fp->previous ;

            /* Insert free_p in free_r-list               */
            fp->next = free_r ;
            free_r = fp ;
            return( fp->area_p ) ;
        }
        else if( fp->length > size ) {

            /* Update that free_p                         */
            fp->length -= size ;
            fp->area_p += size ;
            return( fp->area_p - size ) ;
        }
        fp = fp->next ;
    }
}

/***********************************************************
New_l_p() returns a pointer to a nonused row_p-structure.
If the free_r list is empty a new block of row_p is
allocated.
***********************************************************/

static struct row_p *new_l_p()
{
    struct row_p *tmp, *row_alloc() ;
    int i ;

    if( free_r != NULL ) {
        tmp = free_r ;
        free_r = tmp->next ;
        return( tmp ) ;
    }
    else {
        tmp = row_alloc() ;
        for( i = 0 ; i < 64 ; i++ ) {
            tmp->next = free_r ;
            free_r = tmp++ ;
        }
        return( new_l_p() ) ;
    }
}

/***********************************************************
Row_alloc() allocates a new block of row-pointers. Each
block contains space for 64 row_p structures.
***********************************************************/

static struct row_p *row_alloc()
{
    char *sbrk() ;
    struct row_p *tmp ; 

    tmp = (struct row_p *) sbrk(64 * sizeof(struct row_p)) ;
    if( tmp == NULL ) {

        /* OUT OF MEMORY */
        printf( "\nOUT OF MEMORY\n" ) ;
        panic_exit() ;
    }
    else return( tmp ) ;
}

/***********************************************************
Txt_alloc() allocates a new text-buffer of size 4096 bytes.
***********************************************************/

static char *txt_alloc() 
{
    char *sbrk(), *tmp ;
    
    tmp = sbrk( 4096 ) ;
    if( tmp == NULL ) {

        /* OUT OF MEMORY */
        printf( "\nOUT OF MEMORY\n" ) ;
        panic_exit() ;
    }
    else return( tmp ) ;
}

/***********************************************************
Panic_exit() gives "soft" siv crashes by saving the text in
"$HOME/crash.siv" and reseting terminal.
The function is as memory-saving as possible, because there
might not be much memory left.
***********************************************************/

panic_exit()
{
    FILE *fopen(), *fp;

    strcpy( f_name, hom_dir );
    strcat( f_name, "/crash.siv" );
    printf( "Saving text in: %s\n", f_name );
    printf( "The text may be incomplete!\n" );
    printf( "To try recovering text type: siv %s\n", f_name );
    sterm(OLD | XON);
    if ( ( fp = fopen( f_name, "w" ) ) == NULL )
	printf( "COULDN'T SAVE FILE!" );
    else
	savefile( fp, TRUE );
    exit (1);
}
