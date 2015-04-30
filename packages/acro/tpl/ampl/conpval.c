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

#include "jacpdim.h"

 static real
#ifdef KR_headers
copeval(f K_ASL) ps_func *f; D_ASL
#else
copeval(ps_func *f A_ASL)
#endif
{
	psb_elem *b, *be;
	expr *e;
	real t = 0.;

	for(b = f->b, be = b + f->nb; b < be; b++) {
		e = b->D.e;
		t += (*e->op)(e K_ASL);
		}
	return t;
	}

 static real
#ifdef KR_headers
cogeval(asl, f) ASL_pfgh *asl; ps_func *f;
#else
cogeval(ASL_pfgh *asl, ps_func *f)
#endif
{
	psb_elem *b, *be;
	psg_elem *g, *ge;
	expr *e;
	expr_v *V;
	linpart *L, *Le;
	real t, t1;

	V = var_e;
	t = 0.;
	for(g = f->g, ge = g + f->ng; g < ge; g++) {
		t1 = g->g0;
		if (g->nlin) {
			L = g->L;
			for(Le = L + g->nlin; L < Le; L++)
				t1 += L->fac * V[L->v.i].v;
			}
		if (g->ns) {
			b = g->E;
			for(be = b + g->ns; b < be; b++) {
				e = b->D.e;
				t1 += (*e->op)(e C_ASL);
				}
			}
		g->esum.v = t1;
		e = g->g;
		t += g->scale * (*e->op)(e C_ASL);
		}
	return t;
	}

 static void
#ifdef KR_headers
psderprop(asl, f) ASL_pfgh *asl; ps_func *f;
#else
psderprop(ASL_pfgh *asl, ps_func *f)
#endif
{
	int L;
	psb_elem *b, *be;
	psg_elem *g, *ge;
	expr *e, *ee;
	real *A, t, t1, t2;
	ograd *og;

	for(b = f->b, be = b + f->nb; b < be; b++)
		if (L = b->D.zaplen) {
			memset(adjoints_nv1, 0, L);
			derprop(b->D.d);
			}
	if (!f->ng)
		return;
	for(g = f->g, ge = g + f->ng; g < ge; g++) {
		e = g->g;
		ee = g->ge;
		if (e == ee) {
			t = e->dL;
			t2 = e->dL2;
			}
		else {
			t = e->dL;
			do {
				e = e->L.e;
				t *= e->dL;
				}
				while(e != ee);
			e = g->g;
			if (t != 0.) {
				t1 = ee->dL;
				t2 = ee->dL2 * (t / t1);
				for(;;) {
					ee = ee->R.e;
					t2 += (t / ee->dL) * t1 * ee->dL2;
					if (ee == e)
						break;
					t1 *= ee->dL;
					}
				}
			else
				for(L = 0, t2 = 1.;; e = e->L.e) {
					if (e->dL) {
						t2 *= e->dL;
						if (L)
							t2 *= e->dL;
						}
					else if (L++) {
						t2 = 0;
						break;
						}
					else
						t2 *= e->dL2;
					if (e == ee)
						break;
					}
			}
		g->g1 = t *= g->scale;
		g->g2 = t2 * g->scale;
		A = adjoints;
		for(og = g->og; og; og = og->next)
			A[og->varno] += t*og->coef;
		}
	}

 static void
