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

#include "nlp.h"
#include "errchk.h"

#ifdef MULTIPLE_THREADS
#define MTNot_Used(x) Not_Used(x)
#else
#define MTNot_Used(x) /*nothing*/
#define asl cur_ASL
#endif

 static void
#ifdef KR_headers
jmp_check(J, jv) Jmp_buf *J; int jv;
#else
jmp_check(Jmp_buf *J, int jv)
#endif
{
	if (J)
		longjmp(J->jb, jv);
	}

 static void
#ifdef KR_headers
Errprint(msg) char *msg;
#else
Errprint(char *msg)
#endif
{
#ifndef NO_PERROR
	if (errno)
		fprintf(Stderr, "\n%s: %s.\n", msg, strerror(errno));
	else
#endif
		fprintf(Stderr, "%s.\n", msg);
	fflush(Stderr);
	}

 static void
#ifdef KR_headers
introuble(who,a,jv K_ASL) char *who; real a; int jv; D_ASL
#else
introuble(char *who, real a, int jv A_ASL)
#endif
{
	char buf[64];
	jmp_check(err_jmp, jv);
	report_where(asl);
	snprintf(buf, sizeof(buf), "can't evaluate %s(%g)", who, a);
	Errprint(buf);
	jmp_check(err_jmp1, jv);
	exit(1);
	}

 static void
#ifdef KR_headers
introuble2(who, a, b, jv K_ASL) char *who; real a, b; int jv; D_ASL
#else
introuble2(char *who, real a, real b, int jv A_ASL)
#endif
{
	char buf[96];
	jmp_check(err_jmp, jv);
	report_where(asl);
	snprintf(buf, sizeof(buf), "can't evaluate %s(%g,%g)", who, a, b);
	Errprint(buf);
	jmp_check(err_jmp1, jv);
	exit(1);
	}

 static real
#ifdef KR_headers
f_OPPLUS(e K_ASL) expr *e; D_ASL
#else
f_OPPLUS(expr *e A_ASL)
#endif
{
	expr *L;
	/* e->dL = e->dR = 1.; */
	L = e->L.e;
	e = e->R.e;
	return (*L->op)(L K_ASL) + (*e->op)(e K_ASL);
	}

 static real
#ifdef KR_headers
f_OPMINUS(e K_ASL) expr *e; D_ASL
#else
f_OPMINUS(expr *e A_ASL)
#endif
{
	expr *L;
	/* e->dL = 1.;  */
	/* e->dR = -1.; */
	L = e->L.e;
	e = e->R.e;
	return (*L->op)(L K_ASL) - (*e->op)(e K_ASL);
	}

 static real
#ifdef KR_headers
f_OPMULT(e K_ASL) expr *e; D_ASL
#else
f_OPMULT(expr *e A_ASL)
#endif
{
	expr *e1, *e2;
	e1 = e->L.e;
	e2 = e->R.e;
	return (e->dR = (*e1->op)(e1 K_ASL)) * (e->dL = (*e2->op)(e2 K_ASL));
	}

 static void
#ifdef KR_headers
zero_div(L, op K_ASL) real L; char *op; D_ASL
#else
zero_div(real L, char *op A_ASL)
#endif
{
	errno_set(EDOM);
	jmp_check(err_jmp, 1);
	report_where(asl);
	fprintf(Stderr, "can't compute %g%s0.\n", L, op);
	fflush(Stderr);
	jmp_check(err_jmp1, 1);
	exit(1);
	}

 static real
#ifdef KR_headers
f_OPDIV(e K_ASL) expr *e; D_ASL
#else
f_OPDIV(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R, rv;
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	if (!(R = (*e1->op)(e1 K_ASL))
#ifdef WANT_INFNAN
	 && !L
#endif
		)
		zero_div(L, "/" K_ASL);
	rv = L / R;
	if (want_deriv)
		e->dR = -rv * (e->dL = 1. / R);
	return rv;
	}

 static real
