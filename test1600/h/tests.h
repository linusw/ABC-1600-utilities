#define SCCB	0x7F600
#define SCCA	0x7F604
#define DATAB	0x7F602
#define DATAA	0x7F606

#define SIZE	0x7f800
#define TO	0x7f804
#define FROM	0x7f900
#define MASK	0x7fa00
#define FLAGS	0x7fa03

#define SCREEN	((unsigned char *)0x40000)

#define	BUS0DMA	((unsigned char *)0x7f300)
#define	BUS1DMA	((unsigned char *)0x7f400)
#define	BUS2DMA	((unsigned char *)0x7f500)

#define	BUS0HI	((unsigned char *)0x7fd06)
#define	BUS1HI	((unsigned char *)0x7fd04)
#define	BUS2HI	((unsigned char *)0x7fd00)

#define SPCR    0x7FE00
#define spcr	 *((char *) SPCR)

#define WDOG	0x80007
#define PRTR	0x7F206
#define DARTA	0x7F204
#define KEYS	0x7F202
#define DART	0x7F200
#define CIO	0x7F706

#define byte	short unsigned int

#define	outb(x,y)	(*((unsigned char*)(x))=(y))
#define	inb(x)		(*((unsigned char*)(x)))
#define	out2b(x,y)	(*((unsigned short*)(x))=(y))
#define in2b(x)		(*((unsigned short*)(x)))

#define outw(val,addr)	(*((unsigned short *)(addr)) = (unsigned short) (val))
#define outl(val,addr)	(*((unsigned int *)(addr)) = (unsigned int) (val))
#define flags	 *((char *) FLAGS)
#define wait()	while ((*((char *)SIZE)) & 0x80)

#define sccb	 *((char *) SCCB)
#define scca	 *((char *) SCCA)
#define datab	 *((char *) DATAB)
#define dataa	 *((char *) DATAA)

#define kick	 (*((char *) WDOG)=0)

#define set	 *((char *) DARTA)
#define print	 *((char *) PRTR)
#define keyb	 *((char *) KEYS)
#define status	 *((char *) DART)
#define cio	 *((char *) CIO)

#define SASIDATA	0x7e4a0
#define SASISTAT	0x7e4a2
#define SASISEL		0x7e4a4
#define SASIOPS		0x7e4a4
#define SASIRST		0x7e4a8
#define SASIMODE	0x7e4aa
#define SASICSB		0x7eca0
#define expintSASI	0x7ecb0
#define expscbSASI	0x7ecb8

#define FLOPDATA	0x7f006
#define FLOPSECT	0x7f004
#define FLOPTRACK	0x7f002
#define FLOPCOM		0x7f000
#define FLOPSTAT	0x7f000

#define FLOPCTRL	0x7fb00

#include <stdio.h>

extern int prexist;
