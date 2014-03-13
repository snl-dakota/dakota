#include <stdio.h>
#include <limits>
#include "DLSfuncs.H"
using std::endl;

typedef unsigned int ftnlen;

typedef int (*c_fp)(int*,int*,int*,int*,double*,double*,double*,
		int*,char*,int*,int*,int*,double*,int*,ftnlen);
typedef int (*o_fp)(int*,int*,double*,double*,double*,int*,
		char*,int*,int*,int*,double*,int*,ftnlen);

extern "C" void flush_();

extern "C" int sninit_(int *iprint, int *isumm, char *cw,
	int *lencw, int *iw, int *leniw, double *rw, int *lenrw,
	ftnlen cw_len);

typedef void fgCon (int *MODE, int *M, int *N, int *NJAC,
		double *X, double *F, double *G, int *NSTATE,
		char *CU, int *LENCU, int *IU, int *LENIU,
		double *RU, int *LENRU, ftnlen len_cu);

typedef void fgObj (int *MODE, int *N,
		double *X, double *F, double *G, int *NSTATE,
		char *CU, int *LENCU, int *IU, int *LENIU,
		double *RU, int *LENRU, ftnlen len_cu);

extern "C" int snopt_(const char *start, int *m, int *n, int *ne,
		int *nname, int *nncon, int *nnobj, int *nnjac,
		int *iobj, double *objadd, const char *prob, fgCon *fgcon,
		fgObj *fgobj, double *jcol, int *indj, int *locj, double *bl,
		double *bu, const char *names, int *hs, double *x, double *pi,
		double *rc, int *info, int *mincw, int *miniw,
		int *minrw, int *ns, int *ninf, double *sinf, double *obj,
		const char *cu, int *lencu, int *iu, int *leniu, double *ru,
		int *lenru, const char *cw, int *lencw, int *iw, int *leniw,
		double *rw, int *lenrw, ftnlen start_len, ftnlen prob_len,
		ftnlen names_len, ftnlen cu_len, ftnlen cw_len);

extern "C" int snmemb_(int *info, int *m, int *n, int *ne, int *negcon,
	int *nncon, int *nnjac, int *nnobj, int *mincw, int *miniw,
	int *minrw, char *cw, int *lencw, int *iw, int *leniw,
	double *rw, int *lenrw, ftnlen cw_len);

extern "C" int snset_(char *buffer, int *iprint, int *isumm,
	int *inform, char *cw, int *lencw, int *iw, int *leniw,
	double *rw, int *lenrw, ftnlen buffer_len, ftnlen cw_len);

namespace Dakota {

Dakota_funcs *DF;

 struct
Snopt_Details {
	Optimizer1 *D;
	int co, m, maxfe, n, nf, nnlc, numgflag;
	double *Lastx;
	};

 static Snopt_Details *snopt_details;

 static int
eval_check(int *asvreq, int *MODE, double *X)
{
	Optimizer1 *D;
	Snopt_Details *T;
	int n;
	size_t L;

	T = snopt_details;
	D = T->D;
	n = T->n;
	L = n*sizeof(double);

	*asvreq = *MODE + 1;
	if (T->numgflag && (*asvreq & 2) ) {
		*asvreq -= 2;	// downgrade request
		if (T->nnlc == 0) { // else already printed
			Cout << "NPSOL has requested objective gradient for "
				"case of vendor numerical gradients.\n";
			if (*asvreq)
				Cout << "Request will be downgraded to objective "
					"value alone.\n" <<endl;
			else
				Cout << "Request will be ignored and no "
					"evaluation performed.\n" <<endl;
			}
		}

	if (memcmp(X, T->Lastx, L)) {
		if (++T->nf >= T->maxfe) {
			Cout << "Iteration terminated: max_function_evaluations "
				"limit has been met." << endl;
			return 1;
			}
		memcpy(T->Lastx, X, L);
		DF->ComputeResponses1(D, *asvreq, n, X);
		}
	return 0;
	}

 static void
funcon(int *MODE, int *M1, int *N, int *NJAC, double *X, double *F, double *G,
	int *NSTATE, char *CU, int *LENCU, int *IU, int *LENIU, double *RU,
	int *LENRU, ftnlen len_cu)
{
	Optimizer1 *D;
	Snopt_Details *T;
	int asv_req, co, nnlc;

	T = snopt_details;
	D = T->D;
	nnlc = T->nnlc;

	if (eval_check(&asv_req, MODE, X)) {
		*MODE = -2;
		return;
		}
	co = T->co;
	if (asv_req & 1)
		GetFuncs(D, co, co + nnlc, F);
	if (asv_req & 2)
		GetGrads(D, co, co + nnlc, T->n, 1, T->m, G);
	}

