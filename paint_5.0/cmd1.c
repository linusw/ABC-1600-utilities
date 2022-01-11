/*	cmd1.c	(C)	MITHRIL MJUKVARA HB	1985

	This file contains the modules for:

	pencil(), line(), box(), fill_box() and paint_box().

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for icons and messages.
*/
	
extern int paper,language;
extern char mouse[],mouseln,font; 
extern char cross_buffer[],cross,cross_blen,*msg[][2];
extern char pos_req[];
extern int color,pattern;
extern char buffer[512];
extern int  n;


pencil()
{ register int i,m;
  char mbuffer[20];

  /*
  	MOVE TO BEGINNING OF LINE
  */
  sput(mouse);

  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,mouse,mouseln);

  if(cross==0 || cross==2)
  {
    /*
	CROSS OFF
    */
    while(1)
    { do
      { write(paper,pos_req,4);
        n=read(paper,buffer,32)-2;
      } while(*buffer!='\33');
      if(cross)
      { write(paper,cross_buffer,cross_blen);
        cross_blen=draw_inv_cross(buffer,cross_buffer);
      }

      if(buffer[n]=='0') break;

      if(pattern/10!=0) buffer[n++]=pattern/10+48;
      buffer[n++]=pattern%10+48;
      buffer[n++]=';';
      buffer[n++]=color+48;
      buffer[n++]='d';
      buffer[n]=0;
      sput(buffer);
      write(paper,buffer,n);
    }
  }
  else
  {
    /*
    	CROSS ON
    */
    cross_blen=0;
    i=mouseln;
    while(1)
    { do
      { write(paper,pos_req,4);
        n=read(paper,buffer,32)-2;
      } while(*buffer!='\33');
      m=n-1;

      if(buffer[n]=='0') break;

      if(pattern/10!=0) buffer[n++]=pattern/10+48;
      buffer[n++]=pattern%10+48;
      buffer[n++]=';';
      buffer[n++]=color+48;
      buffer[n++]='d';
      buffer[n]=0;
      sput(buffer);
      write(paper,cross_buffer,cross_blen);
      write(paper,mouse,i);
      write(paper,buffer,n);
      for(i=0;i<m;i++) mouse[i]=buffer[i];
      mouse[i++]='m';
      cross_blen=draw_inv_cross(buffer,cross_buffer); 

    }
    write(paper,cross_buffer,cross_blen);
  }

  if(cross) cross_blen=draw_inv_cross(buffer,cross_buffer);
  cput(1);
}




line()
{ register char m;

  do
  { write(paper,pos_req,4);
    n=read(paper,buffer,32)-2;
  } while(*buffer!='\33');
  if(cross)
  { write(paper,cross_buffer,cross_blen);
    cross_blen=draw_inv_cross(buffer,cross_buffer);
  }
  do
  { buffer[n-1]='i';
    m=mouseln;
    n=0;
    while((mouse[m++]=buffer[n++])!='i');
    write(paper,mouse,m);
    do
    { write(paper,pos_req,4);
      n=read(paper,buffer,32)-2;
    } while(*buffer!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(buffer,cross_buffer);
    }
    write(paper,mouse,m);
  } while(buffer[n]=='1');

  mouse[m-1]=';';
  if(pattern/10!=0) mouse[m++]=pattern/10+48;
  mouse[m++]=pattern%10+48;
  mouse[m++]=';';
  mouse[m++]=color+48;
  mouse[m++]='d';
  mouse[m]=0;
  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,mouse,m);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(mouse);
  cput(1);
}

box()
{ register char oc[32];
  
  draw_inv_box(mouse,oc,buffer);

  if(cross) write(paper,cross_buffer,cross_blen);
  draw_box(mouse,oc,buffer,pattern,color);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(buffer);
  cput(1);
}

fill_box()
{ char oc[32],c;
  int x0,y0,x1,y1;
  
  draw_inv_box(mouse,oc,buffer);

  sscanf(mouse,"%c%c%d%c%d",&c,&c,&x0,&c,&y0);
  sscanf(oc,"%c%c%d%c%d",&c,&c,&x1,&c,&y1);
  n=sprintf(buffer,"\33:%d;%dm\33:%d;%d;%d;%df"
          ,x0,y0,x1,y1,pattern,color);
  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,buffer,n);
  sput(buffer);
  draw_box(mouse,oc,buffer,0,1);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(buffer);
  cput(1);
}

paint_box()
{ register char oc[32];
  char c;
  int x0,y0,x1,y1;
    
  draw_inv_box(mouse,oc,buffer);

  sscanf(mouse,"%c%c%d%c%d",&c,&c,&x0,&c,&y0);
  sscanf(oc,"%c%c%d%c%d",&c,&c,&x1,&c,&y1);
  n=sprintf(buffer,"\33:%d;%dm\33:%d;%d;%d;%df",x0,y0,x1,y1,pattern,color);
  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,buffer,n);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(buffer);
  cput(1);
}
