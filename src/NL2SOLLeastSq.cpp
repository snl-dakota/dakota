/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NL2SOLLeastSq
//- Description: Implementation code for the NL2SOLLeastSq class
//- Owner:       David Gay
//- Checked by:

#include "dakota_system_defs.hpp"
#include "NL2SOLLeastSq.hpp"
#include "ProblemDescDB.hpp"
#include <boost/math/special_functions/fpclassify.hpp>


// We use statistical notation:  p is the number of parameters being estimated
// (the number of optimization variables being determined) and n is the number
// of observations (terms the sum of whose squares we wish to minimize).

namespace Dakota {

NL2SOLLeastSq* NL2SOLLeastSq::nl2solInstance(NULL);


NL2SOLLeastSq::NL2SOLLeastSq(ProblemDescDB& problem_db, Model& model):
  LeastSq(problem_db, model),
  // output controls
  auxprt(31), outlev(1), // normal/verbose/debug
  // finite differencing
  dltfdj(0.), delta0(0.), dltfdc(0.),
  // max limits
  mxfcal(maxFunctionEvals), mxiter(maxIterations),
  // convergence tolerances
  rfctol( convergenceTol ),
  afctol( probDescDB.get_real("method.nl2sol.absolute_conv_tol") ),
  xctol(  probDescDB.get_real("method.nl2sol.x_conv_tol") ),
  sctol(  probDescDB.get_real("method.nl2sol.singular_conv_tol") ),
  lmaxs(  probDescDB.get_real("method.nl2sol.singular_radius") ),
  xftol(  probDescDB.get_real("method.nl2sol.false_conv_tol") ),
  // post-processing options
  covreq( probDescDB.get_int ("method.nl2sol.covariance") ),
  rdreq(  probDescDB.get_bool("method.nl2sol.regression_diagnostics") ),
  // function precision
  fprec(  probDescDB.get_real("method.function_precision") ),
  // initial TR radius
  lmax0(  probDescDB.get_real("method.nl2sol.initial_trust_radius") )
{
  const RealVector&   fd_g_ss = iteratedModel.fd_gradient_step_size();
  const RealVector& fd_hbg_ss = iteratedModel.fd_hessian_by_grad_step_size();
  const RealVector& fd_hbf_ss = iteratedModel.fd_hessian_by_fn_step_size();
  if (  !fd_g_ss.empty()) dltfdj =   fd_g_ss[0];
  if (!fd_hbg_ss.empty()) delta0 = fd_hbg_ss[0];
  if (!fd_hbf_ss.empty()) dltfdc = fd_hbf_ss[0];

  if (outputLevel == SILENT_OUTPUT)
    auxprt = outlev = 0;
  else if (outputLevel == QUIET_OUTPUT) {
    auxprt = 3; outlev = 0;
  }
}


NL2SOLLeastSq::NL2SOLLeastSq(Model& model): LeastSq(NL2SOL, model),
  // output controls
  auxprt(31), outlev(1), // normal/verbose/debug
  // finite differencing
  dltfdj(0.), delta0(0.), dltfdc(0.),
  // max limits
  mxfcal(maxFunctionEvals), mxiter(maxIterations),
  // convergence tolerances: -1 = use internal NL2SOL default
  rfctol(convergenceTol),
  afctol(-1.), xctol(-1.), sctol(-1.), lmaxs(-1.), xftol(-1.),
  // post-processing options
  covreq(0), rdreq(false),
  // function precision
  fprec(1.e-10),
  // initial TR radius
  lmax0(-1.)
{
  const RealVector&   fd_g_ss = iteratedModel.fd_gradient_step_size();
  const RealVector& fd_hbg_ss = iteratedModel.fd_hessian_by_grad_step_size();
  const RealVector& fd_hbf_ss = iteratedModel.fd_hessian_by_fn_step_size();
  if (  !fd_g_ss.empty()) dltfdj =   fd_g_ss[0];
  if (!fd_hbg_ss.empty()) delta0 = fd_hbg_ss[0];
  if (!fd_hbf_ss.empty()) dltfdc = fd_hbf_ss[0];

  if (outputLevel == SILENT_OUTPUT)
    auxprt = outlev = 0;
  else if (outputLevel == QUIET_OUTPUT) {
    auxprt = 3; outlev = 0;
  }
}


NL2SOLLeastSq::~NL2SOLLeastSq()
{ }


typedef void (*Calcrj)(int *n, int *p, Real *x, int *nf, Real *r, int *ui,
		       void *ur, Vf vf);

extern "C" void dn2f_(int *n, int *p, Real *x, Calcrj, int *iv, int *liv,
		      int *lv, Real *v, int *ui, void *ur, Vf);
extern "C" void dn2fb_(int *n, int *p, Real *x, Real *b, Calcrj, int *iv,
		       int *liv, int *lv, Real *v, int *ui, void *ur, Vf);
extern "C" void dn2g_(int *n, int *p, Real *x, Calcrj, Calcrj, int *iv,
		      int *liv, int *lv, Real *v, int *ui, void *ur, Vf);
extern "C" void dn2gb_(int *n, int *p, Real *x, Real *b, Calcrj, Calcrj,
		       int *iv, int *liv, int *lv, Real *v, int *ui, void *ur,
		       Vf);
extern "C" void divset_(int*, int*, int*, int*, Real*);
extern "C" double dr7mdc_(int*);


/// Auxiliary information passed to calcr and calcj via ur.
struct NL2Res {
	Real *r; ///< residual r = r(x)
	Real *J; ///< Jacobian J = J(x)
	Real *x; ///< corresponding parameter vector
	int nf;  ///< function invocation count for r(x)
	};
struct Nl2Misc {
  NL2Res RC[4];	///< Cache the current best and two most recent residuals
  NL2Res *R[4]; ///< pointers to RC, so we can quickly update the current view
  int *nfgcal;	///< pointer to NL2SOL record of nf value at best iterate
  int specgrad;	///< whether to cache J values (0 == no, 1 == yes)
  int ic;	///< which saved residual to update
  int newR;	///< set to 1 if on next call we should check on updating the current view
  int n, p;	///< problem dimensions: n residuals, p parameters being estimated
};

