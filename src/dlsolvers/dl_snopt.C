#include <stdio.h>
#include <limits>
#include "DLSolver.H"
#define NO_DAKOTA_DLSOLVER_FUNCS_INLINE
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

extern "C" int snopt_(const char *start, int *m, int *n, int *ne,
	int *nname, int *nncon, int *nnobj, int *nnjac,
	int *iobj, double *objadd, const char *prob, c_fp fgcon, o_fp fgobj,
	double *jcol, int *indj, int *locj, double *bl,
	double *bu, char *names, int *hs, double *x, double *pi,
	double *rc, int *inform, int *mincw, int *miniw,
	int *minrw, int *ns, int *ninf, double *sinf,
	double *obj, char *cu, int *lencu, int *iu, int *leniu,
	double *ru, int *lenru, char *cw, int *lencw, int *iw,
	int *leniw, double *rw, int *lenrw, ftnlen start_len,
	ftnlen prob_len, ftnlen names_len,
	ftnlen cu_len, ftnlen cw_len);

extern "C" int snmemb_(int *m, int *n, int *ne, int *negcon,
	int *nncon, int *nnjac, int *nnobj, int *mincw, int *miniw,
	int *minrw, char *cw, int *lencw, int *iw, int *leniw,
	double *rw, int *lenrw, ftnlen cw_len);

extern "C" int snset_(char *buffer, int *iprint, int *isumm,
	int *inform, char *cw, int *lencw, int *iw, int *leniw,
	double *rw, int *lenrw, ftnlen buffer_len, ftnlen cw_len);

namespace Dakota {

 struct
Snopt_Details {
	Model *M;
	Optimizer1 *D;
	int co, n, nf, nnlc;
	double *Lastx;
	};

 static Snopt_Details *snopt_details;

 static int
eval_check(int *asvreq, int *MODE, double *X)
{
	Model *M;
	Optimizer1 *D;
	Snopt_Details *T;
	int n;
	size_t L;

	T = snopt_details;
	D = T->D;
	n = T->n;
	L = n*sizeof(double);

	*asvreq = *MODE + 1;
	if (D->vendorNumericalGradFlag_() && (*asvreq & 2) ) {
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
		if (++T->nf >= D->maxFunctionEvals_()) {
			Cout << "Iteration terminated: max_function_evaluations "
				"limit has been met." << endl;
			return 1;
			}
		memcpy(T->Lastx, X, L);
		RealVector lx(n);
		copy_data(X, n, lx);
		M = T->M;
		M->continuous_variables(lx);
		D->activeSet_()->request_values(*asvreq);
		M->compute_response(*D->activeSet_());
		}
	return 0;
	}

 static int
funcon(int *MODE, int *M1, int *N, int *NJAC, double *X, double *F, double *G,
	int *NSTATE, char *CU, int *LENCU, int *IU, int *LENIU, double *RU,
	int *LENRU, ftnlen len_cu)
{
	Model *M;
	RealMatrix const *RM;
	RealVector const *RV;
	Response const *R;
	Snopt_Details *T;
	const double *gi;
	int asv_req;
	size_t co, i, ie, j, je, k, n, nnlc, nx;

	T = snopt_details;
	M = T->M;
	n = T->n;
	nnlc = T->nnlc;

	if (eval_check(&asv_req, MODE, X)) {
		*MODE = -2;
		return 0;
		}
	R = &M->current_response();
	co = T->co;
	if (asv_req & 1) {
		RV = &R->function_values();
		for(i = 0; i < nnlc; i++)
			F[i] = (*RV)[i + co];
		}
	if (asv_req & 2) {
		RM = &R->function_gradients();
		ie = co + nnlc;
		nx = n * nnlc;
		for(k = 0, i = co; i < ie; ++i, ++k) {
			gi = (*RM)[i];
			for(j = k, je = k + nx; j < je; j += nnlc)
				G[j] = *gi++;
			}
		}
	return 0;
	}

