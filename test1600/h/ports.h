#ifdef DS90M10
#define BUSBASE 0x7f0000
#define SIBASE 0xa000	/* Internal sasi */
#define CSBASE 0xc000	/* 4680 bus adapter */
#define CSSHIFT 8
#define MAXCS	63
#define	SIPORT(c)	(((c)<<CSSHIFT) | (BUSBASE+SIBASE))
#define DPORT(c)	(BUSBASE+CSBASE+(c << 5)+1)
/* Port number for DMI-DB Board */
#define	DMIDBP	(DPORT(0))
/* Port number of interupt vector for DMI-DB board */
#define	DMIDBVP	(DMIDBP + 0x18)
/* Macro to test if a port is a DB4680 port */
#define	DB4680P(port)	(((unsigned)port) > DPORT(0) && ((unsigned)port) < DPORT(64))
#endif
#ifdef	X35
#ifdef	CSSTRUL
#define	BUSBASE 0
#define CSSHIFT 8
#else
#define	CSSHIFT	5
#define	MAXCS	63
#define	BUSBASE		0x7e000
#endif
#define DPORT(c)	(((c)<<CSSHIFT) | (BUSBASE+0))
/* Macro to test if a port is a DB4680 port */
#define	DB4680P(port)	(((unsigned)port) < 0x7f000)
#endif

/*#define	outb(x,y)	(*((unsigned char*)(x))=(y))
#ifdef	DS90M10
#define	inb(x)		(*((unsigned char*)((int)(x) | 1)))
#endif
#ifdef	X35
#define	inb(x)		(*((unsigned char*)(x)))
#define	out2b(x,y)	*((short*)(x))=(y)
#define in2b(x)		(*((short*)(x)))
#endif
*/
#ifdef DS90M10
#define	OUT(c,x)	outb(DPORT(c)+0x00,x)
#define	C1(c,x)	outb(DPORT(c)+0x08,x)
#define	C2(c,x)	outb(DPORT(c)+0x0c,x)
#define	C3(c,x)	outb(DPORT(c)+0x10,x)
#define	C4(c,x)	outb(DPORT(c)+0x14,x)
#define INP(c)	inb(DPORT(c)+0x00)
#define STAT(c)	inb(DPORT(c)+0x04)

#define	INPx(p)		inb(p)
#define	STATx(p)	inb(p+4)
#define	CSBx(p)		inb(p+2048)
#endif
#ifdef	X35
#define	OUT(c,x)	outb(DPORT(c)+0,x)
#define	C1(c,x)	outb(DPORT(c)+4,x)
#define	C2(c,x)	outb(DPORT(c)+6,x)
#define	C3(c,x)	outb(DPORT(c)+8,x)
#define	C4(c,x)	outb(DPORT(c)+10,x)
#define INP(c)	inb(DPORT(c)+0)
#define STAT(c)	inb(DPORT(c)+2)
#define CSB(c)	inb(DPORT(c)+2)

#define	INPx(p)		inb(p)
#define	STATx(p)	inb(p+2)
#define	CSBx(p)	inb(p+2048)

#endif

#define	OUTx(p,x)	outb(p,x)
#define	C1x(p,x)	outb(p+4,x)
#define	C2x(p,x)	outb(p+6,x)
#define	C3x(p,x)	outb(p+8,x)
#define	C4x(p,x)	outb(p+10,x)
 
#define	CS_MT	056	/* 4680 cs of mag tape */
#define	CS_DSKB	040	/* base of all possible disc cs */
#define	CS_SAB	040	/* base of all possible sasi cs */
#define	CS_DP	044	/* 10 mb cartridge disc */
#define	CS_SA	045	/* Sasi bus discs */
#define	CS_FP	054	/* standard floppy */
#define CS_SAFP	054	/* new sasi compatible sf controller */
#define	CS_MF	055	/* mini floppy */


