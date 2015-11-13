/* Supply NL2SOL test problems to Dakota */
#include <cstdio>
#include <cstdlib>
#include <cstring>
/* #include "f2c.h" */


typedef int integer;
typedef double doublereal;
typedef int /* Unknown procedure type */ (*U_fp)();

extern int testj_(integer *n, integer *p, doublereal *x, integer *nfcall,
	doublereal *j, integer *uiparm, doublereal *urparm, U_fp ufparm);

extern int testr_(integer *n, integer *p, doublereal *x, integer *nfcall,
	doublereal *R, integer *uiparm, doublereal *urparm, U_fp ufparm);

 static void
BadRead(int n, char *fname)
{
	std::fprintf(stderr, "Unable to read line %d of file \"%s\"\n", n, fname);
	}

 static void
BadFmt(int n, char *fname, char *Line)
{
	std::fprintf(stderr, "Bad format, line %d of file \"%s\":\n%s\n", n, fname, Line);
	}

 typedef struct
KnownProbs {
	int p;
	int n;
	int nex;
	const char *name;
	} KnownProbs;

 static KnownProbs Probs[] = {
		{2,	2,	1,	"Rosnbrok"},
		{2,	2,	7,	"Branin"},
		{2,	2,	12,	"Frdstein"},
		{2,	3,	8,	"Beale"},
		{2,	3,	24,	"Madsen"},
		{2,	10,	20,	"Jennrich"},
		{3,	3,	2,	"Helix"},
		{3,	3,	5,	"Zangwill"},
		{3,	5,	6,	"Engvall"},
		{3,	10,	10,	"Box"},
		{3,	15,	19,	"Bard"},
		{3,	16,	25,	"Meyer"},
		{4,	4,	3,	"Singular"},
		{4,	5,	9,	"Cragg"},
		{4,	7,	4,	"Woods"},
		{4,	11,	21,	"Kowalik"},
		{4,	20,	18,	"Brown"},
		{5,	33,	22,	"Osborne1"},
		{6,	31,	13,	"Watson6"},
		{8,	8,	17,	"Chebqd8"},
		{9,	31,	14,	"Watson9"},
		{11,	65,	23,	"Osborne2"},
		{12,	31,	15,	"Watson12"},
		{15,	15,	11,	"Davidon1"},
		{20,	31,	16,	"Watson20"},
		{999,	0,	0,	0}
		};

 static int
Bsearch(int p, int n)
{
	KnownProbs *kp;
	char *b;
	int i, j, k, p1, n1;

	i = 0;
	k = sizeof(Probs)/sizeof(KnownProbs) - 1;
	for(;;) {
		kp = Probs + i + (j = k >> 1);
		p1 = kp->p;
		n1 = kp->n;
		if (p1 == p && n1 == n)
			break;
		if (p1 < p || (p1 == p && n1 < n)) {
			i += ++j;
			k -= j;
			}
		else
			k = j;
		if (k <= 0)
			return 0;
		}
	while (kp > Probs && kp[-1].p == p && kp[-1].n == n)
		--kp;
	if ((b = std::getenv("ProbName"))) {
		while(std::strcmp(b,kp->name)) {
			++kp;
			if (kp->p != p || kp->n != n)
				return 0;
			}
		}
	return kp->nex;
	}

 static void
Bad_pn(int p, int n)
{
	KnownProbs *kp;
	std::fprintf(stderr, "Unexpected combination of %d variables and %d responses.\n",
		p, n);
	std::fprintf(stderr, "Acceptable combinations:\n\tvars\tresps\t$ProbName\n");
	for(kp = Probs; kp->name; kp++)
		std::fprintf(stderr, "\t%d\t%d\t%s\n", kp->p, kp->n, kp->name);
	}

 int
main(int argc, char **argv)
{
	FILE *f;
	int asv[65], i, j, k, rc;
	integer n, nex, nf, p;
	char *b, buf[256];
	double J[65*11], R[65], x[20];

	if (argc != 3) {
		std::fprintf(stderr, "%s: %s,\nbut got %d args\n",
			argv[0], "Expected two args, the input and output file names", argc);
		return 1;
		}
	f = std::fopen(argv[1], "r");
	if (!f) {
		std::fprintf(stderr, "%s: could not open \"%s\"\n", argv[0], argv[1]);
		return 1;
		}
	rc = 1;
	if (!fgets(buf, sizeof(buf), f)) {
		BadRead(1, argv[1]);
		goto done;
		}
	p = std::strtol(buf, &b, 10);
	if (b <= buf || *b > ' ' || p <= 0) {
		BadFmt(1, argv[1], buf);
		goto done;
		}
	/*
	while(*b && *b <= ' ')
		b++;
	if (std::strncmp(b,"variables ",10)) {
		BadFmt(1, argv[1], buf);
		goto done;
		}
	*/
	for (i = 0; i < p; i++) {
		if (!fgets(buf, sizeof(buf), f)) {
			BadRead(i+2, argv[1]);
			goto done;
			}
		x[i] = std::strtod(buf, &b);
		if (b <= buf || *b > ' ') {
			BadFmt(i+2, argv[1], buf);
			goto done;
			}
		}
	if (!fgets(buf, sizeof(buf), f)) {
		BadRead(p + 2, argv[1]);
		goto done;
		}
	n = std::strtol(buf, &b, 10);
	if (b <= buf || *b > ' ' || n <= 0) {
		BadFmt(p + 2, argv[1], buf);
		goto done;
		}
	for (i = j = 0; i < n; i++) { /* j = 0 to stop erroneous warning with -Wall */
		if (!fgets(buf, sizeof(buf), f)) {
			BadRead(i+p+3, argv[1]);
			goto done;
			}
		j = asv[i] = std::strtol(buf, &b, 10);
		if (b <= buf || *b > ' ' || j < 1 || j > 3) {
			BadFmt(i+p+3, argv[1], buf);
			goto done;
			}
		}
	if (!(nex = Bsearch(p,n))) {
		Bad_pn(p,n);
		return 1;
		}
	for(i = k = 0; i < n; i++)
		if (asv[i] != j)
			k = asv[i];
	if (k) {
		std::fprintf(stderr, "Got varying ASV values, including %d and %d\n", j, k);
		goto done;
		}
	std::fclose(f);
	f = std::fopen(argv[2], "w");
	if (!f) {
		std::fprintf(stderr, "%s: could not open \"%s\"\n", argv[0], argv[2]);
		return 1;
		}
	nf = 1;
	if (j & 1) {
		testr_(&n, &p, x, &nf, R, &nex, 0, 0);
		for(i = 0; i < n; i++)
			std::fprintf(f, " %.17g\n", R[i]);
		}
	if (j & 2) {
		testj_(&n, &p, x, &nf, J, &nex, 0, 0);
		for(i = 0; i < n; i++) {
			std::fprintf(f, "[ %.17g", J[i]);
			for(k = 1; k < p; k++)
				std::fprintf(f, " %.17g", J[i+n*k]);
			std::fprintf(f, " ]\n");
			}
		}
	rc = 0;
 done:
	if (f)
		fclose(f);
	return rc;
	}
