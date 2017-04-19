/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalReliability
//- Description: Implementation code for NonDLocalReliability class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaGraphics.hpp"
#include "NonDLocalReliability.hpp"
#include "NonDAdaptImpSampling.hpp"
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
using OPTPP::NLPFunction;
using OPTPP::NLPGradient;
#endif
#include "RecastModel.hpp"
#include "DataFitSurrModel.hpp"
#include "NestedModel.hpp"
#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include <algorithm>
#include "dakota_data_io.hpp"

//#define MPP_CONVERGE_RATE
//#define DEBUG

static const char rcsId[] = "@(#) $Id: NonDLocalReliability.cpp 4058 2006-10-25 01:39:40Z mseldre $";

namespace Dakota {

extern PRPCache data_pairs; // global container

// initialization of statics
NonDLocalReliability* NonDLocalReliability::nondLocRelInstance(NULL);


NonDLocalReliability::
NonDLocalReliability(ProblemDescDB& problem_db, Model& model):
  NonDReliability(problem_db, model), 
  initialPtUserSpec(
    probDescDB.get_bool("variables.uncertain.initial_point_flag")),
  warmStartFlag(true), nipModeOverrideFlag(true),
  curvatureDataAvailable(false), kappaUpdated(false),
  secondOrderIntType(HOHENRACK), curvatureThresh(1.e-10), warningBits(0)
{
  // check for suitable gradient and variables specifications
  if (iteratedModel.gradient_type() == "none") {
    Cerr << "\nError: local_reliability requires a gradient specification."
	 << std::endl;
    abort_handler(-1);
  }
  if (numEpistemicUncVars) {
    Cerr << "Error: epistemic variables are not supported in local "
	 << "reliability methods." << std::endl;
    abort_handler(-1);
  }

  if (mppSearchType) { // default is MV = 0

    // Map MPP search NIP/SQP algorithm specification into an NPSOL/OPT++
    // selection based on configuration availability.
    unsigned short mpp_optimizer = probDescDB.get_ushort("method.sub_method");
    if (mpp_optimizer == SUBMETHOD_SQP) {
#ifdef HAVE_NPSOL
      npsolFlag = true;
#else
      Cerr << "\nError: this executable not configured with NPSOL SQP.\n"
	   << "         Please select OPT++ NIP within local_reliability."
	   << std::endl;
      abort_handler(-1);
#endif
    }
    else if (mpp_optimizer == SUBMETHOD_NIP) {
#ifdef HAVE_OPTPP
      npsolFlag = false;
#else
      Cerr << "\nError: this executable not configured with OPT++ NIP.\n"
	   << "         please select NPSOL SQP within local_reliability."
	   << std::endl;
      abort_handler(-1);
#endif
    }
    else if (mpp_optimizer == SUBMETHOD_DEFAULT) {
#ifdef HAVE_NPSOL
      npsolFlag = true;
#elif HAVE_OPTPP
      npsolFlag = false;
#else
      Cerr << "\nError: this executable not configured with NPSOL or OPT++.\n"
	   << "       NonDLocalReliability cannot perform MPP search."
	   << std::endl;
      abort_handler(-1);
#endif
    }

    // Error check for a specification of at least 1 level for MPP methods
    if (!totalLevelRequests) {
      Cerr << "\nError: An MPP search method requires the specification of at "
	   << "least one response, probability, or reliability level."
	   << std::endl;
      abort_handler(-1);
    }
  }

  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for SOL presence.
  // Note 1: This check is performed for DOT, CONMIN, and SOLBase, but not
  //         for LHS since it is only active in pre-processing.
  // Note 2: NPSOL/NLSSOL on the outer loop with NonDLocalReliability on the
  //         inner loop precludes all NPSOL-based MPP searches;
  //         NonDLocalReliability on the outer loop with NPSOL/NLSSOL on an
  //         inner loop is only a problem for the no_approx MPP search (since
  //         iteratedModel is not invoked w/i an approx-based MPP search).
  if (mppSearchType == NO_APPROX && npsolFlag) {
    Iterator sub_iterator = iteratedModel.subordinate_iterator();
    if (!sub_iterator.is_null() && 
	( sub_iterator.method_name() ==  NPSOL_SQP ||
	  sub_iterator.method_name() == NLSSOL_SQP ||
	  sub_iterator.uses_method() ==  NPSOL_SQP ||
	  sub_iterator.uses_method() == NLSSOL_SQP ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	  ( sub_iterator.method_name() ==  NPSOL_SQP ||
	    sub_iterator.method_name() == NLSSOL_SQP ||
	    sub_iterator.uses_method() ==  NPSOL_SQP ||
	    sub_iterator.uses_method() == NLSSOL_SQP ) )
	sub_iterator.method_recourse();
    }
  }

  // Map response Hessian specification into taylorOrder for use by MV/AMV/AMV+
  // variants.  Note that taylorOrder and integrationOrder are independent
  // (although the Hessian specification required for 2nd-order integration
  // means that taylorOrder = 2 will be used for MV/AMV/AMV+; taylorOrder = 2
  // may however be used with 1st-order integration).
  const String& hess_type = iteratedModel.hessian_type();
  taylorOrder = (hess_type != "none" && mppSearchType <= AMV_PLUS_U) ? 2 : 1;

  // assign iterator-specific defaults for approximation-based MPP searches
  if (maxIterations <  0          && // DataMethod default = -1
      mppSearchType >= AMV_PLUS_X && mppSearchType < NO_APPROX) // approx-based
    maxIterations = 25;

  // Map integration specification into integrationOrder.  Second-order
  // integration requires an MPP search in u-space, and is not warranted for
  // unconverged MPP's (AMV variants).  In addition, AMV variants only compute
  // verification function values at u* (no Hessians).  For an AMV-like
  // approach with 2nd-order integration, use AMV+ with max_iterations = 1.
  const String& integration_method
    = probDescDB.get_string("method.nond.reliability_integration");
  if (integration_method.empty() || integration_method == "first_order")
    integrationOrder = 1;
  else if (integration_method == "second_order") {
    if (hess_type == "none") {
      Cerr << "\nError: second-order integration requires Hessian "
	   << "specification." << std::endl;
      abort_handler(-1);
    }
    else if (mppSearchType <= AMV_U) {
      Cerr << "\nError: second-order integration only supported for fully "
	   << "converged MPP methods." << std::endl;
      abort_handler(-1);
    }
    else
      integrationOrder = 2;
  }
  else {
    Cerr << "Error: bad integration selection in NonDLocalReliability."
	 << std::endl;
    abort_handler(-1);
  }

  // The model of the limit state in u-space (uSpaceModel) is constructed here
  // one time.  The RecastModel for the RIA/PMA formulations varies with the
  // level requests and is constructed for each level within mpp_search().

  // Instantiate the Nataf Recast and any DataFit model recursions.  Recast is
  // bounded to 10 std devs in u space.  This is particularly important for PMA
  // since an SQP-based optimizer will not enforce the constraint immediately
  // and min +/-g has been observed to have significant excursions early on
  // prior to the u'u = beta^2 constraint enforcement bringing it back.  A
  // large excursion can cause overflow; a medium excursion can cause poor 
  // performance since far-field info is introduced into the BFGS Hessian.
  short recast_resp_order = 3; // grad-based quasi-Newton opt on mppModel
  if (mppSearchType ==  AMV_X || mppSearchType == AMV_PLUS_X ||
      mppSearchType == TANA_X) { // Recast( DataFit( iteratedModel ) )

    // Construct g-hat(x) using a local/multipoint approximation over the
    // uncertain variables (using the same view as iteratedModel).
    Model g_hat_x_model;
    String sample_reuse, approx_type = (mppSearchType == TANA_X) ?
      "multipoint_tana" : "local_taylor";
    UShortArray approx_order(1, taylorOrder);
    short corr_order = -1, corr_type = NO_CORRECTION,
      data_order = (taylorOrder == 2) ? 7 : 3;
    int samples = 0, seed = 0;
    Iterator dace_iterator;
    //const Variables& curr_vars = iteratedModel.current_variables();
    ActiveSet surr_set = iteratedModel.current_response().active_set(); // copy
    surr_set.request_values(3); // surrogate gradient evals
    g_hat_x_model.assign_rep(new DataFitSurrModel(dace_iterator, iteratedModel,
      surr_set, approx_type, approx_order, corr_type, corr_order, data_order,
      outputLevel, sample_reuse), false);

    // transform g_hat_x_model from x-space to u-space
    transform_model(g_hat_x_model, uSpaceModel, true); // truncated dist bounds
  }
  else if (mppSearchType ==  AMV_U || mppSearchType == AMV_PLUS_U ||
	   mppSearchType == TANA_U) { // DataFit( Recast( iteratedModel ) )

    // Recast g(x) to G(u)
    Model g_u_model;
    transform_model(iteratedModel, g_u_model, true); // truncated dist bounds

    // Construct G-hat(u) using a local/multipoint approximation over the
    // uncertain variables (using the same view as iteratedModel/g_u_model).
    String sample_reuse, approx_type = (mppSearchType == TANA_U) ?
      "multipoint_tana" : "local_taylor";
    UShortArray approx_order(1, taylorOrder);
    short corr_order = -1, corr_type = NO_CORRECTION,
      data_order = (taylorOrder == 2) ? 7 : 3;
    int samples = 0, seed = 0;
    Iterator dace_iterator;
    //const Variables& g_u_vars = g_u_model.current_variables();
    ActiveSet surr_set = g_u_model.current_response().active_set(); // copy
    surr_set.request_values(3); // surrogate gradient evals
    uSpaceModel.assign_rep(new DataFitSurrModel(dace_iterator, g_u_model,
      surr_set, approx_type, approx_order, corr_type, corr_order, data_order,
      outputLevel, sample_reuse), false);
  }
  else if (mppSearchType == NO_APPROX) { // Recast( iteratedModel )
    // Recast g(x) to G(u)
    transform_model(iteratedModel, uSpaceModel, true); // truncated dist bounds
    // detect PMA2 condition and augment mppModel data requirements
    bool pma2_flag = false;
    if (integrationOrder == 2)
      for (size_t i=0; i<numFunctions; ++i)
	if (!requestedProbLevels[i].empty() ||
	    !requestedGenRelLevels[i].empty())
	  { pma2_flag = true; break; }
    if (pma2_flag) // mirrors PMA2_set_mapping()
      recast_resp_order |= 4;
  }

  // configure a RecastModel with one objective and one equality constraint
  // using the alternate minimalist constructor
  if (mppSearchType) {
    SizetArray recast_vars_comps_total;  // default: empty; no change in size
    BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relax
    mppModel.assign_rep(
      new RecastModel(uSpaceModel, recast_vars_comps_total, all_relax_di,
		      all_relax_dr, 1, 1, 0, recast_resp_order), false);
    RealVector nln_eq_targets(1, false); nln_eq_targets = 0.;
    mppModel.nonlinear_eq_constraint_targets(nln_eq_targets);

    // Use NPSOL/OPT++ in "user_functions" mode to perform the MPP search
    if (npsolFlag) {
      // NPSOL deriv level: 1 = supplied grads of objective fn, 2 = supplied
      // grads of constraints, 3 = supplied grads of both.  Always use the
      // supplied grads of u'u (deriv level = 1 for RIA, deriv level = 2 for
      // PMA).  In addition, use supplied gradients of G(u) in most cases.
      // Exception: deriv level = 3 results in a gradient-based line search,
      // which could be too expensive for FORM with numerical grads unless
      // seeking parallel load balance.
      //int npsol_deriv_level;
      //if ( mppSearchType == NO_APPROX && !iteratedModel.asynch_flag()
      //     && iteratedModel.gradient_type() != "analytic" )
      //  npsol_deriv_level = (ria_flag) ? 1 : 2;
      //else
      //  npsol_deriv_level = 3;
      //Cout << "Derivative level = " << npsol_deriv_level << '\n';

      // The gradient-based line search (deriv. level = 3) appears to be
      // outperforming the value-based line search in PMA testing.  In
      // addition, the RIA warm start needs fnGradU so deriv. level = 3 has
      // superior performance there as well.  Therefore, deriv level = 3 can
      // be used for all cases.
      int npsol_deriv_level = 3;

      // run a tighter tolerance on approximation-based MPP searches
      //Real conv_tol = (mppSearchType == NO_APPROX) ? 1.e-4 : 1.e-6;
      Real conv_tol = -1.; // use NPSOL default

#ifdef HAVE_NPSOL
      mppOptimizer.assign_rep(new NPSOLOptimizer(mppModel, npsol_deriv_level,
	conv_tol), false);
#endif
    }
#ifdef HAVE_OPTPP
    else
      mppOptimizer.assign_rep(new SNLLOptimizer("optpp_q_newton", mppModel),
	false);
#endif
  }

  // integration refinement is only active for RIA mappings --> prevent
  // importanceSampler definition and PDF estimation if no RIA.
  if (integrationRefinement) {
    bool no_ria_override = true;
    for (size_t i=0; i<numFunctions; ++i)
      if (!requestedRespLevels[i].empty())
	{ no_ria_override = false; break; }
    if (no_ria_override) {
      Cerr << "\nWarning: probability_refinement specification is ignored in "
	   << "the absence of response_level mappings.\n";
      integrationRefinement = NO_INT_REFINE;
    }
  }

  if (integrationRefinement) {
    // integration refinement requires an MPP, but it may be unconverged (AMV)
    if (!mppSearchType) {
      Cerr << "\nError: integration refinement only supported for MPP methods."
	   << std::endl;
      abort_handler(-1);
    }

    // For NonDLocal, integration refinement is applied to the original model
    int refine_samples = 1000; // context-specific default
    const IntVector& db_refine_samples = 
      probDescDB.get_iv("method.nond.refinement_samples");
    if (db_refine_samples.length() == 1)
      refine_samples = db_refine_samples[0];
    else if (db_refine_samples.length() > 1) {
      Cerr << "\nError (NonDLocalReliability): refinement_samples must be "
           << "length 1 if specified." << std::endl;
      abort_handler(PARSE_ERROR);
    }
    int refine_seed    = probDescDB.get_int("method.random_seed");
   
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    String rng; // empty string: use default

    // Note: truncated distribution bounds in transform_model() can be true
    // (to bound an optimizer search) with AIS use_model_bounds = false
    // (AIS will ignore these global bounds); extreme values are needed
    // to define bounds for outer PDF bins.
    bool x_model_flag = false, use_model_bounds = false, vary_pattern = true,
      track_extreme = pdfOutput;

    // AIS is performed in u-space WITHOUT a surrogate: pass a truth u-space
    // model when available, construct one when not.
    NonDAdaptImpSampling* import_sampler_rep = NULL;
    switch (mppSearchType) {
    case AMV_X: case AMV_PLUS_X: case TANA_X: {
      Model g_u_model;
      transform_model(iteratedModel, g_u_model); // original dist bounds
      import_sampler_rep = new NonDAdaptImpSampling(g_u_model, sample_type,
	refine_samples, refine_seed, rng, vary_pattern, integrationRefinement,
	cdfFlag, x_model_flag, use_model_bounds, track_extreme);
      break;
    }
    case AMV_U: case AMV_PLUS_U: case TANA_U:
      import_sampler_rep = new NonDAdaptImpSampling(uSpaceModel.truth_model(),
	sample_type, refine_samples, refine_seed, rng, vary_pattern,
	integrationRefinement, cdfFlag, x_model_flag, use_model_bounds,
	track_extreme);
      break;
    case NO_APPROX:
      import_sampler_rep = new NonDAdaptImpSampling(uSpaceModel, sample_type,
	refine_samples, refine_seed, rng, vary_pattern, integrationRefinement,
	cdfFlag, x_model_flag, use_model_bounds, track_extreme);
      break;
    }
    importanceSampler.assign_rep(import_sampler_rep, false);
    // set up the x-space data within the importance sampler
    import_sampler_rep->initialize_random_variables(natafTransform);
  }

  // Size the output arrays, augmenting sizing in NonDReliability.  Relative to
  // other NonD methods, the output storage for reliability methods is greater
  // since there may be differences between requested and computed levels for
  // the same level type (the request is not always achieved) and since
  // probability and reliability are carried along in parallel (due to their
  // direct correspondence).
  computedRelLevels.resize(numFunctions); // others sized in NonDReliability
  for (size_t i=0; i<numFunctions; i++) {
    size_t num_levels = requestedRespLevels[i].length() + 
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();
    computedRespLevels[i].resize(num_levels);
    computedProbLevels[i].resize(num_levels);
    computedRelLevels[i].resize(num_levels);
    computedGenRelLevels[i].resize(num_levels);
  }

  // Size class-scope arrays.
  mostProbPointX.sizeUninitialized(numUncertainVars);
  mostProbPointU.sizeUninitialized(numUncertainVars);
  fnGradX.sizeUninitialized(numUncertainVars);
  fnGradU.sizeUninitialized(numUncertainVars);
  if (taylorOrder == 2 || integrationOrder == 2) {
    fnHessX.shapeUninitialized(numUncertainVars);
    fnHessU.shapeUninitialized(numUncertainVars);
    if (hess_type == "quasi") {
      // Note: fnHess=0 in both spaces is not self-consistent for nonlinear
      // transformations.  However, the point is to use a first-order
      // approximation in either space prior to curvature accumulation.
      fnHessX = 0.;
      fnHessU = 0.;
    }
    kappaU.sizeUninitialized(numUncertainVars-1);
  }
}


NonDLocalReliability::~NonDLocalReliability()
{ }


void NonDLocalReliability::derived_init_communicators(ParLevLIter pl_iter)
{
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);
  if (mppSearchType) {
    // uSpaceModel, mppOptimizer, and importanceSampler use NoDBBaseConstructor,
    // so no need to manage DB list nodes at this level

    // maxEvalConcurrency defined from the derivative concurrency in the
    // responses specification.  For FORM/SORM, the NPSOL/OPT++ concurrency
    // is the same, but for approximate methods, the concurrency is dictated
    // by the gradType/hessType logic in the instantiate on-the-fly
    // DataFitSurrModel constructor.
    uSpaceModel.init_communicators(pl_iter, maxEvalConcurrency);
    // TO DO: distinguish gradient concurrency for truth vs. surrogate?
    //        (probably doesn't matter for surrogate)

    mppOptimizer.init_communicators(pl_iter);

    if (integrationRefinement)
      importanceSampler.init_communicators(pl_iter);
  }
}