#ifdef KR_headers
psgcomp(asl, f) ASL_pfgh *asl; ps_func *f;
#else
psgcomp(ASL_pfgh *asl, ps_func *f)
#endif
{
	int n;
	ograd *og;
	psb_elem *b, *be;
	psg_elem *g, *ge;
	linpart *L, *Le;
	linarg *la, **lap, **lape, *tl;
	range *U;
	real t;
	real *A = adjoints;

	g = f->g;
	asl->P.npsgcomp++;
	for(ge = g + f->ng; g < ge; g++) {
		for(og = g->og; og; og = og->next)
			A[og->varno] = 0;
		if (g->nlin) {
			L = g->L;
			for(Le = L + g->nlin; L < Le; L++)
				A[L->v.i] = L->fac;
			}
		if (g->ns) {
			b = g->E;
			tl = 0;
			for(be = b + g->ns; b < be; b++) {
				if ((U = b->U) && (n = U->nintv)) {
					lap = U->lap;
					lape = lap + n;
					do {
						la = *lap++;
						if (!la->termno++) {
							A[la->v->a] = 0.;
							la->tnext = tl;
							tl = la;
							}
						}
						while(lap < lape);
					}
				}
			for(b = g->E; b < be; b++)
				if (n = b->D.zaplen) {
					memset(adjoints_nv1, 0, n);
					derprop(b->D.d);
					}
			while(tl) {
				tl->termno = 0;
				if (t = A[tl->v->a]){
					og = tl->nz;
					do A[og->varno] += t*og->coef;
						while(og = og->next);
					}
				tl = tl->tnext;
				}
			}
		for(og = g->og; og; og = og->next)
			og->coef = A[og->varno];
		}
	}

 void
#ifdef KR_headers
conpval_ASL(a, X, F, nerror) ASL *a; real *X; real *F; fint *nerror;
#else
conpval_ASL(ASL *a, real *X, real *F, fint *nerror)
#endif
{
	real *cscale, f, *vscale;
	int i, j, je;
	cgrad *gr, **gr0;
	ps_func *p;
	expr_n *en;
	expr_v *V;
	Jmp_buf err_jmp0;
	ASL_pfgh *asl;

	ASL_CHECK(a, ASL_read_pfgh, "conpval");
	asl = (ASL_pfgh*)a;
	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		i = setjmp(err_jmp0.jb);
		if (*nerror = i)
			return;
		}
	want_deriv = want_derivs;
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		xp_check_ASL(asl, X);
	x0kind |= ASL_have_conval;
	je = n_conjac[1];
	j = n_conjac[0];
	gr0 = Cgrad + j;
	p = asl->P.cps + j;
	if (cscale = asl->i.cscale)
		cscale += j;
	vscale = asl->i.vscale;
	V = var_e;
	for(; j < je; j++, p++) {
		if (p->nb) {
			co_index = j;
			f = copeval(p C_ASL);
			if (p->ng)
				f += cogeval(asl, p);
			}
		else if (p->ng) {
			co_index = j;
			f = cogeval(asl, p);
			}
		else {
			en = (expr_n*)con_de[j].e;
			f = en->v;
			}
		gr = *gr0++;
		if (vscale)
			for(; gr; gr = gr->next)
				f += V[gr->varno].v * gr->coef;
		else
			for(; gr; gr = gr->next)
				f += X[gr->varno] * gr->coef;
		if (F)
			*F++ = cscale ? *cscale++ * f : f;
		}
	err_jmp = 0;
	}

 void
