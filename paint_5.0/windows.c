/*	windows.c	(C)	MITHRIL MJUKVARA HB	1985

	WRITTEN BY:	Tony Olsson 
			Magnus Ericson 
			Peter AAberg

	FINISHED:	September 30, 1985

	This file contains main(), global declarations and the procedures:

        main_window(), icon_control(), paper_window(),

	mkicon(...), command_icons(), color_icons(), window_open(),

	window_close(), change_mouse_pointer() and iconedit();

        MODIFIED:       November      1985
				Swedish or English texts selectable
				for icons and messages

			December      1985
				Bug fixed - No hangup's.
				Spelling of Wincreat fixed.
				Inclusion of keyboard.h changed to
				abc1600/keybord.h

*/

#include <win/w_macros.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#include <abc1600/keyboard.h>
#include <dnix/fcodes.h>

#define not !

extern char buffer[];
extern int n,storagepnt;

char *conv_name = "/usr/bin/converter";
char *pdriv_name = "/usr/bin/pdriver";
char *prname = "/dev/lp";

int paper=0,window=0,xmax,ymax,icon_mode=0;

char mouse[32],mouseln,font='I';
char cross_buffer[128],cross=0,cross_blen=0;
char pos_req[5],strxmax[4],strymax[4],coords[21],mode;
int color=1,pattern=0,command=1,saved=1,language;
char buffer[512];
int  n;

char *msg[][2]={
    {"\n  Select font (A-Z): ",					/* 0 */
     "\n  V{lj font (A-Z): "},
    {"\33c\n  Enter text, end with RETURN:\n\n  ",		/* 1 */
     "\33c\n  Mata in text, avsluta med RETURN:\n\n  "},
    {"\n  The picture hasn't been saved.\n  Do you want to proceed anyway (y/n) ? ",								/* 2 */
     "\n  Bilden {r inte sparad.\n  Vill du forts{tta {nd} (j/n) ? "},
    {"\33c\n  Filename ? ",					/* 3 */
     "\33c\n  Filnamn ? "},
    {"\33[?35h\n  Can't create file.\n",			/* 4 */
     "\33[?35h\n  Kan ej skapa filen.\n"},
    {"\33[?35h\n  Dumping the picture to memory.",		/* 5 */
     "\33c\33[?35h\n  \\verf|r bilden till minnet."},
    {"\33c\33[?35h\n  Printing the picture.",			/* 6 */
     "\33c\33[?35h\n  Skriver ut bilden."},
    {"\33[?35h\n  Memory full, last commands lost.\n  You better try to save the picture !",							/* 7 */
     "\33[?35h\n  Minnet fullt, senaste kommandona f|rlorade.\n  Du b|r f|rs|ka spara bilden !"},
    {"\33[?35h\n  Can't open file.",				/* 8 */
     "\33[?35h\n  Kan ej |ppna file."},
    {"PENCIL","PENNA"},
    {"LINE","LINJE"},						/* 10 */
    {"BOX","RUTA"},
    {"FILL BOX","FYLLD RUTA"},
    {"PAINT BOX","M]LAD RUTA"},
    {"FRAME","RAM"},
    {"CIRCLE","CIRKEL"},					/* 15 */
    {"TEXT","TEXT"},
    {"COPY","KOPIERA"},
    {"PAINT","M]LA"},
    {"EDIT ICON","IKON EDIT"},
    {"UNDO","]NGRA"},						/* 20 */
    {"REDO","]TERSKAPA"},
    {"NEW","NY"},
    {"SAVE","SPARA"},
    {"LOAD","LADDA"},
    {"MERGE","\\VERLAGRA"},					/* 25 */
    {"PRINT","SKRIV UT"},
    {"EXIT","AVSLUTA"},
    {"\n  Do you want a printable copy on file (y/n) ? ",	/* 28 */
     "\n  Vill du lagra bilden i printbart format (j/n) ? "},
    {"\n  The picture hasn't been saved.\n  Do you want to exit anyway (y/n) ? ",								/* 29 */
     "\n  Bilden {r inte sparad.\n  Vill du avsluta programet {nd} (j/n) ? "},
    {"\33[?35h\n  Can't print picture.",			/* 30 */
    "\33[?35h\n  Kan ej skriva ut bilden."}
};