void NonDLocalReliability::derived_set_communicators(ParLevLIter pl_iter)
{
  NonD::derived_set_communicators(pl_iter);

  if (mppSearchType) {
    uSpaceModel.set_communicators(pl_iter, maxEvalConcurrency);
    mppOptimizer.set_communicators(pl_iter);
    if (integrationRefinement)
      importanceSampler.set_communicators(pl_iter);
  }
}


void NonDLocalReliability::derived_free_communicators(ParLevLIter pl_iter)
{
  if (mppSearchType) {
    if (integrationRefinement)
      importanceSampler.free_communicators(pl_iter);
    mppOptimizer.free_communicators(pl_iter);
    uSpaceModel.free_communicators(pl_iter, maxEvalConcurrency);
  }
  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


void NonDLocalReliability::core_run()
{
  if (mppSearchType) mpp_search();
  else               mean_value();

  // post-process level mappings to define PDFs (using prob_refined and
  // all_levels_computed modes)
  if (pdfOutput && integrationRefinement) {
    NonDAdaptImpSampling* import_sampler_rep
      = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
    compute_densities(import_sampler_rep->extreme_values(), true, true);
  } // else no extreme values to define outer PDF bins

  numRelAnalyses++;
}


void NonDLocalReliability::mean_value()
{
  // For MV, compute approximate mean, standard deviation, and requested
  // CDF/CCDF data points for each response function and store in 
  // finalStatistics.  Additionally, if uncorrelated variables, compute
  // importance factors.

  initialize_random_variable_parameters();
  initial_taylor_series();

  // initialize arrays
  bool correlation_flag = natafTransform.x_correlation();
  size_t num_imp_fact = (correlation_flag) ?
    (numUncertainVars * (numUncertainVars+1)) / 2 : numUncertainVars;
  impFactor.shapeUninitialized(num_imp_fact, numFunctions);
  statCount = 0;
  initialize_final_statistics_gradients();

  // local reliability data aren't output to tabular, so send directly
  // to graphics window only
  Graphics& dakota_graphics = parallelLib.output_manager().graphics();

  // loop over response functions
  size_t i, j, cntr, moment_offset = (finalMomentsType) ? 2 : 0;
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  Real mean, mom2, std_dev, var, terms_i;
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           bl_len = requestedRelLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length(),
        total_lev = rl_len + pl_len + bl_len + gl_len;

    mean = finalMomentStats(0,respFnCount);
    mom2 = finalMomentStats(1,respFnCount);
    if (finalMomentsType == CENTRAL_MOMENTS)
      { var = mom2; std_dev = std::sqrt(mom2); }
    else
      { std_dev = mom2; var = mom2 * mom2; }

    RealVector dg_ds_meanx;
    bool need_dg_ds = (finalMomentsType && (final_asv[statCount] & 2)); // mean
    if (!need_dg_ds)
      for (i=0, j=statCount+moment_offset; i<total_lev; ++i, ++j)// all lev map
	if (final_asv[j] & 2)
	  { need_dg_ds = true; break; }
    if (need_dg_ds) {
      RealVector fn_grad_mean_x(Teuchos::View, fnGradsMeanX[respFnCount],
				numUncertainVars);
      // evaluate dg/ds at the variable means and store in finalStatistics
      dg_ds_eval(ranVarMeansX, fn_grad_mean_x, dg_ds_meanx);
    }

    if (finalMomentsType) {
      // approximate response mean already computed
      finalStatistics.function_value(mean, statCount);
      // sensitivity of response mean
      if (final_asv[statCount] & 2)
	finalStatistics.function_gradient(dg_ds_meanx, statCount);
      ++statCount;

      // approximate response std deviation or variance already computed
      finalStatistics.function_value(mom2, statCount);
      // sensitivity of response std deviation
      if (final_asv[statCount] & 2) {
	// Differentiating the first-order second-moment expression leads to
	// 2nd-order d^2g/dxds sensitivities which are:
	// > awkward to compute (nonstandard DVV w/ active + inactive vars)
	// > a higher-order term that we will neglect for simplicity
	Cerr << "Warning: response std deviation sensitivity is zero for "
	     << "MVFOSM." << std::endl;
	RealVector final_stat_grad(numUncertainVars); // init to 0.
	finalStatistics.function_gradient(final_stat_grad, statCount);
      }
      ++statCount;
    }

    // If response std_dev is non-zero, compute importance factors.  Traditional
    // impFactors correspond to the diagonal terms in the computation of the
    // response variance, normalized by the total response variance.  If inputs
    // are correlated, then off-diagonal contributions to the output variance
    // are present and "two-way importance factors" are appended to the end of
    // the impFactors matrix.  These two-way factors are not the same as Sobol'
    // interaction indices since the Taylor series basis is not orthogonal; as
    // a result, two-way factors (unlike univariate factors) can be negative.
    if (std_dev > Pecos::SMALL_NUMBER) {
      Real* imp_fact = impFactor[respFnCount];
      Real* fn_grad  = fnGradsMeanX[respFnCount];
      if (correlation_flag) {
	const Pecos::RealSymMatrix& x_corr_mat
	  = natafTransform.x_correlation_matrix();
	for (i=0, cntr=numUncertainVars; i<numUncertainVars; ++i) {
	  imp_fact[i] = std::pow(ranVarStdDevsX[i] / std_dev * fn_grad[i], 2);
	  terms_i = 2. * ranVarStdDevsX[i] * fn_grad[i] / var;
	  for (j=0; j<i; ++j, ++cntr) // collect both off diagonal terms
	    imp_fact[cntr]
	      = terms_i * ranVarStdDevsX[j] * x_corr_mat(i,j) * fn_grad[j];
	}
      }
      else
	for (i=0; i<numUncertainVars; ++i)
	  imp_fact[i] = std::pow(ranVarStdDevsX[i] / std_dev * fn_grad[i], 2);
    }

    // compute probability/reliability levels for requested response levels and
    // compute response levels for requested probability/reliability levels.
    // For failure defined as g<0, beta is simply mean/sigma.  This is extended
    // to compute general cumulative probabilities for g<z or general
    // complementary cumulative probabilities for g>z.
    for (levelCount=0; levelCount<rl_len; levelCount++) {
      // computed = requested in MV case since no validation fn evals
      Real z = computedRespLevels[respFnCount][levelCount]
	= requestedRespLevels[respFnCount][levelCount];
      // compute beta and p from z
      Real beta, p;
      if (std_dev > Pecos::SMALL_NUMBER) {
	Real ratio = (mean - z)/std_dev;
        beta = computedRelLevels[respFnCount][levelCount]
	  = computedGenRelLevels[respFnCount][levelCount]
	  = (cdfFlag) ? ratio : -ratio;
        p = computedProbLevels[respFnCount][levelCount] = probability(beta);
      }
      else {
        if ( ( cdfFlag && mean <= z) ||
	     (!cdfFlag && mean >  z) ) {
          beta = computedRelLevels[respFnCount][levelCount]
	    = computedGenRelLevels[respFnCount][levelCount]
	    = -Pecos::LARGE_NUMBER;
          p = computedProbLevels[respFnCount][levelCount] = 1.;
	}
	else {
          beta = computedRelLevels[respFnCount][levelCount]
	    = computedGenRelLevels[respFnCount][levelCount]
	    = Pecos::LARGE_NUMBER;
          p = computedProbLevels[respFnCount][levelCount] = 0.;
	}
      }
      switch (respLevelTarget) {
      case PROBABILITIES:
	finalStatistics.function_value(p, statCount);
	if (final_asv[statCount] & 2) {
	  // dsigma/ds contains H.O.T. and taken to be 0.
	  // beta_cdf  = (mu - z-bar) / sigma -> dbeta/ds =  1/sigma dmu/ds
	  // beta_ccdf = (z-bar - mu) / sigma -> dbeta/ds = -1/sigma dmu/ds
	  // dp_cdf/ds  = -phi(-beta) dbeta/ds = -phi(-beta)/sigma dmu/ds
	  // dp_ccdf/ds = -phi(-beta) dbeta/ds =  phi(-beta)/sigma dmu/ds
	  RealVector final_stat_grad(dg_ds_meanx); // deep copy
	  Real phi_m_beta = Pecos::NormalRandomVariable::std_pdf(-beta);
	  if (cdfFlag) final_stat_grad.scale(-phi_m_beta/std_dev);
	  else         final_stat_grad.scale( phi_m_beta/std_dev);
	  finalStatistics.function_gradient(final_stat_grad, statCount);
	}
	break;
      case RELIABILITIES: case GEN_RELIABILITIES:
	finalStatistics.function_value(beta, statCount);
	if (final_asv[statCount] & 2) {
	  // dsigma/ds contains H.O.T. and taken to be 0.
	  // beta_cdf  = (mu - z-bar) / sigma -> dbeta/ds =  1/sigma dmu/ds
	  // beta_ccdf = (z-bar - mu) / sigma -> dbeta/ds = -1/sigma dmu/ds
	  RealVector final_stat_grad(dg_ds_meanx); // deep copy
	  if (cdfFlag) final_stat_grad.scale( 1./std_dev);
	  else         final_stat_grad.scale(-1./std_dev);
	  finalStatistics.function_gradient(final_stat_grad, statCount);
	}
	break;
      }
      ++statCount;
      // Update specialty graphics
      if (!subIteratorFlag)
	dakota_graphics.add_datapoint(respFnCount, z, p);
    }
    for (i=0; i<pl_len; i++) {
      levelCount = i+rl_len;
      // computed = requested in MV case since no validation fn evals
      Real p = computedProbLevels[respFnCount][levelCount]
	= requestedProbLevels[respFnCount][i];
      // compute beta and z from p
      Real beta = computedRelLevels[respFnCount][levelCount]
	= computedGenRelLevels[respFnCount][levelCount]	= reliability(p);
      Real z = computedRespLevels[respFnCount][levelCount] = (cdfFlag)
        ? mean - beta * std_dev : mean + beta * std_dev;
      finalStatistics.function_value(z, statCount);
      if (final_asv[statCount] & 2)
	finalStatistics.function_gradient(dg_ds_meanx, statCount);
      ++statCount;
      // Update specialty graphics
      if (!subIteratorFlag)
	dakota_graphics.add_datapoint(respFnCount, z, p);
    }
    for (i=0; i<bl_len+gl_len; i++) {
      levelCount = i+rl_len+pl_len;
      // computed = requested in MV case since no validation fn evals
      Real beta = (i<bl_len) ? requestedRelLevels[respFnCount][i] :
	requestedGenRelLevels[respFnCount][i-bl_len];
      computedRelLevels[respFnCount][levelCount]
	= computedGenRelLevels[respFnCount][levelCount] = beta;
      // compute p and z from beta
      Real p = computedProbLevels[respFnCount][levelCount] = probability(beta);
      Real z = computedRespLevels[respFnCount][levelCount] = (cdfFlag)
        ? mean - beta * std_dev	: mean + beta * std_dev;
      finalStatistics.function_value(z, statCount);
      if (final_asv[statCount] & 2)
	finalStatistics.function_gradient(dg_ds_meanx, statCount);
       ++statCount;
      // Update specialty graphics
      if (!subIteratorFlag)
	dakota_graphics.add_datapoint(respFnCount, z, p);
    }
  }
}


