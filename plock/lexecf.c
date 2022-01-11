/* lexecf.c
 * (c) Luxor Datorer AB 1985-09-18
 * Hans Norlander / Ulf S|dergren
 *
 * En liten rutin f|r att anropa ett filterprogram fr}n ett annat program.
 * ---------------------------------------------------- */

#include	<sccs.h>
SCCSID(@(#)Internversion lexecf.c 1.3@(#));

#include 	<stdio.h>
#include	<signal.h>

/* ---------------------------------------------------- */
	execf(prog, argv, fd)
/* ---------------------------------------------------- */
char *prog;
char *argv[];
int fd[];

/* Startar ett filterprogram som kan k|ras interaktivt
 * fr}n det anropande programmet.
 *
 * In:
 * prog		Namn p} det program som ska startas
 * argv		argument till programmet (p} argv-format)
 * fd[2]	en array med tv} heltal. Vid retur fr}n
 *		funktionen anger f|rsta heltalet det fil-
 *		nummer som ska anv{ndas f|r att l{sa data
 *		fr}n filtret, och andra heltalet vilket fil-
 *		nummer som ska anv{ndas f|r att skicka data
 *		till filtret.
 *
 * Returv{rde:	Process-id p} filterprocessen. Kan anv{ndas
 *		vid signalering, wait m m.
 *		Om fel uppst}r returneras v{rdet -1.
 *
 */

{
	int pip[2][2];
	int pid;
	int i;

	if (pipe(pip[0]) || pipe(pip[1]) || (pid=fork()) < 0)
	{	/* Error */
		close(pip[0][0]);
		close(pip[0][1]);
		close(pip[1][0]);
		close(pip[1][1]);
		return (-1);
	}


	if (pid > 0)	/* Mother */
	{
		fd[0] = pip[1][0];
		fd[1] = pip[0][1];
		close(pip[0][0]);
		close(pip[1][1]);
		return (pid);
	}

	/* Child */

	/* Ignorera signaler */
	for (i = 1; i < NSIG ; i++) {
		signal(i, SIG_IGN);
	}
	signal(SIGHUP, SIG_DFL);	/* Set up "Hangup" signal */

	/* Set up all files */
	dup2(pip[0][0], 0); close(pip[0][0]);	/* stdin */
	dup2(pip[1][1], 1); close(pip[1][1]);	/* stdout */
	close(2); open("/dev/null", 1);		/* stderr */

	close(pip[0][1]);
	close(pip[1][0]);
	
	/* Exekvera filter-programmet */
	execvp(prog, argv);
	exit(1);
}

/* ---------------------------------------------------- */
	fexecf(prog, argv, stream)
/* ---------------------------------------------------- */
char *prog;
char *argv[];
FILE *stream[];

/* som execf, men returnerar tv} streams i st{llet f|r tv} fd.
 * Observera att b}da filerna blir buffrade. Anv{nd setbuf() om
 * detta inte {r |nskv{rt.
 */

{
	int fd[2];
	int pid;

	pid = execf(prog, argv, fd);
	stream[0] = fdopen(fd[0], "r");
	stream[1] = fdopen(fd[1], "w");
	return (pid);
}
