/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalReliability
//- Description: Implementation code for NonDLocalReliability class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "system_defs.h"
#include "DakotaResponse.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "ProblemDescDB.H"
#include "DakotaGraphics.H"
#include "NonDLocalReliability.H"
#include "NonDAdaptImpSampling.H"
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.H"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.H"
using OPTPP::NLPFunction;
using OPTPP::NLPGradient;
#endif
#include "RecastModel.H"
#include "DataFitSurrModel.H"
#include "NestedModel.H"
#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include <algorithm>
#include "data_io.h"
#include "pecos_stat_util.hpp"
using Pecos::phi;
using Pecos::Phi;
using Pecos::Phi_inverse;

//#define MPP_CONVERGE_RATE

static const char rcsId[] = "@(#) $Id: NonDLocalReliability.C 4058 2006-10-25 01:39:40Z mseldre $";

namespace Dakota {

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2


NonDLocalReliability::NonDLocalReliability(Model& model):
  NonDReliability(model), warmStartFlag(true), nipModeOverrideFlag(true),
  curvatureDataAvailable(false), secondOrderIntType(HOHENRACK),
  curvatureThresh(1.e-10), warningBits(0)
{
  // check for suitable gradient and variables specifications
  if ( gradientType == "none" ) {
    Cerr << "\nError: local_reliability requires a gradient specification."
	 << std::endl;
    abort_handler(-1);
  }
  if (numIntervalVars) {
    Cerr << "Error: interval distributions are not supported in local "
	 << "reliability methods." << std::endl;
    abort_handler(-1);
  }

  // Map MPP search user specification into mppSearchType
  const String& mpp_search_type
    = probDescDB.get_string("method.nond.reliability_search_type");
  if (mpp_search_type == "mv")
    mppSearchType = MV;
  else if (mpp_search_type == "amv_x")
    mppSearchType = AMV_X;
  else if (mpp_search_type == "amv_u")
    mppSearchType = AMV_U;
  else if (mpp_search_type == "amv_plus_x")
    mppSearchType = AMV_PLUS_X;
  else if (mpp_search_type == "amv_plus_u")
    mppSearchType = AMV_PLUS_U;
  else if (mpp_search_type == "tana_x")
    mppSearchType = TANA_X;
  else if (mpp_search_type == "tana_u")
    mppSearchType = TANA_U;
  else if (mpp_search_type == "no_approx")
    mppSearchType = NO_APPROX;
  else {
    Cerr << "Error: bad mpp_search type in local_reliability." << std::endl;
    abort_handler(-1); 
  }

  if (mppSearchType) {

    // Map MPP search NIP/SQP algorithm specification into an NPSOL/OPT++
    // selection based on configuration availability.
#if !defined(HAVE_NPSOL) && !defined(HAVE_OPTPP)
    Cerr << "Error: this executable not configured with NPSOL or OPT++.\n"
	 << "       NonDLocalReliability cannot perform MPP search."
         << std::endl;
    abort_handler(-1);
#endif
    const String& mpp_search_alg
      = probDescDB.get_string("method.nond.optimization_algorithm");
    if (mpp_search_alg == "sqp") {
#ifdef HAVE_NPSOL
      npsolFlag = true;
#else
      Cerr << "\nError: this executable not configured with NPSOL SQP.\n"
	   << "         Please select OPT++ NIP within local_reliability."
	   << std::endl;
      abort_handler(-1);
#endif
    }
    else if (mpp_search_alg == "nip") {
#ifdef HAVE_OPTPP
      npsolFlag = false;
#else
      Cerr << "\nError: this executable not configured with OPT++ NIP.\n"
	   << "         please select NPSOL SQP within local_reliability."
	   << std::endl;
      abort_handler(-1);
#endif
    }
    else if (mpp_search_alg.empty()) {
#ifdef HAVE_NPSOL
      npsolFlag = true;
#elif HAVE_OPTPP
      npsolFlag = false;
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
	 ( sub_iterator.method_name().ends("sol_sqp") ||
	   sub_iterator.uses_method().ends("sol_sqp") ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	   ( sub_iterator.method_name().ends("sol_sqp") ||
	     sub_iterator.uses_method().ends("sol_sqp") ) )
	sub_iterator.method_recourse();
    }
  }

  // Map response Hessian specification into taylorOrder for use by MV/AMV/AMV+
  // variants.  Note that taylorOrder and integrationOrder are independent
  // (although the Hessian specification required for 2nd-order integration
  // means that taylorOrder = 2 will be used for MV/AMV/AMV+; taylorOrder = 2
  // may however be used with 1st-order integration).
  taylorOrder = (hessianType != "none" && mppSearchType <= AMV_PLUS_U) ? 2 : 1;

  // assign iterator-specific defaults for approximation-based MPP searches
  if (maxIterations <  0          && // DataMethod default = -1
      mppSearchType >= AMV_PLUS_X && mppSearchType < NO_APPROX) // approx-based
    maxIterations = 25;

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
  if (mppSearchType ==  AMV_X || mppSearchType == AMV_PLUS_X ||
      mppSearchType == TANA_X) { // Recast( DataFit( iteratedModel ) )

    // Construct g-hat(x) using a local/multipoint approximation over the
    // uncertain variables (using the same view as iteratedModel).
    Model g_hat_x_model;
    String approx_type = (mppSearchType == TANA_X) ?
      "multipoint_tana" : "local_taylor";
    String sample_reuse, corr_type;
    UShortArray approx_order(1, taylorOrder);
    short corr_order = -1, data_order = (taylorOrder == 2) ? 7 : 3;
    int samples = 0, seed = 0;
    Iterator dace_iterator;
    //const Variables& curr_vars = iteratedModel.current_variables();
    g_hat_x_model.assign_rep(new DataFitSurrModel(dace_iterator, iteratedModel,
      //curr_vars.view(), curr_vars.variables_components(),
      //iteratedModel.current_response().active_set(),
      approx_type, approx_order, corr_type, corr_order, data_order,
      sample_reuse), false);

    // transform g_hat_x_model from x-space to u-space
    construct_u_space_model(g_hat_x_model, uSpaceModel, true);//globally bounded
  }
  else if (mppSearchType ==  AMV_U || mppSearchType == AMV_PLUS_U ||
	   mppSearchType == TANA_U) { // DataFit( Recast( iteratedModel ) )

    // Recast g(x) to G(u)
    Model g_u_model;
    construct_u_space_model(iteratedModel, g_u_model, true); // globally bounded

    // Construct G-hat(u) using a local/multipoint approximation over the
    // uncertain variables (using the same view as iteratedModel/g_u_model).
    String approx_type = (mppSearchType == TANA_U) ?
      "multipoint_tana" : "local_taylor";
    String sample_reuse, corr_type;
    UShortArray approx_order(1, taylorOrder);
    short corr_order = -1, data_order = (taylorOrder == 2) ? 7 : 3;
    int samples = 0, seed = 0;
    Iterator dace_iterator;
    //const Variables& g_u_vars = g_u_model.current_variables();
    uSpaceModel.assign_rep(new DataFitSurrModel(dace_iterator, g_u_model,
      //g_u_vars.view(), g_u_vars.variables_components(),
      //g_u_model.current_response().active_set(),
      approx_type, approx_order, corr_type, corr_order, data_order,
      sample_reuse), false);
  }
  else if (mppSearchType == NO_APPROX) // Recast( iteratedModel )
    // Recast g(x) to G(u)
    construct_u_space_model(iteratedModel, uSpaceModel, true);//globally bounded

  // configure a RecastModel with one objective and one equality constraint
  // using the alternate minimalist constructor
  if (mppSearchType) {
    mppModel.assign_rep(new RecastModel(uSpaceModel, 1, 1, 0), false);
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

    // Strategy::init_communicators() initializes the parallel configuration 
    // for NonDLocalReliability + iteratedModel using maxConcurrency (defined
    // from the derivative concurrency in the responses specification).  For
    // FORM/SORM, the NPSOL/OPT++ concurrency is the same, but for approximate
    // methods, the concurrency is dictated by the gradType/hessType logic in
    // the instantiate on-the-fly DataFitSurrModel constructor.
    uSpaceModel.init_communicators(maxConcurrency);
    mppModel.init_communicators(mppOptimizer.maximum_concurrency());
  }

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
    if (hessianType == "none") {
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

  const String& integration_refine
    = probDescDB.get_string("method.nond.integration_refinement");
  if (!integration_refine.empty()) {
    for (size_t i=0; i<numFunctions; i++) {
      if (!requestedProbLevels[i].empty() || !requestedRelLevels[i].empty() ||
	  !requestedGenRelLevels[i].empty()) {
	Cerr << "\nError: importance sampling methods only supported for RIA."
	     << "\n\n";
	abort_handler(-1);
      }
    }
    if (integration_refine == "is")
      integrationRefinement = IS;
    else if (integration_refine == "ais")
      integrationRefinement = AIS;
    else if (integration_refine == "mmais")
      integrationRefinement = MMAIS;

    // integration refinement requires an MPP, but it may be unconverged (AMV)
    if (mppSearchType) {
      // For NonDLocal, integration refinement is applied to the original model
      int refinement_samples = probDescDB.get_int("method.samples"),
	  refinement_seed    = probDescDB.get_int("method.random_seed");
      String sample_type, rng; // empty strings: use defaults

      // flags control if/when transformation is needed in importanceSampler
      bool x_data_flag = false, x_model_flag = true, bounded_model = false;

      importanceSampler.assign_rep(
	new NonDAdaptImpSampling(iteratedModel, sample_type, refinement_samples,
	refinement_seed, rng, integrationRefinement, cdfFlag, x_data_flag,
	x_model_flag, bounded_model), false);

      iteratedModel.init_communicators(importanceSampler.maximum_concurrency());
    }
    else {
      Cerr << "\nError: integration refinement only supported for MPP methods."
	   << std::endl;
      abort_handler(-1);
    }
  }

  // Size the output arrays.  Relative to sampling methods, the output storage
  // for reliability methods is more substantial since there may be differences
  // between requested and computed levels for the same measure (the request is
  // not always achieved) and since probability and reliability are carried
  // along in parallel (due to their direct correspondence).
  computedRelLevels.resize(numFunctions);
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
  initialPtU.sizeUninitialized(numUncertainVars);
  mostProbPointX.sizeUninitialized(numUncertainVars);
  mostProbPointU.sizeUninitialized(numUncertainVars);
  fnGradX.sizeUninitialized(numUncertainVars);
  fnGradU.sizeUninitialized(numUncertainVars);
  if (taylorOrder == 2 || integrationOrder == 2) {
    fnHessX.shapeUninitialized(numUncertainVars);
    fnHessU.shapeUninitialized(numUncertainVars);
    if (hessianType == "quasi") {
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
{
  if (mppSearchType) {
    mppModel.free_communicators(mppOptimizer.maximum_concurrency());
    uSpaceModel.free_communicators(maxConcurrency);
    if (integrationRefinement)
      iteratedModel.free_communicators(importanceSampler.maximum_concurrency());
  }
}


void NonDLocalReliability::quantify_uncertainty()
{
  initialize_random_variable_parameters();
  initial_taylor_series();
  if (mppSearchType)
    mpp_search();
  else
    mean_value();

  numRelAnalyses++;
}


/** An initial first- or second-order Taylor-series approximation is
    required for MV/AMV/AMV+/TANA or for the case where meanStats or
    stdDevStats (from MV) are required within finalStatistics for
    subIterator usage of NonDLocalReliability. */
void NonDLocalReliability::initial_taylor_series()
{
  bool init_ts_flag = (mppSearchType < NO_APPROX) ? true : false;
  size_t i, j, k;
  ShortArray asrv(numFunctions, 0);
  short mode = 3;
  if (taylorOrder == 2 && hessianType != "quasi") // no data yet in quasiHess
    mode |= 4;

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  switch (mppSearchType) {
  case MV:
    asrv.assign(asrv.size(), mode);
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
    if (subIteratorFlag) {
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

  if (init_ts_flag) {
    bool correlation_flag = natafTransform.x_correlation();
    // Evaluate response values/gradients at the mean values of the uncertain
    // vars for the (initial) Taylor series expansion in MV/AMV/AMV+.
    Cout << "\n>>>>> Evaluating response at mean values\n";
    if (mppSearchType > MV && mppSearchType < NO_APPROX)
      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    iteratedModel.continuous_variables(natafTransform.x_means());
    activeSet.request_vector(asrv);
    iteratedModel.compute_response(activeSet);
    const Response& local_response = iteratedModel.current_response();
    fnValsMeanX = local_response.function_values();
    fnGradsMeanX = local_response.function_gradients();
    if (mode & 4) {
      fnHessiansMeanX.resize( local_response.function_hessians().size() );
      std::copy( local_response.function_hessians().begin(),
                 local_response.function_hessians().end(),
                 fnHessiansMeanX.begin() );
    }

    // compute the covariance matrix from the correlation matrix
    RealSymMatrix covariance;
    const Pecos::RealVector& x_std_devs = natafTransform.x_std_deviations();
    if (correlation_flag) {
      covariance.shapeUninitialized(numUncertainVars);
      const Pecos::RealSymMatrix& x_corr_mat
	= natafTransform.x_correlation_matrix();
      for (i=0; i<numUncertainVars; i++) {
	for (j=0; j<=i; j++) {
	  covariance(i,j) = x_std_devs[i]*x_std_devs[j]*x_corr_mat(i,j);
	  //if (i != j)
	  //  covariance(j,i) = covariance(i,j);
	}
      }
    }
    else {
      covariance.shape(numUncertainVars); // inits to 0
      for (i=0; i<numUncertainVars; i++)
	covariance(i,i) = std::pow(x_std_devs[i], 2);
    }

    // MVFOSM computes a first-order mean, which is just the response evaluated
    // at the input variable means.  If Hessian data is available, compute a
    // second-order mean including the effect of input variable correlations.
    copy_data(fnValsMeanX, meanStats);                // first-order mean
    if (taylorOrder == 2 && hessianType != "quasi") { // second-order mean
      for (i=0; i<numFunctions; i++) {
	if (asrv[i]) {
	  Real val = 0.;
	  for (j=0; j<numUncertainVars; j++) {
	    if (correlation_flag)
	      for (k=0; k<numUncertainVars; k++)
		val += covariance(j,k)*fnHessiansMeanX[i](j,k);
	    else
	      val += covariance(j,j)*fnHessiansMeanX[i](j,j);
	  }
	  meanStats[i] += val/2.;
	}
      }
    }

    // MVFOSM computes a first-order variance including the effect of input
    // variable correlations.  Second-order variance requires skewness/kurtosis
    // of the inputs and is not practical.  NOTE: if fnGradsMeanX is zero, then
    // stdDevStats will be zero --> bad for MV CDF estimates.
    stdDevStats.size(numFunctions);
    for (i=0; i<numFunctions; i++) {
      if (asrv[i]) {
	Real val = 0.;
	for (j=0; j<numUncertainVars; j++) {
	  if (correlation_flag)
	    for (k=0; k<numUncertainVars; k++)
	      val += covariance(j,k) * fnGradsMeanX(j,i) * fnGradsMeanX(k,i);
	  else
	    val += covariance(j,j) * std::pow(fnGradsMeanX(j,i), 2);
	}
	stdDevStats[i] = std::sqrt(val);
      }
      else
	stdDevStats[i] = 0.;
    }
    // Teuchos/BLAS-based approach.  As a matrix triple-product, this has some
    // unneeded FLOPs.  A vector-matrix triple product would be preferable, but
    // requires vector extractions from fnGradsMeanX.
    //RealSymMatrix variance(numFunctions, false);
    //Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1., covariance,
    //                             fnGradsMeanX, variance);
    //for (i=0; i<numFunctions; i++)
    //  stdDevStats[i] = sqrt(variance(i,i));
    //Cout << "\nvariance = " << variance << "\nstdDevStats = " << stdDevStats;
  }
  else {
    meanStats.size(numFunctions);   // init to 0
    stdDevStats.size(numFunctions); // init to 0
  }
}


void NonDLocalReliability::mean_value()
{
  // For MV, compute approximate mean, standard deviation, and requested
  // CDF/CCDF data points for each response function and store in 
  // finalStatistics.  Additionally, if uncorrelated variables, compute
  // importance factors.

  extern Graphics dakota_graphics; // defined in ParallelLibrary.C

  // initialize arrays
  impFactor.shapeUninitialized(numUncertainVars, numFunctions);
  statCount = 0;
  initialize_final_statistics_gradients();

  // loop over response functions
  size_t i;
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {

    // approximate response means already computed
    finalStatistics.function_value(meanStats[respFnCount], statCount);
    // sensitivity of response mean
    if (final_asv[statCount] & 2) {
      RealVector fn_grad_mean_x(numUncertainVars, false);
      for (i=0; i<numUncertainVars; i++)
	fn_grad_mean_x[i] = fnGradsMeanX(i,respFnCount);
      // evaluate dg/ds at the variable means and store in finalStatistics
      RealVector final_stat_grad;
      dg_ds_eval(natafTransform.x_means(), fn_grad_mean_x, final_stat_grad);
      finalStatistics.function_gradient(final_stat_grad, statCount);
    }
    statCount++;

    // approximate response std deviations already computed
    finalStatistics.function_value(stdDevStats[respFnCount], statCount);
    // sensitivity of response std deviation
    if (final_asv[statCount] & 2) {
      // Differentiating the first-order second-moment expression leads to
      // 2nd-order d^2g/dxds sensitivities which would be awkward to compute
      // (nonstandard DVV containing active and inactive vars)
      Cerr << "Error: response std deviation sensitivity not yet supported."
           << std::endl;
      abort_handler(-1);
    }
    statCount++;

    // if inputs are uncorrelated, compute importance factors
    if (!natafTransform.x_correlation() && stdDevStats[respFnCount] > 1.e-25) {
      const Pecos::RealVector& x_std_devs = natafTransform.x_std_deviations();
      for (i=0; i<numUncertainVars; i++)
        impFactor(i,respFnCount) = std::pow(x_std_devs[i] /
	  stdDevStats[respFnCount] * fnGradsMeanX(i,respFnCount), 2);
    }

    // compute probability/reliability levels for requested response levels and
    // compute response levels for requested probability/reliability levels.
    // For failure defined as g<0, beta is simply mean/sigma.  This is extended
    // to compute general cumulative probabilities for g<z or general
    // complementary cumulative probabilities for g>z.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           bl_len = requestedRelLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length();
    for (levelCount=0; levelCount<rl_len; levelCount++) {
      // computed = requested in MV case since no validation fn evals
      Real z = computedRespLevels[respFnCount][levelCount]
	= requestedRespLevels[respFnCount][levelCount];
      // compute beta and p from z
      Real beta, p;
      if (stdDevStats[respFnCount] > 1.e-25) {
	Real ratio = (meanStats[respFnCount] - z)/stdDevStats[respFnCount];
        beta = computedRelLevels[respFnCount][levelCount]
	  = computedGenRelLevels[respFnCount][levelCount]
	  = (cdfFlag) ? ratio : -ratio;
        p = computedProbLevels[respFnCount][levelCount]
	  = probability(beta, cdfFlag);
      }
      else {
        if ( ( cdfFlag && meanStats[respFnCount] <= z) ||
	     (!cdfFlag && meanStats[respFnCount] >  z) ) {
          beta = computedRelLevels[respFnCount][levelCount]
	    = computedGenRelLevels[respFnCount][levelCount] = -1.e50;
          p = computedProbLevels[respFnCount][levelCount] = 1.;
	}
	else {
          beta = computedRelLevels[respFnCount][levelCount]
	    = computedGenRelLevels[respFnCount][levelCount] = 1.e50;
          p = computedProbLevels[respFnCount][levelCount] = 0.;
	}
      }
      switch (respLevelTarget) {
      case PROBABILITIES:
	finalStatistics.function_value(p, statCount);    break;
      case RELIABILITIES: case GEN_RELIABILITIES:
	finalStatistics.function_value(beta, statCount); break;
      }
      if (final_asv[statCount] & 2) {
	Cerr << "Error: response probability/reliability/gen_reliability level "
	     << "sensitivity not supported for Mean Value." << std::endl;
	abort_handler(-1);
      }
      statCount++;
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
	= computedGenRelLevels[respFnCount][levelCount]
	= reliability(p, cdfFlag);
      Real z = computedRespLevels[respFnCount][levelCount] = (cdfFlag)
        ? meanStats[respFnCount] - beta * stdDevStats[respFnCount]
        : meanStats[respFnCount] + beta * stdDevStats[respFnCount];
      finalStatistics.function_value(z, statCount);
      if (final_asv[statCount] & 2) {
	Cerr << "Error: response level sensitivity not supported for Mean "
	     << "Value." << std::endl;
	abort_handler(-1);
      }
      statCount++;
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
      Real p = computedProbLevels[respFnCount][levelCount]
	= probability(beta, cdfFlag);
      Real z = computedRespLevels[respFnCount][levelCount] = (cdfFlag)
        ? meanStats[respFnCount] - beta * stdDevStats[respFnCount]
	: meanStats[respFnCount] + beta * stdDevStats[respFnCount];
      finalStatistics.function_value(z, statCount);
      if (final_asv[statCount] & 2) {
	Cerr << "Error: response level sensitivity not supported for Mean "
	     << "Value." << std::endl;
	abort_handler(-1);
      }
      statCount++;
      // Update specialty graphics
      if (!subIteratorFlag)
	dakota_graphics.add_datapoint(respFnCount, z, p);
    }
  }
}


void NonDLocalReliability::mpp_search()
{
  // set the object instance pointer for use within the static member fns
  NonDReliability* prev_instance = nondRelInstance;
  nondRelInstance = this;

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
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {

    // approximate response means already computed
    finalStatistics.function_value(meanStats[respFnCount], statCount);
    // sensitivity of response mean
    if (final_asv[statCount] & 2) {
      RealVector fn_grad_mean_x(numUncertainVars, false);
      for (i=0; i<numUncertainVars; i++)
	fn_grad_mean_x[i] = fnGradsMeanX(i,respFnCount);
      // evaluate dg/ds at the variable means and store in finalStatistics
      RealVector final_stat_grad;
      dg_ds_eval(natafTransform.x_means(), fn_grad_mean_x, final_stat_grad);
      finalStatistics.function_gradient(final_stat_grad, statCount);
    }
    statCount++;

    // approximate response std deviations already computed
    finalStatistics.function_value(stdDevStats[respFnCount], statCount);
    // sensitivity of response std deviation
    if (final_asv[statCount] & 2) {
      // Differentiating the first-order second-moment expression leads to
      // 2nd-order d^2g/dxds sensitivities which would be awkward to compute
      // (nonstandard DVV containing active and inactive vars)
      Cerr << "Error: response std deviation sensitivity not yet supported."
           << std::endl;
      abort_handler(-1);
    }
    statCount++;

    // The most general case is to allow a combination of response, probability,
    // reliability, and generalized reliability level specifications for each
    // response function.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           bl_len = requestedRelLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length(),
           num_levels = rl_len + pl_len + bl_len + gl_len;

    // Initialize (or warm-start for repeated reliability analyses) initialPtU,
    // mostProbPointX/U, computedRespLevel, fnGradX/U, and fnHessX/U.
    curvatureDataAvailable = false; // no data (yet) for this response function
    if (num_levels)
      initialize_level_data();

    // Loop over response/probability/reliability levels
    for (levelCount=0; levelCount<num_levels; levelCount++) {

      // The rl_len response levels are performed first using the RIA
      // formulation, followed by the pl_len probability levels and the
      // bl_len reliability levels using the PMA formulation.
      bool ria_flag = (levelCount < rl_len) ? true : false;
      if (ria_flag) {
        requestedRespLevel = requestedRespLevels[respFnCount][levelCount];
	Cout << "\n>>>>> Reliability Index Approach (RIA) for response level "
	     << levelCount+1 << " = " << requestedRespLevel << '\n';
      }
      else if (levelCount < rl_len + pl_len) { 
	size_t index = levelCount - rl_len;
	const Real& p = requestedProbLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for probability "
	     << "level " << index + 1 << " = " << p << '\n';
	requestedCDFProbLevel = (cdfFlag) ? p : 1. - p;
        requestedCDFRelLevel  = reliability(requestedCDFProbLevel, true);
      }
      else if (levelCount < rl_len + pl_len + bl_len) {
	size_t index = levelCount - rl_len - pl_len;
	const Real& rel = requestedRelLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for reliability "
	     << "level " << index + 1 << " = " << rel << '\n';
	requestedCDFRelLevel = (cdfFlag) ? rel : -rel;
      }
      else {
	size_t index = levelCount - rl_len - pl_len - bl_len;
	const Real& gen_rel = requestedGenRelLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for generalized "
	     << "reliability level " << index + 1 << " = " << gen_rel << '\n';
	requestedCDFProbLevel = (cdfFlag) ? Phi(-gen_rel) : Phi(gen_rel);
	requestedCDFRelLevel  = reliability(requestedCDFProbLevel, true);
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
	  mpp_model_rep->initialize(vars_map, false, NULL, NULL,
	    primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	    RIA_objective_eval, RIA_constraint_eval);
	}
	else { // PMA: g is in objective

	  // If PMA SORM with p-level or generalized beta-level,
	  // requestedCDFRelLevel must be updated using current grad/Hessian.
	  void (*set_map) (const Variables& recast_vars,
			   const ActiveSet& recast_set,
			   ActiveSet& sub_model_set) =
	    ( mppSearchType == NO_APPROX && integrationOrder == 2 &&
	      ( levelCount <  rl_len + pl_len ||
		levelCount >= rl_len + pl_len + bl_len ) ) ?
	    PMA2_set_mapping : NULL;

	  primary_resp_map.resize(1);   // one objective, one contributor
	  primary_resp_map[0].resize(1);
	  primary_resp_map[0][0] = respFnCount;
	  secondary_resp_map.resize(1); // one constraint, no contributors
	  nonlinear_resp_map[0] = BoolDeque(1, false);
	  mpp_model_rep->initialize(vars_map, false, NULL, set_map,
	    primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	    PMA_objective_eval, PMA_constraint_eval);
	}
	mppModel.continuous_variables(initialPtU);

        // Execute MPP search and retrieve u-space results
        Cout << "\n>>>>> Initiating search for most probable point (MPP)\n";
	// no summary output since on-the-fly constructed:
	mppOptimizer.run_iterator(Cout);
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
	  if (requestedCDFRelLevel < 0.)
	    Cout << '-';
	  Cout << "G(u)]\n                     " << std::setw(write_precision+7)
	       << fns_star[1] << " [u'u - B^2]\n";
	}

	// Update MPP search data
	update_mpp_search_data(vars_star, resp_star);

      } // end AMV+ while loop

      // Update response/probability/reliability level data
      update_level_data();

      statCount++;
    } // end loop over levels
  } // end loop over response fns

  // Update warm-start data
  if (warmStartFlag && subIteratorFlag) // view->copy
    copy_data(iteratedModel.inactive_continuous_variables(), prevICVars);

  // restore in case of recursion
  nondRelInstance = prev_instance;
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
    prevCumASVLev0.resize(numFunctions);
    prevCumASVLev0.assign(numFunctions, 0);
    prevFnGradDLev0.shape(num_final_grad_vars, numFunctions);
    prevFnGradULev0.shape(numUncertainVars, numFunctions);
  }

  // Modify the correlation matrix (Nataf) and compute its Cholesky factor.
  // Since the uncertain variable distributions (means, std devs, correlations)
  // may change among NonDLocalReliability invocations (e.g., RBDO with design
  // variable insertion), this code block is performed on every invocation.
  natafTransform.trans_correlations();

  // define ranVarMeansU for use in the transformed AMV option
  //if (mppSearchType == AMV_U)
  natafTransform.trans_X_to_U(natafTransform.x_means(), ranVarMeansU);
  // must follow trans_correlations()

  /*
  // Determine median limit state values for AMV/AMV+/FORM/SORM by evaluating
  // response fns at u = 0 (used for determining signs of reliability indices).
  Cout << "\n>>>>> Evaluating response at median values\n";
  if (mppSearchType < NO_APPROX)
    uSpaceModel.component_parallel_mode(TRUTH_MODEL);
  RealVector ep_median_u(numUncertainVars), // inits vals to 0
             ep_median_x(numUncertainVars, false);
  natafTransform.trans_U_to_X(ep_median_u, ep_median_x);
  iteratedModel.continuous_variables(ep_median_x);
  activeSet.request_values(0); // initialize
  for (size_t i=0; i<numFunctions; i++)
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty())
      activeSet.request_value(i, 1); // only fn vals needed at median unc vars
  iteratedModel.compute_response(activeSet);
  medianFnVals = iteratedModel.current_response().function_values();
  */

  // now that vars/labels/bounds/targets have flowed down at run-time from any
  // higher level recursions, propagate them up the instantiate-on-the-fly
  // Model recursion so that they are correct when they propagate back down.
  mppModel.update_from_subordinate_model(); // recurse_flag = true

  // set up the x-space data within the importance sampler
  if (integrationRefinement) { // IS/AIS/MMAIS
    // rep needed for access to functions not mapped to Iterator level
    NonDAdaptImpSampling* importance_sampler_rep
      = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
    importance_sampler_rep->initialize_random_variables(natafTransform);
  }
}


/** For a particular response function prior to the first z/p/beta level,
    initialize/warm-start optimizer initial guess (initialPtU),
    expansion point (mostProbPointX/U), and associated response
    data (computedRespLevel, fnGradX/U, and fnHessX/U). */
void NonDLocalReliability::initialize_level_data()
{
  size_t i;

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
      size_t num_icv = d_k_plus_1.length();
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
      activeSet.request_values(0);
      activeSet.request_value(respFnCount, mode);
      iteratedModel.compute_response(activeSet);
      SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
      SizetArray x_dvv; copy_data(cv_ids, x_dvv);
      const Response& local_resp = iteratedModel.current_response();
      computedRespLevel = local_resp.function_values()[respFnCount];
      fnGradX = local_resp.function_gradient(respFnCount);
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
      if (mode & 4) {
	fnHessX = local_resp.function_hessians()[respFnCount];
	natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
					 fnGradX, x_dvv, cv_ids);
	curvatureDataAvailable = true;
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
    initialPtU = 1.;

    /*
    // fall back if projection is unavailable (or numerics don't work out).
    initialPtU = (ria_flag) ?
      1. : std::fabs(requestedCDFRelLevel)/std::sqrt((Real)numUncertainVars);

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
  size_t i;
  bool ria_flag = (levelCount < requestedRespLevels[respFnCount].length())
                ? true : false;

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
    if (levelCount < requestedRespLevels[respFnCount].length()) {
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
	Real alpha = (requestedRespLevel - 
          requestedRespLevels[respFnCount][levelCount-1])/norm_grad_u_sq;
	for (i=0; i<numUncertainVars; i++)
	  initialPtU[i] = mostProbPointU[i] + alpha*fnGradU[i];
      }
    }
    else {
      // For PMA case, scale mostProbPointU so that its magnitude equals
      // the next beta_target.
      // NOTE 1: use of computed levels instead of requested levels handles
      // an RIA/PMA switch (the observed reliability from the RIA soln is
      // scaled to the requested reliability of the next PMA level).
      // NOTE 2: requested and computed reliability levels should agree very
      // closely in all cases since it is the g term that is linearized, not
      // the u'u term (which defines beta).  However, if the optimizer fails
      // to satisfy the constraint in PMA, then using the computed level is
      // preferable.
      //Real prev_pl = (levelCount == rl_len)
      //  ? computedProbLevels[respFnCount][levelCount-1]
      //  : requestedProbLevels[respFnCount][levelCount-rl_len-1];
      Real prev_bl = computedRelLevels[respFnCount][levelCount-1];
      Real prev_cdf_bl = (cdfFlag) ? prev_bl : -prev_bl;
      if ( std::fabs(prev_cdf_bl)          > 1.e-10 &&
	   std::fabs(requestedCDFRelLevel) > 1.e-10 ) {
	// CDF or CCDF does not matter for scale_factor so long as it is
	// consistent (CDF/CDF or CCDF/CCDF).
	Real scale_factor = requestedCDFRelLevel/prev_cdf_bl;
	for (i=0; i<numUncertainVars; i++)
	  initialPtU[i] = mostProbPointU[i]*scale_factor;
      }
    }
  }
  else { // cold start: reset to mean inputs/outputs
    // initial fnGradX/U for AMV/AMV+ = grads at mean x values, initial
    // expansion point for AMV+ = mean x values.
    if (mppSearchType < NO_APPROX) // AMV/AMV+/TANA
      assign_mean_data();
    // initial optimizer guess in u-space (initialPtU)
    initialPtU = 1.;
  }
}


/** Includes case-specific logic for updating MPP search data for the
    AMV/AMV+/TANA/NO_APPROX methods. */
void NonDLocalReliability::
update_mpp_search_data(const Variables& vars_star, const Response& resp_star)
{
  // Update MPP arrays from optimization results
  const RealVector& mpp_u = vars_star.continuous_variables(); // view
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

  // Set computedRespLevels to the current g(x) value by either performing
  // a validation function evaluation (AMV/AMV+) or retrieving data from
  // resp_star (FORM).  Also update approximations and convergence tols.
  const RealVector& fns_star = resp_star.function_values();
  size_t rl_len = requestedRespLevels[respFnCount].length(),
         pl_len = requestedProbLevels[respFnCount].length(),
         bl_len = requestedRelLevels[respFnCount].length();
  bool ria_flag = (levelCount < rl_len) ? true : false;
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  SizetArray x_dvv; copy_data(cv_ids, x_dvv);

  switch (mppSearchType) {
  case AMV_X: case AMV_U: {
    approxConverged = true; // break out of while loop
    uSpaceModel.component_parallel_mode(TRUTH_MODEL);
    activeSet.request_values(0); activeSet.request_value(respFnCount, 1);
    iteratedModel.continuous_variables(mostProbPointX);
    iteratedModel.compute_response(activeSet); 
    computedRespLevel
      = iteratedModel.current_response().function_values()[respFnCount];
    break;
  }
  case AMV_PLUS_X: case AMV_PLUS_U: case TANA_X: case TANA_U: {
    // Assess AMV+/TANA iteration convergence.  ||del_u|| is not a perfect
    // metric since cycling between MPP estimates can occur.  Therefore,
    // a maximum number of iterations is also enforced.
    //conv_metric = std::fabs(local_fns[respFnCount] - requestedRespLevel);
    approxIters++;
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
    activeSet.request_value(respFnCount, mode);
    iteratedModel.continuous_variables(mostProbPointX);
    iteratedModel.compute_response(activeSet);
    computedRespLevel
      = iteratedModel.current_response().function_values()[respFnCount];
#ifdef MPP_CONVERGE_RATE
    Cout << "u'u = "  << mostProbPointU.dot(mostProbPointU)
	 << " G(u) = " << computedRespLevel << '\n';
#endif // MPP_CONVERGE_RATE
    if (mode & 2) {
      fnGradX
        = iteratedModel.current_response().function_gradient_copy(respFnCount);
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
    }
    if (mode & 4) {
      const RealSymMatrixArray& local_hessians
	= iteratedModel.current_response().function_hessians();
      fnHessX = local_hessians[respFnCount];
      natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
				       fnGradX, x_dvv, cv_ids);
      curvatureDataAvailable = true;
    }

    // Update the limit state surrogate model
    update_limit_state_surrogate();

    // Update requestedCDFRelLevel if PMA with second-order integrations
    // for specified probability level
    if ( !approxConverged && !ria_flag && integrationOrder == 2 && 
	 ( levelCount <  rl_len + pl_len ||
	   levelCount >= rl_len + pl_len + bl_len ) )
      requestedCDFRelLevel = reliability(requestedCDFProbLevel, true);
    break;
  }
  case NO_APPROX: { // FORM/SORM

    // direct optimization converges to MPP: no new approximation to compute
    approxConverged = true; // break out of while loop
    if (ria_flag) // RIA computed response = eq_con_star + response target
      computedRespLevel = fns_star[1] + requestedRespLevel;
    else          // PMA computed response = +/- obj_fn_star
      computedRespLevel // back out negation of g in PMA_objective_eval()
	= (requestedCDFRelLevel >= 0.) ? fns_star[0] : -fns_star[0];

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
    extern PRPCache data_pairs; // global container
    if (mode & 2) { // avail in all RIA/PMA cases (exception: numerical grads)
      // query data_pairs to retrieve the fn gradient at the MPP
      Variables search_vars = iteratedModel.current_variables().copy();
      search_vars.continuous_variables(mostProbPointX);
      ActiveSet search_set = resp_star.active_set();
      ShortArray search_asv(numFunctions, 0);
      search_asv[respFnCount] = 2;
      search_set.request_vector(search_asv);
      Response desired_resp;
      if( lookup_by_val(data_pairs, iteratedModel.interface_id(), search_vars,
			search_set, desired_resp) ) {
	fnGradX = desired_resp.function_gradient(respFnCount);
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
      Response desired_resp;
      if( lookup_by_val(data_pairs, iteratedModel.interface_id(), search_vars,
			search_set, desired_resp) ) {
        fnHessX = desired_resp.function_hessians()[respFnCount];
	found_mode |= 4;
      }
    }
    // evaluate any remaining required data which could not be retrieved
    short remaining_mode = mode - found_mode;
    if (remaining_mode) {
      Cout << "\n>>>>> Evaluating limit state derivatives at MPP\n";
      iteratedModel.continuous_variables(mostProbPointX);
      activeSet.request_values(0);
      activeSet.request_value(respFnCount, remaining_mode);
      iteratedModel.compute_response(activeSet);
      if (remaining_mode & 2)
	fnGradX
	 = iteratedModel.current_response().function_gradient(respFnCount);
      if (remaining_mode & 4)
        fnHessX
	  = iteratedModel.current_response().function_hessians()[respFnCount];
    }
    if (mode & 2)
      natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, mostProbPointX,
				       x_dvv, cv_ids);
    if (mode & 4) {
      natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, mostProbPointX,
				       fnGradX, x_dvv, cv_ids);
      curvatureDataAvailable = true;
    }
    break;
  }
  }

  // Set computedRelLevel after retrieving u'u from resp_star
  Real norm_u_sq = (ria_flag) ? fns_star[0]
                 : fns_star[1] + std::pow(requestedCDFRelLevel, 2);

  // z>median: CDF p(g<=z)>0.5, CDF beta<0, CCDF p(g>z)<0.5, CCDF beta>0
  // z<median: CDF p(g<=z)<0.5, CDF beta>0, CCDF p(g>z)>0.5, CCDF beta<0
  // z=median: CDF p(g<=z) = CCDF p(g>z) = 0.5, CDF beta = CCDF beta = 0
  //Real beta_cdf = (computedRespLevel > medianFnVals[respFnCount])
  //              ? -std::sqrt(norm_u_sq) : std::sqrt(norm_u_sq);

  // This approach avoids the need to evaluate medianFnVals.  Thanks to
  // Barron Bichon for suggesting it.
  // if <mppU, fnGradU> > 0, then G is increasing along u and G(u*) > G(0)
  // if <mppU, fnGradU> < 0, then G is decreasing along u and G(u*) < G(0)
  Real beta_cdf = (mostProbPointU.dot(fnGradU) > 0.)
                ? -std::sqrt(norm_u_sq) : std::sqrt(norm_u_sq);
  computedRelLevel = (cdfFlag) ? beta_cdf : -beta_cdf;
}


/** Updates computedRespLevels/computedProbLevels/computedRelLevels,
    finalStatistics, warm start, and graphics data. */
void NonDLocalReliability::update_level_data()
{
  // Update computed Resp/Prob/Rel levels arrays.  Note that finalStatistics
  // uses generalized beta in the case of second-order integrations, but
  // computedRelLevels does not.
  Real computed_prob_level = probability(computedRelLevel, cdfFlag),
    computed_gen_rel_level = -Phi_inverse(computed_prob_level);
  computedRespLevels[respFnCount][levelCount]   = computedRespLevel;
  computedProbLevels[respFnCount][levelCount]   = computed_prob_level;
  computedRelLevels[respFnCount][levelCount]    = computedRelLevel;
  computedGenRelLevels[respFnCount][levelCount] = computed_gen_rel_level;

  // Final statistics are the z, beta, or p output of the MPP search
  size_t i, j, k;
  bool ria_flag = (levelCount < requestedRespLevels[respFnCount].length())
                ? true : false;
  if (ria_flag) { // z -> p/beta/beta*
    switch (respLevelTarget) {
    case PROBABILITIES:
      finalStatistics.function_value(computed_prob_level,    statCount); break;
    case RELIABILITIES:
      finalStatistics.function_value(computedRelLevel,       statCount); break;
    case GEN_RELIABILITIES:
      finalStatistics.function_value(computed_gen_rel_level, statCount); break;
    }
  }
  else // p/beta/beta* -> z
    finalStatistics.function_value(computedRespLevel, statCount);

  // Final statistic gradients are dz/ds, dbeta/ds, or dp/ds
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  if (final_asv[statCount] & 2) {

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
      Real factor = (cdfFlag) ?  1./norm_grad_u : -1./norm_grad_u;
      if (integrationOrder == 2 && respLevelTarget != RELIABILITIES) {
	if (secondOrderIntType == HONG) { // addtnl complexity not warranted
	  Cerr << "\nError: final statistic gradients not implemented for Hong."
	       << std::endl;
	  abort_handler(-1);
	}
	// For beta < 0, beta_corr = -beta and p_corr = 1 - p:
	// dp/ds = -dp_corr/ds = -(dp_corr/dbeta_corr * dbeta_corr/ds)
	//       = -(dp_corr/dbeta_corr * -dbeta/ds)
	//       = dp_corr/dbeta_corr * dbeta/ds
	Real beta_corr
	  = (computedRelLevel >= 0.) ? computedRelLevel : -computedRelLevel;
	RealVector kappa = kappaU;
	if ( ( cdfFlag && computedRelLevel <  0.) ||
	     (!cdfFlag && computedRelLevel >= 0.) )
	  kappa.scale(-1.);
	bool apply_correction = true;
	Real psi_m_beta, dpsi_m_beta_dbeta;
	if (secondOrderIntType != BREITUNG) {
	  psi_m_beta        = phi(-beta_corr) / Phi(-beta_corr);
	  dpsi_m_beta_dbeta = psi_m_beta*(beta_corr + psi_m_beta);
	}
	Real sum = 0., ktk, prod1, prod2 = 1.;
	Real kterm = (secondOrderIntType == BREITUNG) ? beta_corr : psi_m_beta;
	size_t num_kappa = numUncertainVars - 1;
	for (i=0; i<num_kappa; i++)
	  if (1. + kterm * kappa[i] <= curvatureThresh)
	    apply_correction = false;
	if (apply_correction) {
	  for (i=0; i<num_kappa; i++) {
	    ktk = kterm * kappa[i];
	    prod2 /= std::sqrt(1. + ktk);
	    prod1 = 1.;
	    for (j=0; j<num_kappa; j++)
	      if (j != i)
		prod1 /= std::sqrt(1. + kterm * kappa[j]);
	    prod1 *= kappa[i] / 2. / std::pow(1. + ktk, 1.5);
	    if (secondOrderIntType != BREITUNG)
	      prod1 *= dpsi_m_beta_dbeta;
	    sum -= prod1;
	  }
	  // factor for second-order dp/ds:
	  factor *= Phi(-beta_corr) * sum - phi(-beta_corr) * prod2;
	  // factor for second-order dbeta*/ds
	  if (respLevelTarget == GEN_RELIABILITIES)
	    factor *= -1. / phi(-computed_gen_rel_level);
	}
	else {
	  Cerr << "\nWarning: second-order probability sensitivity bypassed "
	       << "due to numerical issues.\n";
	  warningBits |= 2; // second warning in output summary
	  if (respLevelTarget == PROBABILITIES) // factor for first-order dp/ds
	    factor *= -phi(-computedRelLevel);
	}
      }
      else if (respLevelTarget == PROBABILITIES) // factor for first-order dp/ds
	factor *= -phi(-computedRelLevel);
      // apply factor:
      size_t num_final_grad_vars
	= finalStatistics.active_set_derivative_vector().size();
      for (k=0; k<num_final_grad_vars; k++)
	final_stat_grad[k] *= factor;
    }
    finalStatistics.function_gradient(final_stat_grad, statCount);
  }

  // Update warm-start data and graphics
  if (warmStartFlag && subIteratorFlag && levelCount == 0) {
    // for warm-starting next run
    prevMPPULev0[respFnCount] = mostProbPointU;
    prevCumASVLev0[respFnCount] |= final_asv[statCount];
    for (i=0; i<numUncertainVars; i++)
      prevFnGradULev0(i,respFnCount) = fnGradU[i];
  }
  if (!subIteratorFlag) {
    extern Graphics dakota_graphics; // defined in ParallelLibrary.C
    dakota_graphics.add_datapoint(respFnCount, computedRespLevel,
				  computed_prob_level);
    for (i=0; i<numUncertainVars; i++) {
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


/** For PMA SORM with prescribed p-level or prescribed generalized beta-level,
    requestedCDFRelLevel must be updated.  This virtual function redefinition
    is called from NonDReliability::PMA_constraint_eval(). */
void NonDLocalReliability::update_pma_reliability_level()
{
  size_t rl_len = requestedRespLevels[respFnCount].length(),
         pl_len = requestedProbLevels[respFnCount].length(),
         bl_len = requestedRelLevels[respFnCount].length();
  if ( mppSearchType == NO_APPROX && integrationOrder == 2 &&
       ( ( levelCount >= rl_len && levelCount < rl_len + pl_len ) ||
	 levelCount >= rl_len + pl_len + bl_len ) ) {
    // In the NO_APPROX case, we know that uSpaceModel provides convenient
    // access to u-space data without any approximations and that the u-space
    // transformations have been applied to the iteratedModel response data.
    const Variables& u_vars = uSpaceModel.current_variables();
    const Response&  u_resp = uSpaceModel.current_response();
    // mostProbPointU needed for alternate R0 initialization
    copy_data(u_vars.continuous_variables(), mostProbPointU); // view -> copy
    // grad/Hessian availability enforced by NonDReliability::PMA2_set_mapping()
    fnGradU = u_resp.function_gradient(respFnCount);
    fnHessU = u_resp.function_hessians()[respFnCount];
    curvatureDataAvailable = true;
    requestedCDFRelLevel   = reliability(requestedCDFProbLevel, true);
  }
}


void NonDLocalReliability::update_limit_state_surrogate()
{
  RealVector mpp, fn_grad(fnGradX.length());
  const RealSymMatrix* fn_hess_ptr = 0;
  if (mppSearchType ==  AMV_X || mppSearchType == AMV_PLUS_X ||
      mppSearchType == TANA_X) {
    mpp = mostProbPointX;
    fn_grad = fnGradX;
    if (taylorOrder == 2)
      fn_hess_ptr = &fnHessX;
  }
  else { // AMV_U, AMV_PLUS_U, TANA_U
    mpp = mostProbPointU;
    fn_grad = fnGradU;
    if (taylorOrder == 2)
      fn_hess_ptr = &fnHessU;
  }

  // construct local Variables object
  const Variables& curr_vars = iteratedModel.current_variables();
  Variables mpp_vars(curr_vars.shared_data());
  mpp_vars.continuous_variables(mpp);

  // construct Response object
  ShortArray asv(numFunctions, 0);
  asv[respFnCount] = (taylorOrder == 2) ? 7 : 3;
  ActiveSet set;//(numFunctions, numUncertainVars);
  set.request_vector(asv);
  set.derivative_vector(iteratedModel.continuous_variable_ids());
  Response response(set);
  response.function_value(computedRespLevel, respFnCount);
  response.function_gradient(fn_grad, respFnCount);
  if (taylorOrder == 2) {
    const RealSymMatrix& fn_hess = *fn_hess_ptr;
    response.function_hessian(fn_hess, respFnCount);
  }

  // After a design variable change, history data (e.g., TANA) needs
  // to be cleared (build_approximation() only calls clear_current())
  if (numRelAnalyses && levelCount == 0)
    uSpaceModel.approximations()[respFnCount].clear_all();
  // build the new local/multipoint approximation
  uSpaceModel.build_approximation(mpp_vars, response);
}


void NonDLocalReliability::assign_mean_data()
{
  const Pecos::RealVector& x_means = natafTransform.x_means();
  mostProbPointX = x_means;
  mostProbPointU = ranVarMeansU;
  computedRespLevel = fnValsMeanX(respFnCount);
  for (size_t i=0; i<numUncertainVars; i++)
    fnGradX[i] = fnGradsMeanX(i,respFnCount);
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  SizetArray x_dvv; copy_data(cv_ids, x_dvv);
  natafTransform.trans_grad_X_to_U(fnGradX, fnGradU, x_means, x_dvv, cv_ids);
  if (taylorOrder == 2 && hessianType != "quasi") {
    fnHessX = fnHessiansMeanX[respFnCount];
    natafTransform.trans_hess_X_to_U(fnHessX, fnHessU, x_means, fnGradX,
				     x_dvv, cv_ids);
    //curvatureDataAvailable = true; // TO DO: test if this helps or not
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
    inactive_grad_set.request_value(respFnCount, 2);
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
    iteratedModel.compute_response(inactive_grad_set);
    if (secondaryACVarMapTargets.empty()) {
      final_stat_grad
        = iteratedModel.current_response().function_gradient(respFnCount);
    }
    else {
      const RealMatrix& local_grads
        = iteratedModel.current_response().function_gradients();
      size_t cntr = 0;
      for (i=0; i<num_final_grad_vars; i++)
	if (secondaryACVarMapTargets[i] == Pecos::NO_TARGET)
	  final_stat_grad[i] = local_grads(cntr++,respFnCount);
    }
  }
}


/** Converts beta into a probability using either first-order (FORM) or
    second-order (SORM) integration.  The SORM calculation first calculates
    the principal curvatures at the MPP (using the approach in Ch. 8 of 
    Haldar & Mahadevan), and then applies correction formulations from the 
    literature (Breitung, Hohenbichler-Rackwitz, or Hong). */
Real NonDLocalReliability::probability(const Real& beta, bool cdf_flag)
{
  Real p = Phi(-beta); // FORM approximation
  Cout << "First-order p:  " << p << '\n';

  if (integrationOrder == 2 && curvatureDataAvailable) {
    //Cout << "\nfnHessU:" << fnHessU;

    principal_curvatures();
    int i, num_kappa = numUncertainVars - 1;

    // The correction to p is applied for beta >= 0 (FORM p <= 0.5).
    // For beta < 0, apply correction to complementary problem (Tvedt 1990).
    //   > beta changes sign
    //   > p becomes complement
    //   > principal curvature sign convention defined for CDF beta > 0
    //     (negate for CDF beta < 0 or CCDF beta > 0, OK for CCDF beta < 0)
    Real beta_corr = (beta >= 0.) ? beta : -beta;
    Real p_corr    = (beta >= 0.) ? p    : 1. - p;
    RealVector kappa = kappaU;
    if ( (cdf_flag && beta < 0.) || (!cdf_flag && beta >= 0.) )
      kappa.scale(-1.);

    // Test for numerical exceptions in sqrt.  Problematic kappa are large and
    // negative (kterm is always positive).  Skipping individual kappa means
    // neglecting a primary curvature and including secondary curvatures, which
    // may be counter-productive (potentially less accurate than FORM).
    // Therefore, the entire correction is skipped if any curvature is
    // problematic.  A consistent approach must be used in reliability().
    bool apply_correction = true;
    Real psi_m_beta;
    if (secondOrderIntType != BREITUNG)
      psi_m_beta = phi(-beta_corr)/Phi(-beta_corr);
    Real kterm = (secondOrderIntType == BREITUNG) ? beta_corr : psi_m_beta;
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
	  C1 += Phi(-beta_corr-hterm) / Phi(-beta_corr)*exp(psi_m_beta*hterm);
	}
      }
      if (secondOrderIntType == HONG) {
	C1 /= num_kappa;
	p_corr *= C1;
      }
      if (p_corr >= 0. && p_corr <= 1.) { // verify p_corr within valid range
	p = (beta >= 0.) ? p_corr : 1. - p_corr;
	Cout << "Second-order p: " << p << '\n';
      }
      else {
	Cerr << "\nWarning: second-order probability integration bypassed due "
	     << "to numerical issues.\n";
	warningBits |= 2; // second warning in output summary
      }
    }
    else {
      Cerr << "\nWarning: second-order probability integration bypassed due "
	   << "to numerical issues.\n";
      warningBits |= 2; // second warning in output summary
    }
  }

  if (integrationRefinement) { // IS/AIS/MMAIS
    // rep needed for access to functions not mapped to Iterator level
    NonDAdaptImpSampling* importance_sampler_rep
      = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
    // copy mostProbPointU from Teuchos structure to RealVector
    importance_sampler_rep->initialize(mostProbPointU, respFnCount, p, 
				       requestedRespLevel);
    // no summary output since on-the-fly constructed:
    importanceSampler.run_iterator(Cout);
    p = importance_sampler_rep->get_probability();
    Cout << "Refined p:      " << p << '\n';
  }

  return p;
}


/** Converts a probability into a reliability using the inverse of the
    first-order or second-order integrations implemented in
    NonDLocalReliability::probability(). */
Real NonDLocalReliability::reliability(const Real& p, bool cdf_flag)
{
  Real beta = -Phi_inverse(p); // FORM approximation

  // fnGradU, fnHessU, and possibly mostProbPointU (alternate R0 initialization)
  // must be in synch and be a reasonable approximation to converged MPP data.
  if (integrationOrder == 2 && curvatureDataAvailable) {

    principal_curvatures();

    // NOTE: these conversions are currently done once.  It may be necessary
    // to redo them for each beta estimate (when inverting near beta = zero).
    Real beta_corr = (beta >= 0.) ? beta : -beta;
    Real p_corr    = (beta >= 0.) ? p    : 1. - p;
    RealVector kappa = kappaU;
    if ( (cdf_flag && beta < 0.) || (!cdf_flag && beta >= 0.) )
      kappa.scale(-1.);

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
    psi_m_beta = phi(-beta) / Phi(-beta);
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
      C1 += Phi(-beta - hterm) / Phi(-beta) * exp(psi_m_beta * hterm);
    }
  }
  if (secondOrderIntType == HONG)
    res = p * prod - C1 * Phi(-beta);
  else
    res = p * prod - Phi(-beta);

  return false;
}


Real NonDLocalReliability::
reliability_residual_derivative(const Real& p, const Real& beta,
				const RealVector& kappa)
{
  int i, j, num_kappa = numUncertainVars - 1;
  Real psi_m_beta, dpsi_m_beta_dbeta;
  if (secondOrderIntType != BREITUNG) {
    psi_m_beta = phi(-beta) / Phi(-beta);
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
    dres_dbeta = p * sum + phi(-beta);

  return dres_dbeta;
}


void NonDLocalReliability::principal_curvatures()
{
  // compute R matrix
  int i, j, k, num_kappa = numUncertainVars - 1;
  RealMatrix R0(numUncertainVars, numUncertainVars);// init to 0
  // initialize R0: last row values are direction cosines
  for (i=0; i<num_kappa; i++)
    R0(i, i) = 1.;
  // Haldar & Mahadevan, p.227: last row = unit gradient vector of limit state
  Real norm_grad_u = fnGradU.normFrobenius();
  if (norm_grad_u > DBL_MIN)
    for (i=0; i<numUncertainVars; i++)
      R0(num_kappa, i) = fnGradU[i]/norm_grad_u;
  else { // fallback: try to use +/- mPPU[i]/norm(mPPU)
    Real norm_mpp_u = mostProbPointU.normFrobenius(); // unsigned beta
    if (norm_mpp_u > DBL_MIN) {
      // Can match the sign of fnGradU[i]/norm_grad_u in PMA case (align for
      // max G, oppose for min G), but can't in general in RIA case (vectors
      // may align or oppose).  Fortunately, the R0 sign does not appear to
      // matter since R is applied twice in R fnHessU R^T.
      bool pma_max = (levelCount >= requestedRespLevels[respFnCount].length() &&
		      requestedCDFRelLevel < 0.) ? true : false;
      for (i=0; i<numUncertainVars; i++)
	R0(num_kappa, i) = (pma_max) ?  mostProbPointU[i]/norm_mpp_u  // aligned
	                             : -mostProbPointU[i]/norm_mpp_u; // opposed
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
  for (i=numUncertainVars-2; i>=0; i--) {  // update the ith row vector
    for (j=i+1; j<numUncertainVars; j++) { // orthogonalize to jth row vector
      Real scale1 = 0., scale2 = 0.;
      for (k=0; k<numUncertainVars; k++) { // kth column
	scale1 += R(j, k) * R0(i, k);
	scale2 += std::pow(R(j, k), 2);
      }
      Real scale = scale1 / scale2;
      for (k=0; k<numUncertainVars; k++)   // kth column
	R(i, k) -= scale * R(j, k);
    }
    // renormalize ith row vector to unit length
    Real len = 0.;
    for (j=0; j<numUncertainVars; j++)
      len += std::pow(R(i, j), 2);
    len = std::sqrt(len);
    for (j=0; j<numUncertainVars; j++)
      R(i, j) /= len;
  }
  //Cout << "\nR:" << R;

  // compute A matrix = (R fnHessU R^T)/norm(fnGradU)
  RealSymMatrix A(numUncertainVars, false);
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1./norm_grad_u,
			       fnHessU, R, A);
  //Cout << "\nA:" << A;
  A.reshape(num_kappa); // upper left portion of matrix
  //Cout << "\nReshaped A:" << A;

  // compute eigenvalues of A --> principal curvatures
  Teuchos::LAPACK<int, Real> la;
  int info, lwork = 3*num_kappa - 1;
  double* work = new double [lwork];
  // LAPACK eigenvalue solution for real, symmetric A
  la.SYEV('N', A.UPLO(), num_kappa, A.values(), A.stride(), kappaU.values(),
	  work, lwork, &info);
  delete [] work;
  if (info) {
    Cerr << "\nError: internal error in LAPACK eigenvalue routine."
         << std::endl;
    abort_handler(-1);
  }
  //Cout << "\nkappaU:" << kappaU;
}


void NonDLocalReliability::print_results(std::ostream& s)
{
  size_t i, j, width = write_precision+7;
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

  for (i=0; i<numFunctions; i++) {

    // output MV-specific statistics
    if (!mppSearchType) {
      s << "MV Statistics for " << fn_labels[i] << ":\n";
      // approximate response means and std deviations and importance factors
      s << "  Approximate Mean Response                  = " << std::setw(width)
	<< meanStats[i]	<< "\n  Approximate Standard Deviation of Response = "
	<< std::setw(width)<< stdDevStats[i] << '\n';
      if (natafTransform.x_correlation() || stdDevStats[i] <= 1.e-25)
	s << "  Importance Factors not available.\n";
      else
	for (j=0; j<numUncertainVars; j++)
	  s << "  Importance Factor for variable "
	    << std::setiosflags(std::ios::left) << std::setw(11)
	    << uv_labels[j].data() << " = "
	    << std::resetiosflags(std::ios::adjustfield)
	    << std::setw(width) << impFactor(j,i) << '\n';
    }

    // output CDF/CCDF response/probability pairs
    size_t num_levels = computedRespLevels[i].length();
    if (num_levels) {
      if (!mppSearchType && stdDevStats[i] <= 1.e-25)
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

  //s << "Final statistics:\n" << finalStatistics;

  s << "-----------------------------------------------------------------"
    << std::endl;
}


void NonDLocalReliability::method_recourse()
{
  Cerr << "\nWarning: method recourse invoked in NonDLocalReliability due to "
       << "detected method conflict.\n\n";
  if (mppSearchType && npsolFlag) {
#ifdef HAVE_OPTPP
    mppModel.free_communicators(mppOptimizer.maximum_concurrency());
    mppOptimizer.assign_rep(
      new SNLLOptimizer("optpp_q_newton", mppModel), false);
    mppModel.init_communicators(mppOptimizer.maximum_concurrency());
#else
    Cerr << "\nError: method recourse not possible in NonDLocalReliability "
	 << "(OPT++ NIP unavailable).\n";
    abort_handler(-1);
#endif
    npsolFlag = false;
  }
}


/*
void NonDLocalReliability::g_eval(int& mode, const RealVector& u)
{
    // ---------------------------
    // FORM/SORM: no approximation
    // ---------------------------

#ifdef MPP_CONVERGE_RATE
    Cout << "u'u = " << u.dot(u) << " G(u) = "
         << iteratedModel.current_response().function_values()[respFnCount]
	 << '\n';
#endif // MPP_CONVERGE_RATE
}


void NonDLocalReliability::
RIA_objective_eval(int mode, int n, const NEWMAT::ColumnVector& u,
		   NEWMAT::Real& f, NEWMAT::ColumnVector& grad_f,
		   int& result_mode)
{
  // In all OPT++ evaluator functions:
  if (nondLocRelInstance->nipModeOverrideFlag)
    mode |= 3; // allows consistent comparisons with SQP
}
*/

} // namespace Dakota
