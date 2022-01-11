%{
#include	"hoc.h"
#define	code2(c1,c2)	code(c1); code(c2)
#define	code3(c1,c2,c3)	code(c1); code(c2); code(c3);
%}
%union {		/* stack type	*/
	symbol	*sym;	/* symbol table pointer	*/
	Inst	*inst;	/* machine instruction	*/
}
%token	<sym>	NUMBER VAR BLTIN UNDEF
%right	'='
%left	'+' '-'		/* left associative, same precedence	*/
%left	'*' '/'		/* 	-"-	     higher precedence	*/
%left	UNARYMINUS	/*      -"-            -"-      -"-	*/
%right	'^'		/* exponentiation			*/
%%
list:			/* nothing	*/
	| list '\n'		{ printf("hoc3> ");		}
	| list asgn '\n'	{ code2(pop, STOP);
				  printf("hoc4> ");
				  return 1;
				}
	| list expr '\n'	{ code2(print, STOP);
				  printf("hoc4> ");
				  return 1;
				}
	| list error '\n'	{ yyerrok;
				  printf("hoc4> ");
				}
	;
asgn:	  VAR '=' expr		{ code3(varpush, (Inst)$1,assign);	}
	;
expr:	  NUMBER		{ code2(constpush, (Inst)$1);		}
	| VAR			{ code3(varpush, (Inst)$1, eval);	}
	| asgn
	| BLTIN '(' expr ')'	{ code2(bltin, (Inst)$1->u.ptr);	}
	| '(' expr ')'
	| expr '+' expr		{ code(add);				}
	| expr '-' expr		{ code(sub);				}
	| expr '*' expr		{ code(mul);				}
	| expr '/' expr		{ code(div);				}
	| expr '^' expr		{ code(power);				}
	| '-' expr  %prec UNARYMINUS	{ code(negate);			}
	;
%%
#include	<stdio.h>
#include	<ctype.h>
#include	<signal.h>
#include	<setjmp.h>
jmp_buf  begin;

char  *progname;		/* for error messages	*/
int   lineno = 1;

main(arc,argv)			/* hoc4	*/

char  *argv[];
{
	int fpecatch();

	progname = argv[0];
	init();
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	for (initcode();printf("hoc4> "),yyparse();initcode())
	    execute(prog);
	return 0;
}

execerror(s, t)		/* recover from run-time error	*/

char  *s, *t;
{
	warning(s, t);
	longjmp(begin, 0);
}

fpecatch()		/* catch floating point exceptions	*/
{
	execerror("floating point exception", (char *) 0);
}

yylex()
{
	int c;

	while  ((c = getchar()) == ' ' || c == '\t')
	    ;
	if  (c == EOF)
	    return 0;
	if  (c == '.' || isdigit(c)) {	/* number	*/
	    double d;

	    ungetc(c, stdin);
	    scanf("%lf", &d);
	    yylval.sym = install("", NUMBER, d);
	    return NUMBER;
	}
	if  (isalpha(c)) {
	    symbol	*s;
	    char 	sbuf[100], *p = sbuf;
	    do {
	    	*p++ = c;
	    } while ((c = getchar()) != EOF && isalnum(c));
	    ungetc(c, stdin);
	    *p = '\0';
	    if  ((s = lookup(sbuf)) == 0 )
	    	s = install(sbuf, UNDEF, 0.0);
	    yylval.sym = s;
	    return s->type == UNDEF ? VAR : s->type;
	}
	if  (c == '\n')
	    lineno++;
	return c;
}

yyerror(s)	/* called for yacc syntax error	*/

char  *s;
{
	warning(s, (char *) 0);
}

warning(s, t)	/* print warning message	*/

char  *s, *t;
{
	fprintf(stderr, "%s: %s", progname, s);
	if  (t)
	    fprintf(stderr, " %s", t);
	fprintf(stderr, " near line %d\n", lineno);
}


