/*---------------------------------------------------------------------------
 *
 *
 *     setup.c
 *
 *     Program to define special keys on ABC-99 keyboard
 *
 *     By Kjell Sandberg LUXOR DATORER AB 841221
 *---------------------------------------------------------------------------
 *
 * Modified by K. Sandberg 85 11 14 in line 134, 164, 413
 *
 */



#include "setup.h"

setty()

   {

    gtty(1,&iocb);
    iocb.sg_flags |=(CBREAK);
    iocb.sg_flags &= ~ECHO;
    stty(1,&iocb);
   }



restty()

   {
    iocb.sg_flags &= ~(CBREAK);
    iocb.sg_flags |= ECHO;
    stty(1,&iocb);
   }

pos(x,y)

    register int x,y;

   {
    printf("\033[%d;%dH",y,x);
   }


cls()
   {
    printf("\033[H\033[2J");
   }

clst()
   {
    printf("\033[1J");
   }

clsf()
   {
    printf("\033[0J");
   }

inv()
   {
    printf("\033[7m");
   }

norm()
   {
    printf("\033[m");
   }

und()
   {
    printf("\033[4m");
   }

cl()
   {
    printf("\033[K");
   }


getfstr(c)

    int c;

   {
    register int i;
    
    register unsigned char *from, *to;
  
    struct fnkst fks;
  

    fks.u.st.code=c;
  
    if ( Ioctlr(1,PFNKRD,&fks,sizeof(fks)) < 0){
	pos(1,24);
	pr_msg(R_KEY_ERR,NULL);
    }

    from = &fks.string[0];
    to = (unsigned char*) &fnkarr[c&0x7f][0];
    i = fks.u.st.len;
    if (i>MAXLEN) {
	pos(1,24);
	pr_msg(TO_L_FU_STR,NULL);
        return;
    }

    fnklen[c&0x7f]=i;
    while (i-->0) {
        *to++ = *from++;
     }

    }


getint()

   {

    register int val=0, pos=0;
    unsigned char c;

more:
    while ((c=gettc())>='0' && c<='9'){
      pos++;
      val=val*10+(c-'0');
    }
    if (c=='\b') {
      val /=10;
      if (pos) {
        pos--;
        putchar('\b'); putchar(' '); putchar('\b');
        putchar('\b'); putchar(' '); putchar('\b');
	fflush(stdout);                                 /*Modified 851114*/
	goto more;
      }
      if (pos == 0) {
	 putchar('\b'); putchar(' '); putchar('\b');
	 fflush(stdout);
      }
      goto more;
    }
    putchar('\b'); putchar(' '); putchar('\b');
    fflush(stdout);
    if(pos == 0) val = 99;       /*Fix f|r val 0 i huvudmenyn*/
    return(val);
   }

getstr(pek)

    char *pek;

   {

    register char *start;
    register unsigned char c;

    start=pek;
    while ((c=gettc())!='\n') {
      if (c=='\b') {
	if (pek > start) {
          pek--;
          putchar(c); putchar(' '); putchar(c); /* erase '8' */
          putchar(c); putchar(' '); putchar(c); /* erase prev. char */
	  fflush(stdout);
        }
	else if (pek == start) {
	   putchar(c); putchar(' '); putchar(c);
	   fflush(stdout);
	}
      }
      else if (c=='\\') {
        putchar('\b'); putchar(' '); putchar('\b'); /* erase '\' */
	*pek++ =gettc();
      }
      else
        *pek++ =c;
    }
    putchar('\b'); putchar(' '); putchar('\b');
    fflush(stdout);
    *pek='\0';
   }

gettc()

   {

    register unsigned char c;

    c=getchar();
    c &=0x7f;
    /* echo in special way */
    if (c>=0x20)
      putchar(c);
    else {
      inv();
      putchar(c+0x40);
      norm();
    }
    fflush(stdout);
    return(c);
   }

    


display(line,col,c,ch)

    int line,col,c;
    int *ch;
   {

    register int i;
    int j = 0;
    char ast = '*';
    
    register unsigned char *from;
  

    pos(col,line);
    cl();
    from = (unsigned char*) &fnkarr[c&0x7f][0];
    i = fnklen[c&0x7f];

    while (i-->0) {
        if (*from<0x20) {
            inv();
            putchar(*from+0x40);
            norm();
        }
        else {
            und();
            putchar (*from);
            norm();
        }
        from++;
	j++;
	if ((*ch >= 7) && j >= 20){
	  putchar(ast);
	  putchar(ast);
	  break;
	}
        
    }

   }


putfstr(c)

    int c;

   {


    register int i, len;
    
    register unsigned char *from, *to;
  
    struct fnkst fks;
  

    fks.u.st.code=c;
    i=fks.u.st.len=fnklen[c&0x7f]; 
    len=i;
    to = &fks.string[0];
    from = (unsigned char*) &fnkarr[c&0x7f][0];
    if (i>MAXLEN) {
	pos(1,24);
	pr_msg(TO_L_FU_STR,NULL);
        return;
        
    }
    while (i-->0) {
        *to++ = *from++;
    }

    if ( Ioctlr(1,PFNKLD,&fks,sizeof(fks)) < 0){
	pos(1,24);
	pr_msg(L_KEY_ERR,NULL);
        return;
    }
   }

