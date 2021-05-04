#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "engine.h"

/* For MATLAB Engine functions, see
 http://www.mathworks.co.uk/access/helpdesk/help/techdoc/apiref/index.html?/access/helpdesk/help/techdoc/apiref/
*/

static const char *progname;

 char Title[] = "\nDAKOTA/MATLAB Driver Version 20061012\n";

 static void
Squawk(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	}

 void *
Malloc(size_t len)
{
	void *rv = malloc(len);
	if (!rv) {
		Squawk("malloc(%lu) failure!\n", (unsigned long)len);
		exit(1);
		}
	return rv;
	}

 static int
usage(int rc)
{
	const char *b, *s;

	/* b = basename(progname); */
	for(b = s = progname; *s; )
		switch(*s++) {
#ifdef _WIN32
			case ':':
			case '\\':
#endif
			case '/':
				b = s;
			}

	fprintf(rc ? stderr : stdout, "usage: %s [-s] [arg ...]\n\n"
		"Option -s ==> start server that talks with MATLAB and\n"
		"talks to other instances of %s over pipes that by default\n"
		"are named dakmatlab_pipe1 and dakmatlab_pipe2; if shell variable\n"
		"DAKMATLAB_PIPE is set, they are named ${DAKMATLAB_PIPE}1 and\n"
		"${DAKMATLAB_PIPE}2.  If files with these names already exist,\n"
		"%s complains and gives return code 1; otherwise it creates\n"
		"the named pipes, starts MATLAB, and passes any arguments\n"
		"after the \"-s\" to MATLAB for it to interpret as though\n"
		"they were typed at the MATLAB prompt.\n\n"
		"When invoked as an analysis_program by DAKOTA, the invocation is\n\n"
		"\t%s parameters_file results_file\n\n"
		"This invocation causes %s to talk over the named pipes with\n"
		"a server instance of itself, so MATLAB sees assignments to x and\n"
		"asv, followed by one or more of\n\n\t\"y = f(x);\"\n"
		"\typ = fp(x);\n\typp = fpp(x);\n\n"
		"(depending on what DAKOTA wants: functions, derivatives, or\n"
		"second derivatives), after which the server retrieves y,\n"
		"yp = y' = Jacobian of f w.r.t. x, and/or ypp = Hessians of\n"
		"y w.r.t. x.  The server writes a results_file for DAKOTA and\n"
		"(over the second named pipe) tells the analyis_program instance\n"
		"to exit.  If the named pipes do not exist, %s itself\n"
		"initially invokes\n\n\t%s -s\n\n"
		"Writing \"quit\" or an empty file to the first named pipe, as in\n\n"
		"\techo quit >dakmatlab_pipe1\nor\n"
		"\tcat /dev/null >dakmatlab_pipe1\n\n"
		"causes the server to remove the named pipes, shut MATLAB down\n"
		"and exit.\n", progname,
		b,b,b,b,b,b);
	return rc;
	}

#define stderr stderr /* for the following */
typedef double real;
/**** Stuff modified from dakampl.c -- could it go into a common file? ****/

 typedef struct
FileInfo {
	FILE *f;
	char *fname;
	char *se;
	char *stub;
	char *nlext;
	long lineno;
	char buf[1024];
	} FileInfo;

 static int
BadOpen(const char *what, const char *name)
{
	Squawk("Unable to open %s file \"%s\"\n", what, name);
	return 1;
	}

 static void
BadRead(FileInfo *fi)
{
	Squawk("Unable to read line %ld of file \"%s\"\n",
		fi->lineno, fi->fname);
	exit(1);
	}

 static void
BadFmt(FileInfo *fi)
{
	Squawk("Bad format, line %ld of file \"%s\":\n%s\n",
		fi->lineno, fi->fname);
	exit(1);
	}

 int
