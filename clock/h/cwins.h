#define bwrite(b,l) { strcat(wrbuff,b); bufflen+=l; }

#define wprint(w) { write(w,wrbuff,bufflen); bufflen=0; wrbuff[0]='\0'; }

#define line(x,y,ptn,color)	{ len=sprintf(sbuff,"%c:%d;%d;%d;%dd",27,x,y,ptn,color);bwrite(sbuff,len); }

#define iline(x,y)		{ len=sprintf(sbuff,"%c:%d;%di",27,x,y);bwrite(sbuff,len); }

#define arc(x,y,alen,ptn,color) { len=sprintf(sbuff,"%c:%d;%d;%d;%d;%da",27,x,y,alen,ptn,color); bwrite(sbuff,len); }

#define cur(x,y)		{ len=sprintf(sbuff,"%c:%d;%dm",27,x,y);bwrite(sbuff,len); }

#define cls()			{ len=sprintf(sbuff,"%c[2J",27);bwrite(sbuff,len); }

#define txtcur(x,y)		{ len=sprintf(sbuff,"%c[%d;%dH",27,y,x);bwrite(sbuff,len); }

#define font(ftyp)		{ len=sprintf(sbuff,"%c(%c",27,ftyp);bwrite(sbuff,len); }

#define lite()			{ sprintf(sbuff,"%c[?5l",27);bwrite(sbuff,5); }

#define dark()			{ sprintf(sbuff,"%c[?5h",27);bwrite(sbuff,5); }

unsigned char sbuff[15];
int len;
int bufflen;