#ifdef KR_headers
jacpval_ASL(a, X, G, nerror) ASL *a; real *X, *G; fint *nerror;
#else
jacpval_ASL(ASL *a, real *X, real *G, fint *nerror)
#endif
{
	cgrad **gr0, **gre;
	cgrad *gr;
	real *Adjoints, *cscale, t, *vscale;
	ps_func *p;
	range *U;
	psb_elem *b, *be;
	linarg *la, **lap, **lape;
	ograd *og;
	int i, nx, xksave;
	fint ne0;
	Jmp_buf err_jmp0;
	ASL_pfgh *asl;
	static char who[] = "jacpval";

	ASL_CHECK(a, ASL_read_pfgh, who);
	asl = (ASL_pfgh*)a;
	if (!want_derivs)
		No_derivs_ASL(who);
	ne0 = -1;
	if (nerror && (ne0 = *nerror) >= 0) {
		err_jmp = &err_jmp0;
		i = setjmp(err_jmp0.jb);
		if (*nerror = i)
			return;
		}
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known && xp_check_ASL(asl,X)
	 || !(x0kind & ASL_have_conval)) {
		xksave = asl->i.x_known;
		conpval_ASL(a,X,0,nerror);
		asl->i.x_known = xksave;
		if (ne0 >= 0 && *nerror)
			return;
		}
	nx = asl->i.nxval;
	Adjoints = adjoints;
	gr0 = Cgrad;
	gre = gr0 + n_conjac[1];
	gr0 += i = n_conjac[0];
	if (cscale = asl->i.cscale)
		cscale += i;
	vscale = asl->i.vscale;
	for(p = asl->P.cps + i; gr0 < gre; gr0++) {
		p->nxval = nx;
		if (p->ng)
			psgcomp(asl, p);
		for(gr = *gr0; gr; gr = gr->next)
			Adjoints[gr->varno] = gr->coef;
		b = p->b;
		for(be = b + p->nb; b < be; b++)
			if ((U = b->U) && (i = U->nintv)) {
				lap = U->lap;
				lape = lap + i;
				do Adjoints[(*lap++)->v->a] = 0.;
					while(lap < lape);
				}
		b = p->b;
		psderprop(asl, p++);
		while(b < be)
			if ((U = (b++)->U) && (i = U->nintv)) {
				lap = U->lap;
				lape = lap + i;
				while(lap < lape)
					if (t = Adjoints[(la = *lap++)->v->a]){
						Adjoints[la->v->a] = 0;
						og = la->nz;
						do Adjoints[og->varno] +=
								t*og->coef;
							while(og = og->next);
						}
				}
		if (vscale)
			for(gr = *gr0; gr; gr = gr->next) {
				i = gr->varno;
				Adjoints[i] *= vscale[i];
				}
		gr = *gr0;
		if (cscale)
			for(t = *cscale++; gr; gr = gr->next)
				G[gr->goff] = t*Adjoints[gr->varno];
		else
			for(; gr; gr = gr->next)
				G[gr->goff] = Adjoints[gr->varno];
		}
	err_jmp = 0;
	}

 int
#ifdef KR_headers
jacpdim_ASL(asl, stub, M, N, NO, NZ, MXROW, MXCOL, stub_len)
	ASL *asl; char *stub;
	fint *M, *N, *NO, *NZ, *MXROW, *MXCOL;
	ftnlen stub_len;
#else
jacpdim_ASL(ASL *asl, char *stub, fint *M, fint *N, fint *NO, fint *NZ,
	fint *MXROW, fint *MXCOL, ftnlen stub_len)
#endif
{
	FILE *nl;

	nl = jac_dim_ASL(asl, stub, M, N, NO, NZ, MXROW, MXCOL, stub_len);
	if (!nl)
		return ASL_readerr_nofile;
	X0 = (real *)M1alloc(n_var*sizeof(real));
	return pfgh_read_ASL(asl, nl, ASL_findgroups | ASL_return_read_err);
	}

/******** objpval, objpgrd ********/

 static void
#ifdef KR_headers
NNOBJ_chk(asl, i, who) ASL *asl; int i; char *who;
#else
NNOBJ_chk(ASL *asl, int i, char *who)
#endif
{
	ASL_CHECK(asl, ASL_read_pfgh, who);
	if (i < 0 || i >= n_obj) {
		fprintf(Stderr,
			"%s: got NOBJ = %d; expected 0 <= NOBJ < %d\n",
			who, i, n_obj);
		exit(1);
		}
	}

 real
#ifdef KR_headers
objpval_ASL(a, i, X, nerror) ASL *a; int i; fint *nerror; real *X;
#else
objpval_ASL(ASL *a, int i, real *X, fint *nerror)
#endif
{
	real f;
	int ij;
	ograd *gr;
	ps_func *p;
	expr_n *en;
	expr_v *V;
	Jmp_buf err_jmp0;
	ASL_pfgh *asl;

	NNOBJ_chk(a, i, "objpval");
	asl = (ASL_pfgh*)a;
	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		ij = setjmp(err_jmp0.jb);
		if (*nerror = ij)
			return 0.;
		}
	want_deriv = want_derivs;
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		xp_check_ASL(asl,X);
	co_index = -(i + 1);
	p = asl->P.ops + i;
	if (p->nb) {
		f = copeval(p C_ASL);
		if (p->ng)
			f += cogeval(asl, p);
		}
	else if (p->ng)
		f = cogeval(asl, p);
	else {
		en = (expr_n*)obj_de[i].e;
		f = en->v;
		}
	asl->i.noxval[i] = asl->i.nxval;
	gr = Ograd[i];
	if (asl->i.vscale)
		for(V = var_e; gr; gr = gr->next)
			f += gr->coef * V[gr->varno].v;
	else
		for(; gr; gr = gr->next)
			f += gr->coef * X[gr->varno];
	err_jmp = 0;
	return f;
	}

 void
