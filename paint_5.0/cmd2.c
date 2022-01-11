/*	cmd2.c	(C)	MITHRIL MJUKVARA HB	1985

	This file contains the modules for:

	frame(), circle(), text(), copy() and paint().

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for icons and messages.
*/
#include <stdio.h>

extern int paper,window,xmax,ymax,icon_mode,language;
extern char mouse[],mouseln,font,*msg[][2];
extern char cross_buffer[],cross,cross_blen;
extern char pos_req[];
extern int color,pattern;
extern char buffer[];
extern int n;



frame()
{ register char oc[32];
  register int x1=0,x2=0,y1=0,y2=0,min,r;
  
  draw_inv_box(mouse,oc,buffer);

  for(n=2;mouse[n]!=';';n++) x1=x1*10+mouse[n]-48;
  for(n++;mouse[n]!='m';n++) y1=y1*10+mouse[n]-48;
  for(n=2;oc[n]!=';';n++) x2=x2*10+oc[n]-48;
  for(n++;oc[n]!=';';n++) y2=y2*10+oc[n]-48;
  if(x1>x2) { n=x1; x1=x2; x2=n;}
  if(y1>y2) { n=y1; y1=y2; y2=n;}
  min=((x2-x1)<(y2-y1)?(x2-x1):(y2-y1));
  if(min>99) r=20;
  else if(min>49) r=10;
  else r=min/5;
  n=sprintf(buffer,"\33:%d;%dm\33:%d;%d;%d;%d;%da\33:%d;%d;%d;%dd\33:%d;%d;%d;%d;%da\33:%d;%d;%d;%dd\33:%d;%d;%d;%d;%da\33:%d;%d;%d;%dd\33:%d;%d;%d;%d;%da\33:%d;%d;%d;%dd",
    x2,y2-r,x2-r,y2-r,2*r,pattern,color,x1+r,y2,pattern,color,
    x1+r,y2-r,2*r,pattern,color,x1,y1+r,
    pattern,color,x1+r,y1+r,2*r,pattern,color,
    x2-r,y1,pattern,color,x2-r,y1+r,2*r,pattern,color,x2,y2-r,pattern,color);
  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,buffer,n);
  sput(buffer);
  if(cross) write(paper,cross_buffer,cross_blen);
  cput(1);
}




circle()
{ register char oc[32];
  register int x1=0,x2=0,y1=0,y2=0;
    
  draw_inv_box(mouse,oc,buffer);

  for(n=2;mouse[n]!=';';n++) x1=x1*10+mouse[n]-48;
  for(n++;mouse[n]!='m';n++) y1=y1*10+mouse[n]-48;
  for(n=2;oc[n]!=';';n++) x2=x2*10+oc[n]-48;
  for(n++;oc[n]!=';';n++) y2=y2*10+oc[n]-48;
  n=sprintf(buffer,"\33:%d;%dm\33:%d;%d;%d;%d;%da"
          ,x1,(y1+y2)/2
	  ,(x1+x2)/2,(y1+y2)/2,abs(x1-x2)*4,pattern,color);
  if(cross) write(paper,cross_buffer,cross_blen);
  write(paper,buffer,n);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(buffer);
  cput(1);
}




text()
{ int font_height=0,font_width=0,y=0,x=0,xo;
  char *text,txt[1024],c;

  if(cross) write(paper,cross_buffer,cross_blen);
  window_open();
  text= txt;
  /*
  	READ FONT TYPE FROM USER
  */

  n=sprintf(buffer,"%s",msg[0][language]);
  write(window,buffer,n);
  n=read(window,buffer,4);
  if(buffer[0]>='a' && buffer[0]<='z') buffer[0]=buffer[0]-'a'+'A';
  if(buffer[0]>64 && buffer[0]<91)
  { 
    /*
  	SET FONT ON PAPER
    */
    buffer[3]=0;
    buffer[2]=buffer[0];
    buffer[1]='(';
    buffer[0]='\33';
    write(paper,buffer,3);
    sput(buffer);
    /*
    	GET SELECTED FONT SIZE
    */
    strcpy(buffer,"\33:3n");
    write(paper,buffer,4);
    read(paper,buffer,64);
    for(n=2;buffer[n]!=';';n++);
    for(n++;buffer[n]!=';';n++);
    for(n++;buffer[n]!=';';n++) font_width=font_width*10+buffer[n]-48;
    for(n++;buffer[n]!=';';n++) font_height=font_height*10+buffer[n]-48;
    /*
    	READ TEXT FROM USER
    */
    write(window,msg[1][language],strlen(msg[1][language]));
    n=read(window,text,1024);
    text[n-1]=0;
    /*
    	SELECT BACKGROUND (BLACK OR WHITE)
    */
    if(pattern==0 && color==1) n=7; else n=0;
    n=sprintf(buffer,"\33[%dm",n);
    write(paper,buffer,n);
    buffer[n]=0;
    sput(buffer);
    /*
    	PUT TEXT ON PAPER
    */
    for(n=2;mouse[n]!=';';n++) x=x*10+mouse[n]-48;
    for(n++;mouse[n]!='m';n++) y=y*10+mouse[n]-48;
    if(y+font_height>ymax) y=ymax-font_height+1;
    if(x+font_width>xmax) x=xmax-font_width+1;
    xo=x;
    while(y>=0 && *text!=0 && x+font_width<=xmax+1)
    { n=sprintf(buffer,"\33:%d;%dm\33:1H",xo,y);
      write(paper,buffer,n);
      buffer[n]=0;
      sput(buffer);
      while(x+font_width<=xmax+1 && *text!=0)
      {	write(paper,text,1);
	cput(*text++);
	x+=font_width;
      }
      x=xo;
      y-=font_height;
    }
    cput(1);
  }
  window_close();
  if(cross) write(paper,cross_buffer,cross_blen);
}




