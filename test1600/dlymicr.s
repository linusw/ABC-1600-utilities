WDOG	=	/80007
	.globl	_dlymicr
_dlymicr:
	movl	4(sp),d0
	lsrl	#2,d0
1$:	movb	WDOG,d1
2$:	nop
	dbra	d0,2$
	subl	#/10000,d0
	bcc	1$
	rts
	