static struct sgttyb tty;
static struct headstruc hd;
static int main_fd;

/*
	MAIN WINDOW
*/

static main_window()
{ struct winstruc mwin;

  main_fd=open("/win",2);

/*
	CHANGE STANDARD INPUT AND OUTPUT TO MAIN WINDOW
*/
	
  close(0);
  close(1);
  dup(main_fd);
  dup(main_fd);

  mwin.wp_xorig=23;
  mwin.wp_yorig=23;
  mwin.wp_xsize=722;
  mwin.wp_ysize=978;
  mwin.wp_vxorig=0;
  mwin.wp_vyorig=0;
  mwin.wp_vxsize=722;
  mwin.wp_vysize=978;
  mwin.wl_xorig=23;
  mwin.wl_yorig=23;
  mwin.wl_xsize=978;
  mwin.wl_ysize=722;
  mwin.wl_vxorig=0;
  mwin.wl_vyorig=0;
  mwin.wl_vxsize=978;
  mwin.wl_vysize=722;
  mwin.w_color=WHITE;
  mwin.w_border=DLBORDER;
  mwin.wp_font=font;
  mwin.wl_font=font;
  mwin.w_uboxes=0;
  mwin.w_xcur=1;
  mwin.w_ycur=1;
  mwin.w_xgcur=0;
  mwin.w_ygcur=0;
  mwin.w_tsig=0;
  mwin.w_ntsig=0;
  mwin.w_rsig=0;
  mwin.w_csig=0;
  mwin.w_flags=PMODE | LMODE | NOCURSOR | ALTMPNT;
  mwin.w_boxes=0;
  mwin.w_pad.w_xxx=0;
  Wincreat(main_fd,&mwin);
  if(mwin.w_rstat!=W_OK)exit(-1);
}