Iread(FileInfo *fi, const char *expected)
{
	int rv;
	char *s, *se;

	++fi->lineno;
	if (!fgets(s = fi->buf, sizeof(fi->buf), fi->f))
		BadRead(fi);
	rv = (int)strtol(s, &se, 10);
	if (s == se || *se > ' ')
		BadFmt(fi);
	fi->se = se;
	if (expected) {
		while(*se <= ' ' && *se)
			se++;
		if (!strcmp(se,expected)) {
			fprintf(stderr, "ampldak: Expected \"%s\", but got \"%s\"\n",
				expected, se);
			BadFmt(fi);
			}
		}
	return rv;
	}

 double
Dread(FileInfo *fi)
{
	char *s, *se;
	double rv;

	++fi->lineno;
	if (!fgets(s = fi->buf, sizeof(fi->buf), fi->f))
		BadRead(fi);
	rv = strtod(s, &se);
	if (s == se || *se > ' ')
		BadFmt(fi);
	fi->se = se;
	return rv;
	}

 static void
grdout(FILE *f, int n1, int *p, real *g)
{
	int i;

	fprintf(f, "[");
	for(i = 0; i < n1; i++)
		fprintf(f, " %.17g", g[p[i]]);
	fprintf(f, " ]\n");
	}

 static void
hesout(FILE *f, int n1, int *p, real *h)
{
	int i, j;

	fprintf(f, "[[");
	for(i = 0; i < n1; i++, h += n1) {
		for(j = 0; j < n1; j++)
			fprintf(f, " %.17g", h[p[j]]);
		}
	fprintf(f, " ]]\n");
	}

 static int
exists(const char *fname, int squawk)
{
	struct stat stb;
	const char *msg[2] = { "already exists", "is not a named pipe" };

	if (!stat(fname, &stb)) {
		if (S_ISFIFO(stb.st_mode))
			return 2;
		if (squawk)
			Squawk("\"%s\" %s.\n", fname, msg[squawk-1]);
		return 1;
		}
	return 0;
	}

/**** End of stuff from dakampl.c  ****/
/**** but process() contains some stuff from main() from dakampl.c */

 static char mbuf[4096]; /*DEBUG*/ /* MATLAB output */

 static int
process(Engine *ep, char *parname, char *resname)
{
	FILE *f;
	FileInfo fi;
	char *s;
	int *asv, asvkind[3], i, j, k, m1, n1, n2, nd, *p, rc;
	mxArray *X;
	real *x;

	rc = 1;
	if (!(fi.f = fopen(parname,"r"))) {
		BadOpen("parameters_file", parname);
		goto done1;
		}
	fi.fname = s;
	fi.lineno = 0;

	n1 = Iread(&fi, "variables");
	X = mxCreateDoubleMatrix(1, n1, mxREAL);
	x = (real*)mxGetPr(X);

	for(i = 0; i < n1; i++)
		x[i] = Dread(&fi);

	m1 = Iread(&fi, "functions");

	asvkind[0] = asvkind[1] = asvkind[2] = 0;
	asv = (int*)Malloc((n1+m1)*sizeof(int));
	p = asv + m1;
	for(i = 0; i < m1; i++) {
		asv[i] = j = Iread(&fi,0);
		for(k = 0; k < 3 && j; k++, j >>= 1)
			if (j & 1)
				++asvkind[k];
		}
	nd = Iread(&fi, "derivative_variables");
	for(i = 0; i < nd; i++)
		p[i] = Iread(&fi, 0) - 1;
	fclose(fi.f);
	f = fopen(resname, "w");
	if (!f) {
		BadOpen("results_file", resname);
		mxDestroyArray(X);
		goto done1;
		}
	engPutVariable(ep, "x", X);
	mxDestroyArray(X);

	X = mxCreateDoubleMatrix(1, 3, mxREAL);
	x = (real*)mxGetPr(X);
	for(i = 0; i < 3; i++)
		x[i] = asvkind[i];
	engPutVariable(ep, "asv", X);
	mxDestroyArray(X);

	if (asvkind[0]) {
		engEvalString(ep, "y = f(x);");
		X = engGetVariable(ep,"y");
		if (!X) {
			Squawk("MATLAB didn't return y = f(x)\n");
			goto done;
			}
		x = (real*)mxGetPr(X);
		for(i = 0; i < m1; i++)
			if (asv[i] & 1)
				fprintf(f, " %.17g\n", x[i]);
		mxDestroyArray(X);
		}
	if (asvkind[1]) {
		engEvalString(ep, "yp = fp(x);");
		X = engGetVariable(ep,"yp");
		if (!X) {
			Squawk("MATLAB didn't return yp = fp(x)\n");
			goto done;
			}
		x = (real*)mxGetPr(X);
		for(i = 0; i < m1; i++, x += n1)
			if (asv[i] & 2)
				grdout(f, n1, p, x);
		mxDestroyArray(X);
		}
	if (asvkind[2]) {
		engEvalString(ep, "ypp = fpp(x);");
		X = engGetVariable(ep,"ypp");
		if (!X) {
			Squawk("MATLAB didn't return ypp = fpp(x)\n");
			goto done;
			}
		x = (real*)mxGetPr(X);
		n2 = n1*n1;
		for(i = 0; i < m1; i++, x += n2)
			if (asv[i] & 4)
				hesout(f, n1, p, x);
		mxDestroyArray(X);
		}
	rc = 0;
 done:
	fclose(f);
 done1:
	free(asv);
	return rc;
	}

 void
