/*	others.c	(C)	MITHRIL MJUKVARA HB 	1985

	This file contains the procedures:

	get_language(),init_storage(), cput(c), sput(s), draw_inv_box(c,oc,b),
	draw_box(s,d,b,pattern,color), draw_inv_cross(c,buffer),
	movetofile(t) and movefromfile(t,i)

        MODIFIED:       November      1985
                                Swedish or English texts selectable
				for icons and messages.

*/

extern int paper,window,xmax,ymax,saved,icon_mode;
extern int command,language;
extern char cross_buffer[],cross,cross_blen,font,*msg[][2];
extern char pos_req[],strxmax[],strymax[],coords[];

static char buffer[513];
static int n;

char *storage[1024];
int storagesize=0,storagepnt=0;

get_language()
{
    char *l,*getenv();
    if(!(l=getenv("LANGUAGE")))l="english";

    if(!strcmp(l,"english")){
	language=0;
	font='I';
    }
    else{
	language=1;
	font='H';
    }
}

initstorage()
{ int i;
  for(i=0;i<1024;i++) storage[i]=0;
}

mal_error()
{ int wopened=window;

  if(!wopened) window_open();
  write(window,msg[7][language],strlen(msg[7][language]));
  sleep(5);
  if(!wopened) window_close();
}

cput(c)
char c;
{ if(storage[storagepnt/512]==0)
    if((storage[storagepnt/512]=(char *)malloc(512))==0)
    { mal_error();
      return;
    }
  storage[storagepnt/512][storagepnt%512]=c;
  storagepnt++;
  storagesize=storagepnt;
}

sput(s)
char *s;
{ while(*s!=0)
  { if(storage[storagepnt/512]==0)
      if((storage[storagepnt/512]=(char *)malloc(512))==0)
      { mal_error();
        return;
      }
    storage[storagepnt/512][storagepnt%512]=(*s++);
    storagepnt++;
  }
  storagesize=storagepnt;
}

draw_inv_box(c,oc,b)
char *c,*oc,*b;
{ register int n,i,j,ind;
  register char nc[32];
  short int x1,x2,y1,y2;

  do
  { write(paper,pos_req,4);
    n=read(paper,nc,32)-2;
  } while(*nc!='\33');
  for(i=0;(oc[i]=nc[i])!='P';i++);
  if(cross)
  { write(paper,cross_buffer,cross_blen);
    cross_blen=draw_inv_cross(nc,cross_buffer);
  }
  while(nc[n]=='1')
  { if(command==7)
    { x1=x2=y1=y2=0;

      for(i=2;c[i]!=';';i++) x1=x1*10+c[i]-48;
      for(i++;c[i]!='m';i++) y1=y1*10+c[i]-48;
      for(i=2;nc[i]!=';';i++) x2=x2*10+nc[i]-48;
      for(i++;nc[i]!=';';i++) y2=y2*10+nc[i]-48;

      ind=abs(x1-x2)-abs(y1-y2);
      if(ind>0) x2+=(x2>x1 ? -ind : ind);
      else if(ind<0) y2+=(y2>y1 ? ind : -ind);
      sprintf(nc,"\33:%d;%d;1P",x2,y2);
    }

    for(n=0;(oc[n]=nc[n])!='P';n++);

    i=j=0;

    while(b[j++]=c[i++]); j--;

    for(i=0;c[i]!=';';i++) b[j++]=c[i];
    b[j++]=';';
    for(i=3;nc[i++]!=';';);
    for(;nc[i]!=';';i++) b[j++]=nc[i];
    b[j++]='i';

    for(i=0;(b[j++]=nc[i++])!=';';);
    while((b[j]=nc[i++])!=';') j++;
    b[j++]= (cross_blen!=0 ? (ind<0 && command==7 ? 'i' : 'm') : 'i');

    for(i=0;nc[i]!=';';i++) b[j++]=nc[i];
    b[j++]=';';
    for(i=3;c[i++]!=';';);
    for(;c[i]!='m';i++) b[j++]=c[i];
    b[j++]= (cross_blen!=0 ? (ind>0 && command==7 ? 'i' : 'm') : 'i');

    for(i=0;(b[j++]=c[i++])!=';';);
    while((b[j]=c[i++])!='m') j++;
    b[j++]='i';

    b[j]=0;
    
    write(paper,b,j);

    do
    { write(paper,pos_req,4);
      n=read(paper,nc,32)-2;
    } while(*nc!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(nc,cross_buffer);
    }
    write(paper,b,j);
  }
}

draw_box(s,d,b,pattern,color)
register char *s,*d,*b;
int pattern,color;
{ register i=0,j=0;

  while(b[j++]=s[i++]);
  j--;

  for(i=0;s[i]!=';';i++) b[j++]=s[i];
  b[j++]=';';
  for(i=3;d[i++]!=';';);
  for(;d[i]!=';';i++) b[j++]=d[i];
  b[j++]=';';
  if(pattern/10!=0) b[j++]=pattern/10+48;
  b[j++]=pattern%10+48;
  b[j++]=';';
  b[j++]=color+48;
  b[j++]='d';

  for(i=0;(b[j++]=d[i++])!=';';);
  while((b[j]=d[i++])!=';') j++;
  b[j++]=';';
  if(pattern/10!=0) b[j++]=pattern/10+48;
  b[j++]=pattern%10+48;
  b[j++]=';';
  b[j++]=color+48;
  b[j++]='d';

  for(i=0;d[i]!=';';i++) b[j++]=d[i];
  b[j++]=';';
  for(i=3;s[i++]!=';';);
  for(;s[i]!='m';i++) b[j++]=s[i];
  b[j++]=';';
  if(pattern/10!=0) b[j++]=pattern/10+48;
  b[j++]=pattern%10+48;
  b[j++]=';';
  b[j++]=color+48;
  b[j++]='d';

  for(i=0;(b[j++]=s[i++])!=';';);
  while((b[j]=s[i++])!='m') j++;
  b[j++]=';';
  if(pattern/10!=0) b[j++]=pattern/10+48;
  b[j++]=pattern%10+48;
  b[j++]=';';
  b[j++]=color+48;
  b[j++]='d';

  b[j]=0;

  write(paper,b,j);
}