#ifdef KR_headers
objpgrd_ASL(a, i, X, G, nerror)
	ASL *a; int i; fint *nerror; real *X, *G;
#else
objpgrd_ASL(ASL *a, int i, real *X, real *G, fint *nerror)
#endif
{
	ograd *gr, *gr0;
	real *Adjoints;
	Jmp_buf err_jmp0;
	int ij, xksave, *z;
	ps_func *p;
	linarg *la;
	real t, *vscale;
	fint ne0;
	ASL_pfgh *asl;
	static char who[] = "objpgrd";

	NNOBJ_chk(a, i, who);
	asl = (ASL_pfgh*)a;
	if (!want_derivs)
		No_derivs_ASL(who);
	p = asl->P.ops + i;
	ne0 = -1;
	if (nerror && (ne0 = *nerror) >= 0) {
		err_jmp = &err_jmp0;
		ij = setjmp(err_jmp0.jb);
		if (*nerror = ij)
			return;
		}
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		xp_check_ASL(asl,X);
	if (!asl->i.noxval || asl->i.noxval[i] != asl->i.nxval) {
		xksave = asl->i.x_known;
		asl->i.x_known = 1;
		objpval_ASL(a, i, X, nerror);
		asl->i.x_known = xksave;
		if (ne0 >= 0 && *nerror)
			return;
		}
	Adjoints = adjoints;
	p->nxval = asl->i.nxval;
	if (p->ng)
		psgcomp(asl, p);
	gr0 = Ograd[i];
	for(gr = gr0; gr; gr = gr->next)
		Adjoints[gr->varno] = gr->coef;
	for(la = asl->P.lalist; la; la = la->lnext)
		Adjoints[la->v->a] = 0.;
	psderprop(asl, p);
	if (!G)
		return;
	for(la = asl->P.lalist; la; la = la->lnext)
		if (t = Adjoints[la->v->a]) {
			gr = la->nz;
			do Adjoints[gr->varno] += t*gr->coef;
				while(gr = gr->next);
			}
	if (zerograds) {	/* sparse gradients */
		z = zerograds[i];
		while((i = *z++) >= 0)
			G[i] = 0;
		}
	gr = gr0;
	if (vscale = asl->i.vscale)
		for(; gr; gr = gr->next) {
			i = gr->varno;
			G[i] = vscale[i] * Adjoints[i];
			}
	else
		for(; gr; gr = gr->next) {
			i = gr->varno;
			G[i] = Adjoints[i];
			}
	err_jmp = 0;
	}

 static void
#ifdef KR_headers
INchk(asl, who, i, ix) ASL *asl; char *who; int i, ix;
#else
INchk(ASL *asl, char *who, int i, int ix)
#endif
{
	ASL_CHECK(asl, ASL_read_pfgh, who);
	if (i < 0 || i >= ix) {
		fprintf(Stderr, "%s: got I = %d; expected 0 <= I < %d\n",
			who, i, ix);
		exit(1);
		}
	}

 static real