static icon_control()
{ char c[30];
  int n,new_com;

  /*
	DISABLE ICON SELECTION
  */	
  signal(SIGUSR1,SIG_IGN);

  /*
	READ ICON SEQUENCE
  */
  n=read(0,c,30);

  /*
	CHECK IF ICON IS COMMAND (I) OR COLOR ICON (C). RETURN COMMAND
	IN new_com, COMMAND ICON POSITIVE AND COLOR ICON NEGATIVE
  */
  if(*c=='I')
    new_com=  (c[1]-48)*10+c[2]-48;
  else
    new_com= -((c[1]!=' ' ? c[1]-48 : 0)*10+c[2]-48);

  /*
	CHECK IF COMMAND ICON IN LOWER HALF (ICON_EDIT .. EXIT) OR
	COLOR ICONS
  */
  if(new_com<1 || new_com>10)
  { if(cross) write(paper,cross_buffer,cross_blen);

    /*
    	CHECK IF COMMAND ICON AND THEN INVERT SELECTED ICON
    */
    if(new_com>0)
      if(mode=='P')
        printf("\33:7;%d;7;%d;96;36;1r",976-45*new_com,976-45*new_com);
      else
        printf("\33:7;%d;7;%d;96;29;1r",718-37*new_com,718-37*new_com);

    /*
    	PROCESS SELECTED ICON
    */
    switch(new_com)
    { case 11: iconedit();break;
      case 12: undo();break;
      case 13: redo();break;
      case 14: new();break;
      case 15: save();break;
      case 16: load();break;
      case 17: merge();break;
      case 18: print();break;
      case 19: stop_program();break;

      /*
      	UPDATE SELECTED COLOR
      */
      default: color= (-new_com)/16;
               pattern=(-new_com)%16;
	       if(mode=='P')
               printf("\33:22;22m\33:89;89;0;0f\33:22;22m\33:89;89;%d;%df"
	              ,pattern,color);
               else
               printf("\33:182;15m\33:249;82;0;0f\33:182;15m\33:249;82;%d;%df"
	              ,pattern,color);
	       break;
    }

    /*
      	WARNING: AD HOC

	DRAW BOX IN ICON MODE FOR PROPER LOOK
    */
    if(icon_mode){
        n=sprintf(buffer,"\33:0;0m\33:0;%d;0;1d\33:%d;%d;0;1d\33:%d;0;0;1d\33:0;0;0;1d",ymax,xmax,ymax,xmax);
    write(paper,buffer,n);
    }


    /*
    	INVERT SELECTED ICON (LOWER PART)
    */
    if(new_com>0)
      if(mode=='P')
        printf("\33:7;%d;7;%d;96;36;1r",976-45*new_com,976-45*new_com);
      else
        printf("\33:7;%d;7;%d;96;29;1r",718-37*new_com,718-37*new_com);
    if(cross) write(paper,cross_buffer,cross_blen);
  }
  else
  { /*
  	INVERT PREVIOUS ICON (UPPER PART)
    */
    if(mode=='P')
      printf("\33:7;%d;7;%d;96;36;1r",980-45*command,980-45*command);
    else
      printf("\33:7;%d;7;%d;96;29;1r",722-37*command,722-37*command);

    /*
    	TOGGLE FOR CROSS AND TRACE. cross=0 => NO CROSS  NO TRACE
				    cross=1 =>    CROSS  NO TRACE
				    cross=2 => NO CROSS     TRACE
				    cross=3 =>    CROSS     TRACE
    */
    if(command==new_com)
    { if(cross==1 || cross==3) write(paper,cross_buffer,cross_blen);
      cross_blen=0;
      cross=(cross+1)%4;
      if(mode=='P')
        write(1,"\33:373;95m\33:450;116;1;1f",24);
      else
	write(1,"\33:505;89m\33:582;108;1;1f",24);
      if(cross==2 || cross==3)
        if(mode=='P')
	  write(1,"\33:373;96m\33:450;115;0;1f",24);
        else
	  write(1,"\33:505;89m\33:582;108;0;1f",24);
    }      

    /*
    	SET command TO SELECTED COMMAND
    */
    command=new_com;

    /*
     	INVERT SELECTED ICON (UPPER PART)
    */
    if(mode=='P')
      printf("\33:7;%d;7;%d;96;36;1r",980-45*command,980-45*command);
    else
      printf("\33:7;%d;7;%d;96;29;1r",722-37*command,722-37*command);
  }

  /*
  	ENABLE ICON SELECTION
  */
  signal(SIGUSR1,icon_control);
}





