/* Act as analysis driver for DAKOTA */

#include "getstub.h"

 char Title[] = "\nDAKOTA/AMPL Driver Version 20061010\n";

 static void
Squawk(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(Stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	}

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

 static void
BadOpen(const char *what, const char *name)
{
	Squawk("Unable to open %s file \"%s\"\n", what, name);
	exit(1);
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

 static void
BadNo_of(FileInfo *fi, const char *what, int ndak, int nnl)
{
	Squawk("DAKOTA sees %d %s, but %s%s has %d\n",
		ndak, what, fi->stub, fi->nlext, nnl);
	exit(1);
	}

 static void
BadEval(FileInfo *fi, const char *what)
{
	Squawk("error evaluating %s in %s%s\n",
		what, fi->stub, fi->nlext);
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
			Squawk("Expected \"%s\", but got \"%s\"\n",
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

 typedef struct
Option { const char *name, *desc; } Option;

 static Option options[] = {
	{"-help", "explain usage"},
	{"?", "explain usage"},
	{"f", "list available user-defined functions"}
	};

 static int
usage(int rc)
{
	const Option *kw, *kwe;

	fprintf(Stderr, "Usage: %s dakota.in dakota.out\n%s\n%s\n%s\n%s\nOptions:\n",
		progname,
		"This program is meant to be invoked by DAKOTA to do evaluations of",
		"a .nl file whose stub is $DAKAMPLPROB if set, else \"dakamplprob\".",
		"dakota.in  = name of file DAKOTA wrote to request computations.",
		"dakota.out = name of file DAKOTA will read for results.");
	kw = options;
	kwe = kw + sizeof(options)/sizeof(options[0]);
	for(; kw < kwe; kw++)
		fprintf(Stderr, "\t-%s\t==> %s.\n", kw->name, kw->desc);
	return rc;
	}

 static void
grdout(FILE *f, int n1, int *p, real *g)
{
	int i;

	fprintf(f, "[");
	for(i = 0; i < n1; i++)
		fprintf(f, " %.g", g[p[i]]);
	fprintf(f, " ]\n");
	}

 static void
hesout(ASL *asl, FILE *f, int n1, real *y, int *p, real *h, real *v)
{
	int i, j, no;

	no = y ? -1 : 0;
	fprintf(f, "[[");
	for(i = 0; i < n1; i++) {
		v[j = p[i]] = 1.;
		hvcomp(h, v, no, 0, y);
		v[j] = 0.;
		for(j = 0; j < n1; j++)
			fprintf(f, " %.g", h[p[j]]);
		}
	fprintf(f, " ]]\n");
	}

 int
main(int argc, char **argv)
{
	ASL *asl;
	FILE *f;
	FileInfo fi;
	char *s, *s1;
	fint nerror;
	int *asv, *asv1, asvkind[3], i, j, k, m, m1, mno, n, n1, no, *p;
	real F, *c, *g, *v, *x;

	progname = *argv++;
	asl = ASL_alloc(ASL_read_pfgh); /* for Stderr */

	if ((s = *argv++) && *s == '-')
		switch(s[1]) {
			case '?':
				return usage(0);
			case 'f':
				show_funcs();
				return 0;
			case '-':
				if (!s[2]) {
					--argc;
					s = *argv++;
					break;
					}
				return usage(strcmp(s+2,"help") ? 1 : 0);
			default:
				return usage(1);
			}
	if (argc != 3)
		return usage(1);
	if (!(fi.stub = getenv("DAKAMPLPROB")))
		fi.stub = "dakamplprob";
	for(s1 = fi.stub; *s1; s1++);
	fi.nlext = s1 - fi.stub > 3 && !strcmp(s1-3,".nl") ? "" : ".nl";
	f = jac0dim(fi.stub, 0);
	if (!f) {
		Squawk("Cannot open %s%s\n", fi.stub, fi.nlext);
		return 1;
		}
	if ((no = n_obj) > 1) {
		Squawk("%s%s has %d objectives; expected at most 1\n",
			fi.stub, fi.nlext, no);
		return 1;
		}
	if (!(fi.f = fopen(s,"r")))
		BadOpen("parameters_file", s);
	fi.fname = s;
	fi.lineno = 0;

	n1 = Iread(&fi, "variables");
	n = n_var;
	if (n != n1)
		BadNo_of(&fi, "variables", n1, n);

	X0 = x = (real*)M1alloc(2*sizeof(real)*n);
	g = x + n;
	pfgh_read_ASL(asl, f, ASL_findgroups);

	for(i = 0; i < n; i++)
		x[i] = Dread(&fi);

	m = n_con;
	mno = m + no;
	m1 = Iread(&fi, "functions");
	if (m1 != mno)
		BadNo_of(&fi, "functions", m1, mno);

	asvkind[0] = asvkind[1] = asvkind[2] = 0;
	asv = asv1 = (int*)M1alloc((n+mno)*sizeof(int));
	p = asv + mno;
	for(i = 0; i < mno; i++) {
		asv[i] = j = Iread(&fi,0);
		for(k = 0; k < 3 && j; k++, j >>= 1)
			if (j & 1)
				++asvkind[k];
		}
	n1 = Iread(&fi, "derivative_variables");
	if (n < n1)
		BadNo_of(&fi, "derivative_variables", n1, n);
	for(i = 0; i < n1; i++)
		p[i] = Iread(&fi, 0) - 1;
	fclose(fi.f);
	f = fopen(*argv, "w");
	if (!f)
		BadOpen("results_file", *argv);
	nerror = 0;
	if (no) {
		asv1++;
		F = objval(0, x, &nerror);
		if (nerror)
			BadEval(&fi, "objective");
		}
	if (m) {
		c = (real*)M1alloc(m*sizeof(real));
		conval(x, c, &nerror);
		if (nerror)
			BadEval(&fi, "constraint(s)");
		}
	if (asvkind[0]) {
		if (no && asv[0] & 1)
			fprintf(f, " %.g\n", F);
		if (m)
			for(i = 0; i < m; i++)
				
				fprintf(f, " %.g\n", c[i]);
		}
	if (asvkind[1]) {
		if (no && asv[0] & 2) {
			objgrd(0, x, g, 0);
			grdout(f, n1, p, g);
			}
		if (m)
			for(i = 0; i < m; i++) {
				if (asv1[i] & 2) {
					congrd(i, x, g, 0);
					grdout(f, n1, p, g);
					}
				}
		}
	if (asvkind[2]) {
		v = (real*)M1zapalloc(n*sizeof(real));
		if (no && asv[0] & 4) {
			hesout(asl, f, n1, 0, p, g, v);
			}
		if (m) {
			memset(c, 0, m*sizeof(real));
			for(i = 0; i < m; i++) {
				if (asv1[i] & 4) {
					c[i] = 1.;
					hesout(asl, f, n1, c, p, g, v);
					c[i] = 0.;
					}
				}
			}
		}
	fclose(f);
	return 0;
	}