#ifdef KR_headers
cpval(asl, i, X, nerror) ASL_pfgh *asl; int i; fint *nerror; real *X;
#else
cpval(ASL_pfgh *asl, int i, real *X, fint *nerror)
#endif
{
	Jmp_buf err_jmp0;
	expr *e;
	expr_n *en;
	int L;
	ps_func *p;
	real f;

	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		L = setjmp(err_jmp0.jb);
		if (*nerror = L)
			return 0.;
		}
	want_deriv = want_derivs;
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		xp_check_ASL(asl, X);
	co_index = i;
	if (i >= n_con) {
		e = con_de[i].e;
		f = (*e->op)(e C_ASL);
		goto done;
		}
	asl->i.ncxval[i] = asl->i.nxval;
	p = asl->P.cps + i;
	if (p->nb) {
		f = copeval(p C_ASL);
		if (p->ng)
			f += cogeval(asl, p);
		}
	else if (p->ng)
		f = cogeval(asl, p);
	else {
		en = (expr_n*)con_de[i].e;
		f = en->v;
		}
 done:
	err_jmp = 0;
	return f;
	}

 real
#ifdef KR_headers
conpival_ASL(a, i, X, nerror) ASL *a; int i; fint *nerror; real *X;
#else
conpival_ASL(ASL *a, int i, real *X, fint *nerror)
#endif
{
	ASL_pfgh *asl;
	cgrad *gr;
	expr_v *V;
	real f;
	real scale;

	INchk(a, "conpival", i, a->i.n_con_);
	f = cpval(asl = (ASL_pfgh*)a, i, X, nerror);
	scale = asl->i.cscale ? asl->i.cscale[i] : 1.;
	gr = Cgrad[i];
	if (asl->i.vscale)
		for(V = var_e; gr; gr = gr->next)
			f += V[gr->varno].v * gr->coef;
	else
		for(; gr; gr = gr->next)
			f += X[gr->varno] * gr->coef;
	return scale * f;
	}

 int
#ifdef KR_headers
lconpval(a, i, X, nerror) ASL *a; int i; fint *nerror; real *X;
#else
lconpval(ASL *a, int i, real *X, fint *nerror)
#endif
{
	real f;

	INchk(a, "lconpival", i, a->i.n_lcon_);
	f = cpval((ASL_pfgh*)a, i + a->i.n_con0, X, nerror);
	return f != 0.;
	}

 void
#ifdef KR_headers
conpgrd_ASL(a, i, X, G, nerror)
	ASL *a; int i; fint *nerror; real *X, *G;
#else
conpgrd_ASL(ASL *a, int i, real *X, real *G, fint *nerror)
#endif
{
	cgrad *gr, *gr0;
	real *Adjoints;
	ps_func *p;
	range *U;
	psb_elem *b, *be;
	linarg *la, **lap, **lape;
	ograd *og;
	int i0, xksave;
	fint ne0;
	real t, *vscale;
	Jmp_buf err_jmp0;
	ASL_pfgh *asl;
	real scale;
	static char who[] = "conpgrd";

	INchk(a, who, i, a->i.n_con_);
	asl = (ASL_pfgh*)a;
	if (!want_derivs)
		No_derivs_ASL(who);
	ne0 = -1;
	if (nerror && (ne0 = *nerror) >= 0) {
		err_jmp = &err_jmp0;
		i0 = setjmp(err_jmp0.jb);
		if (*nerror = i0)
			return;
		}
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		xp_check_ASL(asl, X);
	if ((!asl->i.ncxval || asl->i.ncxval[i] != asl->i.nxval)
	 && (!(x0kind & ASL_have_conval)
	     || i < n_conjac[0] || i >= n_conjac[1])) {
		xksave = asl->i.x_known;
		asl->i.x_known = 1;
		conpival_ASL(a,i,X,nerror);
		asl->i.x_known = xksave;
		if (ne0 >= 0 && *nerror)
			return;
		}
	Adjoints = adjoints;
	p = asl->P.cps + i;
	p->nxval = asl->i.nxval;
	gr0 = Cgrad[i];
	scale = asl->i.cscale ? asl->i.cscale[i] : 1.;
	if (p->ng)
		psgcomp(asl, p);
	for(gr = gr0; gr; gr = gr->next)
		Adjoints[gr->varno] = gr->coef;
	b = p->b;
	for(be = b + p->nb; b < be; b++)
		if ((U = b->U) && (i = U->nintv)) {
			lap = U->lap;
			lape = lap + i;
			do Adjoints[(*lap++)->v->a] = 0.;
				while(lap < lape);
			}
	b = p->b;
	psderprop(asl, p);
	if (!G)
		return;
	while(b < be)
		if ((U = (b++)->U) && (i = U->nintv)) {
			lap = U->lap;
			lape = lap + i;
			while(lap < lape)
				if (t = Adjoints[(la = *lap++)->v->a]){
					Adjoints[la->v->a] = 0;
					og = la->nz;
					do Adjoints[og->varno] +=
							t*og->coef;
						while(og = og->next);
					}
			}

	if (vscale = asl->i.vscale)
		for(gr = gr0; gr; gr = gr->next) {
			i = gr->varno;
			Adjoints[i] *= vscale[i];
			}
	gr = gr0;
	i0 = 0;
	switch(asl->i.congrd_mode) {
	  case 1:
		for(; gr; gr = gr->next)
			G[i0++] = scale * Adjoints[gr->varno];
		break;
	  case 2:
		for(; gr; gr = gr->next)
			G[gr->goff] = scale * Adjoints[gr->varno];
		break;
	  default:
		for(; gr; gr = gr->next) {
			i = gr->varno;
			while(i0 < i)
				G[i0++] = 0;
			G[i] = scale * Adjoints[i];
			i0 = i + 1;
			}
		i = n_var;
		while(i0 < i)
			G[i0++] = 0;
	  }
	err_jmp = 0;
	}

 static void
