typedef	struct	symbol {	/* symbol table entry	*/
	char	*name;
	short	type;		/* VAR, BLTIN, UNDEF	*/
	union {
		double	val;		/* if VAR	*/
		double	(*ptr)();	/* if BLTIN	*/
	} u ;
	struct	symbol	*next;	/* to link to another	*/
} symbol;
symbol	*install(), *lookup();

typedef	union	Datum {		/* interpreter stack type	*/
	double	val;
	symbol	*sym;
} Datum;
extern	Datum pop();

typedef	int	(*Inst)();	/* machine instruction		*/
#define	STOP	(Inst) 0

extern	Inst prog[];
extern	eval(), add(), sub(), mul(), div(), negate(); power();
extern	assign(), bltin(), varpush(), constpush(), print();
