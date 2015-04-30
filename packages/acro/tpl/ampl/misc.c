/****************************************************************
Copyright (C) 1997-2001 Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

/* include vararg/stdarg stuff first to avoid trouble with C++ */
#ifdef KR_headers
#include "varargs.h"
#else
#include "stddef.h"
#include "stdarg.h"
#endif

#include "asl.h"

#ifdef __cplusplus
extern "C" {
#endif

real edagread_one = 1.;
char *progname;
ASL *cur_ASL;
ASLhead ASLhead_ASL = {&ASLhead_ASL, &ASLhead_ASL};

 static char anyedag[] = "fg_read (or one of its variants)";
 static char psedag[] = "pfg_read, pfgh_read, or jacpdim";

 ASL *
#ifdef KR_headers
set_cur_ASL(a) ASL *a;
#else
set_cur_ASL(ASL *a)
#endif
{
	ASL *rv = cur_ASL;
	cur_ASL = a;
	return rv;
	}

  ASL *
get_cur_ASL(VOID)
{ return cur_ASL; }

 void
#ifdef KR_headers
exit_ASL(R, n) EdRead *R; int n;
#else
exit_ASL(EdRead *R, int n)
#endif
{
	Jmp_buf *J;
	if ((J = R->asl->i.err_jmp_) && n > 0)
		longjmp(J->jb, n);
	exit(n);
	}

 void
scream
#ifdef KR_headers
	(va_alist)
 va_dcl
{
	EdRead *R;
	char *fmt;
	int n;
	va_list ap;
	va_start(ap);
	R = va_arg(ap, EdRead*);
	n = va_arg(ap, int);
	fmt = va_arg(ap, char*);
#else
	(EdRead *R, int n, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
#endif
	vfprintf(Stderr, fmt, ap);
	exit_ASL(R, n);
	} /*}*/

 static real
#ifdef KR_headers
notread(what, pred) char *what, *pred;
#else
notread(char *what, char *pred)
#endif
{
	fprintf(Stderr, "\n*** %s called before %s.\n", what, pred);
	exit(1);
	/* not reached */
	return 0;
	}

 static real
#ifdef KR_headers
obj0val(a, nobj, X, nerror) ASL *a; real *X; int nobj; fint *nerror;
#else
obj0val(ASL *a, int nobj, real *X, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(nobj);
	Not_Used(X);
	Not_Used(nerror);
	return notread("objval", anyedag);
	}

 static void
#ifdef KR_headers
obj0grd(a, nobj, X, G, nerror) ASL *a; int nobj; fint *nerror; real *X, *G;
#else
obj0grd(ASL *a, int nobj, real *X, real *G, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(nobj);
	Not_Used(X);
	Not_Used(G);
	Not_Used(nerror);
	notread("objgrd", anyedag);
	}

 static void
#ifdef KR_headers
con0val(a, X, R, nerror) ASL *a; real *X, *R; fint *nerror;
#else
con0val(ASL *a, real *X, real *R, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(X);
	Not_Used(R);
	Not_Used(nerror);
	notread("conval", anyedag);
	}

 static void
#ifdef KR_headers
jac0val(a, X, J, nerror) ASL *a; real *X, *J; fint *nerror;
#else
jac0val(ASL *a, real *X, real *J, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(X);
	Not_Used(J);
	Not_Used(nerror);
	notread("jacval", anyedag);
	}

 static real
#ifdef KR_headers
con0ival(a, i, X, nerror) ASL *a; int i; real *X; fint *nerror;
#else
con0ival(ASL *a, int i, real *X, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(i);
	Not_Used(X);
	Not_Used(nerror);
	notread("conival", anyedag);
	return 0.;
	}

 static int
#ifdef KR_headers
lcon0val(a, i, X, nerror) ASL *a; int i; real *X; fint *nerror;
#else
lcon0val(ASL *a, int i, real *X, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(i);
	Not_Used(X);
	Not_Used(nerror);
	notread("conival", anyedag);
	return 0;
	}

 static void
#ifdef KR_headers
con0grd(a, i, X, G, nerror) ASL *a; int i; real *X, *G; fint *nerror;
#else
con0grd(ASL *a, int i, real *X, real *G, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(i);
	Not_Used(X);
	Not_Used(G);
	Not_Used(nerror);
	notread("congrd", anyedag);
	}

 static void
#ifdef KR_headers
hv0comp(a, hv, p, nobj, ow, y) ASL *a; real *hv, *p, *ow, *y; int nobj;
#else
hv0comp(ASL *a, real *hv, real *p, int nobj, real *ow, real *y)
#endif
{
	Not_Used(a);
	Not_Used(hv);
	Not_Used(p);
	Not_Used(nobj);
	Not_Used(ow);
	Not_Used(y);
	notread("hvcomp", "pfgh_read or fgh_read");
	}

 static void
#ifdef KR_headers
hv0init(a, n, no, ow, y) ASL *a; int n, no; real *ow, *y;
#else
hv0init(ASL *a, int n, int no, real *ow, real *y)
#endif
{
	Not_Used(a);
	Not_Used(n);
	Not_Used(no);
	Not_Used(ow);
	Not_Used(y);
	notread("hvinit", "pfgh_read");
	}

 static void
#ifdef KR_headers
hes0set(a, flags, obj, nobj, con, ncon) ASL *a; int flags; int obj;
					int nobj; int con; int ncon;
#else
hes0set(ASL *a, int flags, int obj, int nobj, int con, int ncon)
#endif
{
	Not_Used(a);
	Not_Used(flags);
	Not_Used(obj);
	Not_Used(nobj);
	Not_Used(con);
	Not_Used(ncon);
	notread("duthes, fullhes, or sputhes", "pfgh_read or jacpdim");
	}

 static void
#ifdef KR_headers
x0known(a, x, nerror) ASL *a; real *x; fint *nerror;
#else
x0known(ASL *a, real *x, fint *nerror)
#endif
{
	Not_Used(a);
	Not_Used(x);
	Not_Used(nerror);
	notread("xknown", psedag);
	}

 static void
#ifdef KR_headers
dut0hes(a, H, nobj, ow, y) ASL *a; real *H; int nobj; real *ow, *y;
#else
dut0hes(ASL *a, real *H, int nobj, real *ow, real *y)
#endif
{
	Not_Used(a);
	Not_Used(H);
	Not_Used(nobj);
	Not_Used(ow);
	Not_Used(y);
	notread("duthes", "pfgh_read or jacpdim");
	}

 static void
#ifdef KR_headers
ful0hes(a, H, LH, nobj, ow, y) ASL *a; real *H, *ow, *y; fint LH; int nobj;
#else
ful0hes(ASL *a, real *H, fint LH, int nobj, real *ow, real *y)
#endif
{
	Not_Used(a);
	Not_Used(H);
	Not_Used(LH);
	Not_Used(nobj);
	Not_Used(ow);
	Not_Used(y);
	notread("fullhes", "pfgh_read or jacpdim");
	}

 static void
#ifdef KR_headers
sut0hes(a, p, H, nobj, ow, y) ASL *a; SputInfo **p; real *H; int nobj; real *ow, *y;
#else
sut0hes(ASL *a, SputInfo **p, real *H, int nobj, real *ow, real *y)
#endif
{
	Not_Used(a);
	Not_Used(p);
	Not_Used(H);
	Not_Used(nobj);
	Not_Used(ow);
	Not_Used(y);
	notread("sputhes", "pfgh_read or jacpdim");
	}

 static fint
#ifdef KR_headers
sut0set(a, p, nobj, have_ow, have_y, both)
	ASL *a; SputInfo **p; int nobj, have_ow, have_y, both;
#else
sut0set(ASL *a, SputInfo **p, int nobj, int have_ow, int have_y, int both)
#endif
{
	Not_Used(a);
	Not_Used(p);
	Not_Used(nobj);
	Not_Used(have_ow);
	Not_Used(have_y);
	Not_Used(both);
	notread("sputset", "pfgh_read or jacpdim");
	return 0;
	}

Edagpars edagpars_ASL = {
	{0,0},	/* h */
	1.,	/* hffactor */
	5,	/* FUNNEL_MIN */
	5,	/* maxfwd */
	1,	/* need_funcadd */
	100,	/* vrefGulp */
	1,	/* want_derivs */
	12,	/* ihd_limit */
	-1,	/* solve_code */
	obj0val,
	obj0grd,
	con0val,
	jac0val,
	con0ival,
	con0grd,
	hv0comp,
	hv0init,
	hes0set,
	lcon0val,
	x0known,
	dut0hes,
	ful0hes,
	sut0hes,
	sut0set
	};

 int
#ifdef KR_headers
edag_peek(R) EdRead *R;
#else
edag_peek(EdRead *R)
#endif
{
	int c;
	R->Line++;
	R->lineinc = 0;
	R->rl_buf[0] = c = getc(R->nl);
	return c;
	}

 static void
#ifdef KR_headers
eatcr(nl) FILE *nl;
#else
eatcr(FILE *nl)
#endif
{
	int c;

	while((c = getc(nl)) == '\r');
	if (c >= 0 && c != '\n')
		ungetc(c, nl);
	}

 char *
#ifdef KR_headers
read_line(R) EdRead *R;
#else
read_line(EdRead *R)
#endif
{
	char *s, *se;
	int x;
	char *rv;
	FILE *nl = R->nl;

	s = R->rl_buf;
	se = s + sizeof(R->rl_buf) - 1;
	if (R->lineinc)
		R->Line++;
	else {
		s++;
		R->lineinc = 1;
		}
	rv = s;
	for(;;) {
		x = getc(nl);
		if (x < ' ') {
			if (x < 0) {
 eof:
				if (R->can_end)
					return 0;
				fprintf(Stderr,
				 "Premature end of file, line %ld of %s\n",
					R->Line, R->asl->i.filename_);
				exit_ASL(R,1);
				}
			if (x == '\n')
				break;
			if (x == '\r') {
				eatcr(nl);
				break;
				}
			}
		*s++ = x;
		if (s >= se) {
			for(;;) {
				x = getc(nl);
				if (x == '\r') {
					eatcr(nl);
					goto eol;
					}
				if (x == '\n')
					goto eol;
				if (x < 0)
					goto eof;
				}
			}
		}
 eol:
	*s = 0;
	return rv;
	}

 static void
#ifdef KR_headers
memfailure(who, what, len) char *who, *what; size_t len;
#else
memfailure(char *who, char *what, size_t len)
#endif
{
	fprintf(Stderr, "%s(%lu) failure: %s.\n", who, (long)len, what);
	exit(1);
	}

static char	ran_out[] =	"ran out of memory";

#ifdef KR_headers
 Char *
mymalloc(len) size_t len;
#else
 void *
mymalloc(size_t len)
#endif
{
#ifdef KR_headers
	char *rv;
#else
	void *rv;
#endif
	static char who[] = "malloc";
	rv = malloc(len);
	if (!rv) {
		/* Defend against stupid systems: malloc(0) */
		/* should return a nonzero value.  Routines in */
		/* amplsolver.a should never call malloc(0), but */
		/* solvers may do so. */
		if (!len)
			rv = malloc(sizeof(real));
		if (!rv)
			memfailure(who, ran_out, len);
		}
	return rv;
	}

#ifdef KR_headers
 Char *
myralloc(rv, len) char *rv; size_t len;
#else
 void *
myralloc(void *rv, size_t len)
#endif
{
	static char who[] = "realloc";
	rv = realloc(rv, len);
	if (!rv) {
		if (!len)
			rv = malloc(sizeof(real));
		if (!rv)
			memfailure(who, ran_out, len);
		}
	return rv;
	}

 void
what_prog(VOID)
{
	if (progname)
		fprintf(Stderr, "%s: ", progname);
	}

 void
#ifdef KR_headers
badread(R) EdRead *R;
#else
badread(EdRead *R)
#endif
{
	what_prog();
	fprintf(Stderr, "error reading line %ld of %s:\n\t", R->Line, R->asl->i.filename_);
	}

 void
#ifdef KR_headers
badline(R) EdRead *R;
#else
badline(EdRead *R)
#endif
{
	ASL *asl = R->asl;
	FILE *nl;
	char *s, *se;
	int x;

	fprintf(Stderr, "bad line %ld of %s", R->Line, filename);
	if (xscanf == ascanf) {
		if (!R->lineinc) {
			nl = R->nl;
			s = R->rl_buf;
			se = s + sizeof(R->rl_buf) - 1;
			while(s < se && (x = getc(nl)) >= ' ')
				*++s = x;
			*s = 0;
			}
		fprintf(Stderr, ": %s\n", R->rl_buf);
		}
	else
		fprintf(Stderr, "\n");
	exit_ASL(R,1);
	}

#undef asl

#define Mb_gulp 31
 typedef struct Mblock {
	struct Mblock *next;
	Char *m[Mb_gulp];
	} Mblock;

 Char **
#ifdef KR_headers
M1record_ASL(I, x) Edaginfo *I; Char *x;
#else
M1record_ASL(Edaginfo *I, Char *x)
#endif
{
	Mblock *mb;
	Char **rv;

	if (I->Mbnext >= I->Mblast) {
		mb = (Mblock *)Malloc(sizeof(Mblock));
		mb->next = (Mblock*)I->Mb;
		I->Mb = (Char*)mb;
		I->Mbnext = mb->m;
		I->Mblast = mb->m + Mb_gulp;
		}
	rv = I->Mbnext++;
	*rv = x;
	return rv;
	}

 Char *
#ifdef KR_headers
M1alloc_ASL(I, n) Edaginfo *I; size_t n;
#else
M1alloc_ASL(Edaginfo *I, size_t n)
#endif
{
	Mblock *mb;

	if (I->Mbnext >= I->Mblast) {
		mb = (Mblock *)Malloc(sizeof(Mblock));
		mb->next = (Mblock*)I->Mb;
		I->Mb = (Char*)mb;
		I->Mbnext = mb->m;
		I->Mblast = mb->m + Mb_gulp;
		}
	return *I->Mbnext++ = Malloc(n);
	}

 Char *
#ifdef KR_headers
M1zapalloc_ASL(I, n) Edaginfo *I; size_t n;
#else
M1zapalloc_ASL(Edaginfo *I, size_t n)
#endif
{
	Char *rv;

	memset(rv = M1alloc_ASL(I, n), 0, n);
	return rv;
	}

 void
#ifdef KR_headers
M1free_ASL(I, mnext, mlast) Edaginfo *I; Char **mnext, **mlast;
#else
M1free_ASL(Edaginfo *I, Char **mnext, Char **mlast)
#endif
{
	Char **x, **x0;
	Mblock *Mb, *mb;
	Char **Mblast;

	if (!(Mb = (Mblock *)I->Mb))
		return;
	x = (Char **)I->Mbnext;
	Mblast = I->Mblast;
	I->Mbnext = mnext;
	I->Mblast = mlast;
	x0 = Mb->m;
	for(;;) {
		if (mlast == Mblast)
			x0 = mnext;
		while(x > x0)
			if (*--x)
				free(*x);
		if (mlast == Mblast) {
			I->Mb = (Char*)Mb;
			return;
			}
		mb = Mb->next;
		free(Mb);
		if (!(Mb = mb))
			break;
		x0 = Mb->m;
		Mblast = x = x0 + Mb_gulp;
		}
	I->Mb = 0;
	}

 void
#ifdef KR_headers
xknown_(x) real *x;
#else
xknown_(real *x)
#endif
{
	ASL *asl;
	if (!(asl = cur_ASL))
		badasl_ASL(asl,0,"xknown");
	xknowne(x, (fint*)0);
	}

 void
#ifdef KR_headers
xknowe_(x, nerror) real *x; fint *nerror;
#else
xknowe_(real *x, fint *nerror)
#endif
{
	ASL *asl;
	if (!(asl = cur_ASL))
		badasl_ASL(asl,0,"xknown");
	xknowne(x, nerror);
	}


 void
xunkno_(VOID)
{
	ASL *asl;
	if (!(asl = cur_ASL))
		badasl_ASL(asl,0,"xunkno");
	asl->i.x_known = 0;
	}

 void
#ifdef KR_headers
mnnzchk_ASL(asl, M, N, NZ, who1) ASL*asl; fint *M,*N,*NZ; char*who1;
#else
mnnzchk_ASL(ASL *asl, fint *M, fint *N, fint *NZ, char *who1)
#endif
{
	int n;
	if (!asl || (n = asl->i.ASLtype) < ASL_read_fg || n > ASL_read_pfgh)
		badasl_ASL(asl, ASL_read_fg, who1);
	ASL_CHECK(asl, n, who1);
	if (*M != n_con || *N != c_vars || *NZ != nzjac) {
		what_prog();
		fprintf(Stderr,
 "%s: got M = %ld, N = %ld, NZ = %ld\nexpected M = %d, N = %d, NZ = %d\n",
			who1, (long)*M, (long)*N, *NZ, n_con, c_vars, nzjac);
		exit(1);
		}
	}

 void
#ifdef KR_headers
LUcopy_ASL(nv, L, U, LU) int nv; real *L, *U, *LU;
#else
LUcopy_ASL(int nv, real *L, real *U, real *LU)
#endif
{
	real *LUe;
	for(LUe = LU + 2*nv; LU < LUe; LU += 2) {
		*L++ = LU[0];
		*U++ = LU[1];
		}
	}

 int
#ifdef KR_headers
already_ASL(who) char *who;
#else
already_ASL(char *who)
#endif
{
	fprintf(Stderr, "%s called after ASL_alloc().\n", who);
	return 1;
	}

 void
#ifdef KR_headers
ASL_free(aslp) ASL **aslp;
#else
ASL_free(ASL **aslp)
#endif
{
	ASL *a;
	ASLhead *h;
	extern void at_end_ASL ANSI((Exitcall*));

	if (!(a = *aslp))
		return;
	if (a == cur_ASL)
		cur_ASL = 0;
	h = a->p.h.prev;
	(h->next = a->p.h.next)->prev = h;
	if (a->i.arprev)
		at_end_ASL(a->i.arprev);
	M1free(&a->i, (Char**)0, (Char**)0);
	free((Char*)a);
	*aslp = 0;
	}

 void
#ifdef KR_headers
badasl_ASL(a, n, who) ASL *a; int n; char *who;
#else
badasl_ASL(ASL *a, int n, char *who)
#endif
{
	if (!Stderr)
		Stderr_init_ASL();	/* set Stderr if necessary */
	if (a)
		fprintf(Stderr,
			"\n*** %s needs ASL_alloc(%d), not ASL_alloc(%d)\n",
			who, n, a->i.ASLtype);
	else if (n)
		fprintf(Stderr, "\n*** %s called before ASL_alloc(%d)\n",
			who, n);
	else
		fprintf(Stderr,
		 "\n*** %s called before ASL_alloc, jacdim, jac2dim, or jacpdim\n", who);
	exit(1);
	}

#define SKIP_NL2_DEFINES
#include "nlp.h"
#include "nlp2.h"
#include "asl_pfg.h"
#include "asl_pfgh.h"

 ASL *
#ifdef KR_headers
ASL_alloc(k) int k;
#else
ASL_alloc(int k)
#endif
{
	static int msize[5] = {
		sizeof(ASL_fg),
		sizeof(ASL_fg),
		sizeof(ASL_fgh),
		sizeof(ASL_pfg),
		sizeof(ASL_pfgh)
		};
	ASL *a;
	ASLhead *h;
	int n;

	if (!Stderr)
		Stderr_init_ASL();	/* set Stderr if necessary */
	Mach_ASL();
	if (k < 1 || k > 5)
		return 0;
	a = (ASL*) mymalloc(n = msize[k-1]);
	memcpy(a, &edagpars_ASL, sizeof(Edagpars));
	memset(&a->i, 0, n - sizeof(Edagpars));
	a->i.ASLtype = k;
	a->i.n_prob = 1;
	switch(k) {
	  case ASL_read_pfg:	((ASL_pfg*)a)->P.merge = 1; break;
	  case ASL_read_pfgh:	((ASL_pfgh*)a)->P.merge = 1;
	  }
	h = a->p.h.next = ASLhead_ASL.next;
	a->p.h.prev = h->prev;
	h->prev = ASLhead_ASL.next = &a->p.h;
	return cur_ASL = a;
	}

#define Egulp 400

 Char *
#ifdef KR_headers
mem_ASL(asl, len) ASL *asl; unsigned int len;
#else
mem_ASL(ASL *asl, unsigned int len)
#endif
{
	fint k;
	char *memNext;

	if (len >= 256)
		return M1alloc(len);
#ifdef Double_Align
	len = (len + (sizeof(real)-1)) & ~(sizeof(real)-1);
#else
	len = (len + (sizeof(int)-1)) & ~(sizeof(int)-1);
#endif
	ACQUIRE_DTOA_LOCK(MEM_LOCK);
	memNext = asl->i.memNext;
	if (memNext + len >= asl->i.memLast) {
		memNext = (char *)M1alloc(k = Egulp*Sizeof(expr) + len);
		asl->i.memLast = memNext + k;
		}
	asl->i.memNext = memNext + len;
	FREE_DTOA_LOCK(MEM_LOCK);
	return memNext;
	}

 EdRead *
#ifdef KR_headers
EdReadInit_ASL(R, asl, nl, S) EdRead *R; ASL *asl; FILE *nl; void *S;
#else
EdReadInit_ASL(EdRead *R, ASL *asl, FILE *nl, void *S)
#endif
{
	R->asl = asl;
	R->nl = nl;
	R->S = S;
	R->Line = 10;
	R->lineinc = 1;
	R->can_end = 0;
	R->dadjfcn = asl->i.dadjfcn;
	R->iadjfcn = asl->i.iadjfcn;
	return R;
	}

 void
#ifdef KR_headers
Suf_read_ASL(R, readall) EdRead *R; int readall;
#else
Suf_read_ASL(EdRead *R, int readall)
#endif
{
	int *d, isreal, i, k, n, nx, nx1;
	real *r, t;
	SufDesc *D;
	char *fmt;
	ASL *asl = R->asl;
	char sufname[128];

	if (xscanf(R, "%d %d %127s", &k, &n, sufname) != 3)
		badline(R);
	if (k < 0 || k > 7 || n <= 0)
		badline(R);
	isreal = k & ASL_Sufkind_real;
	k &= ASL_Sufkind_mask;
	nx = (&asl->i.n_var_)[k];
	if (k == 1)
		nx += n_lcon;
	if (n > nx)
		badline(R);
	if (readall & 1) {
 new_D:
		D = (SufDesc*)M1zapalloc(sizeof(SufDesc) + strlen(sufname) + 1);
		D->next = asl->i.suffixes[k];
		asl->i.suffixes[k] = D;
		asl->i.nsuff[k]++;
		asl->i.nsuffixes++;
		strcpy(D->sufname = (char*)(D+1), sufname);
		D->kind = k;
		if (isreal)
			D->kind |= ASL_Sufkind_real;
		}
	else for(D = asl->i.suffixes[k]; ; D = D->next) {
		if (!D) {
			if (readall)
				goto new_D;
 skip:
			/* Skip this suffix table */
			fmt = (char*)(isreal ? "%d %lf" : "%d %d");
			do if (xscanf(R,fmt,&k,&t) != 2)
					badline(R);
				while(--n);
			return;
			}
		if (k == (D->kind & ASL_Sufkind_mask)
		 && !strcmp(sufname,D->sufname))
			break;
		}
	if ((D->kind & ASL_Sufkind_outonly) == ASL_Sufkind_outonly)
		goto skip;
	nx1 = nx + D->nextra;
	if (D->kind & ASL_Sufkind_real) {
		D->u.i = 0;
		if (!(r = D->u.r))
			D->u.r = r = (real*)mem(nx1*sizeof(real));
		if (n < nx)
			memset(r,0,nx*sizeof(real));
		if (nx < nx1)
			memset(r+nx, 0, (nx1-nx)*sizeof(real));
		if (isreal)
			do  {
				if (xscanf(R,"%d %lf",&i,&t) != 2
				 || i < 0 || i >= nx)
					badline(R);
				r[i] = t;
				}
				while(--n);
		else
			do  {
				if (xscanf(R,"%d %d",&i,&k) != 2
				 || i < 0 || i >= nx)
					badline(R);
				r[i] = k;
				}
				while(--n);
		}
	else {
		D->u.r = 0;
		if (!(d = D->u.i))
			D->u.i = d = (int*)mem(nx1*sizeof(int));
		if (n < nx)
			memset(d,0,nx*sizeof(int));
		if (nx < nx1)
			memset(d+nx, 0, (nx1-nx)*sizeof(int));
		if (isreal)
			do {
				if (xscanf(R,"%d %lf",&i,&t) != 2
				 || i < 0 || i >= nx)
					badline(R);
				d[i] = (int)(t + 0.5);
				} while(--n);
		else
			do {
				if (xscanf(R,"%d %d",&i,&k) != 2
				 || i < 0 || i >= nx)
					badline(R);
				d[i] = k;
				}
				while(--n);
		}
	D->kind |= ASL_Sufkind_input;
	}

 real
#ifdef KR_headers
f_OPNUM_ASL(e) expr_n *e;
#else
f_OPNUM_ASL(expr_n *e)
#endif
{
#ifdef _WIN32	/* Work around a Microsoft linker bug... */
		/* Without the following test, f_OPNUM gets confused */
		/* with f_OPVARVAL.  Both get mapped to the same address */
		/* in the r_ops_ASL array defined in fg_read.c. */
	if (!e) {
		printf("f_OPNUM(e) has e = 0\n");
		return 0.;
		}
#endif
	return e->v;
	}

 void
#ifdef KR_headers
No_derivs_ASL(who) char *who;
#else
No_derivs_ASL(const char *who)
#endif
{
	fprintf(Stderr, "\nBUG: %s called with want_derivs == 0.\n", who);
	exit(1);
	}

 void
#ifdef KR_headers
suf_declare_ASL(asl, sd, n) ASL *asl; SufDecl *sd; int n;
#else
suf_declare_ASL(ASL *asl, SufDecl *sd, int n)
#endif
{
	SufDesc *d, *dnext[4];
	SufDecl *sde;
	int i, j;

	if (!asl)
		badasl_ASL(asl, 0, "suf_declare");
	asl->i.nsuffixes = 0;
	if (n > 0) {
		asl->i.nsuffixes = n;
		d = (SufDesc*)M1alloc(n*sizeof(SufDesc));
		memset(asl->i.nsuff, 0, 4*sizeof(int));
		for(i = 0; i < n; i++)
			asl->i.nsuff[sd[i].kind & ASL_Sufkind_mask]++;
		for(i = 0; i < 4; i++)
			if (j = asl->i.nsuff[i])
				asl->i.suffixes[i] = d += j;
		memset(dnext, 0, 4*sizeof(SufDesc*));
		for(sde = sd + n; sd < sde; sd++) {
			d = --asl->i.suffixes[i = sd->kind & ASL_Sufkind_mask];
			d->next = dnext[i];
			dnext[i] = d;
			d->sufname = sd->name;
			d->table = sd->table;
			d->kind = sd->kind & ~ASL_Sufkind_input;
			d->nextra = sd->nextra;
			d->u.i = 0;
			d->u.r = 0;
			}
		}
	}

 SufDesc *
#ifdef KR_headers
suf_get_ASL(asl, name, kind) ASL *asl; char *name; int kind;
#else
suf_get_ASL(ASL *asl, const char *name, int kind)
#endif
{
	SufDesc *d, *de;
	int ifread;

	if (!asl)
		badasl_ASL(asl, 0, "suf_get");
	ifread = kind & ASL_Sufkind_input;
	d = asl->i.suffixes[kind &= ASL_Sufkind_mask];
	de = d + asl->i.nsuff[kind];
	for(;; d++) {
		if (d >= de) {
			fprintf(Stderr, "suf_get(\"%s\") fails!\n", name);
			exit(1);
			}
		if (!strcmp(name, d->sufname))
			break;
		}
	if (ifread && !(d->kind & ASL_Sufkind_input))
		d = 0;
	return d;
	}

 SufDesc *
#ifdef KR_headers
suf_iput_ASL(asl, name, kind, I) ASL *asl; char *name; int kind, *I;
#else
suf_iput_ASL(ASL *asl, const char *name, int kind, int *I)
#endif
{
	SufDesc *d = suf_get_ASL(asl, name, kind);
	d->u.i = I;
	d->kind &= ~ASL_Sufkind_real;
	d->kind |= ASL_Sufkind_output;
	return d;
	}

 SufDesc *
#ifdef KR_headers
suf_rput_ASL(asl, name, kind, R) ASL *asl; char *name; int kind; real *R;
#else
suf_rput_ASL(ASL *asl, const char *name, int kind, real *R)
#endif
{
	SufDesc *d = suf_get_ASL(asl, name, kind);
	d->u.r = R;
	d->kind |= ASL_Sufkind_output | ASL_Sufkind_real;
	return d;
	}

#ifdef __cplusplus
}
#endif