#ifdef KR_headers
xpsgchk(asl, f0, xv, n, nx, ev, gv, y)
	ASL_pfgh *asl; ps_func *f0; int *xv, n, nx;
	real (*ev)(); void (*gv)(); real *y;
#else
xpsgchk(ASL_pfgh *asl, ps_func *f0, int *xv, int n, int nx,
	real (*ev)(ASL *a, int i, real *X, fint *nerror),
	void (*gv)(ASL *a, int i, real *X, real *G, fint *nerror),
	real *y)
#endif
{
	int i;
	ps_func *f;

	for(i = 0; i < n; i++)
		if (y[i] != 0.) {
			if (xv[i] != nx)
				(*ev)((ASL*)asl, i, Lastx, 0);
			f = f0 + i;
			if (f->ng && f->nxval != nx)
				(*gv)((ASL*)asl, i, Lastx, 0, 0);
			}
	}

 void
#ifdef KR_headers
xpsg_check_ASL(asl, nobj, ow, y) ASL_pfgh *asl; int nobj; real *ow, *y;
#else
xpsg_check_ASL(ASL_pfgh *asl, int nobj, real *ow, real *y)
#endif
{
	int nx, oxk, *xv;
	ps_func *f;
	real *x;

	if (x0kind == ASL_first_x) {
		if (!(x = X0))
			memset(x = Lastx, 0, n_var*sizeof(real));
		xp_check_ASL(asl, x);
		}
	nx = asl->i.nxval;
	oxk = asl->i.x_known;
	asl->i.x_known = 1;
	if (y)
		xpsgchk(asl, asl->P.cps, asl->i.ncxval, nlc,
			nx, conpival_ASL, conpgrd_ASL, y);
	f = asl->P.ops;
	xv = asl->i.noxval;
	if (nobj >= 0 && nobj < n_obj) {
		if (nobj >= nlo)
			goto done;
		if (ow) {
			ow += nobj;
			if (*ow == 0.)
				goto done;
			}
		if (xv[nobj] != nx)
			objpval_ASL((ASL*)asl, nobj, Lastx, 0);
		f += nobj;
		if (!f->ng)
			goto done;
		if (f->nxval != nx)
			objpgrd_ASL((ASL*)asl, nobj, Lastx, 0, 0);
		}
	else if (ow && nlo)
		xpsgchk(asl, f, xv, nlo,
			nx, objpval_ASL, objpgrd_ASL, ow);
 done:
	asl->i.x_known = oxk;
	}
