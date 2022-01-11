/*****************************************************************************/
/*                                                                           */
/* 1615 screen dump to FX80 graphic codes conversion			     */
/*									     */
/* LQ style high density simulated to give 120x108 DPI resulotion	     */
/*   no scaling give a minor aspect ratio error				     */
/*   options: -o outfile						     */
/*	      -c center picture on paper				     */
/*									     */
/* By HS 1985-12-13							     */
/*   compile as: cc FX80dmp.c -o FX80dmp -n -O				     */
/*****************************************************************************/

static char copyrigth[] = "Copyrigth -c- 1985 by Luxor Datorer AB" ;

#include <stdio.h>

#define SCR_W_OUT  960 /* that's 8" in mode 1 */

static short w,h,fdu=1,fdi=0;

char *malloc();
static char *buf;
static int ybase;
static int *access_vector;

static char mask[] = {01,01,01,04,04,04,020,020,020,0100,0100,0100};

main(argc,argv)
int argc;
char *argv[];
{

	short xin,yin,k,bias=0,center=0;
	register int i,j,needle;
	char uline1[SCR_W_OUT],uline2[SCR_W_OUT],uline3[SCR_W_OUT];
	register char *up1,*up2,*up3;
	int base=0;

/* scan for command line options */
	while (--argc) {
		if ( (*++argv)[0] != '-') {
			if ( (fdi = open(*argv,0)) < 0) {
				write(2,"failed to open infile\n",22);
				exit(1);
			}
		}
		else {
			if ((*argv)[1] == 'o') {
				if (strlen(*argv) < 3) {
					++argv; --argc;
				}
				else
					*argv = (char *)((int)(*argv)+2);
				if ( (fdu = creat(*argv,0644)) < 0) {
					write(2,"failed to create outfile\n",25);
					exit(1);
				}
			}
			else if ((*argv)[1] == 'c')  {
				center = 1;
			}

		}
	}

	read(fdi,&xin,2); read(fdi,&yin,2);
	read(fdi,&w,2); read(fdi,&h,2);
	read(fdi,uline1,4);

/* row base address to be used in TSTDOT */
	ybase = w/8;
	if (w%8) ++ybase;

	if ( (access_vector = (int *)malloc( h*sizeof(int)) ) <= 0) {
		write(2,"failed to allocate buffer space\n",32);
		exit(1);
	}
	for (k=0;k<h;++k) {
		access_vector[k] = base;
		base += ybase;
	}

/* allocate buffer space */
	if ( (buf = malloc(h*(w+7)/8 )) <= 0) {
		write(2,"failed to allocate buffer space\n",32);
		exit(1);
	}

/* get graphic dump data */
	read(fdi,buf,h*(w+7)/8);
	close(fdi);

/* output spooler control record */
	write(fdu,"\377\3770trans\000\n",10);
/* remove any slack in printer feeding */
	write(fdu,"\033J\006",3);  

/* simulate 12 needles to obtain 108 DPI */
	needle = 11;
	up1 = &uline1[0]; up2 = &uline2[0]; up3 = &uline3[0];
	for (k=0;k<SCR_W_OUT;++k) {
		*up1++ = 0;
		*up2++ = 0;
		*up3++ = 0;
	}

	if (center) bias = (SCR_W_OUT - w)/2;

	for (i=h-1;i>=0;--i) { /* for all rows on paper */
		for (j=0;j<w;++j) { /* for all columns */

/* see if pixel not set! normal mode in window is inverted, not so in mem. */
			if (!TSTDOT(j,i) ) { 
		/* select needle array */
				switch (needle) {
				
					case 0:
					case 3:
					case 6:
					case 9:
						uline3[j+bias] |= mask[needle];
						break;

					case 1:
					case 4:
					case 7:
					case 10:
						uline2[j+bias] |= mask[needle];
						break;

					case 2:
					case 5:
					case 8:
					case 11:
						uline1[j+bias] |= mask[needle];
						break;
				}
			}
		}
		--needle; /* next needle */
		if (needle < 0) { /* if all needles used -> flush */
			gprintl(uline1,SCR_W_OUT);
			write(fdu,"\033J\002\r",4);
			gprintl(uline2,SCR_W_OUT);
			write(fdu,"\033J\002\r",4);
			gprintl(uline3,SCR_W_OUT);
			write(fdu,"\033J\024\r",4);
			needle = 11;
			up1 = &uline1[0]; up2 = &uline2[0]; up3 = &uline3[0];
			for (k=0;k<SCR_W_OUT;++k) {
				*up1++ = 0;
				*up2++ = 0;
				*up3++ = 0;
			}

		}
	}
	gprintl(uline1,SCR_W_OUT);
	write(fdu,"\033J\002\r",4);
	gprintl(uline2,SCR_W_OUT);
	write(fdu,"\033J\002\r",4);
	gprintl(uline3,SCR_W_OUT);
	write(fdu,"\033J\024\r",4);

	close(fdu);
	exit(0);

}

/* see if pixel set in graphic dump */
TSTDOT(x,y)
short x,y;
{
	if (x >= w || y >= h) return(1);
	return(buf[access_vector[y]+x/8] & 1<<(7-x%8));
}

/* output graphic line without using 8'th bit */
gprintl(buf,num)
char *buf;
int num;
{
	int cnt;
	char tmp[5];

	tmp[0] = 27; /* graphic mode 1 */
	tmp[1] = '*';
	tmp[2] = 1;

	while (num > 0) { /* while more in line buffer */
		if (!(num & 0x80)) { /* ok to print direct */
			tmp[3] = (num & 0xff);
			tmp[4] = ((num >> 8) & 0xff);
			write(fdu,tmp,5);
			write(fdu,buf,num);
			num = 0;
		}
		else {
			if (num & 0x7f == 0x7f) { /* boundary clash, justify */
				tmp[3] = 1;
				tmp[4] = 0;
				write(fdu,tmp,5);
				write(fdu,buf,1);
				--num;++buf;
			}
			else { /* split */
				cnt = (((num >> 8) & 0xff) << 8) + 127;
				tmp[3] = 127;
				tmp[4] = ((num>>8)&0xff);
				write(fdu,tmp,5);
				write(fdu,buf,cnt);
				num -= cnt;buf += cnt;
			}
		}
	}
}