paper_window()
{ struct winstruc pwin;

  paper=open("/win",2);

  /*
  	CREATE STRINGS FOR ymax AND xmax FOR LATER USE IN draw_inv_cross
  */
  sprintf(strxmax,"%3d",xmax);
  sprintf(strymax,"%3d",ymax);
  
  /*
  	CLEAR BACKGROUND
  */
  if(mode=='P') printf("\33:109;117m\33:716;972;1;1f");
  else printf("\33:117;109m\33:972;716;1;1f");

  pwin.wp_xorig=136+(599-xmax)/2;
  pwin.wp_yorig=145+(847-ymax)/2;
  pwin.wp_xsize=(xmax<41 ? 42 : xmax+1);
  pwin.wp_ysize=(ymax<41 ? 42 : ymax+1);
  pwin.wp_vxorig=0;
  pwin.wp_vyorig=0;
  pwin.wp_vxsize=xmax+1;
  pwin.wp_vysize=ymax+1;
  pwin.wl_xorig=144+(847-xmax)/2;
  pwin.wl_yorig=137+(599-ymax)/2;
  pwin.wl_xsize=(xmax<41 ? 42 : xmax+1);
  pwin.wl_ysize=(ymax<41 ? 42 : ymax+1);
  pwin.wl_vxorig=0;
  pwin.wl_vyorig=0;
  pwin.wl_vxsize=xmax+1;
  pwin.wl_vysize=ymax+1;
  pwin.w_color=WHITE;
  pwin.w_border=NOBORDER;
  pwin.wp_font=font;
  pwin.wl_font=font;
  pwin.w_uboxes=0;
  pwin.w_xcur=1;
  pwin.w_ycur=1;
  pwin.w_xgcur=0;
  pwin.w_ygcur=0;
  pwin.w_tsig=0;
  pwin.w_ntsig=0;
  pwin.w_rsig=0;
  pwin.w_csig=0;
  pwin.w_flags=PMODE | LMODE | NOCURSOR | ALTMPNT | LOCK;
  pwin.w_boxes=0;
  pwin.w_pad.w_xxx=0;  
  /*
  	SET UP BORDER FOR PAPER
  */
  if(mode=='P')
    printf("\33:%d;%dm\33:%d;%d;0;1f",
    		pwin.wp_xorig-26,
		pwin.wp_yorig-26,
		pwin.wp_xorig+pwin.wp_vxsize-21,
		pwin.wp_yorig+pwin.wp_vysize-21);
  else
    printf("\33:%d;%dm\33:%d;%d;0;1f",
    		pwin.wl_xorig-26,
		pwin.wl_yorig-26,
		pwin.wl_xorig+pwin.wl_vxsize-21,
		pwin.wl_yorig+pwin.wl_vysize-21);

  Wincreat(paper,&pwin);
  if(pwin.w_rstat!=W_OK)exit(-1);
  /*
  	SET PATTERN FITTING
  */
  write(paper,"\33:2h",4);
}





/*
	USED BY command_icons
*/
static mkicon(fd,xp,yp,xps,yps,xl,yl,xls,yls,c,t)
register int xp,yp,xps,yps,xl,yl,xls,yls;
char *c,*t;
int fd;
{ struct winicon in;

  in.ip_xorig=xp;
  in.ip_yorig=yp;
  in.ip_xsize=xps;
  in.ip_ysize=yps;
  in.il_xorig=xl;
  in.il_yorig=yl;
  in.il_xsize=xls;
  in.il_ysize=yls;
  strcpy(in.i_cmdseq,c);
  in.i_flags=I_PMODE | I_LMODE | I_PRESS;
  Winicon(fd,&in);

  /*
  	PRINT ICON (BLACK BACKGROUND AND WHITE TEXT)
  */
  if(mode=='P')
    printf("\33:%d;%dm\33:%d;%d;0;1f\33:%d;%dm\33:1H\33[7m%s",
          xp,yp,xp+xps-1,yp+yps-1,xp+(xps-9*strlen(t))/2-3,yp+yps/2-6,t);
  else
    printf("\33:%d;%dm\33:%d;%d;0;1f\33:%d;%dm\33:1H\33[7m%s",
          xl,yl,xl+xls-1,yl+yls-1,xl+(xls-9*strlen(t))/2-3,yl+yls/2-6,t);
}