copy()
{ register char oc[32];
  register int x1=0,y1=0,x2=0,y2=0,m;
  int dx,dy,x3,y3,xtest,ytest;
  
  draw_inv_box(mouse,oc,buffer);

  for(n=2;mouse[n]!=';';n++) x1=x1*10+mouse[n]-48;
  for(n++;mouse[n]!='m';n++) y1=y1*10+mouse[n]-48;
  for(n=2;oc[n]!=';';n++) x2=x2*10+oc[n]-48;
  for(n++;oc[n]!=';';n++) y2=y2*10+oc[n]-48;
  dx=x2-x1;
  dy=y2-y1;
  if(dx==0 && dy==0) return;
  m=sprintf(buffer,"\33:%d;%dm\33:%d;%di\33:%d;%di\33:%d;%di\33:%d;%di",
    x2,y2,x1,y2,x1,y1,x2,y1,x2,y2);
  write(paper,buffer,m);

  do
  { write(paper,pos_req,4);
    n=read(paper,oc,32)-2;
  } while(*oc!='\33');
  write(paper,buffer,m);
  if(cross)
  { write(paper,cross_buffer,cross_blen);
    cross_blen=draw_inv_cross(oc,cross_buffer);
  }
  while(oc[n]!='1')
  { x3=y3=0;
    for(n=2;oc[n]!=';';n++) x3=x3*10+oc[n]-48;
    for(n++;oc[n]!=';';n++) y3=y3*10+oc[n]-48;
    if(cross_blen!=0) xtest=ytest=1; else xtest=ytest=0;
    if(x3-dx>xmax) { x3=xmax+dx; xtest=0;}
    if(x3-dx<0000) { x3=dx; xtest=0;}
    if(y3-dy>ymax) { y3=ymax+dy; ytest=0;}
    if(y3-dy<0000) { y3=dy; ytest=0;}
    m=sprintf(buffer,"\33:%d;%dm\33:%d;%d%c\33:%d;%di\33:%d;%di\33:%d;%d%c",
      x3,y3,x3-dx,y3,(ytest ? 'm':'i'),x3-dx,y3-dy,
      x3,y3-dy,x3,y3,(xtest ? 'm':'i'));
    write(paper,buffer,m);
    do
    { write(paper,pos_req,4);
      n=read(paper,oc,32)-2;
    } while(*oc!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(oc,cross_buffer);
    }
    write(paper,buffer,m);
  }    

  if(x1<x2) { x3-=dx; x2=x1;}
  if(y1<y2) { y3-=dy; y2=y1;}
    
  if(cross) write(paper,cross_buffer,cross_blen);
  m=sprintf(buffer,"\33:%d;%d;%d;%d;%d;%d;0r",
     x2,y2,x3,y3,abs(dx)+1,abs(dy)+1);
  write(paper,buffer,m);
  if(cross) write(paper,cross_buffer,cross_blen);

  buffer[m]=0;
  sput(buffer);
  cput(1);
  while(oc[n]=='1')
  { do
    { write(paper,pos_req,4);
      n=read(paper,oc,32)-2;
    } while(*oc!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(oc,cross_buffer);
    }
  }
}




paint()
{ char pix_stat;
  int x,y;

  if(icon_mode)
  { mouse[mouseln]=0;
    sscanf(mouse,"\33:%d;%d",&x,&y);
    if(x==0) x=1;
    if(x==xmax) x=xmax-1;
    if(y==0) y=1;
    if(y==ymax) y=ymax-1;
    mouseln=sprintf(mouse,"\33:%d;%d;0P",x,y)-2;
  }
  mouse[mouseln-1]=';';
  for(n=0;n<mouseln;n++) buffer[n]=mouse[n];
  if(pattern/10!=0) mouse[mouseln++]=pattern/10+48;
  mouse[mouseln++]=pattern%10+48;
  mouse[mouseln++]=';';
  mouse[mouseln++]=color+48;
  mouse[mouseln++]=';';
  mouse[mouseln++]='0';
  mouse[mouseln++]='F';
  mouse[mouseln]=0;
  if(cross) write(paper,cross_buffer,cross_blen);
  if(icon_mode){
    buffer[n++]='1';
    buffer[n++]='0';
    buffer[n++]=';';
    buffer[n++]='0';
    buffer[n++]='p';
    write(paper,buffer,n);
    n=read(paper,buffer,32)-2;
    pix_stat=(buffer[n]=='0' ? '1':'0');
    n=sprintf(buffer,"\33:0;0m\33:0;%d;0;%cd\33:%d;%d;0;%cd\33:%d;0;0;%cd\33:0;0;0;%cd",ymax,pix_stat,xmax,ymax,pix_stat,xmax,pix_stat,pix_stat);
    write(paper,buffer,n);
    buffer[n]=0;
    sput(buffer);
  }
  write(paper,mouse,mouseln);
  if(cross) write(paper,cross_buffer,cross_blen);
  sput(mouse);
  cput(1);
 
  do
  { do
    { write(paper,pos_req,4);
      n=read(paper,buffer,32)-2;
    } while(*buffer!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(buffer,cross_buffer);
    }
  } while(buffer[n]=='1');
}
