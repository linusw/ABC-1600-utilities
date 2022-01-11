/* lutex_rd.c
 * (c) Luxor Datorer AB 1985-09-18
 * Ulf S|dergren
 *
 * "lutex_read" {r en funktion som anv{nds f|r att av "db"
 * f|r att l{sa ett lutex dokument.
 *
*/

#include	<sccs.h>
SCCSID(@(#)Internversion lutex_rd.c 1.2@(#));


#include 	<stdio.h>
#include	"db.h"
#include "/usr/ulf/babs/read.spi"		/* struct READ */


lutex_read()
{
	FILE	*katfp[2]; 	/* filpekare till katalogrutin */
	int	katfd[2]; 	/* fildiscriptor till katalogrutin */
	char	*cp;
	char	*strchr();
	char	*parv[PARSIZE];
	char	par[PARSIZE];
	char	*string;
	char	*name;
	char	*field;
	char	*afunst;
	int	rep;
	char	*indata[];
	int	cc;
	struct READ buf;
	char	physdir[65];
	char	physfile[15];
	char	version[3];

	char	projekt1[sizeof(buf.projekt1)+1];
	char	namn1[sizeof(buf.namn1)+1];
	char	dir1[sizeof(buf.dir1)+1];

	parv[0] = "/usr/lib/yla/alba/db_katalog";
	parv[1] = NULL;
	execf("/usr/lib/yla/alba/db_katalog", parv, katfd);
	katfp[0] = fdopen(katfd[0], "r");
	katfp[1] = fdopen(katfd[1], "w");


	string	= "CS,DP,DR";
	name	= "/usr/lib/yla/alba/read";
	field	= "*";
	afunst	= "";
	rep	= 0;

	cc = ios(string, name , field, &buf, afunst, rep, indata);
	if (cc == 100) {
		unblk(buf.projekt1, sizeof buf.projekt1, projekt1);
		unblk(buf.namn1, sizeof buf.namn1, namn1);
		unblk(buf.dir1, sizeof buf.dir1, dir1);

		fputs("10\nproject=", katfp[1]);
		fputs(projekt1, katfp[1]);
		fputs("\ndirname=", katfp[1]);
		fputs(dir1, katfp[1]);
		fputs("\nfilename=", katfp[1]);
		fputs(namn1, katfp[1]);
		fputs("\n11\n", katfp[1]);
		fflush(katfp[1]);

		getc(katfp[0]);	/* skip '1' */
		getc(katfp[0]);	/* skip '0' */
		getc(katfp[0]);	/* skip '\n' */
		getc(katfp[0]);	/* skip ' ' */
		fgets(physdir, sizeof physdir, katfp[0]);
		if ((cp = strchr(physdir, '\n')) != NULL) {
			*cp = '\0';
		}
		else {
			while (getc(katfp[0]) != '\n');
		}
		getc(katfp[0]);	/* skip ' ' */
		fgets(physfile, sizeof physfile, katfp[0]);
		if ((cp = strchr(physfile, '\n')) != NULL) {
			*cp = '\0';
		}
		else {
			while (getc(katfp[0]) != '\n');
		}
		getc(katfp[0]);	/* skip ' ' */
		fgets(version, sizeof version, katfp[0]);
		if ((cp = strchr(version, '\n')) != NULL) {
			*cp = '\0';
		}
		else {
			while (getc(katfp[0]) != '\n');
		}
		getc(katfp[0]);	/* skip '1' */
		getc(katfp[0]);	/* skip '1' */
		getc(katfp[0]);	/* skip '\n' */


		if (getarg("lutex", par, parv, APPLREG)) {
			exit(1);  /* fel vid l{sning av argument */
		}
		/* parv[0] = path f|r lutex */
		parv[1] = "-i";		/* infil */
		parv[2] = physfile;
		parv[3] = "-r";		/* "readonly" */
		parv[4] = NULL;
		db_lutex(parv, physdir, namn1);
	}
	return(0);
}