#define	BOOTADR	&end	/* where to put track 0 of disk */
#define	BOOTMAGIC 0x4e71	/* what track 0 should start with */

extern	int	sioaddr[];
#define	SIOADR(x)	((caddr_t) sioaddr[x])
#define	SRCISPEED(x)	13
#define	SRCOSPEED(x)	13
#define	SETBAUD(d,s)	setbaud(d,s)
#ifdef	DS90M10
#define	NSIO	12
#define	SIOCSR	(-0x42)
#define	SIODAT(x)  x
#endif
#ifdef	X35
#define	NSIO	4
#define	SIOCSR	(-2)
#define	SIODAT(x)  x
#endif

extern char autoflg;	/* Auto/man */
#define	AUTOSW	(autoflg)
#define	WATCHDOG	{ watchdg();}

#ifdef	DS90M10
#define	CIOCTL	0x7fffce
#define	CIOINTM	0x00
#define	HZI	2000000
#endif
#ifdef	X35
#define	CIOCTL	0x7f706
#define CIOINTM	0xff
#define	HZI	2000000

#define	SRLATCH		0x7fe00
#define	GDMAEN		outb(SRLATCH,0xd)

/* DMA definitions */
#define	BUS0DMA	((unsigned char *)0x7f300)
#define	BUS1DMA	((unsigned char *)0x7f400)
#define	BUS2DMA	((unsigned char *)0x7f500)

#define	BUS0HI	((unsigned char *)0x7fd06)
#define	BUS1HI	((unsigned char *)0x7fd04)
#define	BUS2HI	((unsigned char *)0x7fd00)

/* Floppy definitions */
#define	FPYPORT	((unsigned char *)0x7f000)
#define	FPYCTL	((unsigned char *)0x7fb00)
#define	FP_FPVEK	(gdynvek(FPYPORT,2)+1)
#define	FP_PRVEK	(gdynvek(FPYPORT,2)+0)
#define	FP_FPENI	{ register i ; i = spl7() ; \
			  outb(CIOCTL,0x2f) ; \
			  out2b(CIOCTL,inb(CIOCTL) | 0x2f80) ; \
			  splx(i) ; \
			}
#define	FP_PRENI	{ register i ; i = spl7() ; \
			  outb(CIOCTL,0x2f) ; \
			  out2b(CIOCTL,inb(CIOCTL) | 0x2f40) ; \
			  splx(i) ; \
			}
#define	FP_DIS		{ register i ; i = spl7() ; \
			  outb(CIOCTL,0x2f) ; \
			  out2b(CIOCTL,0x2f00 | (inb(CIOCTL) &  ~0xc0)) ; \
			  splx(i) ; \
			}
#define FP_DMAFREE	(inb(CIOCTL-4) & 0x40)
#define	FP_GETDMA	((char *)SRLATCH)[0]=0xf
#define	FP_RELDMA	((char *)SRLATCH)[0]=0x7
#endif
#define	SSPEED	13		/* deafault speed = 9600 */

/*	Battery clock/nvram definition */
#ifdef	DS90M10
#define	BCLK	0x7fffc8	/* Battery clock port */
#endif
#ifdef	X35
#define	BCLK	0x7f700		/* Battery clock port */
#endif
#define	BATCRD	(inb(BCLK) & 0x02)	/* Get battery clock data */
#define	BATCCSH	(outb(BCLK,0x87))	/* Battery clock - cs high clk high */
#define	BATCCLH	(outb(BCLK,0x83))	/* Battery clock - cs low  clk high */
#define	BATCCLL	(outb(BCLK,0x82))	/* Battery clock - cs low clk low */
#define	BATCWR(x)	{ register v;\
			outb(BCLK,0xa0); \
			v = x ? 0x02 : 0 ; \
			dlymicr(4); \
			outb(BCLK,0x80 | v); \
			outb(BCLK,0x81 | v); \
			}
#define	SZNVRAM		16	/* Size in shorts of nvram */
