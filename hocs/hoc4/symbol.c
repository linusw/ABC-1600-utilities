#include	"hoc.h"
#include	"y.tab.h"

static	symbol	*symlist = 0;		/* symbol table, linked list	*/

symbol	*lookup(s)			/* find s in symbol table	*/

char	*s;
{
	symbol	*sp;

	for (sp = symlist; sp != (symbol *) 0; sp = sp->next)
	    if  (strcmp(sp->name,s) == 0)
	        return sp;
	return 0;			/* not found	*/
}

symbol	*install(s, t, d)		/* install s in symbol table	*/
char	*s;
int	t;
double	d;
{
	symbol	*sp;
	char	*emalloc();

	sp = (symbol *) emalloc(sizeof(symbol));
	sp->name = emalloc(strlen(s)+1);	/* +1 for '\0'	*/
	strcpy(sp->name, s);
	sp->type = t;
	sp->u.val = d;
	sp->next = symlist;	/* put at front of list	*/
	symlist = sp;
	return sp;
}

char	*emalloc(n)		/* check return from malloc	*/

unsigned	n;
{
	char	*p, *malloc();

	p = malloc(n);
	if  (p == 0)
	    execerror("out of memory", (char *) 0);
	return p;
}