void NonDLocalReliability::mpp_search()
{
  // set the object instance pointer for use within the static member fns
  NonDLocalReliability* prev_instance = nondLocRelInstance;
  nondLocRelInstance = this;

  // The following 2 calls must precede use of natafTransform.trans_X_to_U()
  initialize_random_variable_parameters();
  // Modify the correlation matrix (Nataf) and compute its Cholesky factor.
  // Since the uncertain variable distributions (means, std devs, correlations)
  // may change among NonDLocalReliability invocations (e.g., RBDO with design
  // variable insertion), this code block is performed on every invocation.
  transform_correlations();

  // initialize initialPtUSpec on first reliability analysis; needs to precede
  // iteratedModel.continuous_variables() assignment in initial_taylor_series()
  if (numRelAnalyses == 0) {
    if (initialPtUserSpec)
      natafTransform.trans_X_to_U(iteratedModel.continuous_variables(),
				  initialPtUSpec);
    else {
      // don't use the mean uncertain variable defaults from the parser
      // since u ~= 0 can cause problems for some formulations
      initialPtUSpec.sizeUninitialized(numUncertainVars);
      initialPtUSpec = 1.;
    }
  }

  // sets iteratedModel.continuous_variables() to mean values
  initial_taylor_series();

  // Initialize local arrays
  statCount = 0;
  initialize_final_statistics_gradients();

  // Initialize class scope arrays, modify the correlation matrix, and
  // evaluate median responses
  initialize_class_data();

  // Loop over each response function in the responses specification.  It is
  // important to note that the MPP iteration is different for each response 
  // function, and it is not possible to combine the model evaluations for
  // multiple response functions.
  size_t i;
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  for (respFnCount=0; respFnCount<numFunctions; ++respFnCount) {

    if (finalMomentsType) {
      // approximate response mean already computed
      finalStatistics.function_value(finalMomentStats(0,respFnCount),statCount);
      // sensitivity of response mean
      if (final_asv[statCount] & 2) {
	RealVector fn_grad_mean_x(numUncertainVars, false);
	for (i=0; i<numUncertainVars; i++)
	  fn_grad_mean_x[i] = fnGradsMeanX(i,respFnCount);
	// evaluate dg/ds at the variable means and store in finalStatistics
	RealVector final_stat_grad;
	dg_ds_eval(ranVarMeansX, fn_grad_mean_x, final_stat_grad);
	finalStatistics.function_gradient(final_stat_grad, statCount);
      }
      ++statCount;

      // approximate response std deviation or variance already computed
      finalStatistics.function_value(finalMomentStats(1,respFnCount),statCount);
      // sensitivity of response std deviation
      if (final_asv[statCount] & 2) {
	// Differentiating the first-order second-moment expression leads to
	// 2nd-order d^2g/dxds sensitivities which would be awkward to compute
	// (nonstandard DVV containing active and inactive vars)
	Cerr << "Error: response std deviation sensitivity not yet supported."
	     << std::endl;
	abort_handler(-1);
	// TO DO: back out from RIA/PMA equations (use closest level to mean?):
	// RIA: dsigma/ds = (dmean/ds - sigma dbeta_cdf/ds) / beta_cdf
	// PMA: dsigma/ds = (dmean/ds - dz/ds) / beta_cdf
      }
      ++statCount;
    }

    // The most general case is to allow a combination of response, probability,
    // reliability, and generalized reliability level specifications for each
    // response function.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           bl_len = requestedRelLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length(),
           index, num_levels = rl_len + pl_len + bl_len + gl_len;

    // Initialize (or warm-start for repeated reliability analyses) initialPtU,
    // mostProbPointX/U, computedRespLevel, fnGradX/U, and fnHessX/U.
    curvatureDataAvailable = false; // no data (yet) for this response function
    if (num_levels)
      initialize_level_data();

    // Loop over response/probability/reliability levels
    for (levelCount=0; levelCount<num_levels; ++levelCount) {

      // The rl_len response levels are performed first using the RIA
      // formulation, followed by the pl_len probability levels and the
      // bl_len reliability levels using the PMA formulation.
      bool ria_flag = (levelCount < rl_len),
	pma2_flag = ( integrationOrder == 2 && ( levelCount < rl_len + pl_len ||
		      levelCount >= rl_len + pl_len + bl_len ) );
      if (ria_flag) {
        requestedTargetLevel = requestedRespLevels[respFnCount][levelCount];
	Cout << "\n>>>>> Reliability Index Approach (RIA) for response level "
	     << levelCount+1 << " = " << requestedTargetLevel << '\n';
      }
      else if (levelCount < rl_len + pl_len) { 
	index  = levelCount - rl_len;
	Real p = requestedProbLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for probability "
	     << "level " << index + 1 << " = " << p << '\n';
	// gen beta target for 2nd-order PMA; beta target for 1st-order PMA:
	requestedTargetLevel = reliability(p);

	// CDF probability < 0.5  -->  CDF beta > 0  -->  minimize g
	// CDF probability > 0.5  -->  CDF beta < 0  -->  maximize g
	// CDF probability = 0.5  -->  CDF beta = 0  -->  compute g
	// Note: "compute g" means that min/max is irrelevant since there is
	// a single G(u) value when the radius beta collapses to the origin
	Real p_cdf   = (cdfFlag) ? p : 1. - p;
	pmaMaximizeG = (p_cdf > 0.5); // updated in update_pma_maximize()
      }
      else if (levelCount < rl_len + pl_len + bl_len) {
	index = levelCount - rl_len - pl_len;
	requestedTargetLevel = requestedRelLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for reliability "
	     << "level " << index + 1 << " = " << requestedTargetLevel << '\n';
	Real beta_cdf = (cdfFlag) ?
	  requestedTargetLevel : -requestedTargetLevel;
	pmaMaximizeG = (beta_cdf < 0.);
      }
      else {
	index = levelCount - rl_len - pl_len - bl_len;
	requestedTargetLevel = requestedGenRelLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for generalized "
	     << "reliability level " << index + 1 << " = "
	     << requestedTargetLevel << '\n';
	Real gen_beta_cdf = (cdfFlag) ?
	  requestedTargetLevel : -requestedTargetLevel;
	pmaMaximizeG = (gen_beta_cdf < 0.); // updated in update_pma_maximize()
      }

      // Assign cold/warm-start values for initialPtU, mostProbPointX/U,
      // computedRespLevel, fnGradX/U, and fnHessX/U.
      if (levelCount)
	initialize_mpp_search_data();

#ifdef DERIV_DEBUG
      // numerical verification of analytic Jacobian/Hessian routines
      if (mppSearchType == NO_APPROX && levelCount == 0)
        mostProbPointU = ranVarMeansU;//mostProbPointX = ranVarMeansX;
      //natafTransform.verify_trans_jacobian_hessian(mostProbPointU);
      //natafTransform.verify_trans_jacobian_hessian(mostProbPointX);
      natafTransform.verify_design_jacobian(mostProbPointU);
#endif // DERIV_DEBUG

      // For AMV+/TANA approximations, iterate until current expansion point
      // converges to the MPP.
      approxIters = 0;
      approxConverged = false;
      while (!approxConverged) {

	Sizet2DArray vars_map, primary_resp_map, secondary_resp_map;
	BoolDequeArray nonlinear_resp_map(2);
	RecastModel* mpp_model_rep = (RecastModel*)mppModel.model_rep();
	if (ria_flag) { // RIA: g is in constraint
	  primary_resp_map.resize(1);   // one objective, no contributors
	  secondary_resp_map.resize(1); // one constraint, one contributor
	  secondary_resp_map[0].resize(1);
	  secondary_resp_map[0][0] = respFnCount;
	  nonlinear_resp_map[1] = BoolDeque(1, false);
	  mpp_model_rep->init_maps(vars_map, false, NULL, NULL,
	    primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	    RIA_objective_eval, RIA_constraint_eval);
	}
	else { // PMA: g is in objective
	  primary_resp_map.resize(1);   // one objective, one contributor
	  primary_resp_map[0].resize(1);
	  primary_resp_map[0][0] = respFnCount;
	  secondary_resp_map.resize(1); // one constraint, no contributors
	  nonlinear_resp_map[0] = BoolDeque(1, false);
	  // If 2nd-order PMA with p-level or generalized beta-level, use
	  // PMA2_set_mapping() & PMA2_constraint_eval().  For approx-based
	  // 2nd-order PMA, we utilize curvature of the surrogate (if any)
	  // to update beta* 
	  if (pma2_flag)
	    mpp_model_rep->init_maps(vars_map, false, NULL, PMA2_set_mapping,
	      primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	      PMA_objective_eval, PMA2_constraint_eval);
	  else
	    mpp_model_rep->init_maps(vars_map, false, NULL, NULL,
	      primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	      PMA_objective_eval, PMA_constraint_eval);	    
	}
	mppModel.continuous_variables(initialPtU);

        // Execute MPP search and retrieve u-space results
        Cout << "\n>>>>> Initiating search for most probable point (MPP)\n";
	ParLevLIter pl_iter
	  = methodPCIter->mi_parallel_level_iterator(miPLIndex);
	mppOptimizer.run(pl_iter);
        const Variables& vars_star = mppOptimizer.variables_results();
        const Response&  resp_star = mppOptimizer.response_results();
	const RealVector& fns_star = resp_star.function_values();
        Cout << "\nResults of MPP optimization:\nInitial point (u-space) =\n"
             << initialPtU << "Final point (u-space)   =\n";
	write_data(Cout, vars_star.continuous_variables());
	if (ria_flag)
	  Cout << "RIA optimum             =\n                     "
	       << std::setw(write_precision+7) << fns_star[0] << " [u'u]\n"
	       << "                     " << std::setw(write_precision+7)
	       << fns_star[1] << " [G(u) - z]\n";
	else {
	  Cout << "PMA optimum             =\n                     "
	       << std::setw(write_precision+7) << fns_star[0] << " [";
	  if (pmaMaximizeG) Cout << '-';
	  Cout << "G(u)]\n                     " << std::setw(write_precision+7)
	       << fns_star[1];
	  if (pma2_flag) Cout << " [B* - bar-B*]\n";
	  else           Cout << " [u'u - B^2]\n";
	}

	// Update MPP search data
	update_mpp_search_data(vars_star, resp_star);

      } // end AMV+ while loop

      // Update response/probability/reliability level data
      update_level_data();

      ++statCount;
    } // end loop over levels
  } // end loop over response fns

  // Update warm-start data
  if (warmStartFlag && subIteratorFlag) // view->copy
    copy_data(iteratedModel.inactive_continuous_variables(), prevICVars);

  // This function manages either component or system reliability metrics
  // via post-processing of computed{Resp,Prob,Rel,GenRel}Levels
  update_final_statistics();

  // restore in case of recursion
  nondLocRelInstance = prev_instance;
}


/** An initial first- or second-order Taylor-series approximation is
    required for MV/AMV/AMV+/TANA or for the case where finalMomentStats
    (from MV) are required within finalStatistics for subIterator usage
    of NonDLocalReliability. */
void NonDLocalReliability::initial_taylor_series()
{
  bool init_ts_flag = (mppSearchType < NO_APPROX); // updated below
  const String& hess_type = iteratedModel.hessian_type();
  size_t i, j, k;
  ShortArray asrv(numFunctions, 0);
  short mode = 3;
  if (taylorOrder == 2 && hess_type != "quasi") // no data yet in quasiHess
    mode |= 4;

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  switch (mppSearchType) {
  case MV:
    asrv.assign(numFunctions, mode);
    break;
  case AMV_X:      case AMV_U:
  case AMV_PLUS_X: case AMV_PLUS_U:
  case TANA_X:     case TANA_U:
    for (i=0; i<numFunctions; ++i)
      if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	  !requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty() )
	asrv[i] = mode;
    // no break: fall through
  case NO_APPROX:
    if (subIteratorFlag && finalMomentsType) {
      // check final_asv for active mean and std deviation stats
      size_t cntr = 0;
      for (i=0; i<numFunctions; i++) {
	for (j=0; j<2; j++) {
	  if (final_asv[cntr++]) { // mean, std deviation
	    asrv[i] = mode;
	    init_ts_flag = true;
	  }
	}
	cntr += requestedRespLevels[i].length() +
	  requestedProbLevels[i].length() + requestedRelLevels[i].length() +
	  requestedGenRelLevels[i].length();
      }
    }
    break;
  }

  ranVarMeansX   = natafTransform.x_means();
  ranVarStdDevsX = natafTransform.x_std_deviations();

  finalMomentStats.shape(2, numFunctions); // init to 0
  if (init_ts_flag) {
    bool correlation_flag = natafTransform.x_correlation();
    // Evaluate response values/gradients at the mean values of the uncertain
    // vars for the (initial) Taylor series expansion in MV/AMV/AMV+.
    Cout << "\n>>>>> Evaluating response at mean values\n";
    if (mppSearchType && mppSearchType < NO_APPROX)
      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    iteratedModel.continuous_variables(ranVarMeansX);
    activeSet.request_vector(asrv);
    iteratedModel.evaluate(activeSet);
    const Response& curr_resp = iteratedModel.current_response();
    fnValsMeanX       = curr_resp.function_values();
    fnGradsMeanX      = curr_resp.function_gradients();
    if (mode & 4)
      fnHessiansMeanX = curr_resp.function_hessians();

    /*
    // compute the covariance matrix from the correlation matrix
    RealSymMatrix covariance;
    if (correlation_flag) {
      covariance.shapeUninitialized(numUncertainVars);
      const Pecos::RealSymMatrix& x_corr_mat
	= natafTransform.x_correlation_matrix();
      for (i=0; i<numUncertainVars; i++) {
	for (j=0; j<=i; j++) {
	  covariance(i,j) = ranVarStdDevsX[i]*ranVarStdDevsX[j]*x_corr_mat(i,j);
	  //if (i != j)
	  //  covariance(j,i) = covariance(i,j);
	}
      }
    }
    else {
      covariance.shape(numUncertainVars); // inits to 0
      for (i=0; i<numUncertainVars; i++)
	covariance(i,i) = std::pow(ranVarStdDevsX[i], 2);
    }
    */

    // MVFOSM computes a first-order mean, which is just the response evaluated
    // at the input variable means.  If Hessian data is available, compute a
    // second-order mean including the effect of input variable correlations.
    // MVFOSM computes a first-order variance including the effect of input
    // variable correlations.  Second-order variance requires skewness/kurtosis
    // of the inputs and is not practical.  NOTE: if fnGradsMeanX is zero, then
    // std_dev will be zero --> bad for MV CDF estimates.
    bool t2nq = (taylorOrder == 2 && hess_type != "quasi"); // 2nd-order mean
    const Pecos::RealSymMatrix& x_corr_mat
      = natafTransform.x_correlation_matrix();
    for (i=0; i<numFunctions; ++i) {
      if (asrv[i]) {
	Real &mean = finalMomentStats(0,i), &mom2 = finalMomentStats(1,i);
	mean = fnValsMeanX[i]; // first-order mean
	Real v1 = 0., v2 = 0.;
	for (j=0; j<numUncertainVars; ++j) {
	  Real fn_grad_ji = fnGradsMeanX(j,i);
	  if (correlation_flag)
	    for (k=0; k<numUncertainVars; ++k) {
	      Real cov_jk = ranVarStdDevsX[j] * ranVarStdDevsX[k] 
		          * x_corr_mat(j,k);//covariance(j,k);
	      if (t2nq) v1 += cov_jk * fnHessiansMeanX[i](j,k);
	      v2 += cov_jk * fn_grad_ji * fnGradsMeanX(k,i);
	    }
	  else {
	    Real cov_jj = ranVarStdDevsX[j]*ranVarStdDevsX[j];//covariance(j,j);
	    if (t2nq) v1 += cov_jj * fnHessiansMeanX[i](j,j);
	    v2 += cov_jj * std::pow(fn_grad_ji, 2);
	  }
	}
	if (t2nq) mean += v1/2.;
	mom2 = (finalMomentsType == CENTRAL_MOMENTS) ? v2 : std::sqrt(v2);
      }
    }

    // Teuchos/BLAS-based approach.  As a matrix triple-product, this has some
    // unneeded FLOPs.  A vector-matrix triple product would be preferable, but
    // requires vector extractions from fnGradsMeanX.
    //RealSymMatrix variance(numFunctions, false);
    //Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1., covariance,
    //                             fnGradsMeanX, variance);
    //if (finalMomentsType == CENTRAL_MOMENTS)
    //  for (i=0; i<numFunctions; i++)
    //    finalMomentStats(1,i) = variance(i,i);
    //else
    //  for (i=0; i<numFunctions; i++)
    //    finalMomentStats(1,i) = sqrt(variance(i,i));
    //Cout << "\nvariance = " << variance
    //     << "\nfinalMomentStats = " << finalMomentStats;
  }
}


/** Initialize class-scope arrays and perform other start-up
    activities, such as evaluating median limit state responses. */