int draw_inv_cross(c,buffer)
register char *c,*buffer;
{ register n,j=0,i,k=14,cx,cy;

  switch(cross)
  { case 0: break;
    case 1: for(n=0;(buffer[j++]=c[n])!=';';n++);
	    buffer[j++]='0';
	    buffer[j++]='m';

	    for(n=0;(buffer[j++]=c[n])!=';';n++);
	    buffer[j++]= *strymax;
	    buffer[j++]= *(strymax+1);
	    buffer[j++]= *(strymax+2);
	    buffer[j++]='i';

	    for(n=2;c[n++]!=';';);
	    buffer[j++]='';
	    buffer[j++]=':';
	    buffer[j++]='0';
	    buffer[j++]=';';
	    while((buffer[j]=c[n++])!=';') j++;
	    buffer[j++]='m';

	    for(n=2;c[n++]!=';';);
	    buffer[j++]='';
	    buffer[j++]=':';
	    buffer[j++]= *strxmax;
	    buffer[j++]= *(strxmax+1);
	    buffer[j++]= *(strxmax+2);
	    buffer[j++]=';';
	    while((buffer[j]=c[n++])!=';') j++;
	    buffer[j++]='i';
	    buffer[j]=0;
	    write(paper,buffer,j);
	    break;
    case 2: for(n=0;c[n]!=';';n++);
            cx=n;
            for(i=0;i<(6-cx);i++) coords[k++]=' ';
            for(n=2;(coords[k++]=c[n++])!=';';);
            while(c[n++]!=';');
            k--;
            cx++;
            cy=n;
            for(i=0;i<(5+cx-cy);i++) coords[k++]=' ';
            for(i=cx;i<(cy-1);i++) coords[k++]=c[i];
	    write(1,coords,k);
	    j=0;
            break;
    case 3: for(n=0;(buffer[j++]=c[n])!=';';n++);
	    buffer[j++]='0';
	    buffer[j++]='m';

	    for(n=0;(buffer[j++]=c[n])!=';';n++);
	    buffer[j++]= *strymax;
	    buffer[j++]= *(strymax+1);
	    buffer[j++]= *(strymax+2);
	    buffer[j++]='i';

	    cx=n;
	    for(i=0;i<(6-cx);i++) coords[k++]=' ';
	    for(n=2;(coords[k++]=c[n++])!=';';);
	    buffer[j++]='';
	    buffer[j++]=':';
	    buffer[j++]='0';
	    buffer[j++]=';';
	    while((buffer[j]=c[n++])!=';') j++;
	    buffer[j++]='m';

	    k--;
	    cy=n;
	    for(n=2;c[n++]!=';';);
	    buffer[j++]='';
	    buffer[j++]=':';
	    buffer[j++]= *strxmax;
	    buffer[j++]= *(strxmax+1);
	    buffer[j++]= *(strxmax+2);
	    buffer[j++]=';';
	    while((buffer[j]=c[n++])!=';') j++;
	    buffer[j++]='i';
	    buffer[j]=0;
	    cx++;
	    for(i=0;i<(5+cx-cy);i++) coords[k++]=' ';
	    for(i=cx;i<(cy-1);i++) coords[k++]=c[i];
	    write(1,coords,k);
	    write(paper,buffer,j);
	    break;
  }
  return j;
}

movetofile(t)
char *t;
{ register int i,n;
  register int fd;

  if((fd=creat(t,0644))>0)
  { if(icon_mode) write(fd,"\33:2l",4); else write(fd,"\33:2h",4);
    n=sprintf(buffer,"\33:%d;%dm",xmax,ymax);
    write(fd,buffer,n);
    for(i=0;i < storagepnt/512;write(fd,storage[i++],512) )  ;
    write(fd,storage[i],storagepnt%512);
    saved= -1;
  }
  else
  { 
    write(window,msg[4][language],strlen(msg[4][language]));
    sleep(5);
  }
  close(fd);
}

movefromfile(t,i)
char *t;
int i;
{ register int n,j;
  register int fd;

  storagesize=storagepnt=i;
  if((fd=open(t,0))>0)
  { if(i==0)
    { n=sprintf(buffer,"\33:0;0m\33:%d;%d;0;0f",xmax,ymax);
      write(paper,buffer,n);
    }
    do
    { j=read(fd,buffer,1);
    } while(j==1 && *buffer!='m');
    if((n=storagesize%512)!=0)
    { j=read(fd,buffer,512-n);
      buffer[j]=0;
      sput(buffer);
      write(paper,buffer,j);
    }
    j=512;
    for(n=storagesize/512;j==512;storagesize+=j)
    { if(storage[n]==0)
        if((storage[n]=(char *)malloc(512))==0)
	{ mal_error();
	  return;
	}
      j=read(fd,storage[n],512);
      write(paper,storage[n++],j);
    }
    storagepnt=storagesize;
    /*
    	CHECK IF FILE TERMINATES WITH CTRL-A. IF NOT ADD IT TO THE END.
    */
    if (storage[(storagepnt-1)/512][(storagepnt-1)%512]!=0x01) cput(1);

    saved= i!=0;
  }
  else
  { 
    write(window,msg[8][language],strlen(msg[8][language]));
    sleep(5);
  }
}
