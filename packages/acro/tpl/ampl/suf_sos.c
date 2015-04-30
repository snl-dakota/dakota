/****************************************************************
Copyright (C) 1999-2001 Lucent Technologies
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

#include "asl.h"
#define SKIP_NL2_DEFINES
#include "nlp.h"
#include "nlp2.h"
#include "asl_pfg.h"
#include "asl_pfgh.h"
#undef cde

 static void
#ifdef KR_headers
lumap(i, n, z, LU, U) int i; int n; int *z; real *LU; real *U;
#else
lumap(int i, int n, int *z, real *LU, real *U)
#endif
{
	int j;

	if (U)
		for(; i < n; i++) {
			if ((j = z[i]) >= 0) {
				LU[j] = LU[i];
				U[j] = U[i];
				}
			}
	else if (LU)
		for(; i < n; i++) {
			if ((j = z[i]) >= 0) {
				LU[j<<1] = LU[i<<1];
				LU[(j<<1)+1] = LU[(i<<1)+1];
				}
			}
	}

 static void
#ifdef KR_headers
suf_adjust(d, i, n, z) SufDesc *d; int i; int n; int *z;
#else
suf_adjust(SufDesc *d, int i, int n, int *z)
#endif
{
	real *rp;
	int *ip, j;

	if (d->kind & ASL_Sufkind_real) {
		if (rp = d->u.r) {
			for(; i < n; i++)
				if ((j = z[i]) >= 0)
					rp[j] = rp[i];
			}
		}
	else if (ip = d->u.i)
		for(; i < n; i++)
			if ((j = z[i]) >= 0)
				ip[j] = ip[i];
	}

#ifdef __cplusplus
extern "C" {
static int  compar(const void*, const void*, void*);
static int rcompar(const void*, const void*, void*);
}
#endif

 static int
#ifdef KR_headers
compar(a, b, v) char *a, *b, *v;
#else
compar(const void *a, const void *b, void *v)
#endif
{
	Not_Used(v);
	return *(int*)a - *(int*)b;
	}

 static int
#ifdef KR_headers
bsrch(k, n, x) int k; int n; real *x;
#else
bsrch(int k, int n, real *x)
#endif
{
	int n1;
	real *x0, *x1;

	x0 = x;
	while(n > 0) {
		x1 = x + (n1 = n >> 1);
		if (k < *x1) {
			n = n1;
			continue;
			}
		if (k == *x1)
			return x1 - x0;
		n -= n1 + 1;
		x = x1 + 1;
		}
	return -1;
	}

 static int
#ifdef KR_headers
rcompar(a, b, v) char *a, *b, *v;
#else
rcompar(const void *a, const void *b, void *v)
#endif
{
	real *r = (real *)v, t;
	t = r[*(int*)a] - r[*(int*)b];
	if (t == 0.)
		return 0;
	return t < 0. ? -1 : 1;
	}

 static void
#ifdef KR_headers
reorder(ind, ref, pri, j0, k, p) int *ind, *pri, j0, k, *p; real *ref;
#else
reorder(int *ind, real *ref, int j0, int k, int *p)
#endif
{
	int i, j, n, ti;
	real tr;

	ref += j0;
	ind += j0;
	n = k - j0;
	for(i = 0; i < n; i++)
		p[i] = i;
	qsortv(p, n, sizeof(int), rcompar, ref);
	for(i = 0; i < n; i++) {
		if ((j = p[i]) > i) {
			ti = ind[i];
			tr = ref[i];
			for(k = i;;) {
				ind[k] = ind[j];
				ref[k] = ref[j];
				j = p[k = j];
				p[k] = -1 - j;
				if (j == i) {
					ind[k] = ti;
					ref[k] = tr;
					break;
					}
				}
			}
		}
	}

 static void
#ifdef KR_headers
name_map(n, z, nam) int n; int *z; char **nam;
#else
name_map(int n, int *z, char **nam)
#endif
{
	int i, j, k;

	for(i = k = 0; i < n; i++) {
		if ((j = z[i]) >= 0)
			nam[k = j] = nam[i];
		}
	while(++k < n)
		nam[k] = 0;
	}

 int
#ifdef KR_headers
suf_sos_ASL(asl, flags, nsosnz_p, sostype_p, sospri_p, copri,
	sosbeg_p, sosind_p, sosref_p)
	ASL *asl; int flags, *nsosnz_p; char **sostype_p;
	int **sospri_p, *copri, **sosbeg_p, **sosind_p; real **sosref_p;
#else
suf_sos_ASL(ASL *asl, int flags, int *nsosnz_p, char **sostype_p,
	int **sospri_p, int *copri, int **sosbeg_p, int **sosind_p,
	real **sosref_p)
#endif
{
	int f, i, j, j0, jz, k, m, ms1, n, ng, ns1;
	int nsos, nsos1, nsos2, nsosnz, nsosnz1;
	int *c, *col1, *ja, *ja1, *jae, *p, *pri, *rn, *rt;
	int *sospri, *sosbeg, *sosind, *v, *v0, *ve, *z, *zg, *zv;
	SufDesc *cd, *d, *gd, *grefd, *pd, *refd, *vd;
	char *s0, *sostype;
	real *a, *g, *g0, *ge, *gn, *gnos, *sosref, *sufref, t, t1;
	ograd *og, **ogp;
	size_t L;

	nsos1 = nsos2 = 0;
	if (!(flags & ASL_suf_sos_ignore_sosno)
	 && (gd = suf_get("sosno", ASL_Sufkind_var | ASL_Sufkind_input))
	 && (grefd = suf_get("ref", ASL_Sufkind_var | ASL_Sufkind_input)))
		nsos1 = 1;
	if (!(flags & ASL_suf_sos_ignore_amplsos)
	 && (vd = suf_get("sos", ASL_Sufkind_var | ASL_Sufkind_input))
	 && (refd = suf_get("sosref", ASL_Sufkind_var|ASL_Sufkind_input)))
		nsos2 = 1;
	else if (!nsos1)
		return 0;
	nsos = nsosnz = 0;
	n = n_var;
	m = n_con;
	z = zv = 0;
	if (nsos2) {
		v = v0 = vd->u.i;
		ve = v + n;
		z = (int *)M1alloc((m+n)*sizeof(int));
		zv = z + m;
		for(j = 0; j < n && !v[j]; j++)
			zv[j] = j;
		col1 = v + j;
		ns1 = n;
		for(k = j; j < n; j++) {
			if ((i = v[j]) & 2) {
				jz = -1;
				if (ns1 == n)
					ns1 = j;
				}
			else {
				jz = k++;
				if (i) {
					nsosnz++;
					if (nsos < (i >>= 4))
						nsos = i;
					}
				}
			zv[j] = jz;
			}
		if (ns1 == n)
			zv = 0;
		else {
			asl->i.z[0] = zv;
			nbv += k - n;
			n_var = k;
			}
		}
	gnos = 0;
	if (nsos1) {
		typedef union {real r; int i;} Uir; /* for sizeof */
		g = g0 = gd->u.r;
		ge = g + n;
		ng = 0;
		while(g < ge)
			if (i = *g++) {
				ng++;
				}
		if (!ng) {
			/* should not happen */
			if (!nsos2)
				return 0;
			nsos1 = 0;
			goto havenz;
			}
		gnos = (real*)Malloc(ng*sizeof(Uir));
		zg = (int*)gnos;
		for(g = g0; g < ge; )
			if (i = (int)*g++)
				*zg++ = i;
		qsortv((int*)gnos, ng, sizeof(int), compar, 0);
		gn = gnos + ng;
		while(gn > gnos)
			*--gn = *--zg;
		nsosnz += nsosnz1 = ng;
		g = gnos;
		j = (t = *g) < 0.;
		for(i = 1; i < ng; i++)
			if (gnos[i] != t) {
				/* Ignore SOS1 sets of 1 element   */
				/* and SOS2 sets of <= 2 elements. */
				if (i - j >= 2)
					g++;
				t = *g = gnos[j = i];
				if (t < 0.)
					j++;
				}
		if (i - j >= 2)
			g++;
		nsos += nsos1 = g - gnos;
		}
 havenz:
	if (!nsos) {
		if (gnos)
			free(gnos);
		return 0;
		}
	if (nsosnz_p)
		*nsosnz_p = nsosnz;
	if (zv)
		lumap(ns1, n, zv, LUv, Uvx);
	L = nsos + (nsos + 1)*sizeof(int)
		+ nsosnz*(sizeof(int) + sizeof(double));
	if (!copri)
		sospri_p = 0;
	else if (sospri_p)
		L += nsos*sizeof(int);
	sosref = *sosref_p = flags & ASL_suf_sos_explict_free
				? (double*)Malloc(L)
				: (double*)M1alloc(L);
	sosind = *sosind_p = (int *)(sosref + nsosnz);
	sosbeg = *sosbeg_p = sosind + nsosnz;
	sospri = 0;
	sostype = (char*)(sosbeg + nsos + 1);
	if (sospri_p) {
		sospri = *sospri_p = (int*)sostype;
		sostype = (char*)(sospri + nsos);
		}
	*sostype_p = sostype;
	memset(sosbeg, 0, (nsos+1)*sizeof(int));
	f = Fortran;
	if (nsos1) {
		pri = 0;
		if (sospri) {
			memset(sospri, 0, nsos1*sizeof(int));
			if (pd = suf_get("priority",
					ASL_Sufkind_var | ASL_Sufkind_input))
				pri = pd->u.i;
			}
		++sosbeg;
		for(g = g0; g < ge; )
			if ((j = (int)*g++)
			 && (i = bsrch(j, nsos1, gnos)) >= 0)
				sosbeg[i]++;
		g = gnos;
		for(i = j = 0; i < nsos1; i++) {
			sostype[i] = *g++ < 0 ? '2' : '1';
			k = sosbeg[i] + j;
			sosbeg[i] = j;
			j = k;
			}
		sufref = grefd->u.r;
		for(g = g0; g < ge; g++)
			if ((j = (int)*g)
			 && (i = bsrch(j, nsos1, gnos)) >= 0) {
				j = sosbeg[i]++;
				sosref[j] = sufref[k = g - g0];
				sosind[j] = k + f;
				if (pri && sospri[i] < pri[k])
					sospri[i] = pri[k];
				}

		/* reorder each SOS1 set by sosref */
		p = (int*)gnos;
		for(i = k = 0; i < nsos1; i++) {
			j = j0 = k;
			k = sosbeg[i];
			t = sosref[j++];
			while(j < k) {
				t1 = sosref[j++];
				if (t1 < t) {
					reorder(sosind, sosref, j0, k, p);
					break;
					}
				}
			}

		free(gnos);
		nsos -= nsos1;
		if (!nsos2)
			goto sosbeg_adjust;
		sosbeg += nsos1 - 1;
		*sosbeg = 0;
		sosind += nsosnz1;
		sosref += nsosnz1;
		sostype += nsos1;
		if (sospri)
			sospri += nsos1;
		}
	for(v = col1; v < ve; )
		if ((i = *v++) && !(i & 2) && !sosbeg[j = i>>4]++) {
			sostype[--j] = '1' + (i & 1);
			if (sospri)
				sospri[j] = copri[(i & 4) >> 2];
			}
	for(j = 0, i = 1; i <= nsos; i++) {
		k = sosbeg[i] + j;
		sosbeg[i] = j;
		j = k;
		}
	sufref = refd->u.r;
	for(v = col1; v < ve; v++)
		if ((i = *v) && !(i & 2)) {
			j = sosbeg[i>>4]++;
			sosref[j] = sufref[k = v - v0];
			sosind[j] = k + f;
			}
	if (nsos1)
		for(i = 0; i <= nsos; i++)
			sosbeg[i] += nsosnz1;
	if (!(cd = suf_get("sos", ASL_Sufkind_con | ASL_Sufkind_input)))
		z = 0;
	else {
		c = cd->u.i;
		for(i = 0;; i++) {
			if (i >= m) {
				z = 0;
				break;
				}
			if (!(c[i] & 2)) {
				z[i] = i;
				continue;
				}
			asl->i.z[1] = z;
			z[ms1 = j = i] = -1;
			while(++i < m)
				z[i] = c[i] & 2 ? -1 : j++;
			n_con = j;
			break;
			}
		}
	if (zv) {
		for(i = j = 0; i < n_obj; i++) {
			for(ogp = &Ograd[i]; og = *ogp;)
				if ((og->varno = zv[og->varno]) < 0)
					/* should not happen */
					*ogp = og->next;
				else {
					ogp = &og->next;
					j++;
					}
			*ogp = 0;
			}
		nzo = j;
		}
	if (zv && asl->i.varnames)
		name_map(asl->i.n_var0, zv, asl->i.varnames);
	i = m;
	if (z) {
		lumap(ms1, m, z, LUrhs, Urhsx);
		if (asl->i.connames)
			name_map(asl->i.n_con0, z, asl->i.connames);
		for(i = 0; i < m && z[i] >= 0; i++);
		}
	if (!A_vals) {
		if (!z)
			goto adjust_suf;
		L = sizeof(cde);
		switch(asl->i.ASLtype) {
		  case ASL_read_f:
		  case ASL_read_fg:
			s0 = (char*)((ASL_fg*)asl)->I.con_de_;
			break;
		  case ASL_read_fgh:
			L = sizeof(cde2);
			s0 = (char*)((ASL_fgh*)asl)->I.con2_de_;
			break;
		  case ASL_read_pfg:
			s0 = (char*)((ASL_pfg*)asl)->I.con_de_;
			break;
		  case ASL_read_pfgh:
			s0 = (char*)((ASL_pfgh*)asl)->I.con2_de_;
			L = sizeof(cde2);
		  }
		while(++i < m)
			if ((j = z[i]) >= 0) {
				Cgrad[j] = Cgrad[i];
				memcpy(s0 + j*L, s0 + i*L, L);
				}
		goto adjust_suf;
		}
	if (!zv)
		goto adjust_suf1;
	ja = A_colstarts;
	jae = ja + n;
	rn = A_rownos;
	if (nsos1)
		for(rt = sosind - nsosnz1; rt < sosind; rt++)
			*rt = zv[*rt - f] + f;
	for(j = 0; *zv >= 0; ja++, zv++) {
		k = ja[1] - f;
		while(j < k)
			if (rn[j++] - f >= i)
				goto copystart;
		}
 copystart:
	a = A_vals;
	i = k = ja[0] - f;
	ja1 = ja;
	do {
		j = k;
		k = *++ja - f;
		if (*zv++ >= 0) {
			for(*ja1++ = i + f; j < k; j++)
				if ((jz = z[rn[j]-f]) >= 0) {
					rn[i] = jz + f;
					a[i++] = a[j];
					}
			}
		} while(ja < jae);
	*ja1 = (nzc = i) + f;
 adjust_suf:
	if (zv)
		for(d = asl->i.suffixes[0]; d; d = d->next)
			if (d->kind & ASL_Sufkind_input && d != vd && d != refd)
				suf_adjust(d, ns1, asl->i.n_var0, asl->i.z[0]);
 adjust_suf1:
	if (z)
		for(d = asl->i.suffixes[1]; d; d = d->next)
			if (d->kind & ASL_Sufkind_input && d != cd)
				suf_adjust(d, ms1, asl->i.n_con0, asl->i.z[1]);
 sosbeg_adjust:
	nsos += nsos1;
	if (f)
		for(sosbeg = *sosbeg_p, i = 0; i <= nsos; i++)
			sosbeg[i] += f;
	return nsos;
	}