void NonDLocalReliability::initialize_class_data()
{
  // Initialize class-scope arrays used to warm-start multiple reliability
  // analyses within a strategy such as bi-level/sequential RBDO.  Cannot be
  // performed in constructor due to late availability of subIteratorFlag.
  if (warmStartFlag && subIteratorFlag && numRelAnalyses == 0) {
    size_t num_final_grad_vars
      = finalStatistics.active_set_derivative_vector().size();
    prevMPPULev0.resize(numFunctions);
    prevCumASVLev0.assign(numFunctions, 0);
    prevFnGradDLev0.shape(num_final_grad_vars, numFunctions);
    prevFnGradULev0.shape(numUncertainVars, numFunctions);
  }

  // define ranVarMeansU for use in the transformed AMV option
  //if (mppSearchType == AMV_U)
  natafTransform.trans_X_to_U(ranVarMeansX, ranVarMeansU);
  // must follow transform_correlations()

  /*
  // Determine median limit state values for AMV/AMV+/FORM/SORM by evaluating
  // response fns at u = 0 (used for determining signs of reliability indices).
  Cout << "\n>>>>> Evaluating response at median values\n";
  if (mppSearchType && mppSearchType < NO_APPROX)
    uSpaceModel.component_parallel_mode(TRUTH_MODEL);
  RealVector ep_median_u(numUncertainVars), // inits vals to 0
             ep_median_x(numUncertainVars, false);
  natafTransform.trans_U_to_X(ep_median_u, ep_median_x);
  iteratedModel.continuous_variables(ep_median_x);
  activeSet.request_values(0); // initialize
  for (size_t i=0; i<numFunctions; i++)
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty())
      activeSet.request_value(1, i); // only fn vals needed at median unc vars
  iteratedModel.evaluate(activeSet);
  medianFnVals = iteratedModel.current_response().function_values();
  */

  // now that vars/labels/bounds/targets have flowed down at run-time from any
  // higher level recursions, propagate them up the instantiate-on-the-fly
  // Model recursion so that they are correct when they propagate back down.
  mppModel.update_from_subordinate_model(); // depth = max
}


/** For a particular response function prior to the first z/p/beta level,
    initialize/warm-start optimizer initial guess (initialPtU),
    expansion point (mostProbPointX/U), and associated response
    data (computedRespLevel, fnGradX/U, and fnHessX/U). */
void NonDLocalReliability::initialize_level_data()
{
  // All reliability methods need initialization of initialPtU; AMV/AMV+/TANA
  // methods additionally need initialization of fnGradX/U; and AMV+/TANA
  // methods additionally need initialization of mostProbPointX/U and
  // computedRespLevel.

  // If warm-starting across multiple NonDLocalReliability invocations (e.g.,
  // for modified design parameters in RBDO), warm-start using the level 0 final
  // results for the corresponding response fn.  For all subsequent levels,
  // the warm-start procedure is self-contained (i.e., no data from the
  // previous NonDLocalReliability invocation is used).

  // For AMV+ across multiple NonDLocalReliability invocations, the previous
  // level 0 converged MPP provides the basis for the initial expansion point.
  // If inactive variable design sensitivities are available, a projection
  // from the previous MPP is used.  In either case, re-evaluation of response
  // data is required to capture the effect of inactive variable changes
  // (design variables in RBDO).  Since the mean expansion at the new d
  // has already been computed, one could also use this since it may predict
  // an MPP estimate (after one opt cycle, prior to the expense of the second
  // expansion evaluation) as good as the converged/projected MPP at the
  // old d.  However, the former approach has been observed to be preferable
  // in practice (even without projection).

  if (warmStartFlag && subIteratorFlag && numRelAnalyses) {
    // level 0 of each response fn in subsequent UQ analysis: initial
    // optimizer guess and initial expansion point are the converged
    // MPP from the previous UQ analysis, for which the computedRespLevel
    // and fnGradX/U must be re-evaluated due to design variable changes.

    // simplest approach
    initialPtU = prevMPPULev0[respFnCount]; // AMV/AMV+/FORM

    // If inactive var sensitivities are available, then apply a correction
    // to initialPtU using a design sensitivity projection (Burton & Hajela).
    // Note 1: only valid for RIA.
    // Note 2: when the top level RBDO optimizer is performing a value-based
    // line search, it is possible for prevFnGradDLev0 to be older than
    // prevICVars/prevMPPULev0/prevFnGradULev0.  In testing, this appears to
    // be OK and preferable to bypassing the projection when prevFnGradDLev0
    // is out of date (which is why the previous ASV test is cumulative: if
    // prevFnGradDLev0 has been populated, use it whether or not it was from
    // the last analysis).
    bool inactive_grads = (prevCumASVLev0[respFnCount] & 2)    ? true : false;
    bool lev0_ria = (requestedRespLevels[respFnCount].empty()) ? false : true;
    if (inactive_grads && lev0_ria) {
      RealVector fn_grad_d = Teuchos::getCol(Teuchos::View, prevFnGradDLev0,
                                             respFnCount);
      RealVector fn_grad_u = Teuchos::getCol(Teuchos::View, prevFnGradULev0,
                                             respFnCount);
      const RealVector& d_k_plus_1
	= iteratedModel.inactive_continuous_variables(); // view
      Real grad_d_delta_d = 0., norm_grad_u_sq = 0.;
      size_t i, num_icv = d_k_plus_1.length();
      for (i=0; i<num_icv; i++)
	grad_d_delta_d += fn_grad_d[i]*( d_k_plus_1[i] - prevICVars[i] );
      for (i=0; i<numUncertainVars; i++)
	norm_grad_u_sq += std::pow(fn_grad_u[i], 2);
      Real factor = grad_d_delta_d / norm_grad_u_sq;
      for (i=0; i<numUncertainVars; i++)
	initialPtU[i] -= factor * fn_grad_u[i];
    }

    if (mppSearchType == AMV_X || mppSearchType == AMV_U) {
      // Reevaluation for new des vars already performed at uncertain var means
      // in initial_taylor_series()
      assign_mean_data();
    }
    else if (mppSearchType == AMV_PLUS_X || mppSearchType == AMV_PLUS_U ||
	     mppSearchType == TANA_X     || mppSearchType == TANA_U) {
      mostProbPointU = initialPtU;
      natafTransform.trans_U_to_X(mostProbPointU, mostProbPointX);
      if (inactive_grads)
	Cout << "\n>>>>> Evaluating new response at projected MPP\n";
      else
	Cout << "\n>>>>> Evaluating new response at previous MPP\n";
      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
      // set active/uncertain vars augmenting inactive design vars
      iteratedModel.continuous_variables(mostProbPointX);
      short mode = (taylorOrder == 2) ? 7 : 3;
      activeSet.request_values(0); activeSet.request_value(mode, respFnCount);

      iteratedModel.evaluate(activeSet);
      const Response& curr_resp = iteratedModel.current_response();
      computedRespLevel = curr_resp.function_value(respFnCount);
      fnGradX = curr_resp.function_gradient_copy(respFnCount);

      SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
      SizetArray x_dvv; copy_data(cv_ids, x_dvv);
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
      if (mode & 4) {
	fnHessX = curr_resp.function_hessian(respFnCount);
	natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
					 fnGradX, x_dvv, cv_ids);
	curvatureDataAvailable = true; kappaUpdated = false;
      }
    }
  }
  else { // level 0 of each response fn in first or only UQ analysis

    // initial fnGradX/U for AMV/AMV+ = grads at mean x values, initial
    // expansion point for AMV+ = mean x values.
    if (mppSearchType < NO_APPROX) { // AMV/AMV+/TANA
      // update mostProbPointX/U, computedRespLevel, fnGradX/U, fnHessX/U
      assign_mean_data();
#ifdef MPP_CONVERGE_RATE
      if (mppSearchType >= AMV_PLUS_X)
	Cout << "u'u = "  << mostProbPointU.dot(mostProbPointU)
	     << " G(u) = " << computedRespLevel << '\n';
#endif // MPP_CONVERGE_RATE
    }

    // initial optimizer guess in u-space (initialPtU)
    initialPtU = initialPtUSpec; // initialPtUSpec set in ctor

    /*
    // fall back if projection is unavailable (or numerics don't work out).
    initialPtU = (ria_flag) ? initialPtUSpec :
      std::fabs(requestedCDFRelLevel)/std::sqrt((Real)numUncertainVars);

    // if fnValsMeanX/fnGradU are available, then warm start initialPtU using
    // a projection from the means.
    if (warmStartFlag && mv_flag) {
      Real alpha, norm_grad_u_sq = fnGradU.dot(fnGradU);
      if (ria_flag) {
	// use same projection idea as for a z level change,
	// but project from means
	if ( norm_grad_u_sq > 1.e-10 ) {
	  alpha = (requestedRespLevel - fnValsMeanX[respFnCount])
	        / norm_grad_u_sq;
	  for (i=0; i<numUncertainVars; i++)
	    initialPtU[i] = ranVarMeansU[i] + alpha*fnGradU[i];
	}
      }
      else { // pma
	// the simple projection for a beta level change does not work here
	// since beta at means will be near-zero (zero if x-space is normally
	// distributed).  Therefore, solve for the alpha value in
	// u = u_mean + alpha*dg/du which yields ||u|| = beta.  This requires
	// solving the quadratic expression a alpha^2 + b alpha + c = 0 with
	// a = dg/du^T dg/du, b = 2 dg/du^T u_mean, and
	// c = u_mean^T u_mean - beta^2
	Real b = 2. * ranVarMeansU.dot(fnGradU),
	     c = ranVarMeansU.dot(ranVarMeansU) - pow(requestedCDFRelLevel, 2);
	Real b2m4ac = b*b - 4.*norm_grad_u_sq*c;
	if (b2m4ac >= 0. && norm_grad_u_sq > 1.e-10) {
	  Real alpha1 = (-b + std::sqrt(b2m4ac))/2./norm_grad_u_sq,
	       alpha2 = (-b - std::sqrt(b2m4ac))/2./norm_grad_u_sq,
	       g_est1 = fnValsMeanX[respFnCount] + alpha1*norm_grad_u_sq;
	  // Select the correct root based on sign convention involving beta
	  // and relationship of projected G to G(0):
	  if (requestedCDFRelLevel >= 0.) 
	    // if beta_cdf >= 0, then projected G should be <= G(0)
	    alpha = (g_est1 <= medianFnVals[respFnCount]) ? alpha1 : alpha2;
	  else
	    // if beta_cdf <  0, then projected G should be >  G(0)
	    alpha = (g_est1 >  medianFnVals[respFnCount]) ? alpha1 : alpha2;
	  for (i=0; i<numUncertainVars; i++)
	    initialPtU[i] = ranVarMeansU[i] + alpha*fnGradU[i];
	}
      }
    }
    */
  }

  // Create the initial Taylor series approximation used by AMV/AMV+/TANA
  if (mppSearchType < NO_APPROX) {
    // restrict the approximation index set
    IntSet surr_fn_indices;
    surr_fn_indices.insert(respFnCount);
    uSpaceModel.surrogate_function_indices(surr_fn_indices);
    // construct the approximation
    update_limit_state_surrogate();
  }
}


/** For a particular response function at a particular z/p/beta level,
    warm-start or reset the optimizer initial guess (initialPtU),
    expansion point (mostProbPointX/U), and associated response
    data (computedRespLevel, fnGradX/U, and fnHessX/U). */
void NonDLocalReliability::initialize_mpp_search_data()
{
  if (warmStartFlag) {
    // For subsequent levels (including an RIA to PMA switch), warm start by
    // using the MPP from the previous level as the initial expansion
    // point.  The initial guess for the next MPP optimization is warm
    // started either by a simple copy of the MPP in the case of unconverged
    // AMV+ iterations (see AMV+ convergence assessment below) or, in the
    // case of an advance to the next level, by projecting from the current
    // MPP out to the new beta radius or response level.
    // NOTE: premature opt. termination can occur if the RIA/PMA 1st-order
    // optimality conditions (u + lamba*grad_g = 0 or grad_g + lambda*u = 0)
    // remain satisfied for the new level, even though the new equality
    // constraint will be violated.  The projection addresses this concern.

    // No action is required for warm start of mostProbPointX/U, fnGradX/U,
    // and computedRespLevel (not indexed by level)

    // Warm start initialPtU for the next level using a projection.
    size_t rl_len = requestedRespLevels[respFnCount].length();
    if (levelCount < rl_len) {
      // For RIA case, project along fnGradU to next g level using
      // linear Taylor series:  g2 = g1 + dg/du^T (u2 - u1) where
      // u2 - u1 = alpha*dg/du gives alpha = (g2 - g1)/(dg/du^T dg/du).
      // NOTE 1: the requested and computed response levels will differ in
      // the AMV case.  While the previous computed response level could be
      // used in the alpha calculation, the ratio of requested levels should
      // be a more accurate predictor of the next linearized AMV MPP.
      // NOTE 2: this projection could bypass the need for fnGradU with
      // knowledge of the Lagrange multipliers at the previous MPP
      // (u + lamba*grad_g = 0 or grad_g + lambda*u = 0).
      Real norm_grad_u_sq = fnGradU.dot(fnGradU);
      if ( norm_grad_u_sq > 1.e-10 ) { // also handles NPSOL numerical case
	Real alpha = (requestedTargetLevel - 
          requestedRespLevels[respFnCount][levelCount-1])/norm_grad_u_sq;
	for (size_t i=0; i<numUncertainVars; i++)
	  initialPtU[i] = mostProbPointU[i] + alpha * fnGradU[i];
      }
      else
	initialPtU = initialPtUSpec;//mostProbPointU: premature conv w/ some opt
    }
    else {
      // For PMA case, scale mostProbPointU so that its magnitude equals
      // the next beta_target.
      // NOTE 1: use of computed levels instead of requested levels handles
      // an RIA/PMA switch (the observed reliability from the RIA soln is
      // scaled to the requested reliability of the next PMA level).
      // NOTE 2: requested and computed reliability levels should agree very
      // closely in all cases since it is the g term that is linearized, not the
      // u'u term (which defines beta).  However, if the optimizer fails to
      // satisfy the PMA constraint, then using the computed level is preferred.
      //Real prev_pl = (levelCount == rl_len)
      //  ? computedProbLevels[respFnCount][levelCount-1]
      //  : requestedProbLevels[respFnCount][levelCount-rl_len-1];
      size_t pl_len = requestedProbLevels[respFnCount].length(),
	     bl_len = requestedRelLevels[respFnCount].length();
      Real prev_bl = ( integrationOrder == 2 &&
		       ( levelCount <  rl_len + pl_len ||
			 levelCount >= rl_len + pl_len + bl_len ) ) ?
	computedGenRelLevels[respFnCount][levelCount-1] :
	computedRelLevels[respFnCount][levelCount-1];
      // Note: scaling is applied to mppU, so we want best est of new beta.
      // Don't allow excessive init pt scaling if secant Hessian updating.
      Real high_tol = 1.e+3,
	low_tol = ( ( taylorOrder == 2 || integrationOrder == 2 ) &&
		    iteratedModel.hessian_type() == "quasi" ) ? 1.e-3 : 1.e-10;
      if ( std::abs(prev_bl) > low_tol && std::abs(prev_bl) < high_tol &&
	   std::abs(requestedTargetLevel) > low_tol &&
	   std::abs(requestedTargetLevel) < high_tol ) {
	// CDF or CCDF does not matter for scale_factor so long as it is
	// consistent (CDF/CDF or CCDF/CCDF).
	Real scale_factor = requestedTargetLevel / prev_bl;
#ifdef DEBUG
	Cout << "PMA warm start: previous = " << prev_bl
	     << " current = " << requestedTargetLevel
	     << " scale_factor = " << scale_factor << std::endl;
#endif // DEBUG
	for (size_t i=0; i<numUncertainVars; i++)
	  initialPtU[i] = mostProbPointU[i] * scale_factor;
      }
      else
	initialPtU = initialPtUSpec;//mostProbPointU: premature conv w/ some opt
    }
  }
  else { // cold start: reset to mean inputs/outputs
    // initial fnGradX/U for AMV/AMV+ = grads at mean x values, initial
    // expansion point for AMV+ = mean x values.
    if (mppSearchType < NO_APPROX) // AMV/AMV+/TANA
      assign_mean_data();
    // initial optimizer guess in u-space (initialPtU)
    initialPtU = initialPtUSpec; // initialPtUSpec set in ctor
  }
}


