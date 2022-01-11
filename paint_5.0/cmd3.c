/*	cmd3.c 	(C)	MITHRIL MJUKVARA HB	1985

	This file contains the modules for:

	undo(), redo(), new(), save(), load(), merge(), print() and
	stop_program() .

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for icons and messages

			December      1985
				Bug fixed in print()
				Inclusion of keyboard.h changed to
				abc1600/keyboard.h
			April	      1986
				Print() changed and debugged
*/

#include <stdio.h>
#include <dnix/fcodes.h>
#include <abc1600/keyboard.h>
#include <signal.h>
#include <sgtty.h>
#include <fcntl.h>
#include <win/w_macros.h>

extern char *conv_name, *pdriv_name, *prname;

extern int paper,window,xmax,ymax,icon_mode,language;
extern char cross_buffer[],cross,cross_blen,mouse[],font,*msg[][2];
extern char pos_req[],mode;
extern int saved;

extern char *storage[1024];
extern int storagesize,storagepnt;

extern char buffer[];
extern int n;


undo()
{ register int i,j;

  if(storagepnt==0) return;
  n=sprintf(buffer,"\33:0;0m\33:%d;%d;0;0f",xmax,ymax);
  write(paper,buffer,n);
  if(--storagepnt==0) return;
  storagepnt--;
  while(storage[storagepnt/512][storagepnt%512]!=1 &&
        storagepnt!=0) storagepnt--;
  if(storagepnt!=0) storagepnt++;
  for(i=0;i<storagepnt/512;i++) write(paper,storage[i],512);
  write(paper,storage[storagepnt/512],storagepnt%512);
  saved=0;
}




redo()
{ register char buff[512];
  register int i=0;

  if(storagepnt==storagesize) return;
  while(storage[storagepnt/512][storagepnt%512]!=1)
  { buff[i++]=storage[storagepnt/512][storagepnt%512];
    if(i==512)
    { write(paper,buff,512);
      i=0;
    }
    storagepnt++;
  }
  write(paper,buff,i);
  storagepnt++;
  saved=0;
}




new()
{ if(saved==0)
  { window_open();
    write(window,msg[2][language],strlen(msg[2][language]));
    read(window,buffer,4);
    window_close();
    if(buffer[0] != 'Y' && buffer[0] != 'y' &&
    	buffer[0] != 'J' && buffer[0] != 'j')return;
  }
  storagepnt=0;
  n=sprintf(buffer,"\33:0;0m\33:%d;%d;0;0f",xmax,ymax);
  write(paper,buffer,n);
  saved= -1;
}




save()
{ char mvbuff[256];
  int mvn;

  window_open();

  write(window,msg[3][language],strlen(msg[3][language]));
  n=read(window,buffer,60);
  buffer[--n]=0;
  if(n>0) {
      /*
      	WARNING - AD HOC

	IN ICON-MODE A BLACK BOX IS PLACED LAST IN THE FILE. THIS IS TO
	MAKE SURE THAT A BLACK (NOT A WHITE) BOX SURROUNDS THE ICON
      */
      if(icon_mode){
          mvn=sprintf(mvbuff,"\33:0;0m\33:0;%d;0;1d\33:%d;%d;0;1d\33:%d;0;0;1d\33:0;0;0;1d",ymax,xmax,ymax,xmax);
  	  write(paper,mvbuff,mvn);
	  mvbuff[mvn]=0;
	  sput(mvbuff);
          cput(1);
      }

      movetofile(buffer);
      saved= -1;
  }
  window_close();
}




load()
{ window_open();
  if(saved==0)
  { write(window,msg[2][language],strlen(msg[2][language]));
    read(window,buffer,4);
    if(buffer[0] != 'Y' && buffer[0] != 'y' &&
    	buffer[0] != 'J' && buffer[0] != 'j')
    { window_close();
      return;
    }
  } 
  write(window,msg[3][language],strlen(msg[3][language]));
  n=read(window,buffer,60);
  buffer[--n]=0;
  if(n>0){
      movefromfile(buffer,0);
      saved= -1;
  }
  window_close();
}




merge()
{ window_open();
  if(saved==0)
  { write(window,msg[2][language],strlen(msg[2][language]));
    read(window,buffer,4);
    if(buffer[0] != 'Y' && buffer[0] != 'y' &&
        buffer[0] != 'J' && buffer[0] != 'j')
    { window_close();
      return;
    }
  }
  write(window,msg[3][language],strlen(msg[3][language]));
  n=read(window,buffer,60);
  buffer[--n]=0;
  if(n>0){
      movefromfile(buffer,storagepnt);
      saved=0;
  }
  window_close();
}