defkeys()
   {
    int fk;

    for (fk=1; fk<16;fk++){
    putfstr(0x3f+0x80+fk);
    putfstr(0x4f+0x80+fk);
    putfstr(0x5f+0x80+fk);
    putfstr(0x6f+0x80+fk);
    }
    for(fk=0; fk<23; fk++){
    putfstr(keytb[2*fk+language].code);
    }
}

deflang()
   {
    char        *tmp;
    char        *getenv();

    if((tmp = getenv( "LANGUAGE" )) == NULL){
	language = ENGLISH;
    }
    else if( strcmp( "swedish",tmp ) == 0){
	language = SWEDISH;
    }
    else language = ENGLISH;
   }

pr_msg(n,str)
    int n;
    char        *str;
   {
	printf(messages[ 2 * n + language ] , str);
	fflush(stdout);
   }


main(argc,argv)
    int argc;
    char *argv[];

   {
    int i,fk, base, sel;
    int fd,j,k,l;
    char *ptr;
    char s;

    int onintr();

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, onintr);

    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	signal(SIGQUIT, onintr);

    deflang();

    setty();

    setbuf(stdout,strmbuf);

    if (argc > 2){
     pr_msg(TO_M_ARG,NULL);
     restty();
     exit(1);
    }
    else if(argc == 2){
     if((fd = open(argv[1],0)) == -1){
     pr_msg(NO_OPEN,argv[1]);
     restty();
     exit(1);
    }
    fk = read(fd,(char*)(&filarr[0]),sizeof(filarr));
    if(fk <= 128){
     pr_msg(RE_ERR,NULL);
     restty();
     exit(1);
    }

	    i = 0;
	    j = 0;
	    k = 0;
	    while(k < 128){
	      fnklen[k] = filarr[k];
	      k++;
	    }
	    while(i < 128){
	      l = filarr[i];
	      while(j < l){
		fnkarr[i][j++] = filarr[k++];
	      }
	      i++;
	      j = 0;
	    }
    defkeys();
    cls();
    }

    while (1) {
      clst();
      pos(35,1);
      inv();
      pr_msg(SETUP,NULL);
      norm();
      pos(35,4);
      pr_msg(PF1,NULL);
      pos(35,6);
      pr_msg(PF2,NULL);
      pos(35,8);
      pr_msg(PF3,NULL);
      pos(35,10);
      pr_msg(PF4,NULL);
      pos(35,12);
      pr_msg(OTHER,NULL);
      pos(35,14);
      pr_msg(R_FILE,NULL);
      pos(35,16);
      pr_msg(S_FILE,NULL);
      pos(35,18);
      pr_msg(CL_KEYS,NULL);
      pos(35,20);
      pr_msg(EXIT,NULL);
      pos(35,22);
      inv();
      pr_msg(SELECT,NULL);
      norm();
      printf(" ");
      fflush(stdout);

      sel = getint();
      switch (sel) {
    
        case 1:
            cls();
            pos(30,1);
            inv();
	    pr_msg(PF1,NULL);
            norm();
            fkload(0x3f);
	    cls();
            break;

        case 2:
            cls();
            pos(30,1);
            inv();
	    pr_msg(PF2,NULL);
            norm();
            fkload(0x4f);
	    cls();
            break;

        case 3:
            cls();
            pos(30,1);
            inv();
	    pr_msg(PF3,NULL);
            norm();
            fkload(0x5f);
	    cls();
            break;

        case 4:
            cls();
            pos(30,1);
            inv();
	    pr_msg(PF4,NULL);
            norm();
            fkload(0x6f);
	    cls();
            break;

        case 5:
            other();
	    cls();
            break;

        case 6:
	    pos(35,23);
	    printf("\033[J");
	    pos(35,24);
	    pr_msg(F_NAME,NULL);
	    getstr(&name[0]);
	    if(name[0] == '\0'){
	     printf( "?");
	     pos(35,23);
	     break;
	    }
	    if((fd = open(&name[0],0)) == -1){
	     pos(35,24);
	     pr_msg(NO_OPEN,&name[0]);
	     pos(35,23);
	     break;
	    }
	    fk = read(fd,(char*)(&filarr[0]),sizeof(filarr));
	    if(fk < 128){
	     pos(35,24);
	     pr_msg(RE_ERR,NULL);
	     pos(35,23);
	     break;
	    }
	    close(fd);

	    i = 0;
	    j = 0;
	    k = 0;
	    while(k < 128){
	      fnklen[k] = filarr[k];
	      k++;
	    }
	    while(i < 128){
	      l = filarr[i];
	      while(j < l){
		fnkarr[i][j++] = filarr[k++];
	      }
	      i++;
	      j = 0;
	    }

	    defkeys();
            break;

	case 7:
	    pos(35,23);
	    printf("\033[J");
	    pos(35,24);
	    pr_msg(F_NAME,NULL);
	    getstr(&name[0]);
	    if(name[0] == '\0'){
	     printf("?");
	     pos(35,23);
	     break;
	    }
	    if((fd = creat(&name[0],PMODE)) == -1){
	     pos(35,34);
	     pr_msg(NO_CREAT,&name[0]);
	     pos(35,23);
	     break;
	    }

	    i = 0;              /*Number of keys        */
	    j = 0;              /*Keylengt              */
	    k = 0;              /*Keyindex on file      */
	    while(k < 128){
	      filarr[k] = fnklen[k];
	      k++;
	    }
	    while(i < 128){
	      l = fnklen[i];
	      while((j < l) && (l != '\0')){
		filarr[k++] = fnkarr[i][j++];
	      }
	      i++;
	      j = 0;
	    }

	    fk=write(fd,(char*)(&filarr[0]),k);
	    if(fk <= --k){
	     pos(35,24);
	     pr_msg(WR_ERR,NULL);
	     pos(35,23);
	     break;
	    }
	    close(fd);
	    break;
	case 8:
	    cls();
	    for(fk=0; fk<129; fk++){
	    fnklen[fk] = 0;
	    }
	    for(sel=0; sel<23; sel++){
	     for(fk=0; fk<129; fk++){
	     fnkarr[fk][sel] = 0;
	     }
	    }
	    defkeys();
	    break;

	default:
	    cls();
	    break;

      case 0:
	    cls();
	    restty();
	    exit(0);

      } /* switch*/

    } /* while */
            
   }