/** Includes case-specific logic for updating MPP search data for the
    AMV/AMV+/TANA/NO_APPROX methods. */
void NonDLocalReliability::
update_mpp_search_data(const Variables& vars_star, const Response& resp_star)
{
  size_t rl_len = requestedRespLevels[respFnCount].length(),
         pl_len = requestedProbLevels[respFnCount].length(),
         bl_len = requestedRelLevels[respFnCount].length();
  bool ria_flag = (levelCount < rl_len);
  const RealVector&    mpp_u = vars_star.continuous_variables(); // view
  const RealVector& fns_star = resp_star.function_values();

  // Update MPP arrays from optimization results
  Real conv_metric;
  switch (mppSearchType) {
  case AMV_PLUS_X: case AMV_PLUS_U: case TANA_X: case TANA_U:
    RealVector del_u(numUncertainVars, false);
    for (size_t i=0; i<numUncertainVars; i++)
      del_u[i] = mpp_u[i] - mostProbPointU[i];
    conv_metric = del_u.normFrobenius();
    break;
  }
  copy_data(mpp_u, mostProbPointU); // view -> copy
  natafTransform.trans_U_to_X(mostProbPointU, mostProbPointX);

  // Set computedRespLevel to the current g(x) value by either performing
  // a validation function evaluation (AMV/AMV+) or retrieving data from
  // resp_star (FORM).  Also update approximations and convergence tols.
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  SizetArray x_dvv; copy_data(cv_ids, x_dvv);
  switch (mppSearchType) {
  case AMV_X: case AMV_U: {
    approxConverged = true; // break out of while loop
    uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    activeSet.request_values(0); activeSet.request_value(1, respFnCount);
    iteratedModel.continuous_variables(mostProbPointX);
    iteratedModel.evaluate(activeSet); 
    computedRespLevel
      = iteratedModel.current_response().function_value(respFnCount);
    break;
  }
  case AMV_PLUS_X: case AMV_PLUS_U: case TANA_X: case TANA_U: {
    // Assess AMV+/TANA iteration convergence.  ||del_u|| is not a perfect
    // metric since cycling between MPP estimates can occur.  Therefore,
    // a maximum number of iterations is also enforced.
    //conv_metric = std::fabs(fn_vals[respFnCount] - requestedRespLevel);
    ++approxIters;
    if (conv_metric < convergenceTol)
      approxConverged = true;
    else if (approxIters >= maxIterations) {
      Cerr << "\nWarning: maximum number of limit state approximation cycles "
	   << "exceeded.\n";
      warningBits |= 1; // first warning in output summary
      approxConverged = true;
    }
    // Update response data for local/multipoint MPP approximation
    short mode = 1;
    if (approxConverged) {
      Cout << "\n>>>>> Approximate MPP iterations converged.  "
	   << "Evaluating final response.\n";
      // fnGradX/U needed for warm starting by projection, final_stat_grad,
      // and/or 2nd-order integration.
      const ShortArray& final_asv = finalStatistics.active_set_request_vector();
      if ( warmStartFlag || ( final_asv[statCount] & 2 ) )
	mode |= 2;
      if (integrationOrder == 2) {
	mode |= 4;
	if (numUncertainVars != numNormalVars)
	  mode |= 2; // fnGradX needed to transform fnHessX to fnHessU
      }
    }
    else { // not converged
      Cout << "\n>>>>> Updating approximation for MPP iteration "
	   << approxIters+1 << "\n";
      mode |= 2;            // update AMV+/TANA approximation
      if (taylorOrder == 2) // update AMV^2+ approximation
	mode |= 4;
      if (warmStartFlag) // warm start initialPtU for next AMV+ iteration
	initialPtU = mostProbPointU;
    }
    // evaluate new expansion point
    uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    activeSet.request_values(0);
    activeSet.request_value(mode, respFnCount);
    iteratedModel.continuous_variables(mostProbPointX);
    iteratedModel.evaluate(activeSet);
    const Response& curr_resp = iteratedModel.current_response();
    computedRespLevel = curr_resp.function_value(respFnCount);
#ifdef MPP_CONVERGE_RATE
    Cout << "u'u = "  << mostProbPointU.dot(mostProbPointU)
	 << " G(u) = " << computedRespLevel << '\n';
#endif // MPP_CONVERGE_RATE
    if (mode & 2) {
      fnGradX = curr_resp.function_gradient_copy(respFnCount);
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
    }
    if (mode & 4) {
      fnHessX = curr_resp.function_hessian(respFnCount);
      natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
				       fnGradX, x_dvv, cv_ids);
      curvatureDataAvailable = true; kappaUpdated = false;
    }

    // Update the limit state surrogate model
    update_limit_state_surrogate();

    // Update pmaMaximizeG if 2nd-order PMA for specified p / beta* level
    if ( !approxConverged && !ria_flag && integrationOrder == 2 )
      update_pma_maximize(mostProbPointU, fnGradU, fnHessU);

    break;
  }
  case NO_APPROX: { // FORM/SORM

    // direct optimization converges to MPP: no new approximation to compute
    approxConverged = true; // break out of while loop
    if (ria_flag) // RIA computed response = eq_con_star + response target
      computedRespLevel = fns_star[1] + requestedTargetLevel;
    else          // PMA computed response = +/- obj_fn_star
      computedRespLevel = (pmaMaximizeG) ? -fns_star[0] : fns_star[0];

    // fnGradX/U needed for warm starting by projection, final_stat_grad, and/or
    // 2nd-order integration (for nonlinear transformations), and should be
    // retrievable from previous evals.  If second-order integration for RIA,
    // fnHessX/U also needed, but not retrievable.  If second-order PMA with
    // specified p-level, Hessian should be retrievable since it was computed
    // during the update of requestedCDFRelLevel from requestedCDFProbLevel.
    // When data should be retrievable, we cannot in general assume that the
    // last grad/Hessian eval corresponds to the converged MPP; therefore, we
    // use a DB search.  If the DB search fails (e.g., the eval cache is
    // deactivated), then we resort to reevaluation.
    short mode = 0, found_mode = 0; // computedRespLevel already retrieved
    const ShortArray& final_asv = finalStatistics.active_set_request_vector();
    if ( warmStartFlag || ( final_asv[statCount] & 2 ) )
      mode |= 2;
    if ( integrationOrder == 2 ) {// apply 2nd-order integr in all RIA/PMA cases
      mode |= 4;
      if (numUncertainVars != numNormalVars)
	mode |= 2; // fnGradX needed to transform fnHessX to fnHessU
    }

    // retrieve previously evaluated gradient information, if possible
    if (mode & 2) { // avail in all RIA/PMA cases (exception: numerical grads)
      // query data_pairs to retrieve the fn gradient at the MPP
      Variables search_vars = iteratedModel.current_variables().copy();
      search_vars.continuous_variables(mostProbPointX);
      ActiveSet search_set = resp_star.active_set();
      ShortArray search_asv(numFunctions, 0);
      search_asv[respFnCount] = 2;
      search_set.request_vector(search_asv);
      PRPCacheHIter cache_it = lookup_by_val(data_pairs,
	iteratedModel.interface_id(), search_vars, search_set);
      if (cache_it != data_pairs.get<hashed>().end()) {
	fnGradX = cache_it->response().function_gradient_copy(respFnCount);
	found_mode |= 2;
      }
    }
    // retrieve previously evaluated Hessian information, if possible
    // > RIA and std PMA beta-level: Hessian not avail since not yet evaluated
    // > PMA p-level and generalized beta-level: Hessian should be available
    if ( ( mode & 4 ) && !ria_flag &&
	 ( levelCount <  rl_len + pl_len ||
	   levelCount >= rl_len + pl_len + bl_len ) ) {
      // query data_pairs to retrieve the fn Hessian at the MPP
      Variables search_vars = iteratedModel.current_variables().copy();
      search_vars.continuous_variables(mostProbPointX);
      ActiveSet search_set = resp_star.active_set();
      ShortArray search_asv(numFunctions, 0);
      search_asv[respFnCount] = 4;
      search_set.request_vector(search_asv);
      PRPCacheHIter cache_it = lookup_by_val(data_pairs,
	iteratedModel.interface_id(), search_vars, search_set);
      if (cache_it != data_pairs.get<hashed>().end()) {
        fnHessX = cache_it->response().function_hessian(respFnCount);
	found_mode |= 4;
      }
    }
    // evaluate any remaining required data which could not be retrieved
    short remaining_mode = mode - found_mode;
    if (remaining_mode) {
      Cout << "\n>>>>> Evaluating limit state derivatives at MPP\n";
      iteratedModel.continuous_variables(mostProbPointX);
      activeSet.request_values(0);
      activeSet.request_value(remaining_mode, respFnCount);
      iteratedModel.evaluate(activeSet);
      const Response& curr_resp = iteratedModel.current_response();
      if (remaining_mode & 2)
	fnGradX = curr_resp.function_gradient_copy(respFnCount);
      if (remaining_mode & 4)
        fnHessX = curr_resp.function_hessian(respFnCount);
    }
    if (mode & 2)
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
    if (mode & 4) {
      natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
				       fnGradX, x_dvv, cv_ids);
      curvatureDataAvailable = true; kappaUpdated = false;
    }
    break;
  }
  }

  // set computedRelLevel using u'u from fns_star; must follow fnGradU update
  if (ria_flag)
    computedRelLevel = signed_norm(std::sqrt(fns_star[0]));
  else if (integrationOrder == 2) { // second-order PMA
    // no op: computed{Rel,GenRel}Level updated in PMA2_constraint_eval()
  }
  else { // first-order PMA
    Real norm_u_sq = fns_star[1] + std::pow(requestedTargetLevel, 2);
    computedRelLevel = signed_norm(std::sqrt(norm_u_sq));
  }
}


/** Updates computedRespLevels/computedProbLevels/computedRelLevels,
    finalStatistics, warm start, and graphics data. */
void NonDLocalReliability::update_level_data()
{
  // local reliability data aren't output to tabular, so send directly
  // to graphics window only
  Graphics& dakota_graphics = parallelLib.output_manager().graphics();

  bool ria_flag = (levelCount < requestedRespLevels[respFnCount].length());

  // Update computed Resp/Prob/Rel/GenRel levels arrays.  finalStatistics
  // is updated within update_final_statistics() for all resp fns & levels.
  computedRespLevels[respFnCount][levelCount] = computedRespLevel;
  computedRelLevels[respFnCount][levelCount]  = computedRelLevel;
  Real computed_prob_level;
  if (!ria_flag && integrationOrder == 2) {
    computedGenRelLevels[respFnCount][levelCount] = computedGenRelLevel;
    computedProbLevels[respFnCount][levelCount] = computed_prob_level =
      probability(computedGenRelLevel);
  }
  else {
    computedProbLevels[respFnCount][levelCount] = computed_prob_level =
      probability(computedRelLevel, cdfFlag, mostProbPointU, fnGradU, fnHessU);
    computedGenRelLevels[respFnCount][levelCount] = computedGenRelLevel =
      reliability(computed_prob_level);
  }

  // Final statistic gradients are dz/ds, dbeta/ds, or dp/ds
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  bool system_grad_contrib = false;
  if (respLevelTargetReduce &&
      levelCount < requestedRespLevels[respFnCount].length()) {
    size_t sys_stat_count = 2*numFunctions + totalLevelRequests + levelCount;
    if (final_asv[sys_stat_count] & 2)
      system_grad_contrib = true;
  }
  if ( (final_asv[statCount] & 2) || system_grad_contrib ) {

    // evaluate dg/ds at the MPP and store in final_stat_grad
    RealVector final_stat_grad;
    dg_ds_eval(mostProbPointX, fnGradX, final_stat_grad);

    // for warm-starting next run
    if (warmStartFlag && subIteratorFlag && levelCount == 0)
      Teuchos::setCol(final_stat_grad, respFnCount, prevFnGradDLev0);

    // RIA: sensitivity of beta/p/beta* w.r.t. inactive variables
    //   dbeta/ds     = 1/norm_grad_u * dg/ds       (first-order)
    //   dp/ds        = -phi(-beta) * dbeta/ds      (first-order)
    //   dp_2/ds      = [Phi(-beta_corr)*sum - phi(-beta_corr)*prod] * dbeta/ds
    //                                              (second-order)
    //   dbeta*/ds    = -1/phi(-beta*) * dp_2/ds    (second-order)
    // PMA: sensitivity of g function w.r.t. inactive variables
    //   dz/ds        = dg/ds
    if (ria_flag) {
      // beta_cdf = -beta_ccdf, p_cdf = 1. - p_ccdf
      // -->> dbeta_cdf/ds = -dbeta_ccdf/ds, dp_cdf/ds = -dp_ccdf/ds
      Real norm_grad_u = fnGradU.normFrobenius();
      // factor for first-order dbeta/ds:
      Real factor = (cdfFlag) ? 1./norm_grad_u : -1./norm_grad_u;
      if (integrationOrder == 2 && respLevelTarget != RELIABILITIES) {
	factor *= dp2_dbeta_factor(computedRelLevel, cdfFlag);
	if (respLevelTarget == GEN_RELIABILITIES) // for 2nd-order dbeta*/ds
	  factor /= -Pecos::NormalRandomVariable::std_pdf(-computedGenRelLevel);
      }
      else if (respLevelTarget == PROBABILITIES)  // for 1st-order dp/ds
	factor *= -Pecos::NormalRandomVariable::std_pdf(-computedRelLevel);

      // apply factor:
      size_t i, num_final_grad_vars
	= finalStatistics.active_set_derivative_vector().size();
      for (i=0; i<num_final_grad_vars; ++i)
	final_stat_grad[i] *= factor;
    }
    finalStatistics.function_gradient(final_stat_grad, statCount);
  }

  // Update warm-start data and graphics
  if (warmStartFlag && subIteratorFlag && levelCount == 0) {
    // for warm-starting next run
    prevMPPULev0[respFnCount] = mostProbPointU;
    prevCumASVLev0[respFnCount] |= final_asv[statCount];
    for (size_t i=0; i<numUncertainVars; i++)
      prevFnGradULev0(i,respFnCount) = fnGradU[i];
  }
  if (!subIteratorFlag) {
    dakota_graphics.add_datapoint(respFnCount, computedRespLevel,
				  computed_prob_level);
    for (size_t i=0; i<numUncertainVars; i++) {
      dakota_graphics.add_datapoint(numFunctions+i, computedRespLevel,
				    mostProbPointX[i]);
      if (numFunctions > 1 && respFnCount < numFunctions-1 &&
	  levelCount == requestedRespLevels[respFnCount].length() +
	                requestedProbLevels[respFnCount].length() +
	                requestedRelLevels[respFnCount].length() +
	                requestedGenRelLevels[respFnCount].length() - 1)
	dakota_graphics.new_dataset(numFunctions+i);
    }
  }
}