print()
{ 
  int pid,pipeline[2],psave,ret;
  char a1[30],a2[30];
  struct bit_image  {
      struct wpictblk pict;
      byte b[63600];
      } *bp;

  char filename[256];
  int i;

  window_open();
  write(window,msg[28][language],strlen(msg[28][language]));
  n=read(window,buffer,60);
  if(n<=1){
      window_close();
      return(0);
      }

  if(buffer[n-2]=='y' || buffer[n-2]=='Y' ||
  	     buffer[n-2]=='J' || buffer[n-2]=='j'){
	write(window,msg[3][language],strlen(msg[3][language]));
	n=read(window,buffer,60);
	buffer[n-1]=0;
	if(n<=1){
	    window_close();
	    return(0);
	    }
	strcpy(filename, buffer);
	if ((n = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) {
	    write(window,msg[4][language],strlen(msg[4][language]));
	    sleep(5);
	    window_close();
	    return(-1);
	    }
	close(n);
	psave=1;
	}
  else {
	strcpy(filename, "/usr/tmp/paintXXXXXX");
	mktemp(filename);
	if ((n = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) {
	    write(window, msg[30][language], strlen(msg[30][language]));
	    write(window, filename, strlen(filename));
	    sleep(5);
	    window_close();
	    return(-1);
	    }	    
	close(n);
        psave=0;
        }

  pipe(pipeline);
  if((pid=fork())==0){
      /*
      	CHILD PROCESS (CONVERTER)
   	SET UP THE PIPE AS STANDARD INPUT
	SET UP THE LITTLE WINDOW AS STANDARD OUTPUT AND STANDARD ERROR
      */
      dup2(pipeline[0], 0);
      dup2(window, 1);
      dup2(window, 2);
 
      /*
      	PREPARE ARGUMENTS FOR CONVERTER
      */
      sprintf(a1,"%d",xmax+1);
      sprintf(a2,"%d",ymax+1);

      execl("/usr/bin/converter","/usr/bin/converter",a1,a2,filename,0);
      write(window, "Can't find converter program", 29);
      sleep(5);
      exit(1);
  }
  else{
      /*
      	ALLOCATE A DUMP AREA FOR PICTURE MEMORY
      */
      bp=(struct bit_image *)malloc(sizeof(struct bit_image));

      /*
      	DUMP PICTURE TO MEMORY
      */
      write(window,msg[5][language],strlen(msg[5][language]));
      bp->pict.p_xaddr=0; bp->pict.p_yaddr=0;
      bp->pict.p_width=xmax+1; bp->pict.p_height=ymax+1;
      Wpictrd(paper,bp,sizeof(struct bit_image));

      /*
      	PIPE PICTURE TO CONVERTER (CHILD)
      */
      n=(xmax+8)/8;
      for (i = 0; i < n * (ymax+1); i += 512)
        write(pipeline[1], bp->b + i, 512);
     
      /*
      	FREE DUMP AREA, WAIT UNTIL PICTURE CONVERTED
      */
      free(bp);

      while (wait(&ret) != -1 && ret != pid)
	  sleep(1);

      /*
      	CLOSE PIPE
      */
      close(pipeline[1]);

      if((pid=fork())==0){
	  /*
	    CHILD PROCESS (PDRIVER)
	    SET UP THE LITTLE WINDOW AS STANDARD OUTPUT AND STANDARD ERROR
	  */
	  dup2(window, 1);
	  dup2(window, 2);
 
          execl("/usr/bin/pdriver","/usr/bin/pdriver",prname,filename,0);
          write(window, "Can't find pdriver program", 27);
	  sleep(5);
	  exit(1);
          }
      else {
          write(window,msg[6][language],strlen(msg[6][language]));

          while (wait(&ret) != -1 && ret != pid)
	      sleep(1);
	  if (!psave)
	      unlink(filename);

	  /*
	    CLOSE WINDOW
	  */
	  window_close();
	  }
      }
}




stop_program()
{ int i;
  char tmp1,tmp2;

  if(saved==0)
  { window_open();
    write(window,msg[29][language],strlen(msg[29][language]));
    read(window,buffer,4);
    if(buffer[0] != 'Y' && buffer[0] != 'y' &&
    	buffer[0]!='J' && buffer[0]!='j')
    { window_close();
      return;
    }
  }

  /*
  	RESET TO TWO STEP INCREMENT (MOUSE)
  */
  tmp1=K_PREM;
  ptokbrd(3,&tmp1);
  tmp1=32;tmp2=33;
  ptokbrd(3,&tmp1);
  ptokbrd(3,&tmp2); 
  ptokbrd(3,&tmp1); 
  ptokbrd(3,&tmp2); 
  tmp2=34;
  ptokbrd(3,&tmp1); 
  ptokbrd(3,&tmp2); 
  ptokbrd(3,&tmp1);
  ptokbrd(3,&tmp2); 
  exit(0);
}