other()

   {
    register int sel,fk;
    int ch;

    cls();
    pos(30,1);
    inv();
    pr_msg(SETUP_OK,NULL);
    norm();
    for (sel=0; sel<15; sel++) {
      pos(1,keytb[2*sel+language].l);
      und();
      printf("%s",keytb[2*sel+language].string);
      fflush(stdout);
      norm();
      getfstr(keytb[2*sel+language].code);
      ch = sel;
      display(keytb[2*sel+language].l,
      keytb[2*sel+language].r,keytb[2*sel+language].code,&ch);
    }
    pos(1,11);
    norm();
    inv();
    pr_msg(ARR_KEY,NULL);
    pos(40,11);
    pr_msg(ARR_KEY_SH,NULL);
    norm();
    for (sel=15; sel<23; sel++) {
      pos(40,keytb[2*sel+language].l);
      und();
      printf("%s",keytb[2*sel+language].string);
      fflush(stdout);
      norm();
      getfstr(keytb[2*sel+language].code);
      ch = sel;
      display(keytb[2*sel+language].l,
      keytb[2*sel+language].r,keytb[2*sel+language].code,&ch);
    }

    while (1) {
      pos(1,22);
      clsf();
      pr_msg(KEYNR,NULL);
      sel=getint();
      if (sel<1 || sel>23) return;
      pos(1,23);
      pr_msg(ED,NULL);
      pos(1,24);
      cl();
      pr_msg(INPUT,NULL);
      fk = keytb[(2*sel+language)-2].code&0x7f;
      getstr(&fnkarr[fk][0]);
      pos(1,24);
      cl();
      fnklen[fk]= strlen(&fnkarr[fk][0]);
      putfstr(fk+0x80);
      getfstr(fk+0x80);
      ch = sel - 1;
      display(keytb[(2*sel+language)-2].l,
      keytb[(2*sel+language)-2].r,fk+0x80,&ch);
      if (sel>=8 && sel<=15 ) {
        sel+=7;
	pos(40,keytb[2*sel+language].l);
        und();
	printf("%s",keytb[2*sel+language].string);
	fflush(stdout);
        norm();
	ch = sel - 1;
	display(keytb[2*sel+language].l,
	keytb[2*sel+language].r,keytb[2*sel+language].code,&ch);
      }
    }
   }




fkload(base)

    int base;

   {
    int i,fk;
    char *ptr;
    
    
    for (i=1;i<16;i++) {
	pos(1,i+3);
        und();
        printf("PF%d :",i);
        norm();
    }
    fkld(base);    
    
   }

fkld(base)

    int base;
   {
    int fk;
    int ch = 0;

    for (fk=1; fk<16;fk++) {
     getfstr(base+0x80+fk);
     display(fk+3,8,base+0x80+fk,&ch);
    }
    while (1){
     pos(1,22);
     clsf();
     pr_msg(KEYNR,NULL);
     fk=getint();
     if (fk<1 || fk>15) return;
     pos(1,23);
     pr_msg(ED,NULL);
     pos(1,24);
     cl();
     pr_msg(INPUT,NULL);
     getstr(&fnkarr[base+fk][0]);
     pos(1,24);
     cl();
     fnklen[base+fk]= strlen(&fnkarr[base+fk][0]);
     putfstr(base+0x80+fk);
     getfstr(base+0x80+fk);
     display(fk+3,8,base+0x80+fk,&ch);
    }
   }

onintr()
   {
    restty();
    exit(1);
   }
