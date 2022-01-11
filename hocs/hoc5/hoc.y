%{
#include	"hoc.h"
#define	code2(c1,c2)	code(c1); code(c2)
#define	code3(c1,c2,c3)	code(c1); code(c2); code(c3);
%}
%union {		/* stack type	*/
	symbol	*sym;	/* symbol table pointer	*/
	Inst	*inst;	/* machine instruction	*/
}
%token	<sym>	NUMBER PRINT VAR BLTIN UNDEF WHILE IF ELSE
%type	<inst>	stmt asgn expr stmtlist cond while if end
%right	'='
%left	OR
%left	AND
%left	GT GE LT LE EQ NE
%left	'+' '-'			/* left associative, same precedence	*/
%left	'*' '/'			/* 	-"-	     higher precedence	*/
%left	UNARYMINUS NOT		/*      -"-            -"-      -"-	*/
%right	'^'			/* exponentiation			*/
%%
list:			/* nothing	*/
	| list '\n'		{ printf("hoc5> ");		}
	| list asgn '\n'	{ code2(pop, STOP);
				  printf("hoc5> ");
				  return 1;
				}
	| list stmt '\n'	{ code(STOP);
				  printf("hoc5> ");
				  return 1;
				}
	| list expr '\n'	{ code2(print, STOP);
				  printf("hoc5> ");
				  return 1;
				}
	| list error '\n'	{ yyerrok;
				  printf("hoc5> ");
				}
	;
asgn:	  VAR '=' expr		{ $$=$3;
				  code3(varpush, (Inst)$1,assign);	}
	;
stmt:	  expr			{ code(pop);				}
	| PRINT expr		{ code(prexpr); $$ = $2;		}
	| while cond stmt end	{ ($1)[1] = (Inst)$3;	/* body of loop	*/
				  ($1)[2] = (Inst)$4;	/* end if fail	*/
				}
	| if cond stmt end	{ ($1)[1] = (Inst)$3;	/* then part	*/
				  ($1)[2] = (Inst)$4;	/* end if fail	*/
				}
	| if cond stmt end
	    ELSE stmt end	{ ($1)[1] = (Inst)$3;	/* then part	*/
				  ($1)[2] = (Inst)$6;	/* else part	*/
				  ($1)[3] = (Inst)$7;	/* end if fail	*/
				}
	| '{' stmtlist '}'	{ $$ = $2;				}
	;
cond:	  '(' expr ')'		{ code(STOP); $$ = $2;			}
	;
while:	  WHILE			{ $$ = code3(whilecode, STOP, STOP);	}
	;
if:	  IF			{ $$ = code(ifcode);
				  code3(STOP, STOP, STOP);		}
end:	  /* nothing	*/	{ code(STOP); $$ = progp;		}
	;
stmtlist:	/* nothing */	{ $$ = progp;				}
	| stmtlist '\n'
	| stmtlist stmt '\n'
	;
expr:	  NUMBER		{ $$ = code2(constpush, (Inst)$1);	}
	| VAR			{ $$ = code3(varpush, (Inst)$1, eval);	}
	| asgn
	| BLTIN '(' expr ')'	{ $$ = $3;
				  code2(bltin, (Inst)$1->u.ptr);	}
	| '(' expr ')'		{ $$ = $2;				}
	| expr '+' expr		{ code(add);				}
	| expr '-' expr		{ code(sub);				}
	| expr '*' expr		{ code(mul);				}
	| expr '/' expr		{ code(div);				}
	| expr '^' expr		{ code(power);				}
	| '-' expr  %prec UNARYMINUS	{ $$ = $2; code(negate);	}
	| expr GT  expr		{ code(gt);				}
	| expr GE  expr		{ code(ge);				}
	| expr LT  expr		{ code(lt);				}
	| expr LE  expr		{ code(le);				}
	| expr EQ  expr		{ code(eq);				}
	| expr NE  expr		{ code(ne);				}
	| expr AND expr		{ code(and);				}
	| expr OR  expr		{ code(or);				}
	|      NOT expr		{ $$ = $2; code(not);			}
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


