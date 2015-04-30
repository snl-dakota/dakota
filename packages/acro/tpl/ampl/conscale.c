#include "asl.h"

 static real *
#ifdef KR_headers
ones(asl, n) ASL *asl; int n;
#else
ones(ASL *asl, int n)
#endif
{
	real *x, *x0, *xe;

	x = x0 = (real*)mem_ASL(asl, n*sizeof(real));
	xe = x + n;
	while(x < xe)
		*x++ = 1.;
	return x0;
	}

 static int
#ifdef KR_headers
zcheck(asl, i, s, n, ierror, who) ASL *asl; real s; int i, n; fint *ierror; char *who;
#else
zcheck(ASL *asl, int i, real s, int n, fint *ierror, char *who)
#endif
{
#undef Word0
#ifdef IEEE_MC68k
#define Word0(x) ((Long*)&(x))[0]
#endif
#ifdef IEEE_8087
#define Word0(x) ((Long*)&(x))[1]
#endif
#ifdef Word0
#define Inftest(z) ((Word0(z)&0x7ff00000) == 0x7ff00000)
#else
#define Inftest(z) (z <= negInfinity || z >= Infinity)
#endif
	if (n >= 0 && (i < 0 || i >= n)
	 || s == 0.
	 || Inftest(s)) {
			if (ierror && *ierror >= 0) {
				*ierror = 1;
				return 1;
				}
			fprintf(Stderr, "%s(", who);
			if (n >= 0)
				fprintf(Stderr, "%d, ", i);
			fprintf(Stderr, "%.g, nerror): bad argument\n", s);
			fflush(Stderr);
			if (err_jmp1)
				longjmp(err_jmp1->jb, 1);
			exit(1);
			}
	if (ierror && *ierror >= 0)
		*ierror = 0;
	cur_ASL = asl;
	return 0;
	}

 static void
#ifdef KR_headers
scaleadj(s, i, m, scale, L, U, x) int i, m; real s, *scale, *L, *U, *x;
#else
scaleadj(real s, int i, int m, real *scale, real *L, real *U, real *x)
#endif
{
	real u, v;

	scale += i;
	if (x)
		x[i] /= s;
	if (!U) {
		U = L + 1;
		i <<= 1;
		}
	L += i;
	U += i;
	*scale *= s;
	if (s > 0.) {
		if (*L > negInfinity)
			if (m)
				*L *= s;
			else
				*L /= s;
		if (*U < Infinity)
			if (m)
				*U *= s;
			else
				*U /= s;
		}
	else {
		u = -*L;
		v = -*U;
		if (u < Infinity)
			if (m)
				u = *L * s;
			else
				u = *L / s;
		if (v > negInfinity)
			if (m)
				v = *U * s;
			else
				v = *U / s;
		*L = v;
		*U = u;
		}
	}

 void
#ifdef KR_headers
conscale_ASL(asl, i, s, ierror) ASL *asl; int i; real s; fint *ierror;
#else
conscale_ASL(ASL *asl, int i, real s, fint *ierror)
#endif
{
	static char who[] = "conscale";

	if (!asl
	 || asl->i.ASLtype < ASL_read_fg
	 || asl->i.ASLtype > ASL_read_pfgh)
		badasl_ASL(asl, ASL_read_fg, who);
	if (zcheck(asl, i, s, n_con, ierror, who))
		return;
	if (!asl->i.cscale)
		asl->i.lscale = asl->i.cscale = ones(asl, n_con);
	scaleadj(s, i, 1, asl->i.cscale, LUrhs, Urhsx, pi0);
	if (asl->i.lscale != asl->i.cscale)
		asl->i.lscale[i] *= s;
	}

 void
#ifdef KR_headers
varscale_ASL(asl, i, s, ierror) ASL *asl; int i; real s; fint *ierror;
#else
varscale_ASL(ASL *asl, int i, real s, fint *ierror)
#endif
{
	static char who[] = "varscale";

	if (!asl
	 || asl->i.ASLtype < ASL_read_fg
	 || asl->i.ASLtype > ASL_read_pfgh)
		badasl_ASL(asl, ASL_read_fg, who);
	if (zcheck(asl, i, s, n_var, ierror, who))
		return;
	if (!asl->i.vscale)
		asl->i.vscale = ones(asl, n_var);
	scaleadj(s, i, 0, asl->i.vscale, LUv, Uvx, X0);
	}

 void
#ifdef KR_headers
lagscale_ASL(asl, s, ierror) ASL *asl; real s; fint *ierror;
#else
lagscale_ASL(ASL *asl, real s, fint *ierror)
#endif
{
	static char who[] = "lagscale";
	real *c, *ce, *l;

	if (!asl
	 || asl->i.ASLtype != ASL_read_pfgh
	 && asl->i.ASLtype != ASL_read_fgh)
		badasl_ASL(asl, ASL_read_pfgh, who);
	if (zcheck(asl, 0, s, -1, ierror, who))
		return;
	if (s == 1. && asl->i.lscale == asl->i.cscale)
		return;
	if (!asl->i.cscale)
		asl->i.lscale = asl->i.cscale = ones(asl, n_con);
	if (asl->i.lscale == asl->i.cscale)
		asl->i.lscale = (real*)mem_ASL(asl, n_con*sizeof(real));
	l = asl->i.lscale;
	c = asl->i.cscale;
	ce = c + n_con;
	while(c < ce)
		*l++ = s * *c++;
	}
