# wshppp.l
# 85-09-04
# Luxor Datorer AB
# Hans Norlander
#
# Indatafil till Lex.
#
# Programmet byter ut logiska namn p} f|nsterdefinitioner
# mot den typ av namn som f|nsterhanteraren klarar av.

DIGIT		[0-9]
LOGNAM		{DIGIT}+(_{DIGIT}+)*
%Start		Kword
%%

substitute	|
pointer		|
background	|
window		|
icon		|
string		|
header		|
environ		|
directory	|
command		|
flags		|
zoomlist	|
menu		|
choice		|
action		|
terminal	|
command		{ ECHO; BEGIN Kword; }
<Kword>{LOGNAM}	{ printf("%d", exchange(yytext)); BEGIN 0; }
.		{ ECHO; BEGIN 0; }
%%

#include	<sccs.h>
SCCSID(@(#)Internversion wshppp.l 1.3@(#));

#define LOGNAMLEN	10
#define LOGNAMMAX	500

char	lognamtab[LOGNAMMAX][LOGNAMLEN];
int	lognamcnt = 0;

exchange(l)
char *l;
/*
 * S|ker upp ett logiskt namn i tabellen. Om det inte finns
 * l{ggs det in i tabellen. Tabellindex returneras.
 *
 * Om tabellen blir full skriver funktionen
 * felmeddelande och g|r exit(1).
 */
{
	char *p, *lsearch();
	extern char lognamtab[][LOGNAMLEN];
	extern int lognamcnt;
	int strcmp();

	p = lsearch(l, lognamtab, &lognamcnt, LOGNAMLEN, strcmp);
	if (lognamcnt == LOGNAMMAX) {
		fprintf(stderr, "Tabell |ver logiska namn full\n");
		exit(1);
	}
	return (p - (char *)lognamtab)/LOGNAMLEN + 1;
}