void NonDLocalReliability::update_limit_state_surrogate()
{
  bool x_space = (mppSearchType ==  AMV_X || mppSearchType == AMV_PLUS_X ||
		  mppSearchType == TANA_X);

  // construct local Variables object
  Variables mpp_vars(iteratedModel.current_variables().shared_data());
  if (x_space) mpp_vars.continuous_variables(mostProbPointX);
  else         mpp_vars.continuous_variables(mostProbPointU);

  // construct Response object
  ShortArray asv(numFunctions, 0);
  asv[respFnCount] = (taylorOrder == 2) ? 7 : 3;
  ActiveSet set;//(numFunctions, numUncertainVars);
  set.request_vector(asv);
  set.derivative_vector(iteratedModel.continuous_variable_ids());
  Response response(SIMULATION_RESPONSE, set);
  response.function_value(computedRespLevel, respFnCount);
  if (x_space) {
    response.function_gradient(fnGradX, respFnCount);
    if (taylorOrder == 2)
      response.function_hessian(fnHessX, respFnCount);
  }
  else {
    response.function_gradient(fnGradU, respFnCount);
    if (taylorOrder == 2)
      response.function_hessian(fnHessU, respFnCount);
  }
  IntResponsePair response_pr(0, response); // dummy eval id

  // After a design variable change, history data (e.g., TANA) needs
  // to be cleared (build_approximation() only calls clear_current())
  if (numRelAnalyses && levelCount == 0)
    uSpaceModel.approximations()[respFnCount].clear_all();
  // build the new local/multipoint approximation
  uSpaceModel.build_approximation(mpp_vars, response_pr);
}


void NonDLocalReliability::
update_pma_maximize(const RealVector& mpp_u, const RealVector& fn_grad_u,
		    const RealSymMatrix& fn_hess_u)
{
  Real p_cdf; bool update = false;
  size_t rl_len  = requestedRespLevels[respFnCount].length(),
    rl_pl_len    = rl_len + requestedProbLevels[respFnCount].length(),
    rl_pl_bl_len = rl_pl_len + requestedRelLevels[respFnCount].length();
  if (levelCount <  rl_pl_len) {
    Real p = requestedProbLevels[respFnCount][levelCount-rl_len];
    p_cdf = (cdfFlag) ? p : 1. - p;
    update = true;
  }
  else if (levelCount >= rl_pl_bl_len) {
    Real gen_beta = requestedGenRelLevels[respFnCount][levelCount-rl_pl_bl_len];
    Real gen_beta_cdf = (cdfFlag) ? gen_beta : -gen_beta;
    p_cdf = probability(gen_beta_cdf);
    update = true;
  }
  if (update) {
    Real beta_cdf = reliability(p_cdf, true, mpp_u, fn_grad_u, fn_hess_u);
    pmaMaximizeG = (beta_cdf < 0.);
  }
}