 static void
funobj(int *MODE, int *N, double *X, double *F, double *G, int *NSTATE,
	char *cu, int *lencu, int *iu, int *leniu, double *ru, int *lenru,
	ftnlen cu_len)
{
	Optimizer1 *D;
	Snopt_Details *T;
	int asv_req;

	T = snopt_details;
	D = T->D;

	if (eval_check(&asv_req, MODE, X)) {
		*MODE = -2;
		return;
		}
	if (asv_req & 1)
		GetFuncs(D, 0, 1, F);
	if (asv_req & 2)
		GetGrads(D, 0, 1, T->n, T->n, 1, G);
	}


 static void
my_findopt(void *v, Optimizer1 *D, char *options)
{
	Dakota_probsize *ps;
	Opt_Info OI(options);
	RealMatrix const *RMe, *RMi;
	RealVector const *Ru,  *Rv;
	Snopt_Details *T, *T0;
	char *cu, *cw, *ve;
	double *A, *bl, *bu, f, objadd, *pi, *rc, *ru, *rw, sinf, t, *x;
	int co, i, i1, inform, j, k, k1, lencu, lencw, leniu, leniw, lenru, lenrw;
	int m, mincw, miniw, minrw, n, ninf, nlc, nlce, nlci, nlnz;
	int nm, nname, nnjac, nnlc, nnobj, ns, nz;
	int *hs, *ind, *iu, *iw, *loc;
	size_t L, Lc, Li;
	ftnlen cu_len, cw_len, prob_len, start_len;

	T0 = snopt_details;
	snopt_details = T = (Snopt_Details *)v;
	T->D = D;	//should be unnecessary
	ps = DF->ps;
	T->nf = 0;
	T->n = n = ps->n_var;
	T->nnlc = nnlc = ps->n_nlinc;
	T->co = co = ps->n_obj;
	T->numgflag = ps->numgflag;
	T->maxfe = ps->maxfe;
	nlc = ps->n_linc;
	T->m = m = nlc + nnlc;
	// Since Dakota does not provide sparsity information,
	// we simply assume all variables are nonlinear and
	// involved with every nonlinear function.
	nlnz = nnlc * n;
	loc = new int[n + 1];
	memset(loc, 0, n*sizeof(int));
	RMe = linear_eq_constraint_coeffs(D);
	nlce = RMe->numRows();
	for(i = 0; i < nlce; i++) {
		for(j = 0; j < n; j++)
			if ((*RMe)[i][j] != 0.)
				++loc[j];
		}
	RMi = linear_ineq_constraint_coeffs(D);
	nlci = RMi->numRows();
	for(i = 0; i < nlci; i++) {
		for(j = 0; j < n; j++)
			if ((*RMi)[i][j] != 0.)
				++loc[j];
		}
	for(i = nz = 0; i < n; i++) {
		j = loc[i];
		loc[i] = nz;
		nz += j + nnlc;
		}
	ind = new int[nz];
	A = new double[nz];
	memset(A, 0, nz*sizeof(double));
	for(i = 0; i < n; i++) {
		j = loc[i];
		for(k = 1; k <= nnlc; k++)
			ind[j++] = k;
		loc[i] = j;
		}
	for(i = 0; i < nlce; ++i) {
		i1 = i + nnlc + 1;
		for(j = 0; j < n; j++)
			if ((t = (*RMe)[i][j]) != 0.) {
				k = loc[j]++;
				ind[k] = i1;
				A[k] = t;
				}
		}
	for(i = 0; i < nlci; ++i) {
		i1 = i + nnlc + 1;
		for(j = 0; j < n; j++)
			if ((t = (*RMi)[i][j]) != 0.) {
				k = loc[j]++;
				ind[k] = i1;
				A[k] = t;
				}
		}
	for(i = n; i > 0; --i)
		loc[i] = loc[i-1] + 1;
	loc[0] = 1;

	lencw = leniw = lenrw = 500;
	rw = new double[lenrw];
	iw = new int[leniw];
	cw = new char[lencw*8];
	mincw = miniw = minrw = -1;
	cu_len = cw_len = prob_len = 8;
	nnjac = nnobj = n;	//snmemb_ may clobber nnjac and nnobj if nnlc == 0
	i = j = 0; // no output from first call on sninit_
	sninit_(&i, &j, cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	i = 0;
	snmemb_(&i, &m, &n, &nz, &nlnz, &nnlc, &nnjac, &nnobj, &mincw, &miniw, &minrw,
		cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	delete [] cw;
	delete [] iw;
	delete [] rw;
	i = 20 * (m + n);
	leniw = miniw + i;
	lenrw = minrw + i;
	lencw = mincw;
	nm = n + m;
	L = lenrw + 3*nm + 2*n + m + 1;
	rw = new double[L];
	memset(rw, 0, L*sizeof(double));
	bl = rw + lenrw;
	bu = bl + nm;
	rc = bu + nm;
	pi = rc + nm;
	x = pi + m;
	T->Lastx = x + n;
	ru = T->Lastx + n;
	Li = leniw + 1 + nm;
	iw = new int[Li];
	hs = iw + leniw;
	iu = hs + nm;
	Lc = lencw*8;
	cw = new char[Lc + 8];
	i = 6;	// detailed output
	j = 0;	// no summary output
	// Look for initial outlev = n:
	// n & 1 ==> want detailed output; n & 2 ==> summary output
	if (dlsolver_option(&OI)) {
		if (!strncmp(OI.name, "outlev", OI.name_len)) {
			k = (int)strtol(OI.val, &ve, 10);
			if (ve > OI.val) {
				i = k & 1 ? 6 : 0;
				j = k & 2 ? 6 : 0;
				}
			}
		else
			OI.begin = options;	// reset
		}
	sninit_(&i, &j, cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	i = 0;
	snmemb_(&i, &m, &n, &nz, &nlnz, &nnlc, &nnjac, &nnobj, &mincw, &miniw, &minrw,
		cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	cu = cw + Lc;
	start_len = 4;
	lencu = leniu = lenru = 1;
	Rv = continuous_lower_bounds(D);
	Ru = continuous_upper_bounds(D);
	k = Rv->length();
	for(i = 0; i < k; i++) {
		bl[i] = (*Rv)[i];
		bu[i] = (*Ru)[i];
		}
	Rv = nonlinear_ineq_constraint_lower_bounds(D);
	Ru = nonlinear_ineq_constraint_upper_bounds(D);
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i) {
		bl[i] = (*Rv)[j];
		bu[i] = (*Ru)[j];
		}
	Rv = nonlinear_eq_constraint_targets(D);
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i)
		bl[i] = bu[i] = (*Rv)[j];
	Rv = linear_eq_constraint_targets(D);
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i)
		bl[i] = bu[i] = (*Rv)[j];
	Rv = linear_ineq_constraint_lower_bounds(D);
	Ru = linear_ineq_constraint_upper_bounds(D);
	k = Rv->length();
	for(j = 0; j < k; j++, i++) {
		bl[i] = (*Rv)[j];
		bu[i] = (*Ru)[j];
		}
	j = 0;
	while(dlsolver_option(&OI)) {
		inform = 0;
		snset_(OI.name, &j, &j, &inform, cw, &lencw, iw, &leniw, rw, &lenrw,
			(ftnlen)OI.all_len, cw_len);
		printf(inform	? "\n**** Rejecting \"%.*s\"****\n"
				: "accepted snopt option: %.*s\n", OI.all_len, OI.name);
		}
	GetContVars(D, n, x);
	f = std::numeric_limits<double>::infinity();
	nname = 1;	// no names
	objadd = sinf = 0.;
	inform = ns = ninf = 0;
	if (x[0] == 0.)
		T->Lastx[0] = 1.; // ensure Lastx != x;
	snopt_("Cold", &m, &n, &nz, &nname, &nnlc, &nnobj, &nnjac, &m, &objadd,
		"dakotapr", funcon, funobj, A, ind, loc,
		bl, bu, cu, hs, x, pi, rc, &inform, &mincw, &miniw,
		&minrw, &ns, &ninf, &sinf, &f, cu, &lencu,
		iu, &leniu, ru, &lenru, cw, &lencw, iw, &leniw,
		rw, &lenrw, start_len, prob_len, cu_len,
		cu_len, cw_len);
	flush_();	// flush Fortran buffers
	if (inform > 1) {
		f = std::numeric_limits<double>::infinity();
		printf("snopt returned inform = %d\n", inform);
		}
	SetBestContVars(D, n, x);
	if (!ps->multiobj) {
		Real bestf[m+1];
		bestf[0] = f;
		for (i = 0; i < m; i++)
			bestf[i+1] = x[i];
		SetBestRespFns(D, m+1, bestf);
		}
	delete[] cw;
	delete[] iw;
	delete[] rw;
	delete[] A;
	delete[] ind;
	delete[] loc;
	}

 static void
my_destructor(void **v)
{
	Snopt_Details *T;
	if ((T = *(Snopt_Details **)v)) {
		/*DEBUG*/printf("\ndl_snopt calling ~Snopt_Details\n\n");
		*v = 0;
		delete T;
		}
	}

 void*
dl_constructor(Optimizer1 *D, Dakota_funcs *df, dl_find_optimum_t *findopt, dl_destructor_t *dtor)
{
	Snopt_Details *T;

	*findopt = my_findopt;
	*dtor = my_destructor;
	DF = df;
	T = new Snopt_Details;
	T->D = D;
	return (void*)T;
	}

} // namespace Dakota