 static void
Rswapchk(Nl2Misc *q)
{
	NL2Res *R;
	int i, ic, nf;

	q->newR = 0;
	ic = q->ic;
	R = q->R[ic];
	if (R->nf == *q->nfgcal) {
		q->R[ic] = q->R[2];
		q->R[2] = R;
		}
	if (q->nfgcal[5] != q->RC[3].nf) {
		// doing finite differences for covariance or regression diagnostic
		nf = q->nfgcal[5];
		for(i = 0; i < 3; ++i)
			if (q->RC[i].nf == nf) {
				q->RC[3].nf = nf;
				memcpy(q->RC[3].x, q->RC[i].x, q->p*sizeof(Real));
				memcpy(q->RC[3].r, q->RC[i].r, q->n*sizeof(Real));
				memcpy(q->RC[3].J, q->RC[i].J, q->n*q->p*sizeof(Real));
				break;
				}
		}
	}

#if 0  /* alternative to isfinite() loop below */
// Solaris makes isfinite() availble in C but not C++,
// so we roll our own here...
 static int
hasnaninf(const double *d, int n)
{
	// assume sizeof(unsigned int) == 4, sizeof(double) == 8
	union {double d; unsigned int u[2]; } u;
	unsigned int *x, *xe;

	x = (unsigned int*)d;
	u.d = 1.;
	if (u.u[1])
		++x;	// adjustment, if needed, for little-endian systems
	for(xe = x + 2*n; x < xe; x += 2)
		if ((*x & 0x7ff00000) == 0x7ff00000)
			return 1;
	return 0;
	}
#endif

void NL2SOLLeastSq::
calcr(int *np, int *pp, Real *x, int *nfp, Real *r, int *ui, void *ur, Vf vf)
{
  using boost::math::isfinite;

  int i, ic, j, k, n = *np, nf = *nfp, nfc, p = *pp, spec;
  Nl2Misc *q = (Nl2Misc*)ur;
  NL2Res *R;
  Real *J, *rc, *xc;
  RealVector xd(p);

  spec = q->specgrad;
  ic = q->ic;
  nfc = q->R[ic]->nf;
  if (q->newR)
	Rswapchk(q);
  copy_data(x, p, xd);
  nl2solInstance->iteratedModel.continuous_variables(xd);
  nl2solInstance->activeSet.request_values(spec + 1);
  nl2solInstance->iteratedModel.evaluate(nl2solInstance->activeSet);
  const Response& lr = nl2solInstance->iteratedModel.current_response();

  const RealVector& lf = lr.function_values();

  for(i = 0; i < n; ++i)
	if (!isfinite(lf[i])) {
		*nfp = 0;	/* indicate failed evaluation */
		return;
		}

  if (nfc != nf) {
	/* In the midst of finite differences, nf and q->nf[ic] will agree */
	/* but x and the resulting residual will vary. */
	if (nf == 1)
		q->ic = 1; /* ic == 2 */
	else {
		q->ic = ic = 1 - ic;
		q->newR = 1;
		}
	R = q->R[ic];
	R->nf = nf;
	rc = R->r;
	xc = R->x;
	for(i = 0; i < p; i++)
		xc[i] = x[i];
	for(i = 0; i < n; i++)
		r[i] = rc[i] = lf[i];
	if (spec) {
		J = R->J;
		const RealMatrix& lg = lr.function_gradients();
		const Real* Gradi;
		for(i = 0; i < n; i++) {
			Gradi = lg[i];
			for(j = 0, k = i; j < p; j++, k += n)
				J[k] = Gradi[j];
			}
		}
	}
  else
	for(i = 0; i < n; i++)
		r[i] = lf[i];
}


void NL2SOLLeastSq::
calcj(int *np, int *pp, Real *x, int *nfp, Real *J, int *ui, void *ur, Vf vf)
{
  using boost::math::isfinite;

  int i, j, k, n = *np, nf = *nfp, p = *pp;
  Nl2Misc *q = (Nl2Misc*)ur;
  Real *Jq;

  if (q->newR)
	Rswapchk(q);
  if (q->specgrad) {
	for(i = 0; i < 3; ++i)
		if (nf == q->RC[i].nf) {
			Jq = q->RC[i].J;
			k = n*p;
			for(i = 0; i < k; ++i)
				if (!isfinite(J[i] = Jq[i])) {
					*nfp = 0; /* indicate failed evaluation */
					break;
					}
			return;
			}
	}
    RealVector xd(p);
    copy_data(x, p, xd);
    nl2solInstance->iteratedModel.continuous_variables(xd);

    nl2solInstance->activeSet.request_values(2);
    nl2solInstance->iteratedModel.evaluate(nl2solInstance->activeSet);
    const Response& lr = nl2solInstance->iteratedModel.current_response();

    const RealMatrix& lg = lr.function_gradients();
    const Real* Gradi;
    for(i = 0; i < n; i++) {
      Gradi = lg[i];
      for(j = 0, k = i; j < p; j++, k += n)
	J[k] = Gradi[j];
    }
  k = n*p;
  for(i = 0; i < k; ++i)
	if (!isfinite(J[i])) {
		*nfp = 0;	/* indicate failed evaluation */
		break;
		}
}


void NL2SOLLeastSq::core_run()
{
  // set the object instance pointer for use within the static member fns
  NL2SOLLeastSq* prev_instance = nl2solInstance;
  nl2solInstance = this;

  Nl2Misc q;
  Real *b, macheps, t, *v, *x;
  const Real *R;
  int i, *iv, j, lb, liv, lv, n, p;
  size_t Jlen, L;
  static int L1 = 1, L3 = 3;

  /// Details on the following subscript values appear in
  /// "Usage Summary for Selected Optimization Routines" by David M. Gay,
  /// Computing Science Technical Report No. 153, AT&T Bell Laboratories, 1990.
  /// http://netlib.bell-labs.com/cm/cs/cstr/153.ps.gz

  enum IV_subscripts {
    iv_covprt = 13,
    iv_covreq = 14,
    iv_dradpr = 100,
    iv_mxfcal = 16,
    iv_mxiter = 17,
    iv_outlev = 18,
    iv_parprt = 19,
    iv_rdreq  = 56,
    iv_solprt = 21,
    iv_statpr = 22,
    iv_x0prt  = 23
  };
  enum V_subsripts {
    v_afctol  = 30,
    v_cosmin  = 46,
    v_dltfdc  = 41,
    v_dltfdj  = 42,
    v_delta0  = 43,
    v_lmax0   = 34,
    v_lmaxs   = 35,
    v_rfctol  = 31,
    v_sctol   = 36,
    v_xctol   = 32,
    v_xftol   = 33
  };

  q.RC[0].nf = q.RC[1].nf = q.RC[2].nf = -1;
  q.RC[3].nf = 0;
  q.ic = 2;
  q.newR = 0;
  q.specgrad = (speculativeFlag || iteratedModel.gradient_type() == "analytic")
    ? 2 : 0;
  if (vendorNumericalGradFlag)
    q.specgrad = 0;

  p = numContinuousVars;
  n = numLeastSqTerms;
  liv = 103 + 4*p;
  lv = 105 + p*(n + 2*p + 21) + 2*n;
  lb = boundConstraintFlag ? 2*p : 0;
  Jlen = q.specgrad ? n*p : 0;
  x = (Real*)malloc(L = (lv + 5*p + 4*n + lb + 4*Jlen)*sizeof(Real) + liv*sizeof(int));
  if (!x) {
    Cerr << "nl2: malloc(" << L << ") failed!\n";
    return;
  }
  b = x + p;
  v = b + lb;
  q.p = p;
  q.n = n;
  q.RC[0].J = v + lv;
  q.RC[1].J = q.RC[0].J + Jlen;
  q.RC[2].J = q.RC[1].J + Jlen;
  q.RC[3].J = q.RC[2].J + Jlen;
  q.RC[0].x = q.RC[3].J + Jlen;
  q.RC[1].x = q.RC[0].x + p;
  q.RC[2].x = q.RC[1].x + p;
  q.RC[3].x = q.RC[2].x + p;
  q.RC[0].r = q.RC[3].x + p;
  q.RC[1].r = q.RC[0].r + n;
  q.RC[2].r = q.RC[1].r + n;
  q.RC[3].r = q.RC[2].r + n;
  for(i = 0; i < 4; ++i)
	q.R[i] = &q.RC[i];
  iv = (int*)(q.RC[3].r + n);
  q.nfgcal = iv + 6;

  divset_(&L1, iv, &liv, &lv, v);
  iv[iv_outlev] = outlev;
  iv[iv_x0prt] = auxprt & 1;
  iv[iv_solprt] = (auxprt & 2)  >> 1;
  iv[iv_statpr] = (auxprt & 4)  >> 2;
  iv[iv_parprt] = (auxprt & 8)  >> 3;
  iv[iv_dradpr] = (auxprt & 16) >> 4;
  i = 0;
  if (!covreq || covreq > 3 || covreq < -3)
    iv[iv_covreq] = 0;
  else {
    iv[iv_covreq] = covreq;
    i = 1;
  }
  if (rdreq)
    i += 2;
  iv[iv_covprt] = iv[iv_rdreq] = i;

  macheps = dr7mdc_(&L3);
  if (fprec > macheps && fprec < 1) {
    v[v_xctol] = t = std::sqrt(fprec);
    if (vendorNumericalGradFlag)
      v[v_dltfdj] = t;
    if (i)
      v[v_delta0] = t;
    t = std::pow(fprec, 1./3.);
    if (i)
      v[v_dltfdc] = t;
    t *= t;
    if (t > 1e-10)
      v[v_rfctol] = t;
    v[v_xftol] = fprec < 1e-4 ? 100.*fprec : 1e-2;
    if (fprec > 1e-8) {
      t = 100*fprec;
      if (t > 1e-3)
	t = 1e-3;
      v[v_cosmin] = t;
    }
  }
  if (afctol > 0)
    v[v_afctol] = afctol;
  if (dltfdc > 0 && i)
    v[v_dltfdc] = dltfdc;
  if (vendorNumericalGradFlag && dltfdj > 0)
    v[v_dltfdj] = dltfdj;
  if (delta0 > 0 && i)
    v[v_delta0] = delta0;
  if (lmax0 > 0)
    v[v_lmax0] = lmax0;
  if (lmaxs > 0)
    v[v_lmaxs] = lmaxs;
  if (mxfcal > 0)
    iv[iv_mxfcal] = mxfcal;
  if (mxiter > 0)
    iv[iv_mxiter] = mxiter;
  if (rfctol > 0)
    v[v_rfctol] = rfctol;
  if (sctol > 0)
    v[v_sctol] = sctol;
  if (xctol > 0)
    v[v_xctol] = xctol;
  if (xftol > 0)
    v[v_xftol] = xftol;

  copy_data(iteratedModel.continuous_variables(), x, p);

  if (boundConstraintFlag) {
    const RealVector& Lb = iteratedModel.continuous_lower_bounds();
    const RealVector& Ub = iteratedModel.continuous_upper_bounds();
    for(i = j = 0; i < p; i++) {
      b[j++] = Lb[i];
      b[j++] = Ub[i];
    }
    if (vendorNumericalGradFlag)
      dn2fb_(&n, &p, x, b, calcr,        iv, &liv, &lv, v, 0, &q, 0);
    else
      dn2gb_(&n, &p, x, b, calcr, calcj, iv, &liv, &lv, v, 0, &q, 0);
  }
  else if (vendorNumericalGradFlag)
    dn2f_(&n, &p, x, calcr,	   iv, &liv, &lv, v, 0, &q, 0);
  else
    dn2g_(&n, &p, x, calcr, calcj, iv, &liv, &lv, v, 0, &q, 0);

  RealVector xd(p);
  copy_data(x, p, xd);
  bestVariablesArray.front().continuous_variables(xd);
  R = 0;
  for(i = 0; i < 4; ++i)
	if (q.RC[i].nf > 0 && !memcmp(x, q.RC[i].x, p*sizeof(Real))) {
		R = q.RC[i].r;
		break;
		}
  if (R == 0) { // possible with "numerical_gradients method_source vendor"
	i = 0;
	calcr(&n, &p, x, &i, q.RC[0].r, 0, &q, 0);
	R = q.RC[0].r;
	}
  // If no interpolation, numUserPrimaryFns <= numLsqTerms.  Copy the
  // first block of inbound model fns to best.  If data transform,
  // will be further transformed back to user space (weights, scale,
  // data) if needed in LeastSq::post_run
  if ( !(calibrationDataFlag && expData.interpolate_flag()) )
    for (size_t i=0; i<numUserPrimaryFns; ++i)
      bestResponseArray.front().function_value(R[i], i);

  free(x);

  nl2solInstance = prev_instance; // restore in case of recursion
}

} // namespace Dakota