void NonDLocalReliability::assign_mean_data()
{
  mostProbPointX = ranVarMeansX;
  mostProbPointU = ranVarMeansU;
  computedRespLevel = fnValsMeanX(respFnCount);
  for (size_t i=0; i<numUncertainVars; i++)
    fnGradX[i] = fnGradsMeanX(i,respFnCount);
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  SizetArray x_dvv; copy_data(cv_ids, x_dvv);
  natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, ranVarMeansX,
				   x_dvv, cv_ids);
  if (taylorOrder == 2 && iteratedModel.hessian_type() != "quasi") {
    fnHessX = fnHessiansMeanX[respFnCount];
    natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, ranVarMeansX, fnGradX,
				     x_dvv, cv_ids);
    curvatureDataAvailable = true; kappaUpdated = false;
  }
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an RIA objective function. */
void NonDLocalReliability::
RIA_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // ----------------------------------------
  // The RIA objective function is (norm u)^2
  // ----------------------------------------

  short       asv_val = recast_response.active_set_request_vector()[0];
  const RealVector& u = recast_vars.continuous_variables();
  size_t i, num_vars = u.length();
  if (asv_val & 1) {
    Real f = 0.;
    for (i=0; i<num_vars; i++)
      f += std::pow(u[i], 2); // f = u'u
    recast_response.function_value(f, 0);
  }
  if (asv_val & 2) {
    RealVector grad_f = recast_response.function_gradient_view(0);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f = recast_response.function_hessian_view(0);
    hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
  }

  // Using f = norm u is a poor choice, since grad f is undefined at u = 0.
  //Real sqrt_sum_sq = std::sqrt(sum_sq);
  //if (sqrt_sum_sq > 0.)
  //  grad_f[i] = u[i]/sqrt_sum_sq;
  //else // gradient undefined at origin, use 0. to keep optimizer happy
  //  grad_f[i] = 0.;
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an RIA equality constraint. */
void NonDLocalReliability::
RIA_constraint_eval(const Variables& sub_model_vars,
		    const Variables& recast_vars,
		    const Response& sub_model_response,
		    Response& recast_response)
{
  // --------------------------------------------------------
  // The RIA equality constraint is G(u) - response level = 0
  // --------------------------------------------------------

  short asv_val = recast_response.active_set_request_vector()[1];
  int   resp_fn = nondLocRelInstance->respFnCount;
  if (asv_val & 1) {
    const Real& sub_model_fn = sub_model_response.function_value(resp_fn);
    recast_response.function_value(
      sub_model_fn - nondLocRelInstance->requestedTargetLevel, 1);
  }
  if (asv_val & 2) // dG/du: no additional transformation needed
    recast_response.function_gradient(
      sub_model_response.function_gradient_view(resp_fn), 1);
  if (asv_val & 4) // d^2G/du^2: no additional transformation needed
    recast_response.function_hessian(
      sub_model_response.function_hessian(resp_fn), 1);
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an PMA objective function. */
void NonDLocalReliability::
PMA_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // ----------------------------------
  // The PMA objective function is G(u)
  // ----------------------------------

  int   resp_fn    = nondLocRelInstance->respFnCount;
  short sm_asv_val = sub_model_response.active_set_request_vector()[resp_fn];
  Real fn; RealVector fn_grad_u; RealSymMatrix fn_hess_u;
  if (sm_asv_val & 2)
    fn_grad_u = sub_model_response.function_gradient_view(resp_fn);
  if (sm_asv_val & 4)
    fn_hess_u = sub_model_response.function_hessian_view(resp_fn);

  // Due to RecastModel, objective_eval always called before constraint_eval,
  // so perform NO_APPROX updates here.
  if (nondLocRelInstance->mppSearchType == NO_APPROX &&
      nondLocRelInstance->integrationOrder == 2) {
    nondLocRelInstance->curvatureDataAvailable = true;
    nondLocRelInstance->kappaUpdated = false; // new fn_{grad,hess}_u data
    nondLocRelInstance->update_pma_maximize(recast_vars.continuous_variables(),
					    fn_grad_u, fn_hess_u);
  }

  short asv_val = recast_response.active_set_request_vector()[0];
  bool  pma_max = nondLocRelInstance->pmaMaximizeG;
  if (asv_val & 1) {
    fn = sub_model_response.function_value(resp_fn);
    if (pma_max) recast_response.function_value(-fn, 0);
    else         recast_response.function_value( fn, 0);
  }
  if (asv_val & 2) { // dG/du: no additional transformation needed
    if (pma_max) {
      RealVector recast_grad = recast_response.function_gradient_view(0);
      size_t i, num_vars = fn_grad_u.length();
      for (i=0; i<num_vars; ++i)
	recast_grad[i] = -fn_grad_u[i];
    }
    else
      recast_response.function_gradient(fn_grad_u, 0);
  }
  if (asv_val & 4) { // d^2G/du^2: no additional transformation needed
    if (pma_max) {
      RealSymMatrix recast_hess	= recast_response.function_hessian_view(0);
      size_t i, j, num_vars = fn_hess_u.numRows();
      for (i=0; i<num_vars; ++i)
	for (j=0; j<=i; ++j)
	  recast_hess(i,j) = -fn_hess_u(i,j);
    }
    else
      recast_response.function_hessian(fn_hess_u, 0);
  }

#ifdef DEBUG
  if (asv_val & 1)
    Cout << "PMA_objective_eval(): sub-model function = " << fn << std::endl;
  if (asv_val & 2) { // dG/du: no additional transformation needed
    Cout << "PMA_objective_eval(): sub-model gradient:\n";
    write_data(Cout, fn_grad_u);
  }
  if (asv_val & 4) { // d^2G/du^2: no additional transformation needed
    Cout << "PMA_objective_eval(): sub-model Hessian:\n";
    write_data(Cout, fn_hess_u);
  }
#endif // DEBUG
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into a first-order PMA equality
    constraint on reliability index beta. */
void NonDLocalReliability::
PMA_constraint_eval(const Variables& sub_model_vars,
		    const Variables& recast_vars,
		    const Response& sub_model_response,
		    Response& recast_response)
{
  // ------------------------------------------------------
  // The PMA equality constraint is (norm u)^2 - beta^2 = 0
  // ------------------------------------------------------

  short       asv_val = recast_response.active_set_request_vector()[1];
  const RealVector& u = recast_vars.continuous_variables();
  size_t i, num_vars = u.length();
  if (asv_val & 1) {
    // calculate the reliability index (beta)
    Real beta_sq = 0.;
    for (i=0; i<num_vars; ++i)
      beta_sq += std::pow(u[i], 2); //use beta^2 to avoid singular grad @ origin
    // calculate the equality constraint: u'u - beta_target^2
    Real c = beta_sq - std::pow(nondLocRelInstance->requestedTargetLevel, 2);
    recast_response.function_value(c, 1);
  }
  if (asv_val & 2) {
    RealVector grad_f = recast_response.function_gradient_view(1);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f = recast_response.function_hessian_view(1);
    hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
  }
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into a second-order PMA equality
    constraint on generalized reliability index beta-star. */
void NonDLocalReliability::
PMA2_constraint_eval(const Variables& sub_model_vars,
		     const Variables& recast_vars,
		     const Response& sub_model_response,
		     Response& recast_response)
{
  // -----------------------------------------------------
  // The PMA SORM equality constraint is beta* = beta*-bar
  // -----------------------------------------------------

  const RealVector& u = recast_vars.continuous_variables();
  short    asv_val = recast_response.active_set_request_vector()[1];
  int      resp_fn = nondLocRelInstance->respFnCount;
  short sm_asv_val = sub_model_response.active_set_request_vector()[resp_fn];
  bool         cdf = nondLocRelInstance->cdfFlag;

  // Calculate beta --> p --> beta*.  Use up-to-date mpp/grad/Hessian info,
  // including surrogate-based data, within signed_norm(), but disallow
  // surrogate-based curvature corrections due to their sensitivity.  The
  // presence of fn_grad_u/fn_hess_u data is enforced in PMA2_set_mapping().
  RealVector fn_grad_u = sub_model_response.function_gradient_view(resp_fn);
  Real comp_rel = nondLocRelInstance->computedRelLevel =
    nondLocRelInstance->signed_norm(u, fn_grad_u, cdf);
  // Don't update curvature correction when nonlinear transformation
  // induces additional curvature on top of a low-order approximation.
  // Note: if linear transformation or u-space AMV^2+, then Hessian is
  // consistent with the previous truth and is constant over the surrogate.
  Real computed_prob_level = (nondLocRelInstance->mppSearchType == NO_APPROX) ?
    nondLocRelInstance->probability(comp_rel, cdf, u, fn_grad_u,
      sub_model_response.function_hessian(resp_fn)) :
    nondLocRelInstance->probability(comp_rel, cdf,
      nondLocRelInstance->mostProbPointU, nondLocRelInstance->fnGradU,
      nondLocRelInstance->fnHessU);
  Real comp_gen_rel = nondLocRelInstance->computedGenRelLevel =
    nondLocRelInstance->reliability(computed_prob_level);

  if (asv_val & 1) { // calculate the equality constraint: beta* - bar-beta*
    Real c = comp_gen_rel - nondLocRelInstance->requestedTargetLevel;
#ifdef DEBUG
    Cout << "In PMA2_constraint_eval, beta* = " << comp_gen_rel 
	 << " bar-beta* = " << nondLocRelInstance->requestedTargetLevel
	 << " eq constr = " << c << std::endl;
#endif
    recast_response.function_value(c, 1);
  }
  if (asv_val & 2) {
    // Note: for second-order integrations, beta* is a function of p and
    // kappa(u).  dbeta*/du involves dkappa/du, but these terms are neglected
    // as in dbeta*/ds (design sensitivities).
    //   dbeta/du_i  = u_i/beta (in factor below)
    //   dp_2/du_i   = [Phi(-beta_corr)*sum - phi(-beta_corr)*prod] * dbeta/du_i
    //                 (term in brackets computed in dp2_dbeta_factor())
    //   dbeta*/du_i = -1/phi(-beta*) * dp_2/du (in factor below)
    Real factor = -nondLocRelInstance->dp2_dbeta_factor(comp_rel, cdf)
      / comp_rel / Pecos::NormalRandomVariable::std_pdf(-comp_gen_rel);
    size_t i, num_vars = u.length();
    RealVector grad_f = recast_response.function_gradient_view(1);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = factor * u[i];
#ifdef DEBUG
    Cout << "In PMA2_constraint_eval, gradient of beta*:\n";
    write_data(Cout, grad_f);
#endif
  }
  if (asv_val & 4) {
    Cerr << "Error: Hessian data not supported in NonDLocalReliability::"
	 << "PMA2_constraint_eval()" << std::endl;
    abort_handler(-1);
    /*
    RealSymMatrix hess_f = recast_response.function_hessian_view(1);
    hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = ;
    */
  }
}


void NonDLocalReliability::
PMA2_set_mapping(const Variables& recast_vars, const ActiveSet& recast_set,
		 ActiveSet& sub_model_set)
{
  // if the constraint value/grad is requested for second-order PMA, then
  // the sub-model response grad/Hessian are required to update beta*
  short recast_request = recast_set.request_vector()[1];
  if (recast_request & 3) { // value/grad request share beta-->p-->beta* calcs
    int   sm_index          = nondLocRelInstance->respFnCount;
    short sub_model_request = sub_model_set.request_value(sm_index);

    // all cases require latest fn_grad_u (either truth-based or approx-based)
    sub_model_request |= 2;
    // PMA SORM requires latest fn_hess_u (truth-based)
    if (nondLocRelInstance->mppSearchType == NO_APPROX)
      sub_model_request |= 4;
    // else value/grad request utilizes most recent truth fnGradU/fnHessU

    sub_model_set.request_value(sub_model_request, sm_index);
  }
}


/** Computes dg/ds where s = design variables.  Supports potentially
    overlapping cases of design variable augmentation and insertion. */
void NonDLocalReliability::
dg_ds_eval(const RealVector& x_vars, const RealVector& fn_grad_x,
	   RealVector& final_stat_grad)
{
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, num_final_grad_vars = final_dvv.size();
  if (final_stat_grad.empty())
    final_stat_grad.resize(num_final_grad_vars);

  // For design vars that are distribution parameters of the uncertain vars,
  // dg/ds = dg/dx * dx/ds where dx/ds is the design Jacobian.  Since dg/dx is
  // already available (passed in as fn_grad_x), these sensitivities do not
  // require additional response evaluations.
  bool dist_param_deriv = false;
  size_t num_outer_cv = secondaryACVarMapTargets.size();
  for (i=0; i<num_outer_cv; i++)
    if (secondaryACVarMapTargets[i] != Pecos::NO_TARGET) // dist param insertion
      { dist_param_deriv = true; break; }
  if (dist_param_deriv) {
    SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
    SizetArray x_dvv; copy_data(cv_ids, x_dvv);
    SizetMultiArrayConstView acv_ids
      = iteratedModel.all_continuous_variable_ids();
    RealVector fn_grad_s(num_final_grad_vars, false);
    natafTransform.trans_grad_X_to_S(fn_grad_x, fn_grad_s, x_vars, x_dvv,
				     cv_ids, acv_ids, primaryACVarMapIndices,
				     secondaryACVarMapTargets);
    final_stat_grad = fn_grad_s;
  }

  // For design vars that are separate from the uncertain vars, perform a new
  // fn eval for dg/ds, where s = inactive/design vars.  This eval must be
  // performed at (s, x_vars) for each response fn for each level as
  // required by final_asv.  RBDO typically specifies one level for 1 or
  // more limit states, so the number of additional evals will usually be small.
  if (secondaryACVarMapTargets.empty() ||
      contains(secondaryACVarMapTargets, Pecos::NO_TARGET)) {
    Cout << "\n>>>>> Evaluating sensitivity with respect to augmented inactive "
	 << "variables\n";
    if (mppSearchType && mppSearchType < NO_APPROX)
      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    iteratedModel.continuous_variables(x_vars);
    ActiveSet inactive_grad_set = activeSet;
    inactive_grad_set.request_values(0);
    inactive_grad_set.request_value(2, respFnCount);
    // final_dvv is mapped from the top-level DVV in NestedModel::set_mapping()
    // and includes augmented and inserted variable ids.  Since we only want the
    // augmented ids in this case, the UQ-level inactive ids are sufficient.
    SizetMultiArrayConstView icv_ids
      = iteratedModel.inactive_continuous_variable_ids();
    inactive_grad_set.derivative_vector(icv_ids);
    /* More rigorous with reqd deriv vars, but equivalent in practice:
    // Filter final_dvv to contain only inactive continuous variable ids:
    SizetArray filtered_final_dvv;
    for (i=0; i<num_final_grad_vars; i++) {
      size_t final_dvv_i = final_dvv[i];
      if (contains(icv_ids, final_dvv_i))
	filtered_final_dvv.push_back(final_dvv_i);
    }
    inactive_grad_set.derivative_vector(filtered_final_dvv);
    */
    iteratedModel.evaluate(inactive_grad_set);
    const Response& curr_resp = iteratedModel.current_response();
    if (secondaryACVarMapTargets.empty())
      final_stat_grad = curr_resp.function_gradient_copy(respFnCount);
    else {
      const RealMatrix& fn_grads = curr_resp.function_gradients();
      size_t cntr = 0;
      for (i=0; i<num_final_grad_vars; i++)
	if (secondaryACVarMapTargets[i] == Pecos::NO_TARGET)
	  final_stat_grad[i] = fn_grads(cntr++, respFnCount);
    }
  }
}


Real NonDLocalReliability::
signed_norm(Real norm_mpp_u, const RealVector& mpp_u,
	    const RealVector& fn_grad_u, bool cdf_flag)
{
  // z>median: CDF p(g<=z)>0.5, CDF beta<0, CCDF p(g>z)<0.5, CCDF beta>0
  // z<median: CDF p(g<=z)<0.5, CDF beta>0, CCDF p(g>z)>0.5, CCDF beta<0
  // z=median: CDF p(g<=z) = CCDF p(g>z) = 0.5, CDF beta = CCDF beta = 0
  //Real beta_cdf = (computedRespLevel > medianFnVals[respFnCount])
  //              ? -std::sqrt(norm_u_sq) : std::sqrt(norm_u_sq);

  // This approach avoids the need to evaluate medianFnVals.  Thanks to
  // Barron Bichon for suggesting it.
  // if <mppU, fnGradU> > 0, then G is increasing along u and G(u*) > G(0)
  // if <mppU, fnGradU> < 0, then G is decreasing along u and G(u*) < G(0)
  Real beta_cdf = (mpp_u.dot(fn_grad_u) > 0.) ? -norm_mpp_u : norm_mpp_u;
#ifdef DEBUG
  Cout << "\nSign of <mppU, fnGradU> is ";
  if (mpp_u.dot(fn_grad_u) > 0.) Cout << " 1.\n\n";
  else                           Cout << "-1.\n\n";
#endif
  return (cdf_flag) ? beta_cdf : -beta_cdf;
}


/** Converts beta into a probability using either first-order (FORM) or
    second-order (SORM) integration.  The SORM calculation first calculates
    the principal curvatures at the MPP (using the approach in Ch. 8 of 
    Haldar & Mahadevan), and then applies correction formulations from the 
    literature (Breitung, Hohenbichler-Rackwitz, or Hong). */
Real NonDLocalReliability::
probability(Real beta, bool cdf_flag, const RealVector& mpp_u,
	    const RealVector& fn_grad_u, const RealSymMatrix& fn_hess_u)
{
  Real p = probability(beta); // FORM approximation
  int wpp7;
  if (outputLevel > NORMAL_OUTPUT) {
    wpp7 = write_precision+7;
    Cout << "Probability:"// << " beta = " << beta
	 << " first-order = " << std::setw(wpp7) << p;
  }

  if (integrationOrder == 2 && curvatureDataAvailable) {

    if (!kappaUpdated) {
      principal_curvatures(mpp_u, fn_grad_u, fn_hess_u, kappaU);
      kappaUpdated = true;
    }

    // The correction to p is applied for beta >= 0 (FORM p <= 0.5).
    // For beta < 0, apply correction to complementary problem (Tvedt 1990).
    //   > beta changes sign
    //   > p becomes complement
    //   > principal curvature sign convention defined for CDF beta > 0
    //     (negate for CDF beta < 0 or CCDF beta > 0, OK for CCDF beta < 0)
    Real beta_corr = std::abs(beta);
    Real p_corr    = (beta >= 0.) ? p : 1. - p;
    RealVector kappa; scale_curvature(beta, cdf_flag, kappaU, kappa);

    // Test for numerical exceptions in sqrt.  Problematic kappa are large and
    // negative (kterm is always positive).  Skipping individual kappa means
    // neglecting a primary curvature and including secondary curvatures, which
    // may be counter-productive (potentially less accurate than FORM).
    // Therefore, the entire correction is skipped if any curvature is
    // problematic.  A consistent approach must be used in reliability().
    bool apply_correction = true;
    Real psi_m_beta;
    if (secondOrderIntType != BREITUNG)
      psi_m_beta = Pecos::NormalRandomVariable::std_pdf(-beta_corr)
	         / Pecos::NormalRandomVariable::std_cdf(-beta_corr);
    Real kterm = (secondOrderIntType == BREITUNG) ? beta_corr : psi_m_beta;
    int i, num_kappa = numUncertainVars - 1;
    for (i=0; i<num_kappa; i++) {
      //Cout << "1 + kterm*kappa = " << 1. + kterm * kappa[i] << std::endl;
      // Numerical exception happens for 1+ktk <= 0., but inaccuracy can happen
      // earlier.  Empirical evidence to date suggests a threshold of 0.5 
      // (1/std::sqrt(0.5) = 1.414 multiplier = 41.4% increase in p.
      if (1. + kterm * kappa[i] <= curvatureThresh)
	apply_correction = false;
    }

    if (apply_correction) {
      // compute SORM estimate (Breitung, Hohenbichler-Rackwitz, or Hong).
      Real C1 = 0., ktk;
      for (i=0; i<num_kappa; i++) {
	// Breitung 1984:              p_corr /= std::sqrt(1+beta_corr*kappa)
	// Hohenbichler-Rackwitz 1988: p_corr /= std::sqrt(1+psi_m_beta*kappa)
	// > Note that psi(-beta) -> beta as beta increases: HR -> Breitung
	// Hong 1999, P3 formulation:  p_corr =  C1 * p_HR
	ktk = kterm * kappa[i];
	p_corr /= std::sqrt(1. + ktk);
	if (secondOrderIntType == HONG) {
	  Real hterm = num_kappa * kappa[i] / 2. / (1. + ktk);
	  C1 += Pecos::NormalRandomVariable::std_cdf(-beta_corr-hterm)
	     /  Pecos::NormalRandomVariable::std_cdf(-beta_corr)
	     *  exp(psi_m_beta*hterm);
	}
      }
      if (secondOrderIntType == HONG) {
	C1 /= num_kappa;
	p_corr *= C1;
      }
      if (p_corr >= 0. && p_corr <= 1.) { // verify p_corr within valid range
	p = (beta >= 0.) ? p_corr : 1. - p_corr;
	if (outputLevel > NORMAL_OUTPUT)
	  Cout << " second-order = " << std::setw(wpp7) << p;
      }
      else {
	Cerr << "\nWarning: second-order probability integration bypassed due "
	     << "to numerical issues (corrected p outside [0,1]).\n";
	warningBits |= 2; // second warning in output summary
      }
    }
    else {
      Cerr << "\nWarning: second-order probability integration bypassed due "
	   << "to numerical issues (curvature threshold exceeded).\n";
      warningBits |= 2; // second warning in output summary
    }
  }

  if (integrationRefinement &&                                  // IS/AIS/MMAIS
      levelCount < requestedRespLevels[respFnCount].length()) { // RIA only
    // rep needed for access to functions not mapped to Iterator level
    NonDAdaptImpSampling* import_sampler_rep
      = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
    bool x_data_flag = false;
    import_sampler_rep->
      initialize(mpp_u, x_data_flag, respFnCount, p, requestedTargetLevel);
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    importanceSampler.run(pl_iter);
    p = import_sampler_rep->final_probability();
    if (outputLevel > NORMAL_OUTPUT)
      Cout << " refined = " << std::setw(wpp7) << p;
  }
  if (outputLevel > NORMAL_OUTPUT)
    Cout << '\n';
#ifdef DEBUG
  if (integrationOrder == 2 && curvatureDataAvailable)
    { Cout << "In probability(), kappaU:\n"; write_data(Cout, kappaU); }
#endif

  return p;
}


/** Compute sensitivity of second-order probability w.r.t. beta for use
    in derivatives of p_2 or beta* w.r.t. auxilliary parameters s (design,
    epistemic) or derivatives of beta* w.r.t. u in PMA2_constraint_eval(). */
Real NonDLocalReliability::dp2_dbeta_factor(Real beta, bool cdf_flag)
{
  //   dp/ds     = -phi(-beta) * dbeta/ds
  //               (fall back to first-order, if needed)
  //   dp_2/ds   = [Phi(-beta_corr)*sum - phi(-beta_corr)*prod] * dbeta/ds
  //               (this function computes term in brackets)
  //   dbeta*/ds = -1/phi(-beta*) * dp_2/ds    (second-order)

  // For beta < 0, beta_corr = -beta and p_corr = 1 - p:
  // dp/ds = -dp_corr/ds = -(dp_corr/dbeta_corr * dbeta_corr/ds)
  //       = -(dp_corr/dbeta_corr * -dbeta/ds)
  //       = dp_corr/dbeta_corr * dbeta/ds

  bool apply_correction; size_t i, j, num_kappa;
  Real kterm, dpsi_m_beta_dbeta, beta_corr; RealVector kappa;
  if (curvatureDataAvailable) {

    //if (!kappaUpdated) { // should already be up to date
    //  principal_curvatures(mpp_u, fn_grad_u, fn_hess_u, kappaU);
    //  kappaUpdated = true;
    //}

    scale_curvature(beta, cdf_flag, kappaU, kappa);
    beta_corr = (beta >= 0.) ? beta : -beta;

    switch (secondOrderIntType) {
    case HONG: // addtnl complexity not warranted
      Cerr << "\nError: final statistic gradients not implemented for Hong."
	   << std::endl;
      abort_handler(-1); break;
    case BREITUNG:
      kterm = beta_corr; break;
    case HOHENRACK:
      // Psi(beta) = phi(beta) / Phi(beta)
      // dPsi/dbeta = (Phi dphi/dbeta - phi^2)/Phi^2
      //   where dphi/dbeta = -beta phi
      // dPsi/dbeta = -phi (beta Phi + phi) / Phi^2
      //            = -Psi (beta + Psi)
      // --> dPsi/dbeta(-beta_corr)
      //       = -Psi(-beta_corr) (-beta_corr + Psi(-beta_corr) )
      //       =  Psi(-beta_corr) ( beta_corr - Psi(-beta_corr) )
      kterm = Pecos::NormalRandomVariable::std_pdf(-beta_corr)
	    / Pecos::NormalRandomVariable::std_cdf(-beta_corr); // psi_m_beta
      dpsi_m_beta_dbeta = kterm*(beta_corr - kterm); // orig (kterm + beta_corr)
      break;
    }

    num_kappa = numUncertainVars - 1;
    apply_correction = true;
    for (i=0; i<num_kappa; ++i)
      if (1. + kterm * kappa[i] <= curvatureThresh)
	apply_correction = false;

    if (apply_correction) {
      Real sum = 0., ktk, prod1, prod2 = 1.;
      for (i=0; i<num_kappa; ++i) {
	ktk = kterm * kappa[i];
	prod2 /= std::sqrt(1. + ktk);
	prod1 = 1.;
	for (j=0; j<num_kappa; ++j)
	  if (j != i)
	    prod1 /= std::sqrt(1. + kterm * kappa[j]);
	prod1 *= kappa[i] / 2. / std::pow(1. + ktk, 1.5);
	if (secondOrderIntType != BREITUNG)
	  prod1 *= dpsi_m_beta_dbeta;
	sum -= prod1;
      }

      // verify p_corr within (0,1) for consistency with probability()
      Real p1_corr = probability(beta_corr), p2_corr = p1_corr * prod2;
      if (p2_corr >= 0. && p2_corr <= 1.) // factor for second-order dp/ds:
	return p1_corr * sum
	  - Pecos::NormalRandomVariable::std_pdf(-beta_corr) * prod2;
    }

    // if not returned, then there was an exception
    Cerr << "\nWarning: second-order probability sensitivity bypassed.\n";
    warningBits |= 2; // second warning in output summary
  }

  return -Pecos::NormalRandomVariable::std_pdf(-beta);
}


// Converts a probability into a reliability using the inverse of the
// first-order or second-order integrations implemented in
// NonDLocalReliability::probability().
Real NonDLocalReliability::
reliability(Real p, bool cdf_flag, const RealVector& mpp_u,
	    const RealVector& fn_grad_u, const RealSymMatrix& fn_hess_u)
{
  Real beta = reliability(p); // FORM approximation

  if (integrationOrder == 2 && curvatureDataAvailable) {

    if (!kappaUpdated) {
      principal_curvatures(mpp_u, fn_grad_u, fn_hess_u, kappaU);
      kappaUpdated = true;
    }

    // NOTE: these conversions are currently done once.  It may be necessary
    // to redo them for each beta estimate (when inverting near beta = zero).
    Real beta_corr = (beta >= 0.) ? beta : -beta;
    Real p_corr    = (beta >= 0.) ? p    : 1. - p;
    RealVector kappa; scale_curvature(beta, cdf_flag, kappaU, kappa);

    // SORM correction to FORM: direct inversion of the SORM formulas is
    // infeasible due to the multiple instances of beta on the RHS, even for
    // the simplest case (Breitung).  Therefore, use Newton's method to solve
    // for beta(p) using Phi_inverse() as an initial guess.
    // > Newton's method uses reliability_residual() to compute the residual f
    //   and reliability_residual_derivative() to compute df/dbeta.
    // > Newton step is then beta -= f(beta)/[df/dbeta(beta)].
    // > Other options include using an inexact df/dbeta = phi(-beta) from
    //   FORM or using a quasi-Newton (Broyden update) or FD Newton approach.

    // evaluate residual
    Real res;
    bool terminate = reliability_residual(p_corr, beta_corr, kappa, res);

    size_t newton_iters = 0, max_iters = 20; // usually converges in ~3 iters
    bool converged = false;
    while (!terminate && !converged) {

      // evaluate derivative of residual w.r.t. beta
      Real dres_dbeta
	= reliability_residual_derivative(p_corr, beta_corr, kappa);

      // compute Newton step
      Real delta_beta;
      if (std::fabs(dres_dbeta) > DBL_MIN) {
	delta_beta = -res/dres_dbeta; // full Newton step
	// assess convergence using delta_beta, rather than residual,
	// since this should be better scaled.
	if (std::fabs(delta_beta) < convergenceTol)
	  converged = true; // but go ahead and take the step, if beneficial
      }
      else
	terminate = true;

      // Simple backtracking line search globalization
      bool reduction = false;
      size_t backtrack_iters = 0;
      while (!reduction && !terminate) { // enter loop even if converged
	Real beta_step = beta_corr + delta_beta;

	// verify that new beta_step doesn't violate safeguards.  If not,
	// evaluate residual res_step at beta_step.
	Real res_step;
	terminate = reliability_residual(p_corr, beta_step, kappa, res_step);

	if (!terminate) {
	  if ( std::fabs(res_step) < std::fabs(res) ) { // accept step
	    reduction = true;
	    beta_corr = beta_step;
	    res       = res_step;
	    //Cout << "residual = " << res << " delta = " << delta_beta
	    //     << " beta = " << beta_corr <<'\n';
	  }
	  else if (converged)
	    terminate = true; // kick out of inner while
	  else { // backtrack
	    //Cout << "Backtracking\n";
	    delta_beta /= 2.; // halve the step
	    if (backtrack_iters++ >= max_iters) {// backtrack iter must complete
	      Cerr << "\nWarning: maximum back-tracking iterations exceeded in "
		   << "second-order reliability inversion.\n";
	      warningBits |= 4; // third warning in output summary
	      terminate = true;
	    }
	  }
	}
      }
      if (++newton_iters >= max_iters && !converged) { // Newton iter completed
	Cerr << "\nWarning: maximum Newton iterations exceeded in second-order "
	     << "reliability inversion.\n";
	warningBits |= 8; // fourth warning in output summary
	terminate = true;
      }
    }
    return (beta >= 0.) ? beta_corr : -beta_corr;
  }
  return beta;
}


bool NonDLocalReliability::
reliability_residual(const Real& p, const Real& beta,
		     const RealVector& kappa, Real& res)
{
  int i, num_kappa = numUncertainVars - 1;

  // Test for numerical exceptions in sqrt.  Problematic kappa are large and
  // negative (kterm is always positive).  Skipping individual kappa means
  // neglecting a primary curvature and including secondary curvatures, which
  // may be counter-productive (potentially less accurate than FORM).  Since
  // the Newton solve can be problematic on its own, skip the entire solve in
  // this case.
  Real psi_m_beta;
  if (secondOrderIntType != BREITUNG)
    psi_m_beta = Pecos::NormalRandomVariable::std_pdf(-beta)
               / Pecos::NormalRandomVariable::std_cdf(-beta);
  Real kterm = (secondOrderIntType == BREITUNG) ? beta : psi_m_beta;
  for (i=0; i<num_kappa; i++)
    if (1. + kterm * kappa[i] <= curvatureThresh) {
      Cerr << "\nWarning: second-order probability integration bypassed due to "
	   << "numerical issues.\n";
      warningBits |= 2; // second warning in output summary
      return true;
    }

  // evaluate residual of f(beta,p) = 0 where p is a prescribed constant:
  //   Breitung: f = 0 = p * Prod_i(sqrt(1+beta*kappa)) - Phi(-beta)
  //   HohRack:  f = 0 = p * Prod_i(sqrt(1+psi(-beta)*kappa)) - Phi(-beta)
  //   Hong:     f = 0 = p * Prod_i(sqrt(1+psi(-beta)*kappa)) - C1*Phi(-beta)
  Real prod = 1., ktk, C1 = 0.;
  for (i=0; i<num_kappa; i++) {
    ktk = kterm * kappa[i];
    prod *= std::sqrt(1. + ktk);
    if (secondOrderIntType == HONG) {
      Real hterm = num_kappa * kappa[i] / 2. / (1. + ktk);
      C1 += Pecos::NormalRandomVariable::std_cdf(-beta - hterm)
	 /  Pecos::NormalRandomVariable::std_cdf(-beta)
	 *  exp(psi_m_beta * hterm);
    }
  }
  if (secondOrderIntType == HONG)
    res = p * prod - C1 * Pecos::NormalRandomVariable::std_cdf(-beta);
  else
    res = p * prod - Pecos::NormalRandomVariable::std_cdf(-beta);

  return false;
}


Real NonDLocalReliability::
reliability_residual_derivative(const Real& p, const Real& beta,
				const RealVector& kappa)
{
  int i, j, num_kappa = numUncertainVars - 1;
  Real psi_m_beta, dpsi_m_beta_dbeta;
  if (secondOrderIntType != BREITUNG) {
    psi_m_beta = Pecos::NormalRandomVariable::std_pdf(-beta)
               / Pecos::NormalRandomVariable::std_cdf(-beta);
    dpsi_m_beta_dbeta = psi_m_beta*(beta + psi_m_beta);
  }

  // evaluate derivative of residual w.r.t. beta
  Real prod, dres_dbeta, sum = 0.;
  Real kterm = (secondOrderIntType == BREITUNG) ? beta : psi_m_beta;
  for (i=0; i<num_kappa; i++) {
    prod = 1.;
    for (j=0; j<num_kappa; j++)
      if (j != i)
	prod *= std::sqrt(1. + kterm*kappa[j]);
    prod *= kappa[i]/2./std::sqrt(1. + kterm*kappa[i]);
    if (secondOrderIntType != BREITUNG)
      prod *= dpsi_m_beta_dbeta;
    sum += prod;
  }
  if (secondOrderIntType == HONG) { // addtnl complexity may not be warranted
    Cerr << "\nError: reliability residual derivative not implemented for Hong."
	 << std::endl;
    abort_handler(-1);
    //dres_dbeta = p * sum + C1 * phi(-beta) - Phi(-beta) * dC1_dbeta;
  }
  else
    dres_dbeta = p * sum + Pecos::NormalRandomVariable::std_pdf(-beta);

  return dres_dbeta;
}


void NonDLocalReliability::
principal_curvatures(const RealVector& mpp_u, const RealVector& fn_grad_u,
		     const RealSymMatrix& fn_hess_u, RealVector& kappa_u)
{
  // fn_grad_u, fn_hess_u, and possibly mpp_u (alternate R0 initialization)
  // must be in synch and be a reasonable approximation to converged MPP data.

  // compute R matrix
  int i, j, k, num_vars = mpp_u.length(), num_kappa = num_vars - 1;
  RealMatrix R0(num_vars, num_vars);// init to 0
  // initialize R0: last row values are direction cosines
  for (i=0; i<num_kappa; i++)
    R0(i, i) = 1.;
  // Haldar & Mahadevan, p.227: last row = unit gradient vector of limit state
  Real norm_grad_u = fn_grad_u.normFrobenius();
  if (norm_grad_u > DBL_MIN)
    for (i=0; i<num_vars; i++)
      R0(num_kappa, i) = fn_grad_u[i]/norm_grad_u;
  else { // fallback: try to use +/- mPPU[i]/norm(mPPU)
    Real norm_mpp_u = mpp_u.normFrobenius(); // unsigned beta
    if (norm_mpp_u > DBL_MIN) {
      // Can match the sign of fn_grad_u[i]/norm_grad_u in PMA case (align for
      // max G, oppose for min G), but can't in general in RIA case (vectors
      // may align or oppose).  Fortunately, the R0 sign does not appear to
      // matter since R is applied twice in R fn_hess_u R^T.
      bool pma_max = (levelCount >= requestedRespLevels[respFnCount].length() &&
		      pmaMaximizeG) ? true : false;
      for (i=0; i<num_vars; i++)
	R0(num_kappa, i) = (pma_max) ?  mpp_u[i]/norm_mpp_u  // aligned
	                             : -mpp_u[i]/norm_mpp_u; // opposed
    }
    else {
      // Note: for Breitung, kappa_i do not matter if beta = 0.
      Cerr << "\nError: unable to initialize R0 in principal_curvatures() "
	   << "calculation." << std::endl;
      abort_handler(-1);
    }
  }
  //Cout << "\nR0:" << R0;

  // orthogonalize using Gram-Schmidt
  RealMatrix R(R0);
  for (i=num_vars-2; i>=0; i--) {  // update the ith row vector
    for (j=i+1; j<num_vars; j++) { // orthogonalize to jth row vector
      Real scale1 = 0., scale2 = 0.;
      for (k=0; k<num_vars; k++) { // kth column
	scale1 += R(j, k) * R0(i, k);
	scale2 += std::pow(R(j, k), 2);
      }
      Real scale = scale1 / scale2;
      for (k=0; k<num_vars; k++)   // kth column
	R(i, k) -= scale * R(j, k);
    }
    // renormalize ith row vector to unit length
    Real len = 0.;
    for (j=0; j<num_vars; j++)
      len += std::pow(R(i, j), 2);
    len = std::sqrt(len);
    for (j=0; j<num_vars; j++)
      R(i, j) /= len;
  }
  //Cout << "\nR:" << R;

  // compute A matrix = (R fn_hess_u R^T)/norm(fn_grad_u)
  RealSymMatrix A(num_vars, false);
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1./norm_grad_u, fn_hess_u,
			       R, A);
  //Cout << "\nA:" << A;
  A.reshape(num_kappa); // upper left portion of matrix
  //Cout << "\nReshaped A:" << A;

  // compute eigenvalues of A --> principal curvatures
  Teuchos::LAPACK<int, Real> la;
  int info, lwork = 3*num_kappa - 1;
  double* work = new double [lwork];
  // LAPACK eigenvalue solution for real, symmetric A
  if (kappa_u.length() != num_kappa)
    kappa_u.sizeUninitialized(num_kappa);
  la.SYEV('N', A.UPLO(), num_kappa, A.values(), A.stride(), kappa_u.values(),
	  work, lwork, &info);
  delete [] work;
  if (info) {
    Cerr << "\nError: internal error in LAPACK eigenvalue routine."
         << std::endl;
    abort_handler(-1);
  }
  //Cout << "\nkappa_u:" << kappa_u;
}


void NonDLocalReliability::print_results(std::ostream& s)
{
  size_t i, j, k, cntr, width = write_precision+7;
  StringMultiArrayConstView uv_labels
    = iteratedModel.continuous_variable_labels();
  const StringArray& fn_labels = iteratedModel.response_labels();
  s << "-----------------------------------------------------------------\n";

  if (warningBits) {
    s << "Warnings accumulated during solution for one or more levels:\n";
    if (warningBits & 1)
      s << "  Maximum number of limit state approximation cycles exceeded.\n";
    if (warningBits & 2)
      s << "  Second-order probability integration bypassed due to numerical "
	<< "issues.\n";
    if (warningBits & 4)
      s << "  Maximum back-tracking iterations exceeded in second-order "
	<< "reliability inversion.\n";
    if (warningBits & 8)
      s << "  Maximum Newton iterations exceeded in second-order reliability "
	<< "inversion.\n";
    s << "Please interpret results with care.\n";
    s << "-----------------------------------------------------------------\n";
  }

  // output MV-specific statistics
  if (!mppSearchType) {
    Real std_dev, *imp_fact_i;
    for (i=0; i<numFunctions; i++) {
      s << "MV Statistics for " << fn_labels[i] << ":\n";
      // approximate response means and std deviations and importance factors
      std_dev = (finalMomentsType == CENTRAL_MOMENTS) ?
	std::sqrt(finalMomentStats(1,i)) : finalMomentStats(1,i);
      s << "  Approximate Mean Response                  = "
	<< std::setw(width) << finalMomentStats(0,i)
	<< "\n  Approximate Standard Deviation of Response = "
	<< std::setw(width)<< std_dev << '\n';
      if (std_dev < Pecos::SMALL_NUMBER)
	s << "  Importance Factors not available.\n";
      else {
	imp_fact_i = impFactor[i];
	for (j=0; j<numUncertainVars; j++)
	  s << "  Importance Factor for " << std::setiosflags(std::ios::left)
	    << std::setw(20) << uv_labels[j].data() << " = "
	    << std::resetiosflags(std::ios::adjustfield)
	    << std::setw(width) << imp_fact_i[j] << '\n';

	if (natafTransform.x_correlation())
	  for (j=0, cntr=numUncertainVars; j<numUncertainVars; ++j)
	    for (k=0; k<j; ++k, ++cntr)
	      s << "  Importance Factor for "
		<< std::setiosflags(std::ios::left) << std::setw(10)
		<< uv_labels[k].data() << std::setw(10) << uv_labels[j].data()
		<< " = " << std::resetiosflags(std::ios::adjustfield)
		<< std::setw(width) << imp_fact_i[cntr] << '\n';
      }
    }
  }

  // output PDFs (defined if pdfOutput and integrationRefinement)
  print_densities(s);

  // output CDF/CCDF level mappings (replaces NonD::print_level_mappings())
  for (i=0; i<numFunctions; i++) {

    size_t num_levels = computedRespLevels[i].length();
    if (num_levels) {
      Real std_dev = (finalMomentsType == CENTRAL_MOMENTS) ?
	std::sqrt(finalMomentStats(1,i)) : finalMomentStats(1,i);
      if (!mppSearchType && std_dev < Pecos::SMALL_NUMBER)
        s << "\nWarning: negligible standard deviation renders CDF results "
          << "suspect.\n\n";
      if (cdfFlag)
        s << "Cumulative Distribution Function (CDF) for ";
      else
        s << "Complementary Cumulative Distribution Function (CCDF) for ";

      s << fn_labels[i] << ":\n     Response Level  Probability Level  "
	<< "Reliability Index  General Rel Index\n     --------------  "
	<< "-----------------  -----------------  -----------------\n";
      for (j=0; j<num_levels; j++)
        s << "  " << std::setw(width) << computedRespLevels[i][j]
	  << "  " << std::setw(width) << computedProbLevels[i][j]
	  << "  " << std::setw(width) << computedRelLevels[i][j]
	  << "  " << std::setw(width) << computedGenRelLevels[i][j] << '\n';
    }
  }

  s << "-----------------------------------------------------------------"
    << std::endl;
}


void NonDLocalReliability::method_recourse()
{
  Cerr << "\nWarning: method recourse invoked in NonDLocalReliability due to "
       << "detected method conflict.\n\n";
  if (mppSearchType && npsolFlag) {
#ifdef HAVE_OPTPP
    mppOptimizer.assign_rep(
      new SNLLOptimizer("optpp_q_newton", mppModel), false);
#else
    Cerr << "\nError: method recourse not possible in NonDLocalReliability "
	 << "(OPT++ NIP unavailable).\n";
    abort_handler(-1);
#endif
    npsolFlag = false;
  }
}

} // namespace Dakota
