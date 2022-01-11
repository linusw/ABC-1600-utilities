/***********************************************************
Each row is being pointed to by a row_p structure. These are
stored in row_p-buffers.
***********************************************************/

struct row_p {

    struct row_p *previous ;    /* points to previous row */
    char *row ;                 /* points to row          */
    int length ;                /* length of row          */
    struct row_p *next ;        /* points to previous row */
} ;

/***********************************************************
To be able to find nonused areas in the text-buffer, a 
"free_p"-structure is used. These are stored in free_p-
buffers. This is the same as a row_p structure, as defined
in define.h:
     #define free_p row_p
     #define area_p row

If it were declared it would look like this:

struct free_p {
    struct free_p *previous ;  /* points to previous block
    char *area_p ;               /* points to free area
    int length ;              /* length of this block
    struct free_p *next ;      /* points to next free block
} ;
It is not declared separately because this makes it easier
to handle.
***********************************************************/
/***********************************************************

    ROW_P-BUFFERS          TEXT-BUFFERS             FREE_P-BUFFERS

    -----------                                     -----------
    ! first_r !                                     ! first_f !
    -----------                                     -----------
        !                                                !
        V                                                V
    -----------            ------------             -----------
    !   NIL   !            !          ! <------     !   NIL   !
    -----------            -----------!       !     -----------
    !    -----!----------> ! RAD 01   !       ------!-----    !
    -----------            ------------             -----------
    !  TRUE   !            !          ! <---        !    xx   !
    -----------            !          !    !        -----------
    !    7    !            !          !    !    --> !     ----!----
    -----------            !          !    !    !   -----------   !
--> !    -----!----        !          !    !    !                 !
!   -----------   !        !          !    !    !                 ! 
!                 !        !          !    !    !   -----------   !
!                 !        !          !    !    ----!-----    ! <--
!                 !        !          !    !        -----------
!   -----------   !        !          !    ---------!-----    !
----!-----    ! <--        !          !             -----------
    -----------            ------------             !    xx   !
    !     ----!----------> ! RAD 2    !             -----------
    -----------            ------------         --> !     ----!----
    !  FALSE  !            !          ! <---    !   -----------   !
         .                       .         .    .         .       .
         .                       .         .    .         .       .
         .                       .         .    .         .       .
    
***********************************************************/
