
typedef union  {		/* stack type	*/
	symbol	*sym;	/* symbol table pointer	*/
	Inst	*inst;	/* machine instruction	*/
} YYSTYPE;
extern YYSTYPE yylval;
# define NUMBER 257
# define VAR 258
# define BLTIN 259
# define UNDEF 260
# define UNARYMINUS 261