#ifdef KR_headers
f_OPREM(e K_ASL) expr *e; D_ASL
#else
f_OPREM(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R, rv;
	/* e->dL = 1.; */
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	R = (*e1->op)(e1 K_ASL);
	rv = fmod(L,R);
	if (errchk(rv))
		introuble2("fmod",L,R,1 K_ASL);
	else
		e->dR = (rv - L) / R;
	return rv;
	}

 static real
#ifdef KR_headers
f_OPPOW(e K_ASL) expr *e; D_ASL
#else
f_OPPOW(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R, rv;
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	R = (*e1->op)(e1 K_ASL);
	rv = mypow(L,R);
	if (errchk(rv)) {
#ifdef WANT_INFNAN
		if (!L && R < 0.) {
			errno_set(0);
			return Infinity;
			}
#endif
		introuble2("pow",L,R,1 K_ASL);
		}
	if (want_deriv) {
		if (L > 0.) {
			e->dL = R * (rv/L);
			e->dR = log(L) * rv;
			}
		else if (L != 0.) {
#ifndef WANT_INFNAN
 bad:
#endif
			introuble2("pow'",L,R,2 K_ASL);
			}
		else {
			if (R > 1.)
				e->dL = e->dR = 0.;
			else if (R == 1.) {
				e->dL = 1.;
				e->dR = 0.;
				}
			else
#ifdef WANT_INFNAN
				{
				e->dL = Infinity;
				e->dR = negInfinity;
				}
#else
				goto bad;
#endif
			}
		}
	return rv;
	}
 static real