trim(char *s)
{
	char *s0 = s;
	while(*s)
		++s;
	while(s > s0 && s[-1] <= ' ')
		--s;
	*s = 0;
	}

 static int
mkfifo_fail(const char *pname)
{
	Squawk("Could not create \"%s\" as named pipe.\n", pname);
	return 1;
	}

 static int
server(char **argv, char *pname[2], int p[2])
{
	Engine *ep;
	FILE *f;
	char buf[4096], buf1[4096], *msg, *s, *t;
	int rc;
#if 0/*def SERVER_DEBUG*/
	printf("Server got\tpname[0] = \"%s\"\nand\t\tpname[1] = \"%s\"\n",
		pname[0], pname[1]);
	if (*argv) {
		int i;
		printf("Args for MATLAB to interpret:\n");
		for(i = 0; argv[i]; i++)
			printf("\t\"%s\"\n", argv[i]);
		}
#endif
	if (exists(pname[0], 1) || exists(pname[1], 1))
		return 1;
	if (mkfifo(pname[0], 0600))
		return mkfifo_fail(pname[0]);
	if (mkfifo(pname[1], 0600)) {
		unlink(pname[0]);
		return mkfifo_fail(pname[1]);
		}
	s = *argv;
	ep = engOpen(s ? s : "");
	if (!ep) {
		Squawk("could not start MATLAB\n");
		return 1;
		}
	/*DEBUG*/engOutputBuffer(ep, mbuf, sizeof(mbuf)-1);
	if (s)
		while(s = *++argv)
			engEvalString(ep, s);
	if (p[1] >= 0) {
		close(p[0]);
		write(p[1], "OK\n", 3);
		close(p[1]);
		}
	rc = 1;
	for(;;) {
		f = fopen(pname[0], "r");
		if (!f)
			break;
		s = fgets(buf, sizeof(buf), f);
		if (!s) {
			fclose(f);
			break;
			}
		trim(s);
		if (!*s) {
			Squawk("server: empty parameters_file name\n");\
 bailout:
			fclose(f);
			break;
			}
		if (!strcmp(s,"quit")) {
			rc = 0;
			goto bailout;
			}
		t = fgets(buf1, sizeof(buf1), f);
		fclose(f);
		if (!t) {
			Squawk("server expected 2 lines from \"%s\"; only got 1.\n",
				pname[0]);
			break;
			}
		trim(t);
		msg = process(ep, s, t) ? "evaluation error" : "results_file written";
		f = fopen(pname[1],"w");
		if (!f) {
			Squawk("Could not open pipe2 file \"%s\"\n", pname[1]);
			break;
			}
		fprintf(f, "%s\n", msg);
		fclose(f);
		}
	engClose(ep);
	unlink(pname[0]);
	unlink(pname[1]);
	return rc;
	}

 int