static command_icons()
{ register int i,j;

  i=978;j=720;
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I01",msg[9][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I02",msg[10][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I03",msg[11][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I04",msg[12][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I05",msg[13][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I06",msg[14][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I07",msg[15][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I08",msg[16][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I09",msg[17][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I10",msg[18][language]);
  mkicon(0,5,i-=49,100,40,5,j-=41,100,33,"I11",msg[19][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I12",msg[20][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I13",msg[21][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I14",msg[22][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I15",msg[23][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I16",msg[24][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I17",msg[25][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I18",msg[26][language]);
  mkicon(0,5,i-=45,100,40,5,j-=37,100,33,"I19",msg[27][language]);

  /*
  	INVERT PENCIL ICON (INITIALY SELECTED)
  */
  if(mode=='P')
    printf("\33:7;%d;7;%d;96;36;1r",980-45*command,980-45*command);
  else
    printf("\33:7;%d;7;%d;96;29;1r",722-37*command,722-37*command);

  printf("\33[0m");
}





static color_icons()
{ register int i,j;
  struct winicon ip;

  /*
  	SET INITAL SELECTED PATTERN (LARGE BOX)
  */	
  if(mode=='P')
  { printf("\33:20;20m\33:91;91;0;1f");
    printf("\33:110;20m\33:713;91;1;1f");
    i=110;
  }
  else
  { printf("\33:180;13m\33:251;84;0;1f");
    printf("\33:270;12m\33:873;84;1;1f");
    i=270;
  }

  ip.ip_xsize=34;
  ip.ip_ysize=34;
  ip.il_xsize=34;
  ip.il_ysize=34;
  ip.i_flags=I_PMODE | I_LMODE | I_PRESS;
  if(mode=='P')
    /*
    	MAKE ICON AND PRINT PATTERN
    */
    for(j=0;j<=15;j++)
    { ip.ip_xorig=i;
      ip.ip_yorig=20;
      ip.il_xorig=i;
      ip.il_yorig=20;
      sprintf(ip.i_cmdseq,"C%2u",j);
      Winicon(0,&ip);
      printf("\33:%d;20m\33:%d;53;0;0f\33:%d;21m\33:%d;52;%d;0f"
	   ,i,i+33,i+1,i+32,j);
      ip.ip_yorig=58;
      ip.il_yorig=58;
      sprintf(ip.i_cmdseq,"C%2u",j+16);
      Winicon(0,&ip);
      printf("\33:%d;58m\33:%d;91;0;0f\33:%d;59m\33:%d;90;%d;1f"
              ,i,i+33,i+1,i+32,j);
      i+=38;
    }
  else
    for(j=0;j<=15;j++)
    { ip.ip_xorig=i;
      ip.ip_yorig=13;
      ip.il_xorig=i;
      ip.il_yorig=13;
      sprintf(ip.i_cmdseq,"C%2u",j);
      Winicon(0,&ip);
      printf("\33:%d;13m\33:%d;46;0;0f\33:%d;14m\33:%d;45;%d;0f"
             ,i,i+33,i+1,i+32,j);
      ip.ip_yorig=51;
      ip.il_yorig=51;
      sprintf(ip.i_cmdseq,"C%2u",j+16);
      Winicon(0,&ip);
      printf("\33:%d;51m\33:%d;84;0;0f\33:%d;52m\33:%d;83;%d;1f"
             ,i,i+33,i+1,i+32,j);
      i+=38;
    }
}





window_open()
{ struct winstruc wwin;
  struct flgstruc pwin;

  window=open("/win",2);

  /*
  	CLEAR BACKGROUND
  */
  if(mode=='P')
    printf("\33:110;20m\33:713;91;0;1f");
  else
    printf("\33:270;13m\33:873;84;0;1f");

  wwin.wp_xorig=136;
  wwin.wp_yorig=46;
  wwin.wp_xsize=597;
  wwin.wp_ysize=65;
  wwin.wp_vxorig=0;
  wwin.wp_vyorig=0;
  wwin.wp_vxsize=597;
  wwin.wp_vysize=65;
  wwin.wl_xorig=296;
  wwin.wl_yorig=40;
  wwin.wl_xsize=597;
  wwin.wl_ysize=65;
  wwin.wl_vxorig=0;
  wwin.wl_vyorig=0;
  wwin.wl_vxsize=597;
  wwin.wl_vysize=65;
  wwin.w_color=WHITE;
  wwin.w_border=NOBORDER;
  wwin.wp_font=font;
  wwin.wl_font=font;
  wwin.w_uboxes=0;
  wwin.w_xcur=1;
  wwin.w_ycur=1;
  wwin.w_xgcur=0;
  wwin.w_ygcur=0;
  wwin.w_tsig=0;
  wwin.w_ntsig=0;
  wwin.w_rsig=0;
  wwin.w_csig=0;
  wwin.w_flags=PMODE | LMODE | LOCK;
  wwin.w_boxes=0;
  wwin.w_pad.w_xxx=0;
  /*
  	CHANGE PAPER MODE TO ENABLE NEW WINDOW ON TOP LEVEL
  */
  pwin.f_flags=PMODE | LMODE | NOCURSOR | ALTMPNT;
  Winflags(paper,&pwin);

  Wincreat(window,&wwin);
}





window_close()
{ struct flgstruc pwin;
  int i,j;
  
  /*
  	RESET PAPER TO TOP LEVEL
  */
  pwin.f_flags=PMODE | LMODE | NOCURSOR | ALTMPNT | LOCK;
  close(window);
  Winflags(paper,&pwin);
 
  /*
  	RESTORE PATTERN ICONS
  */
  if(mode=='P')
  { printf("\33:110;20m\33:713;91;1;1f");
    i=110;
    for(j=0;j<=15;j++)
    { printf("\33:%d;20m\33:%d;53;0;0f\33:%d;21m\33:%d;52;%d;0f"
  	   ,i,i+33,i+1,i+32,j);
      printf("\33:%d;58m\33:%d;91;0;0f\33:%d;59m\33:%d;90;%d;1f"
	   ,i,i+33,i+1,i+32,j);
      i+=38;
    }
  }
  else
  { printf("\33:270;12m\33:873;84;1;1f");
    i=270;
    for(j=0;j<=15;j++)
    { printf("\33:%d;13m\33:%d;46;0;0f\33:%d;14m\33:%d;45;%d;0f"
	   ,i,i+33,i+1,i+32,j);
      printf("\33:%d;51m\33:%d;84;0;0f\33:%d;52m\33:%d;83;%d;1f"
           ,i,i+33,i+1,i+32,j);
      i+=38;
    }
  }

  window=0;
}





static change_mouse_pointer()
{ struct npstruc nmp;
  int i;

  nmp.np_xsize  =15;
  nmp.np_ysize  =15;
  nmp.np_xpnt   =7;
  nmp.np_ypnt   =7;
  nmp.np_flags  =0;
  nmp.np_or[0]  =0x03800000;
  nmp.np_or[14] =0x03800000;
  nmp.np_and[0] =0xFFFFFFFF;
  nmp.np_and[14]=0xFFFFFFFF;
  for(i=1;i<14;i++)
  { nmp.np_or[i] =0x02800000;
    nmp.np_and[i]=0xFEFFFFFF;
  }
  nmp.np_or[6]  =0xFC7FFFFF;
  nmp.np_or[7]  =0x80020000;
  nmp.np_or[8]  =0xFC7FFFFF;
  nmp.np_and[6] =0xFC7FFFFF;
  nmp.np_and[7] =0x81020000;
  nmp.np_and[8] =0xFC7FFFFF;
  Winchmpnt(paper,&nmp);
}





static iconedit()
{ int tmp;

  /*
  	CHECK IF PICTURE SAVED. IF NOT, ASK IF TO PROCEED.
  */
  if(saved==0)
  { window_open();
    write(window,msg[2][language],strlen(msg[2][language]));
    read(window,buffer,4);
    if(*buffer!='Y' && *buffer!='y') goto HALT;
  }

  /*
  	RESET STORAGE POINTER (PICTURE HISTORY AREA)
  */
  storagepnt=0;

  if(icon_mode)
  { /*
	FROM ICON MODE TO NORMAL MODE.
    */
    xmax= (mode=='P' ? 599 : 847);
    ymax= (mode=='P' ? 847 : 599);
    icon_mode=0;

    strcpy(hd.h_hdr,"PAINT 1600");
    Winheader(0,&hd);
  }
  else
  { if(not window) window_open();

    /*
    	FROM NORMAL MODE TO ICON MODE

    	READ HEIGHT AND WIDTH
    */
    if(language){
	n=sprintf(buffer,"\33c\n  Ge ikon h|jd (42-%d) : ", mode=='P' ? 848 : 600);
    }
    else{
	n=sprintf(buffer,"\33c\n  Give icon height (42-%d) : ", mode=='P' ? 848 : 600);
    }
    write(window,buffer,n);
    n=read(window,buffer,32);
    if(sscanf(buffer,"%d",&tmp)==0) goto HALT;
    tmp--;
    if(mode=='P')
    { if(tmp<41 || tmp>847) goto HALT;
    }
    else
    { if(tmp<41 || tmp>599) goto HALT;
    }

    if(language){
	n=sprintf(buffer,"\33c\n  Ge ikon bredd  (42-%d) : ", mode=='P' ? 600 : 848);
    }
    else{
	n=sprintf(buffer,"\33c\n  Give icon width  (42-%d) : ", mode=='P' ? 600 : 848);
    }
    write(window,buffer,n);
    n=read(window,buffer,32);
    if(sscanf(buffer,"%d",&n)==0) goto HALT;
    n--;
    if(mode=='P')
    { if(n<41 || n>599) goto HALT;
    }
    else
    { if(n<41 || n>847) goto HALT;
    }

    xmax=n;
    ymax=tmp;
    icon_mode= -1;
    strcpy(hd.h_hdr,"PAINT 1600 - ICON MODE");
    Winheader(0,&hd);
  }

  if(window) window_close();
  /*
  	CLOSE OLD PAPER AND OPEN A NEW ONE
  */
  close(paper);
  paper_window();
  change_mouse_pointer();
  ioctl(paper,TIOCSETP,&tty);

  /*
  	SET PATTERN FITTING IF NORMAL MODE.
	RESET PATTERN FITTING IF ICON MODE.
	PRINT BOX IF ICON MODE.
  */
  if(icon_mode){
     n=sprintf(buffer,"\33:0;0m\33:0;%d;0;1d\33:%d;%d;0;1d\33:%d;0;0;1d\33:0;0;0;1d",ymax,xmax,ymax,xmax);
     write(paper,buffer,n);
     write(paper,"\33:2l",4);
  }
  else
  {  write(paper,"\33:2h",4);
  }
  saved= -1;	

HALT:
  if(window) window_close();
}



main(argc,argv)
int argc;
char *argv[];
{
  struct kysigst ks;
  char c,tmp1,tmp2;
  int i, no_window = 0;

  /*
  	TEST if -nw IS GIVEN OR ILLEGAL SWITCH/SWITCHES IS GIVEN
  */
  for (i = 1; i < argc; i++) {
    if (*argv[i] == '-')
	switch (argv[i][1]) {
	    case 'n':
		no_window = 1;
		break;
	    case 'c':
		if (argv[i][2] == '\0') {
		    conv_name = argv[++i];
		    if (i == argc)
			goto usage;
		    }
		else
		    conv_name = argv[i] + 2;
		break;
	    case 'p':
		if (argv[i][2] == '\0') {
		    pdriv_name = argv[++i];
		    if (i == argc)
			goto usage;
		    }
		else
		    pdriv_name = argv[i] + 2;
		break;
	    case 'd':
		if (argv[i][2] == '\0') {
		    prname = argv[++i];
		    if (i == argc)
			goto usage;
		    }
		else
		    prname = argv[i] + 2;
		break;
	    default:
usage:		fprintf(stderr,
	"Usage: paint [-n] [-d device]\n");
		exit(1);
	    }
    }
    /*
      GET LANGUAGE FROM ENV
    */
    get_language();
  
    /*
      OPEN WINDOW IF -nw IS NOT GIVEN
    */
    if(! no_window){
	setpgrp();
        main_window();
	}
    else{
	/*
	  SET FONT
	*/
	printf("\33(%c",font);
	}
  /*
  	INITIATE FOR ONE STEP INCREMENT (MOUSE)
  */
  tmp1=K_PREM;
  ptokbrd(3,&tmp1);
  tmp1=32;tmp2=33;
  for(i=0;i<4;i++){
      ptokbrd(3,&tmp1); 
      ptokbrd(3,&tmp2); 
  }
	
  /*
  	SET HEADER (MAIN WINDOW)
  */
  hd.h_flags=0;
  strcpy(hd.h_hdr,"PAINT 1600");
  Winheader(0,&hd);

  /*
	SET RAW MODE (MAIN WINDOW)
  */
  tty.sg_flags=RAW;
  ioctl(1,TIOCSETP,&tty);

  /*
  	SET SIGNAL SUPPORT FOR ICON HANDLING
  */
  ks.ks_sig=SIGUSR1;
  ks.ks_flags=0;
  Winkysig(1,&ks);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGINT,SIG_IGN);

  /*
  	CHECK WINDOW MODE (P/L)
  */
  printf("\33[?1n");
  scanf("%c%c%c%c",&c,&c,&c,&mode);
  strcpy(cross_buffer,"\33:0;0m");

  /*
  	SET BACKGROUND PATTERN, TRACE POSITION AND (XMAX,YMAX).
  */
  if(mode=='P')
  { printf("\33:0;0m\33:721;977;1;1f");
    strcpy(coords,"\33:376;102m\33:2H");
    xmax=599;
    ymax=847;
  }
  else
  { printf("\33:0:0m\33:977;721;1;1f");
    strcpy(coords,"\33:508;95m\33:2H");
    xmax=847;
    ymax=599;
  }

  strcpy(pos_req,"\33:7M");
  cross_blen=0;
  cross=0;

  paper_window();
  ioctl(paper,TIOCSETP,&tty);
  change_mouse_pointer();
  color_icons();
  command_icons();
  initstorage();

  /*
  	ENABLE ICON SELECTION
  */
  signal(SIGUSR1,icon_control);


  /*
  	READ MOUSE
  */
  do
  { write(paper,pos_req,4);
    mouseln=read(paper,mouse,30);
    /*
    	LOOP UNTIL esc IS READ. TO ENSURE MOUSE COORDINATES
    */
  } while(*mouse!='\33');
  /*
  	DRAW CROSS AND TRACE
  */
  if(cross) cross_blen=draw_inv_cross(mouse,cross_buffer);
  

  /*
  	MAIN LOOP
  */
  while(1)
  { if(mouse[mouseln-=2]=='1' && mouseln>1)
    { mouse[mouseln-1]='m';
      mouse[mouseln]=0;
      signal(SIGUSR1,SIG_IGN);
      switch(command)
      { case 01: pencil();break;
        case 02: line();break;
	case 03: box();break;
	case 04: fill_box();break;
	case 05: paint_box();break;
	case 06: frame();break;
	case 07: circle();break;
	case 08: text();break;
	case 09: copy();break;
	case 10: paint();break;
      }
      /*
      	WARNING: AD HOC

	DRAW BOX IN ICON MODE FOR PROPER LOOK
      */
      if(icon_mode){
	  n=sprintf(buffer,"\33:0;0m\33:0;%d;0;1d\33:%d;%d;0;1d\33:%d;0;0;1d\33:0;0;0;1d",ymax,xmax,ymax,xmax);
	  write(paper,buffer,n);
      }
      signal(SIGUSR1,icon_control);
      saved=0;
    }
    do
    { write(paper,pos_req,4);
      mouseln=read(paper,mouse,30);
    } while(*mouse!='\33');
    if(cross)
    { write(paper,cross_buffer,cross_blen);
      cross_blen=draw_inv_cross(mouse,cross_buffer);
    }
  }
}
    
