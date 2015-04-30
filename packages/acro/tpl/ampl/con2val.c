/****************************************************************
Copyright (C) 1997, 1999, 2000 Lucent Technologies
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

#include "jac2dim.h"

 void
#ifdef KR_headers
con2val_ASL(a, X, F, nerror) ASL *a; real *X; real *F; fint *nerror;
#else
con2val_ASL(ASL *a, real *X, real *F, fint *nerror)
#endif
{
	cde *d, *dend;
	expr *e1;
	expr_v *V;
	real *cscale, f;
	int i;
	cgrad *gr, **gr0;
	Jmp_buf err_jmp0;
	ASL_fgh *asl;

	ASL_CHECK(a, ASL_read_fgh, "con2val");
	asl = (ASL_fgh*)a;
	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		i = setjmp(err_jmp0.jb);
		if (*nerror = i)
			goto done;
		}
	want_deriv = want_derivs;
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		x2_check_ASL(asl,X);
	if (!(x0kind & ASL_have_concom)) {
		if (comb < combc)
			com2eval_ASL(asl, comb, combc);
		if (comc1)
			com21eval_ASL(asl, 0,comc1);
		x0kind |= ASL_have_concom;
		}
	x0kind |= ASL_have_conval;
	d = con_de;
	dend = d + n_conjac[1];
	co_index = i = n_conjac[0];
	d += i;
	if (cscale = asl->i.cscale)
		cscale += i;
	for(gr0 = Cgrad + i; d < dend; d++, gr0++, co_index++) {
		e1 = d->e;
		f = (*e1->op)(e1 C_ASL);
		gr = *gr0;
		if (asl->i.vscale)
			for(V = var_e; gr; gr = gr->next)
				f += V[gr->varno].v * gr->coef;
		else
			for(; gr; gr = gr->next)
				f += X[gr->varno] * gr->coef;
		if (F)
			*F++ = cscale ? *cscale++ * f : f;
		}
 done:
	err_jmp = 0;
	}

 void
#ifdef KR_headers
jac2val_ASL(a, X, G, nerror) ASL *a; real *X; real *G; fint *nerror;
#else
jac2val_ASL(ASL *a, real *X, real *G, fint *nerror)
#endif
{
	cde *d, *dend;
	cgrad **gr0;
	cgrad *gr;
	real *Adjoints, *cscale, t, *vscale;
	Jmp_buf err_jmp0;
	int L, xksave;
	fint ne0;
	ASL_fgh *asl;
	static char who[] = "jac2val";

	ASL_CHECK(a, ASL_read_fgh, who);
	asl = (ASL_fgh *)a;
	if (!want_derivs)
		No_derivs_ASL(who);
	ne0 = -1;
	if (nerror && (ne0 = *nerror) >= 0) {
		err_jmp = &err_jmp0;
		L = setjmp(err_jmp0.jb);
		if (*nerror = L)
			goto done;
		}
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known && x2_check_ASL(asl,X)
	|| !(x0kind & ASL_have_conval)) {
		xksave = asl->i.x_known;
		asl->i.x_known = 1;
		con2val_ASL(a, X, 0, nerror);
		asl->i.x_known = xksave;
		if (ne0 >= 0 && *nerror)
			goto done;
		}
	Adjoints = adjoints;
	d = con_de;
	dend = d + n_conjac[1];
	d += L = n_conjac[0];
	if (cscale = asl->i.cscale)
		cscale += n_conjac[0];
	if (f_b)
		fun2set_ASL(asl, f_b);
	if (f_c)
		fun2set_ASL(asl, f_c);
	vscale = asl->i.vscale;
	for(gr0 = Cgrad + L; d < dend; d++, gr0++) {
		for(gr = *gr0; gr; gr = gr->next)
			Adjoints[gr->varno] = gr->coef;
		if (L = d->zaplen) {
			memset(adjoints_nv1, 0, L);
			derprop(d->d);
			}
		if (vscale)
			for(gr = *gr0; gr; gr = gr->next) {
				L = gr->varno;
				Adjoints[L] *= vscale[L];
				}
		gr = *gr0;
		if (cscale)
			for(t = *cscale++; gr; gr = gr->next)
				G[gr->goff] = t*Adjoints[gr->varno];
		else
			for(; gr; gr = gr->next)
				G[gr->goff] = Adjoints[gr->varno];
		}
 done:
	err_jmp = 0;
	}

 int
#ifdef KR_headers
jac2dim_ASL(asl, stub, M, N, NO, NZ, MXROW, MXCOL, stub_len)
	ASL *asl; char *stub;
	fint *M, *N, *NO, *NZ, *MXROW, *MXCOL;
	ftnlen stub_len;
#else
jac2dim_ASL(ASL *asl, char *stub, fint *M, fint *N, fint *NO, fint *NZ,
	fint *MXROW, fint *MXCOL, ftnlen stub_len)
#endif
{
	FILE *nl;

	nl = jac_dim_ASL(asl, stub, M, N, NO, NZ, MXROW, MXCOL, stub_len);
	if (!nl)
		return ASL_readerr_nofile;
	X0 = (real *)M1alloc(n_var*sizeof(real));
	return pfgh_read_ASL(asl, nl, ASL_return_read_err);
	}