main(int argc, char **argv, char *const envp[])
{
	FILE *f;
	char *av[3], buf[80], *pb, *pname[2], *s;
	int ex[2], i, j, k, p[2];
	pid_t pid;
	size_t L, L2;

	progname = *argv;
	if (!(pb = getenv("DAKMATLAB_PIPE")))
		pb = "dakmatlab_pipe";
	L = strlen(pb);
	L2 = L + 2;
	pname[0] = (char*)Malloc(2*L2);
	pname[1] = pname[0] + L2;
	strcpy(s = pname[0], pb);
	s[L] = '1';
	s[L+1] = 0;
	strcpy(s = pname[1], pb);
	s[L] = '2';
	s[L+1] = 0;
	if (argc < 2)
		return usage(1);
	s = argv[1];
	buf[0] = 0;
	if (*s == '-')
		switch(s[1]) {
			case '?': return usage(0);
			case '-':
				if (!s[2]) {
					++argv;
					--argc;
					break;
					}
				return usage(strcmp(s+2,"help") ? 1 : 0);
			case 's':
				i = 1;
				if (s[2] == '!' && !s[3]) /*DEBUG*/
					i = 0;
				else if (s[2])
					return usage(1);
				p[0] = p[1] = -1;
				if (i) {
					if (pipe(p)) {
						perror("pipe() failure");
						return 1;
						}
					if ((pid = fork())) {
						close(p[1]);
						k = read(p[0], buf, sizeof(buf));
						close(p[0]);
						if (k == 3 && !strncmp(buf,"OK\n",k))
							return 0;
						Squawk("Failed to start MATLAB engine\n");
						return 1;
						}
					}
				return server(argv+2, pname, p);
			default:
				return usage(1);
			}
	if (argc != 3) {
		Squawk("expected two arguments, parameters_file and results_file.\n");
		return usage(1);
		}
	if ((ex[0] = exists(pname[0], 2)) == 1) {
		Squawk("\"%s\" is not a named pipe.\n", pname[0]);
 bailout:
		creat(argv[2], 0666);	/* empty file for DAKOTA to read */
		return 1;
		}
	if ((ex[1] = exists(pname[1], 2)) == 1) {
		Squawk("\"%s\" is not a named pipe.\n", pname[1]);
		goto bailout;
		}
	if ((k = ex[0] + ex[1]) == 2) {
		i = ex[1] == 2;
		j = 1 - i;
		Squawk("\"%s\" is a named pipe, but \"%s\" is not.\n",
			pname[i], pname[j]);
		goto bailout;
		}
	if (!k) {
		if (pipe(p)) {
			perror("pipe() failure");
			return 1;
			}
		pid = fork();
		if (!pid)
			return server(argv+3, pname, p);
		close(p[1]);
		k = read(p[0], buf, sizeof(buf));
		close(p[0]);
		if (k != 3  || strncmp(buf,"OK\n",k)
		    || exists(pname[0], 0) != 2
		    || exists(pname[1], 0) != 2) {
			Squawk("failed to start self as server\n");
			goto bailout;
			}
		}
	f = fopen(pname[0], "w");
	if (!f) {
		Squawk("Could not open \"%s\"\n", pname[0]);
		goto bailout;
		}
	fprintf(f, "%s\n%s\n", argv[1], argv[2]);
	fclose(f);
	f = fopen(pname[1], "r");
	s = fgets(buf, sizeof(buf), f);
	fclose(f);
	if (!s) {
		Squawk("empty reply from server\n");
		goto bailout;
		}
	if (strcmp(s, "evaluation error\n"))
		return 1;
	if (strcmp(s, "results_file written\n")) {
		Squawk("bad reply from server\n");
		goto bailout;
		}
	return 0;
	}