#ifdef KR_headers
f_OP1POW(e K_ASL) expr *e; D_ASL
#else
f_OP1POW(expr *e A_ASL) /* f_OPPOW for R = numeric constant */
#endif
{
	expr *e1;
	real L, R, rv;
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	R = ((expr_n *)e->R.e)->v;
	rv = mypow(L,R);
	if (errchk(rv)) {
#ifdef WANT_INFNAN
		if (!L && R < 0.) {
			errno_set(0);
			return Infinity;
			}
#endif
		introuble2("pow",L,R,1 K_ASL);
		}
	if (want_deriv) {
		if (L)
			e->dL = R * (rv/L);
		else if (R > 1.)
			e->dL = 0.;
		else
#ifdef WANT_INFNAN
			e->dL = negInfinity;
#else
			introuble2("pow'",L,R,2 K_ASL);
#endif
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP2POW(e K_ASL) expr *e; D_ASL
#else
f_OP2POW(expr *e A_ASL)	/* f_OPPOW for R = 2 */
#endif
{
	expr *e1;
	real L;
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e->dL = L + L;
	return L*L;
	}

 static real
#ifdef KR_headers
f_OPCPOW(e K_ASL) expr *e; D_ASL
#else
f_OPCPOW(expr *e A_ASL)	/* f_OPPOW for L = numeric constant */
#endif
{
	expr *e1;
	real L, R, rv;
	e1 = e->R.e;
	rv = mypow(L = e->L.en->v, R = (*e1->op)(e1 K_ASL));
	if (errchk(rv)) {
#ifdef WANT_INFNAN
		if (!L && R < 0.) {
			errno_set(0);
			return Infinity;
			}
#endif
		introuble2("pow",L,R,1 K_ASL);
		}
	if (want_deriv) {
		if (L > 0) {
			if (e->dL == 1)
				e->dL = log(L);	/* cache value */
			e->dR = e->dL * rv;
			}
		else if (L == 0 && R >= 1.)
			e->dR = 0.;
		else
#ifdef WANT_INFNAN
			e->dR = negInfinity;
#else
			introuble2("pow'",L,R,2 K_ASL);
#endif
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OPLESS(e K_ASL) expr *e; D_ASL
#else
f_OPLESS(expr *e A_ASL)
#endif
{
	expr *e1;
	real L;
	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	L -= (*e1->op)(e1 K_ASL);
	if (L < 0.)
		return e->dL = e->dR = 0;
	e->dL = 1.;
	e->dR = -1.;
	return L;
	}

 static real
#ifdef KR_headers
f_MINLIST(e0 K_ASL) expr *e0; D_ASL
#else
f_MINLIST(expr *e0 A_ASL)
#endif
{
	de *d, *d1;
	real t, rv;
	expr *e1;
	derp *D;
	expr_va *e = (expr_va *)e0;

	d = e->L.d;
	e1 = d->e;
	rv = (*e1->op)(e1 K_ASL);
	for(d1 = d++; e1 = d->e; d++) {
		t = (*e1->op)(e1 K_ASL);
		if (rv > t) {
			rv = t;
			d1 = d;
			}
		}
	if (D = e->R.D) {
		D->a.rp = d1->dv.rp;
		D->next = d1->d;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_MAXLIST(e0 K_ASL) expr *e0; D_ASL
#else
f_MAXLIST(expr *e0 A_ASL)
#endif
{
	de *d, *d1;
	real t, rv;
	expr *e1;
	derp *D;
	expr_va *e = (expr_va *)e0;

	d = e->L.d;
	e1 = d->e;
	rv = (*e1->op)(e1 K_ASL);
	for(d1 = d++; e1 = d->e; d++) {
		t = (*e1->op)(e1 K_ASL);
		if (rv < t) {
			rv = t;
			d1 = d;
			}
		}
	if (D = e->R.D) {
		D->a.rp = d1->dv.rp;
		D->next = d1->d;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_FLOOR(e K_ASL) expr *e; D_ASL
#else
f_FLOOR(expr *e A_ASL)
#endif
{
	/* e->dL = 0.; */
	e = e->L.e;
	return floor((*e->op)(e K_ASL));
	}

 static real
#ifdef KR_headers
f_CEIL(e K_ASL) expr *e; D_ASL
#else
f_CEIL(expr *e A_ASL)
#endif
{
	/* e->dL = 0.; */
	e = e->L.e;
	return ceil((*e->op)(e K_ASL));
	}

 static real
#ifdef KR_headers
f_ABS(e K_ASL) expr *e; D_ASL
#else
f_ABS(expr *e A_ASL)
#endif
{
	real rv;
	expr *e1;
	e1 = e->L.e;
	rv = (*e1->op)(e1 K_ASL);
	if (rv < 0.) {
		e->dL = -1.;
		return -rv;
		}
	e->dL = 1.;
	return rv;
	}

 static real
#ifdef KR_headers
f_OPUMINUS(e K_ASL) expr *e; D_ASL
#else
f_OPUMINUS(expr *e A_ASL)
#endif
{
	/* e->dL = -1.; */
	e = e->L.e;
	return -(*e->op)(e K_ASL);
	}

 static real
#ifdef KR_headers
f_OP_tanh(e K_ASL) expr *e; D_ASL
#else
f_OP_tanh(expr *e A_ASL)
#endif
{
	real rv, t, t1;
	expr *e1;
	e1 = e->L.e;
	rv = tanh(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("tanh",t,1 K_ASL);
	if (want_deriv) {
		t1 = cosh(t);
		if (errchk(t1))
			introuble("tanh'", t, 2 K_ASL);
		t1 = 1. / t1;
		e->dL = t1*t1;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_tan(e K_ASL) expr *e; D_ASL
#else
f_OP_tan(expr *e A_ASL)
#endif
{
	real rv, t, t1;
	expr *e1;
	e1 = e->L.e;
	rv = tan(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("tan",t,1 K_ASL);
	if (want_deriv) {
		t1 = cos(t);
		if (errchk(t1) || !t1)
			introuble("tan'",t,2 K_ASL);
		t1 = 1. / t1;
		e->dL = t1*t1;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_sqrt(e K_ASL) expr *e; D_ASL
#else
f_OP_sqrt(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	t = (*e1->op)(e1 K_ASL);
	if (t < 0. || (rv = sqrt(t), errchk(rv)))
		introuble("sqrt",t,1 K_ASL);
	if (want_deriv) {
		if (rv <= 0.)
			introuble("sqrt'",t,2 K_ASL);
		e->dL = 0.5 / rv;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_sinh(e K_ASL) expr *e; D_ASL
#else
f_OP_sinh(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = sinh(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("sinh",t,1 K_ASL);
	if (want_deriv) {
		e->dL = cosh(t);
		if (errchk(e->dL))
			introuble("sinh'",t,2 K_ASL);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_sin(e K_ASL) expr *e; D_ASL
#else
f_OP_sin(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = sin(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("sin",t,1 K_ASL);
	if (want_deriv) {
		e->dL = cos(t);
		if (errchk(e->dL))
			introuble("sin'",t,2 K_ASL);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_log10(e K_ASL) expr *e; D_ASL
#else
f_OP_log10(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	static real Le10;

	e1 = e->L.e;
	rv = log10(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("log10",t,1 K_ASL);
	if (want_deriv) {
		if (!Le10)
			Le10 = 1. / log(10.);
		e->dL = Le10 / t;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_log(e K_ASL) expr *e; D_ASL
#else
f_OP_log(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = log(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("log",t,1 K_ASL);
	if (want_deriv)
		e->dL = 1. / t;
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_exp(e K_ASL) expr *e; D_ASL
#else
f_OP_exp(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = e->dL = exp(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		if (t >= 0.)
			introuble("exp",t,1 K_ASL);
		else {
			errno_set(0);
			rv = 0.;
			}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_cosh(e K_ASL) expr *e; D_ASL
#else
f_OP_cosh(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = cosh(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("cosh",t,1 K_ASL);
	if (want_deriv) {
		e->dL = sinh(t);
		if (errchk(e->dL))
			introuble("cosh'",t,2 K_ASL);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_cos(e K_ASL) expr *e; D_ASL
#else
f_OP_cos(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;
	e1 = e->L.e;
	rv = cos(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("cos",t,1 K_ASL);
	if (want_deriv) {
		e->dL = -sin(t);
		if (errchk(e->dL))
			introuble("cos'",t,2 K_ASL);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_atanh(e K_ASL) expr *e; D_ASL
#else
f_OP_atanh(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;

	e1 = e->L.e;
	t = (*e1->op)(e1 K_ASL);
	if (t <= -1. || t >= 1.) {
		errno_set(EDOM);
 bad_atan:
		rv = 0.;
		introuble("atanh",t,1 K_ASL);
		}
	else {
		rv = 0.5*log((1. + t) / (1. - t));
		if (errchk(rv))
			goto bad_atan;
		}
	if (want_deriv)
		e->dL = 1. / (1. - t*t);
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_atan2(e K_ASL) expr *e; D_ASL
#else
f_OP_atan2(expr *e A_ASL)
#endif
{
	real L, R, rv, t, t1;
	expr *e1;

	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	R = (*e1->op)(e1 K_ASL);
	rv = atan2(L,R);
	if (errchk(rv))
		introuble2("atan2",L,R,1 K_ASL);
	if (want_deriv) {
		if ((t = L) < 0.)
			t = -t;
		if ((t1 = R) < 0.)
			t1 = -t1;
		if (t1 >= t) {
			t = L / R;
			t1 = 1. / (1. + t*t);
			e->dL = t1 /= R;
			e->dR = -t*t1;
			}
		else {
			t = R / L;
			t1 = -1. / (1. + t*t);
			e->dR = t1 /= L;
			e->dL = -t*t1;
			}
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_atan(e K_ASL) expr *e; D_ASL
#else
f_OP_atan(expr *e A_ASL)
#endif
{
	real t, rv;
	expr *e1;

	e1 = e->L.e;
	rv = atan(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("atan",t,1 K_ASL);
	if (want_deriv)
		e->dL = 1. / (1. + t*t);
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_asinh(e K_ASL) expr *e; D_ASL
#else
f_OP_asinh(expr *e A_ASL)
#endif
{
	expr *e1;
	real rv, t, t0, t1;
	int sign;

	e1 = e->L.e;
	t = t0 = (*e1->op)(e1 K_ASL);
	if (sign = t < 0.)
		t = -t;
	rv = log(t + (t1 = sqrt(t*t + 1.)));
	if (errchk(rv))
		introuble("asinh",t0,1 K_ASL);
	if (sign)
		rv = -rv;
	if (want_deriv)
		e->dL = 1. / t1;
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_asin(e K_ASL) expr *e; D_ASL
#else
f_OP_asin(expr *e A_ASL)
#endif
{
	expr *e1;
	real rv, t, t1;

	e1 = e->L.e;
	rv = asin(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("asin",t,1 K_ASL);
	if (want_deriv) {
		if ((t1 = 1. - t*t) <= 0.)
			introuble("asin'",t,2 K_ASL);
		e->dL = 1. / sqrt(t1);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_acosh(e K_ASL) expr *e; D_ASL
#else
f_OP_acosh(expr *e A_ASL)
#endif
{
	expr *e1;
	real rv, t, t1;

	e1 = e->L.e;
	t = (*e1->op)(e1 K_ASL);
	if (t < 1.) {
		errno_set(EDOM);
 bad_acosh:
		rv = 0.;
		introuble("acosh",t,1 K_ASL);
		}
	else {
		rv = log(t + (t1 = sqrt(t*t - 1.)));
		if (errchk(rv))
			goto bad_acosh;
		}
	if (want_deriv) {
		if (t1 <= 0.)
			introuble("acosh'",t,1 K_ASL);
		e->dL = 1. / t1;
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OP_acos(e K_ASL) expr *e; D_ASL
#else
f_OP_acos(expr *e A_ASL)
#endif
{
	expr *e1;
	real rv, t, t1;

	e1 = e->L.e;
	rv = acos(t = (*e1->op)(e1 K_ASL));
	if (errchk(rv))
		introuble("acos",t,1 K_ASL);
	if (want_deriv) {
		if ((t1 = 1. - t*t) <= 0.)
			introuble("acos'",t,2 K_ASL);
		e->dL = -1. / sqrt(t1);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OPIFnl(e K_ASL) expr *e; D_ASL
#else
f_OPIFnl(expr *e A_ASL)
#endif
{
	expr_if *eif = (expr_if *)e;
	derp *D;

	e = eif->e;
	if ((*e->op)(e K_ASL)) {
		e = eif->T;
		if (D = eif->D) {
			D->a.rp = eif->Tv.rp;
			D->next = eif->dT;
			}
		}
	else {
		e = eif->F;
		if (D = eif->D) {
			D->a.rp = eif->Fv.rp;
			D->next = eif->dF;
			}
		}
	return (*e->op)(e K_ASL);
	}

 static real
#ifdef KR_headers
f_OPOR(e K_ASL) expr *e; D_ASL
#else
f_OPOR(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) || (*e2->op)(e2 K_ASL) ? 1. : 0.;
	}

 static real
#ifdef KR_headers
f_OPAND(e K_ASL) expr *e; D_ASL
#else
f_OPAND(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) && (*e2->op)(e2 K_ASL) ? 1. : 0.;
	}

 static real
#ifdef KR_headers
f_LT(e K_ASL) expr *e; D_ASL
#else
f_LT(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) < (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_LE(e K_ASL) expr *e; D_ASL
#else
f_LE(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) <= (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_EQ(e K_ASL) expr *e; D_ASL
#else
f_EQ(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) == (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_GE(e K_ASL) expr *e; D_ASL
#else
f_GE(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) >= (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_GT(e K_ASL) expr *e; D_ASL
#else
f_GT(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) > (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_NE(e K_ASL) expr *e; D_ASL
#else
f_NE(expr *e A_ASL)
#endif
{
	expr *e2;
	e2 = e->R.e;
	e = e->L.e;
	return (*e->op)(e K_ASL) != (*e2->op)(e2 K_ASL) ? 1. : 0;
	}

 static real
#ifdef KR_headers
f_OPNOT(e K_ASL) expr *e; D_ASL
#else
f_OPNOT(expr *e A_ASL)
#endif
{
	e = e->L.e;
	return (*e->op)(e K_ASL) ? 0. : 1.;
	}

 static real
#ifdef KR_headers
f_ANDLIST(e K_ASL) expr *e; D_ASL
#else
f_ANDLIST(expr *e A_ASL)
#endif
{
	expr **ep, **epe;

	ep = e->L.ep;
	epe = e->R.ep;
	do {
		e = *ep++;
		if ((*e->op)(e K_ASL) == 0.)
			return 0.;
		}
		while(ep < epe);
	return 1.;
	}

 static real
#ifdef KR_headers
f_ORLIST(e K_ASL) expr *e; D_ASL
#else
f_ORLIST(expr *e A_ASL)
#endif
{
	expr **ep, **epe;

	ep = e->L.ep;
	epe = e->R.ep;
	do {
		e = *ep++;
		if ((*e->op)(e K_ASL) != 0.)
			return 1.;
		}
		while(ep < epe);
	return 0.;
	}

 static real
#ifdef KR_headers
f_OPIMPELSE(e K_ASL) expr *e; D_ASL
#else
f_OPIMPELSE(expr *e A_ASL)
#endif
{
	expr_if *eif = (expr_if *)e;

	e = eif->e;
	e = ((*e->op)(e K_ASL) != 0.) ? eif->T : eif->F;
	return (*e->op)(e K_ASL);
	}

 static real
#ifdef KR_headers
f_OP_IFF(e K_ASL) expr *e; D_ASL
#else
f_OP_IFF(expr *e A_ASL)
#endif
{
	expr *e1;
	int a, b;

	e1 = e->L.e;
	a = (*e1->op)(e1 K_ASL) != 0.;
	e1 = e->R.e;
	b = (*e1->op)(e1 K_ASL) != 0.;
	return a == b ? 1. : 0.;
	}

 static real
#ifdef KR_headers
f_OPSUMLIST(e K_ASL) expr *e; D_ASL
#else
f_OPSUMLIST(expr *e A_ASL)
#endif
{
	expr **ep, **epe;
	real x;
	ep = e->L.ep;
	epe = e->R.ep;
	e = *ep++;
	x = (*e->op)(e K_ASL);
	do {
		e = *ep++;
		x += (*e->op)(e K_ASL);
		}
		while(ep < epe);
	return x;
	}

 static real
#ifdef KR_headers
f_OPintDIV(e K_ASL) expr *e; D_ASL
#else
f_OPintDIV(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R;

	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	if (!(R = (*e1->op)(e1 K_ASL)))
		zero_div(L, " div " K_ASL);
	return (L /= R) >= 0 ? floor(L) : ceil(L);
	}

 static real
#ifdef KR_headers
f_OPprecision(e K_ASL) expr *e; D_ASL
#else
f_OPprecision(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R;
	char buf[32];

	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	R = (*e1->op)(e1 K_ASL);
	g_fmtp(buf, L, (int)R);
	return strtod(buf, (char **)0);
	}

#ifdef No_dtoa

 static real
#ifdef KR_headers
Round(x, prec) real x; int prec;
#else
Round(real x, int prec)
#endif
{
	real scale;
	int flip;

	if (!x)
		return x;
	flip = 0;
	if (x < 0.) {
		x = -x;
		flip = 1;
		}
	if (!prec)
		x = floor(x + 0.5);
	else if (prec > 0) {
		scale = mypow(10., (real)prec);
		x = floor(x*scale + 0.5) / scale;
		}
	else {
		scale = mypow(10., -(real)prec);
		x = scale*floor(x/scale + 0.5);
		}
	return flip ? -x : x;
	}
#else

 static real
#ifdef KR_headers
Round(x, prec) real x; int prec;
#else
Round(real x, int prec)
#endif
{
	char *b, *s, *s0, *se;
	int decpt, L, sign;
	char buf[96];

	if (!x)
		return x;
	s = dtoa(x, 3, prec, &decpt, &sign, &se);
	if (decpt == 9999) {
 zreturn:
		freedtoa(s);
		return x;
		}
	L = se - s;
	if (L <= 0) {
		x = 0.;
		goto zreturn;
		}
	if (L > 80)
		se = s + 80;
	s0 = s;
	b = buf;
	if (sign)
	*b++ = '-';
	*b++ = '.';
	while(s < se)
		*b++ = *s++;
	*b = 0;
	freedtoa(s0);
	if (decpt)
		snprintf(b, buf + sizeof(buf) - b, "e%d", decpt);
	return strtod(buf, (char **)0);
	}
#endif

 static real
#ifdef KR_headers
f_OPround(e K_ASL) expr *e; D_ASL
#else
f_OPround(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R;

	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	R = (*e1->op)(e1 K_ASL);
	return Round(L, (int)R);
	}

 static real
#ifdef KR_headers
f_OPtrunc(e K_ASL) expr *e; D_ASL
#else
f_OPtrunc(expr *e A_ASL)
#endif
{
	expr *e1;
	real L, R;
	int prec;

	e1 = e->L.e;
	L = (*e1->op)(e1 K_ASL);
	e1 = e->R.e;
	if (!(R = (*e1->op)(e1 K_ASL)))
		return L >= 0. ? floor(L) : ceil(L);
	R = Round(L, prec = (int)R);
	if (R != L) {
		R = 0.5*mypow(10., (real)-prec);
		R = Round(L > 0 ? L - R : L + R, prec);
		}
	return R;
	}

 static real
#ifdef KR_headers
f_OPPLTERM(e K_ASL) expr *e; D_ASL
#else
f_OPPLTERM(expr *e A_ASL)
#endif
{
	plterm *p = e->L.p;
	real r, t;
	int n = p->n;
	real *bs = p->bs;
	MTNot_Used(asl);

	r = ((expr_v *)e->R.e)->v;
	if (r >= 0) {
		/* should use binary search for large n */
		while(bs[1] <= 0.) {
			bs += 2;
			if (--n <= 1)
				return r*(e->dL = *bs);
			}
		if (r <= bs[1])
			return r*(e->dL = *bs);
		for(t = bs[0]*bs[1]; --n > 1 && r > bs[3]; bs += 2)
			t += (bs[3]-bs[1])*bs[2];
		return t + (r-bs[1])*(e->dL = bs[2]);
		}
	bs += 2*(n-1);
	while(bs[-1] >= 0.) {
		bs -= 2;
		if (--n <= 1)
			return r*(e->dL = *bs);
		}
	if (r >= bs[-1])
		return r*(e->dL = *bs);
	for(t = bs[0]*bs[-1]; --n > 1 && r < bs[-3]; bs -= 2)
		t += (bs[-3]-bs[-1])*bs[-2];
	return t + (r-bs[-1])*(e->dL = bs[-2]);
	}

typedef char * sfunc ANSI((expr* A_ASL));

 static char *
#ifdef KR_headers
f_OPHOL(e K_ASL) expr *e; D_ASL
#else
f_OPHOL(expr *e A_ASL)
#endif
{
	MTNot_Used(asl);
	return ((expr_h *)e)->sym;
	}

#define f_OPVARVAL f_OPVARVAL_ASL
 real
#ifdef KR_headers
f_OPVARVAL(e K_ASL) expr *e; D_ASL
#else
f_OPVARVAL(expr *e A_ASL)
#endif
{
	MTNot_Used(asl);
	return ((expr_v *)e)->v;
	}

 static char *
#ifdef KR_headers
f_OPIFSYM(e K_ASL) expr *e; D_ASL
#else
f_OPIFSYM(expr *e A_ASL)
#endif
{
	expr_if *eif = (expr_if *)e;
	e = eif->e;
	e = (*e->op)(e K_ASL) ? eif->T : eif->F;
	return (*(sfunc*)e->op)(e K_ASL);
	}

 struct
TMInfo {
	union {
		TMInfo *prev;
		double align;
		} u;
	};

 static real
#ifdef KR_headers
f_OPFUNCALL(e K_ASL) expr *e; D_ASL
#else
f_OPFUNCALL(expr *e A_ASL)
#endif
{
	TMInfo T, *T1, *T1prev;
	arglist *al;
	argpair *ap, *ape;
	char *s;
	expr_f *f = (expr_f *)e;
	func_info *fi = f->fi;
	int jv;
	real rv;

	for(ap = f->ap, ape = f->ape; ap < ape; ap++) {
		e = ap->e;
		*ap->u.v = (*e->op)(e K_ASL);
		}
	for(ap = f->sap, ape = f->sape; ap < ape; ap++) {
		e = ap->e;
		*ap->u.s = (*(sfunc*)e->op)(e K_ASL);
		}
	T.u.prev = 0;
	al = f->al;
	al->TMI = &T;
	al->Errmsg = 0;
	rv = (*fi->funcp)(al);
	errno_set(0);
	if ((s = al->Errmsg) && !err_jmp) {
		report_where(asl);
		jv = 1;
		if (*s == '\'') {
			++jv;
			++s;
			}
		fprintf(Stderr, "Error in function %s:\n\t%s\n", fi->name, s);
		fflush(Stderr);
		}
	for(T1 = T.u.prev; T1; T1 = T1prev) {
		T1prev = T1->u.prev;
		free(T1);
		}
	if (s) {
		jmp_check(err_jmp, jv);
		jmp_check(err_jmp1,jv);
		exit(1);
		}
	return rv;
	}

 static real
#ifdef KR_headers
f_OPCOUNT(e K_ASL) expr *e; D_ASL
#else
f_OPCOUNT(expr *e A_ASL)
#endif
{
	expr **ep, **epe;
	real x;
	ep = e->L.ep;
	epe = e->R.ep;
	e = *ep++;
	if (x = (*e->op)(e K_ASL))
		x = 1.;
	do {
		e = *ep++;
		if ((*e->op)(e K_ASL))
			x++;
		}
		while(ep < epe);
	return x;
	}

 typedef struct
jb_st {
	jmp_buf jb;
	} jb_st;

 static int
#ifdef KR_headers
rcompj(a, b, v) char *a, *b, *v;
#else
rcompj(const void *a, const void *b, void *v)
#endif
{
	jb_st *J;
	real t = *(real *)a - *(real *)b;

	if (!t) {
		J = (jb_st*)v;
		longjmp(J->jb, 1);
		}
	return t < 0 ? -1 : 1;
	}

 static real
#ifdef KR_headers
f_OPALLDIFF(e K_ASL) expr *e; D_ASL
#else
f_OPALLDIFF(expr *e A_ASL)
#endif
{
	int n;
	expr **ep, **epe;
	jb_st J;
	real *r, *r1, rbuf[128], t;

	ep = e->L.ep;
	epe = e->R.ep;
	n = epe - ep;
	r = rbuf;
	if (n > sizeof(rbuf)/sizeof(real))
		r = (real*)Malloc(n*sizeof(real));
	r1 = r;
	while(ep < epe) {
		e = *ep++;
		*r1++ = (*e->op)(e K_ASL);
		}
	t = 1.;
	if (setjmp(J.jb)) {
		t = 0.;
		goto done;
		}
	qsortv(r, n, sizeof(real), rcompj, &J);
 done:
	if (r != rbuf)
		free(r);
	return t;
	}

 static real
#ifdef KR_headers
f_OPNUMBEROF(e K_ASL) expr *e; D_ASL
#else
f_OPNUMBEROF(expr *e A_ASL)
#endif
{
	expr **ep, **epe;
	real n, t;

	ep = e->L.ep;
	epe = e->R.ep;
	n = 0.;
	e = *ep++;
	t = (*e->op)(e K_ASL);
	while(ep < epe) {
		e = *ep++;
		if (t == (*e->op)(e K_ASL))
			n++;
		}
	return n;
	}

#define f_OPATLEAST f_LE
#define f_OPATMOST  f_GE
#define f_OPEXACTLY f_EQ
#define f_OPNOTATLEAST f_GT
#define f_OPNOTATMOST  f_LT
#define f_OPNOTEXACTLY f_NE
#define f_OPNUMBEROFs f_OPNUMBEROF /*TEMPORARY*/

 efunc *
r_ops[] = {
#include "r_op.hd"
	};