 static int
funobj(int *MODE, int *N, double *X, double *F, double *G, int *NSTATE,
	char *cu, int *lencu, int *iu, int *leniu, double *ru, int *lenru,
	ftnlen cu_len)
{
	Model *M;
	Response const *R;
	Snopt_Details *T;
	int asv_req;

	T = snopt_details;
	M = T->M;

	if (eval_check(&asv_req, MODE, X)) {
		*MODE = -2;
		return 0;
		}
	R = &M->current_response();
	if (asv_req & 1)
		*F = R->function_values()[0];
	if (asv_req & 2)
		memcpy(G, R->function_gradients()[0], *N * sizeof(double));
	return 0;
	}


 static void
my_findopt(void *v, Optimizer1 *D, char *options)
{
	Model *M;
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
	T->nf = 0;
	T->n = n = D->numContinuousVars_();
	T->nnlc = nnlc = D->numNonlinearConstraints_();
	T->co = co = D->numObjectiveFunctions_();
	nlc = D->numLinearConstraints_();
	m = nlc + nnlc;
	// Since Dakota does not provide sparsity information,
	// we simply assume all variables are nonlinear and
	// involved with every nonlinear function.
	nlnz = nnlc * n;
	M = T->M;
	loc = new int[n + 1];
	memset(loc, 0, n*sizeof(int));
	RMe = &M->linear_eq_constraint_coeffs();
	nlce = RMe->numRows();
	for(i = 0; i < nlce; i++) {
		for(j = 0; j < n; j++)
			if ((*RMe)[i][j] != 0.)
				++loc[j];
		}
	RMi = &M->linear_ineq_constraint_coeffs();
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
	snmemb_(&m, &n, &nz, &nlnz, &nnlc, &nnjac, &nnobj, &mincw, &miniw, &minrw,
		cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	delete[] cw;
	delete[] iw;
	delete[] rw;
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
	memset(iw, 0, Li*sizeof(int));
	hs = iw + leniw;
	iu = hs + nm;
	Lc = lencw*8;
	cw = new char[Lc + 8];
	memset(cw, 0, Lc + 8);
	cu = cw + Lc;
	start_len = 4;
	lencu = leniu = lenru = 1;
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
			OI.begin = D->options;	// reset
		}
	sninit_(&i, &j, cw, &lencw, iw, &leniw, rw, &lenrw, cw_len);
	Rv = &M->continuous_lower_bounds();
	Ru = &M->continuous_upper_bounds();
	k = Rv->length();
	for(i = 0; i < k; i++) {
		bl[i] = (*Rv)[i];
		bu[i] = (*Ru)[i];
		}
	Rv = &M->nonlinear_ineq_constraint_lower_bounds();
	Ru = &M->nonlinear_ineq_constraint_upper_bounds();
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i) {
		bl[i] = (*Rv)[j];
		bu[i] = (*Ru)[j];
		}
	Rv = &M->nonlinear_eq_constraint_targets();
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i)
		bl[i] = bu[i] = (*Rv)[j];
	Rv = &M->linear_eq_constraint_targets();
	k = Rv->length();
	for(j = 0; j < k; ++j, ++i)
		bl[i] = bu[i] = (*Rv)[j];
	Rv = &M->linear_ineq_constraint_lower_bounds();
	Ru = &M->linear_ineq_constraint_upper_bounds();
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
	memcpy(x, &M->continuous_variables()[0], n*sizeof(double));
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
	D->DF->SetBestContVars(D, n, x);
	if (!D->multiObjFlag_()) {
		rc += n-1;
		*rc = f;
		D->DF->SetBestRespFns(D, m+1, rc);
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
	Snopt_Details *T = *(Snopt_Details **)v;
	/*DEBUG*/printf("\ndl_snopt calling ~Snopt_Details\n\n");
	*v = 0;
	delete T;
	}

 void*
dl_constructor(Optimizer1 *D, Dakota_funcs *df, dl_find_optimum_t *findopt, dl_destructor_t *dtor)
{
	Snopt_Details *T;

	*findopt = my_findopt;
	*dtor = my_destructor;
	T = new Snopt_Details;
	T->D = D;
	T->M = D->M0;
	return (void*)T;
	}

} // namespace Dakota
