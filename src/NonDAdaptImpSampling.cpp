/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDAdaptImpSampling - Adaptive Importance Sampling
//- Description: Implementation code for NonDAdaptImpSampling class
//- Owner:       Barron Bichon and Laura Swiler
//- Checked by:
//- Version:

//- Modified by: Tim Wildey on 12/18/2013

#include "NonDAdaptImpSampling.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "RecastModel.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "NonDLHSSampling.hpp"
#include "pecos_stat_util.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[] = "@(#) $Id: NonDAdaptImpSampling.cpp 4058 2006-10-26 01:39:40Z lpswile $";

//#define DEBUG

namespace Dakota {


/** This is the primary constructor.  It accepts a Model reference.  It will
    perform refinement for all response QOI and all probability levels. */
NonDAdaptImpSampling::
NonDAdaptImpSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  importanceSamplingType(
    probDescDB.get_ushort("method.nond.integration_refinement")),
  initLHS(true), useModelBounds(false), invertProb(false),
  trackExtremeValues(pdfOutput) // used for defining PDF bounds
{
  // sampleType default in DataMethod.cpp is SUBMETHOD_DEFAULT (0).
  // Enforce an LHS default for this method.
  if (!sampleType)
    sampleType = SUBMETHOD_LHS;

  // size of refinement batches is separate from initial LHS size (numSamples)
  const IntVector& db_refine_samples = 
    probDescDB.get_iv("method.nond.refinement_samples");
  // if separate refinement batch size not provided, reuse initial LHS size
  refineSamples = numSamples; // default
  if (db_refine_samples.length() == 1)
    refineSamples = db_refine_samples[0];
  else if (db_refine_samples.length() > 1) {
    Cerr << "\nError (NonDAdaptImpSampling): refinement_samples must be length "
         << "1 if specified." << std::endl;
    abort_handler(PARSE_ERROR);
  }

  statsFlag = true;
  initialize_random_variables(STD_NORMAL_U);//ASKEY_U,EXTENDED_U

  transform_model(iteratedModel, uSpaceModel, useModelBounds);

  // maxEvalConcurrency defined from initial LHS size (numSamples)
}


/** This is an alternate constructor for instantiations on the fly using
    a Model but no ProblemDescDB.  It will perform refinement for one
    response QOI and one probability level (passed in initialize()). */
NonDAdaptImpSampling::
NonDAdaptImpSampling(Model& model, unsigned short sample_type,
		     int refine_samples, int refine_seed, const String& rng,
		     bool vary_pattern, unsigned short is_type, bool cdf_flag,
		     bool x_space_model, bool use_model_bounds,
		     bool track_extreme):
  NonDSampling(IMPORTANCE_SAMPLING, model, sample_type, 0, refine_seed, rng,
	       vary_pattern, ALEATORY_UNCERTAIN), // only sample aleatory vars
  importanceSamplingType(is_type), initLHS(false), 
  useModelBounds(use_model_bounds), invertProb(false),
  trackExtremeValues(track_extreme), refineSamples(refine_samples)
{
  if (x_space_model) {
    // This option is currently unused.  If used in the future, care must be
    // taken to ensure that natafTransform.{x,u}_types() inherited from above
    // are synchronized with those from the calling context.
    transform_model(model, uSpaceModel, useModelBounds, 5.);
  }
  else
    uSpaceModel = model;

  cdfFlag = cdf_flag;

  if (trackExtremeValues) {
    extremeValues.resize(numFunctions);
    for (size_t i=0; i<numFunctions; ++i)
      { extremeValues[i].first = DBL_MAX; extremeValues[i].second = -DBL_MAX; }
  }

  if (refineSamples)
    maxEvalConcurrency *= refineSamples;
}


bool NonDAdaptImpSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDAdaptImpSampling::derived_init_communicators(ParLevLIter pl_iter)
{
  // uSpaceModel uses NoDBBaseConstructor, so no need to manage DB
  // list nodes at this level
  if (initLHS)
    uSpaceModel.init_communicators(pl_iter, numSamples);
  uSpaceModel.init_communicators(pl_iter, refineSamples);
}


void NonDAdaptImpSampling::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  if (initLHS)
    uSpaceModel.set_communicators(pl_iter, numSamples);
  else
    uSpaceModel.set_communicators(pl_iter, refineSamples);
}


void NonDAdaptImpSampling::derived_free_communicators(ParLevLIter pl_iter)
{
  uSpaceModel.free_communicators(pl_iter, refineSamples);
  if (initLHS)
    uSpaceModel.free_communicators(pl_iter, numSamples);
}


/** Initializes data using a vector array of starting points. */
void NonDAdaptImpSampling::
initialize(const RealVectorArray& acv_points, bool x_space_data,
	   size_t resp_index, Real initial_prob, Real failure_threshold)
{
  size_t i, j, cntr, num_points = acv_points.size();
  initPointsU.resize(num_points);

  if (numContDesVars) { // store inactive variable values, if present
    designPoint.sizeUninitialized(numContDesVars);
    const RealVector& acv_pt_0 = acv_points[0];
    for (i=0; i<numContDesVars; i++)
      designPoint[i] = acv_pt_0[i];
  }

  RealVector acv_u_point;
  for (i=0; i<num_points; i++) {
    RealVector& init_pt_i = initPointsU[i];
    init_pt_i.sizeUninitialized(numUncertainVars);
    if (x_space_data) {
      natafTransform.trans_X_to_U(acv_points[i], acv_u_point);
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
	init_pt_i[cntr] = acv_u_point[j];
    }
    else {
      const RealVector& acv_init_pt_i = acv_points[i];
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
	init_pt_i[cntr] = acv_init_pt_i[j];
    }
  }
#ifdef DEBUG  
  for (i=0; i<num_points; i++)
    Cout << "Initial Point " << i << initPointsU[i] << '\n';
#endif

  respFnIndex  = resp_index;
  probEstimate = initial_prob;
  failThresh   = failure_threshold;
  invertProb   = (probEstimate > 0.5);
}


/** Initializes data using a matrix of starting points. */
void NonDAdaptImpSampling::
initialize(const RealMatrix& acv_points, bool x_space_data, size_t resp_index,
	   Real initial_prob, Real failure_threshold)
{
  size_t i, j, cntr, num_points = acv_points.numCols();
  initPointsU.resize(num_points);

  if (numContDesVars) { // store inactive variable values, if present
    designPoint.sizeUninitialized(numContDesVars);
    const Real* acv_pt_0 = acv_points[0];
    for (i=0; i<numContDesVars; i++)
      designPoint[i] = acv_pt_0[i];
  }

  RealVector acv_u_point;
  for (i=0; i<num_points; i++) {
    const Real* acv_pt_i = acv_points[i];
    RealVector& init_pt_i = initPointsU[i];
    init_pt_i.sizeUninitialized(numUncertainVars);
    if (x_space_data) {
      RealVector acv_pt_view(Teuchos::View, const_cast<Real*>(acv_pt_i),
			     numContinuousVars);
      natafTransform.trans_X_to_U(acv_pt_view, acv_u_point);
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
	init_pt_i[cntr] = acv_u_point[j];
    }
    else
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
	init_pt_i[cntr] = acv_pt_i[j];
  }
#ifdef DEBUG  
  for (i=0; i<num_points; i++)
    Cout << "Initial Point " << i << initPointsU[i] << '\n';
#endif

  respFnIndex  = resp_index;
  probEstimate = initial_prob;
  failThresh   = failure_threshold;
  invertProb   = (probEstimate > 0.5);
}


/** Initializes data using only one starting point. */
void NonDAdaptImpSampling::
initialize(const RealVector& acv_point, bool x_space_data, size_t resp_index,
	   Real initial_prob, Real failure_threshold)
{
  size_t j, cntr;

  if (numContDesVars) { // store inactive variable values, if present
    designPoint.sizeUninitialized(numContDesVars);
    for (j=0; j<numContDesVars; ++j)
      designPoint[j] = acv_point[j];
  }

  initPointsU.resize(1);
  RealVector& init_pt = initPointsU[0];
  init_pt.sizeUninitialized(numUncertainVars);
  if (x_space_data) {
    RealVector acv_u_point;
    natafTransform.trans_X_to_U(acv_point, acv_u_point);
    for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
      init_pt[cntr] = acv_u_point[j];
  }
  else
    for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
      init_pt[cntr] = acv_point[j];

  respFnIndex  = resp_index;
  probEstimate = initial_prob;
  failThresh   = failure_threshold;
  invertProb   = (probEstimate > 0.5);
}


void NonDAdaptImpSampling::core_run()
{
  //TMW: Only converge COV for MMAIS
  bool cov_flag = (importanceSamplingType == MMAIS);
  RealVector init_fns;
  if (initLHS) {
    // Get initial set of LHS points: samples over the original distributions,
    // evaluates the points, and computes the initial probability estimates.
    // Note: {get,evaluate}_parameter_sets() is heavier weight relative to
    // {get,evaluate}_samples() used elsewhere, since this initial LHS
    // evaluates all response fns, not just the active resp fn being refined.
    get_parameter_sets(uSpaceModel); // generates numSamples points
    evaluate_parameter_sets(uSpaceModel, true, false);
    compute_statistics(allSamples, allResponses);

    init_fns.sizeUninitialized(numSamples);
    size_t level_count, resp_fn_count;
    bool x_data_flag = false;
    for (resp_fn_count=0; resp_fn_count<numFunctions; resp_fn_count++) {
      size_t rl_len = requestedRespLevels[resp_fn_count].length(),
             pl_len = requestedProbLevels[resp_fn_count].length(),
             gl_len = requestedGenRelLevels[resp_fn_count].length(),
             num_levels = rl_len + pl_len + gl_len;
      if (pl_len || gl_len) {
        Cerr << "Error: importance sampling only computes a probability level " 
             << "(CDF or CCDF) for each response level requested.  It does not "
             << "compute an estimated response level given a probability level."
	     << std::endl;
        abort_handler(-1);
      }

      //TMW: Store the response values for reuse later
      size_t i; IntRespMIter it;
      for (i=0, it=allResponses.begin(); it!=allResponses.end(); ++i, ++it)
	init_fns[i] = it->second.function_value(resp_fn_count);
      
      // Loop over response/probability/reliability levels
      for (level_count=0; level_count<num_levels; level_count++) {
 
	Cout << "\n<<<<< Performing importance sampling for response function "
           << resp_fn_count+1 << " level " << level_count+1 << '\n';
 
	Real z = requestedRespLevels[resp_fn_count][level_count];
        Real p_first = computedProbLevels[resp_fn_count][level_count];
	Cout << "z " << z << " pfirst " << p_first << '\n';
	initialize(allSamples, x_data_flag, resp_fn_count, p_first, z);
 
	// select initial set of representative points
        select_rep_points(initPointsU, init_fns);
	// perform refinement iterations
	converge_statistics(cov_flag);
	// update bookkeeping
        computedProbLevels[resp_fn_count][level_count] = probEstimate;
      }
    }
    // infer PDFs from computedProbLevels
    compute_densities(extremeValues, true);
    // propagate computedProbLevels to finalStatistics
    update_final_statistics();
  }
  else {
    // no initial LHS: refine only the active respFnIndex starting
    // from the initial points passed into initialize()

    // compute response samples for the initial points
    evaluate_samples(initPointsU, init_fns);
    // select initial set of representative points
    select_rep_points(initPointsU, init_fns);
    // perform refinement iterations
    converge_statistics(cov_flag);
  }
}


void NonDAdaptImpSampling::
select_rep_points(const RealVectorArray& var_samples_u,
		  const RealVector& fn_samples)
{
  // pick out failures from input samples
  // calculate beta for failures only
  //TMW: Modified this to exclude the previous repPointsU
  size_t i, j, cntr, fail_count = 0, safe_count = 0,
    num_samples = var_samples_u.size();
  RealArray  fail_betas;   fail_betas.reserve(num_samples);
  SizetArray fail_indices; fail_indices.reserve(num_samples);
  RealArray  safe_dist;   safe_dist.reserve(num_samples);
  SizetArray safe_indices; safe_indices.reserve(num_samples);

  //TMW: Additional variables needed if no failures are found
  //size_t closestpt;
  //Real mindist = DBL_MAX, closestbeta, ddist;

  for (i=0; i<num_samples; i++) {
    //TMW: Modified this to exclude the previous repPointsU
    //const RealVector& sample_i = (i<numRepPoints) ? repPointsU[i] :
    //  var_samples_u[i-numRepPoints];
    const RealVector& sample_i = var_samples_u[i];
    //TMW: We now use the stored values rather than recomputing
    Real limit_state_fn = fn_samples[i];
    if ( ( limit_state_fn < failThresh &&
	   ( (!invertProb && cdfFlag) || (invertProb && !cdfFlag) ) ) ||
	 ( limit_state_fn > failThresh &&
	   ( (!invertProb && !cdfFlag) || (invertProb &&  cdfFlag) ) ) ) {
      fail_indices.push_back(i);
      fail_betas.push_back(sample_i.normFrobenius());
      ++fail_count;
    }
    else { //TMW: Determine the distance from the failThresh
      safe_indices.push_back(i);
      safe_dist.push_back(std::abs(limit_state_fn-failThresh));
      ++safe_count;
    }
  }

  // remove points too close together
  // start with most probable failure point (smallest beta)
  // remove all points within the cutoff distance of this point
  // pick most probable point from remaining failure points
  // remove all points within the cutoff distance of this point
  // repeat until all failure points exhausted
  int maxRepPts = (importanceSamplingType == MMAIS) ? num_samples : 1;

  Real cutoff_distance = 1.5;
  IntArray min_indx(maxRepPts);
  size_t new_rep_pts = 0;
  bool exhausted = false;
  if (fail_count > 0) {
    while (!exhausted && new_rep_pts < maxRepPts) {
      Real min_beta = 100.;
      for (i=0; i<fail_count; i++) {
        Real beta = fail_betas[i];
        if (beta < min_beta) {
	  min_beta = beta;
	  min_indx[new_rep_pts] = i;
        }
      }
      if (min_beta == 100.)  
        exhausted = true;
      else {
        size_t idx = min_indx[new_rep_pts], fail_idx = fail_indices[idx];
        //TMW: Modified this to exclude the previous repPointsU
        //const RealVector& ref = (fail_idx < numRepPoints) ?
	  //repPointsU[fail_idx] : var_samples_u[fail_idx - numRepPoints];
        const RealVector& ref = var_samples_u[fail_idx];
        for (i=0; i<fail_count; ++i)
	  if (fail_betas[i] < 99.) {
	    fail_idx = fail_indices[i];
            //TMW: Modified this to exclude the previous repPointsU
	    //const RealVector& pt = (fail_idx < numRepPoints) ?
	      //repPointsU[fail_idx] : var_samples_u[fail_idx - numRepPoints];
	    if (distance(var_samples_u[fail_idx], ref) < cutoff_distance)
	      fail_betas[i] = 100.;
	  }
        ++new_rep_pts;
      }
    }
  }
  else { 
    // TMW: If no points are found in the failure region, we follow a similar 
    // procedure and select the points closest to the failThresh
    while (!exhausted && new_rep_pts < maxRepPts) {
      Real min_dist = 1.0E10;
      for (i=0; i<safe_count; i++) {
        Real dist = safe_dist[i];
        if (dist < min_dist) {
	  min_dist = dist;
	  min_indx[new_rep_pts] = i;
        }
      }
      if (min_dist == 1.0E10)  
        exhausted = true;
      else {
        size_t idx = min_indx[new_rep_pts], safe_idx = safe_indices[idx];
        const RealVector& ref = var_samples_u[safe_idx];
        for (i=0; i<safe_count; ++i)
	  if (safe_dist[i] < 1.0E10) {
	    safe_idx = safe_indices[i];
	    if (distance(var_samples_u[safe_idx], ref) < cutoff_distance)
	      safe_dist[i] = 1.0E10;
	  }
        ++new_rep_pts;
      }
    }

  }

  // store samples in min_indx in repPointsU
  //RealVectorArray prev_rep_pts = repPointsU;

  // define repPointsU and calculate repWeights
  repPointsU.resize(new_rep_pts);
  repWeights.sizeUninitialized(new_rep_pts);
  Real sum_density = 0., rep_pdf;
  for (i=0; i<new_rep_pts; ++i) {
    size_t idx = min_indx[i]; 
    size_t pt_idx;
    if (fail_count > 0)
      pt_idx = fail_indices[idx];
    else
      pt_idx = safe_indices[idx];
    //TMW: Modified this to exclude the previous repPointsU
    //repPointsU[i] = (fail_idx < numRepPoints) ? prev_rep_pts[fail_idx] :
      //var_samples_u[fail_idx - numRepPoints];
    repPointsU[i] = var_samples_u[pt_idx];

    //Real phi_beta = Pecos::NormalRandomVariable::
    //  std_pdf(repPointsU[i].normFrobenius());
    //repWeights[i] = phi_beta;
    //sum_density  += phi_beta;

    rep_pdf = 1.;
    for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
      rep_pdf *= natafTransform.u_pdf(repPointsU[i][cntr], j);

    repWeights[i] = rep_pdf;
    sum_density  += rep_pdf;
  }
  repWeights.scale(1./sum_density);

#ifdef DEBUG //TMW: Debug output to monitor the repPointsU
  Cout << "number of representative points " << new_rep_pts << '\n';
  Cout << "select_rep_point(): #Points = " << repPointsU.size()<< std::endl;
  for (i=0; i<new_rep_pts; i++){
      Cout << " Point " << i << " =  " << repPointsU[i] << '\n';
      Cout << " Beta  " << repPointsU[i].normFrobenius() << '\n';;
  }
#endif
}


void NonDAdaptImpSampling::converge_statistics(bool cov_flag)
{
  if (repPointsU.empty()) {
    // should not happen due to TMW changes to select_rep_points()
    Cerr << "Error: no representative points in NonDAdaptImpSampling::"
	 << "converge_statistics()" << std::endl;
    abort_handler(-1);

    // If no represenative points were found, then p=0 (or 1)
    // If p(success) calculated, set final p accordingly
    //probEstimate = (invertProb) ? 1. : 0.;
    //return;
  }

  // iteratively sample and select representative points until COV & 
  // probability converge
  //Real cov_abs_tol = .0001, p_abs_tol = .000001; // absolute tolerances
  RealVectorArray var_samples_u(refineSamples);
  RealVector fn_samples(refineSamples);
  size_t total_samples = 0,
    max_iter    = (maxIterations < 0) ? 100 : maxIterations, // default to 100
    max_samples = refineSamples * max_iter;
  Real sum_var = 0., cov, old_cov = 0., p, sum_p = 0.,
    old_p = (invertProb) ? 1. - probEstimate : probEstimate;
  bool converged = false, p_converged, cov_converged;

  while (!converged && total_samples < max_samples) {
    // generate samples based on multimodal sampling density
    generate_samples(var_samples_u);
    evaluate_samples(var_samples_u, fn_samples);
    total_samples += refineSamples;

    // calculate probability and, optionally, coeff of variation
    calculate_statistics(var_samples_u, fn_samples, total_samples, sum_p, p,
			 cov_flag, sum_var, cov);
#ifdef DEBUG
    Cout << "converge_statistics(): old_p = " << old_p << " p = " << p;
    if (cov_flag) Cout << " old_cov = " << old_cov << " cov = " << cov;
    Cout << std::endl;
#endif // DEBUG

    if (importanceSamplingType == IS) // for IS, only perform one iteration
      converged = true;
    else {
      // COV convergence metrics
      cov_converged = !cov_flag;
      if (cov_flag) { // optional COV convergence metric
	if (std::abs(old_cov) > 0. && std::abs(cov) > 0. &&
	    std::abs(cov/old_cov - 1.) < convergenceTol) // relative cov
	  cov_converged = true;
	//else if (std::abs(cov - old_cov) < cov_abs_tol) // absolute cov
	//  cov_converged = true;
      }

      // probability convergence metrics:
      // enforce both probabilities to be non-0/1 and employ a relative tol
      // (during refinement, this enforces non-0/1 p for 2 consecutive
      // iterations).  If the true p is 0 or 1, then we will incur the
      // overhead of max_iter in seeking separation.
      p_converged = false;
      if (p > 0. && p < 1. && old_p > 0. && old_p < 1. &&
	  std::abs(p/old_p - 1.) < convergenceTol) // relative p
	p_converged = true;
      //else if (p > 0. && p < 1. && std::abs(p - old_p) < p_abs_tol)// absolute
      //  p_converged = true;

      // current logic: require both metrics for same cycle
      converged = (p_converged && cov_converged);
      if (!converged) {
	select_rep_points(var_samples_u, fn_samples); // select new repPoints
	old_p = p;
	if (cov_flag) old_cov = cov;
      }
    }
  }
  // If p(success) calculated, return 1-p 
  probEstimate = (invertProb) ? 1. - p : p;
#ifdef DEBUG
  Cout << "converge_statistics(): final p = " << probEstimate << std::endl;
#endif
}


void NonDAdaptImpSampling::generate_samples(RealVectorArray& var_samples_u)
{
  // generate std normal samples

  size_t i, j, cntr, num_rep_pts = repPointsU.size(), num_rep_samples;
  RealVector n_std_devs(numUncertainVars, false),
    n_l_bnds(numUncertainVars, false), n_u_bnds(numUncertainVars, false);
  n_std_devs = 1.;

  if (useModelBounds) {
    // Apply a mixture of distribution bounds (when defined) and inferred/global
    // bounds (when distribution bounds are undefined) to the std normal
    // samples.  This feature is used for surrogates with bounded build data
    // (e.g., GPs in EGRA --> prevents the rep point selection from wandering
    // off and extrapolating on the GP trend function).
    const RealVector& c_l_bnds = uSpaceModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = uSpaceModel.continuous_upper_bounds();
    for (i=0; i<numUncertainVars; ++i) {
      n_l_bnds[i] = c_l_bnds[i+numContDesVars];
      n_u_bnds[i] = c_u_bnds[i+numContDesVars];
    }
  }
  else {
    // Apply u-space distribution bounds to the std normal samples to avoid
    // violating physical constraints.  This is the standard mode for cases w/o
    // artificial bound constraints (PCE, SC, local reliability, stand-alone).
    // Note: local reliability employs artificial bounds for the MPP search,
    // but these are not relevant for the AIS process on the truth model.
    RealRealPairArray u_bnds = natafTransform.u_bounds(); // all active cv
    for (i=0, j=numContDesVars; i<numUncertainVars; ++i, ++j)
      { n_l_bnds[i] = u_bnds[j].first; n_u_bnds[i] = u_bnds[j].second; }
  }

  // generate u-space samples by centering std normals around rep points
  RealMatrix lhs_samples_array;
  // Equal apportionment:
  //int num_rep_samples = refineSamples / num_rep_pts,
  //    remainder       = refineSamples % num_rep_pts;
  for (i=0, cntr=0; i<num_rep_pts; ++i) {

    // Equal apportionment:
    //if (remainder)
    //  { ++num_rep_samples; --remainder; }

    // Unequal apportionment:
    if (num_rep_pts == 1)
      num_rep_samples = refineSamples;
    else if (i == num_rep_pts - 1) // last set: include all remaining samples
      num_rep_samples = (refineSamples > cntr) ? refineSamples - cntr : 0;
    else // apportion refineSamples among repPointsU based on repWeights
      num_rep_samples = std::min(refineSamples - cntr,
	(size_t)std::floor(repWeights[i] * refineSamples + .5));
    //Cout << "num_rep_samples = " << num_rep_samples << std::endl;

    // center std normals around i-th rep point
    if (num_rep_samples) {
      initialize_lhs(false, num_rep_samples);
      RealSymMatrix correl;
      lhsDriver.generate_normal_samples(repPointsU[i], n_std_devs,
	n_l_bnds, n_u_bnds, num_rep_samples, correl, lhs_samples_array);

      // copy sample set from lhs_samples_array into var_samples_u
      for (j=0; j<num_rep_samples && cntr<refineSamples; ++j, ++cntr)
	copy_data(lhs_samples_array[j], (int)numUncertainVars,
		  var_samples_u[cntr]);
    }
  }
}


void NonDAdaptImpSampling::
evaluate_samples(const RealVectorArray& var_samples_u, RealVector& fn_samples)
{
  size_t i, j, k, cntr, num_samples = var_samples_u.size();
  if (fn_samples.length() != num_samples)
    fn_samples.sizeUninitialized(num_samples);

  // update designPoint once; update uncertain vars for each sample
  for (j=0; j<numContDesVars; ++j)
    uSpaceModel.continuous_variable(designPoint[j], j);

  // calculate the probability of failure
  ActiveSet set = uSpaceModel.current_response().active_set(); // copy
  set.request_values(0); set.request_value(1, respFnIndex);
  bool asynch_flag = uSpaceModel.asynch_flag();
  for (i=0; i<num_samples; i++) {
    const RealVector& sample_i = var_samples_u[i];
    for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
      uSpaceModel.continuous_variable(sample_i[cntr], j);

    // get response value at the sample point
    if (asynch_flag) // set from uSpaceModel for stand-alone or on-the-fly
      uSpaceModel.evaluate_nowait(set);
    else {
      uSpaceModel.evaluate(set);
      fn_samples[i]
	= uSpaceModel.current_response().function_value(respFnIndex);
    }
  }

  if (asynch_flag) {
    const IntResponseMap& resp_map = uSpaceModel.synchronize();
    IntRespMCIter r_cit;
    for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit)
      fn_samples[i] = r_cit->second.function_value(respFnIndex);
  }

  // optionally track min and max response values
  if (trackExtremeValues) {
    Real min = extremeValues[respFnIndex].first,
         max = extremeValues[respFnIndex].second, val;
    for (i=0; i<num_samples; i++) {
      val = fn_samples[i];
      if (val < min) min = val;
      if (val > max) max = val;
    }
    extremeValues[respFnIndex].first  = min;
    extremeValues[respFnIndex].second = max; 
  }
}


void NonDAdaptImpSampling::
calculate_statistics(const RealVectorArray& var_samples_u,
		     const RealVector& fn_samples, size_t total_samples,
		     Real& sum_prob, Real& prob, bool compute_cov,
		     Real& sum_var, Real& cov)
{
  // Note: The current beta calculation assumes samples input in u-space
  size_t i, j, k, cntr, batch_size = var_samples_u.size(),
    num_rep_pts = repPointsU.size();
  Real n_std_devs = 1., pdf_ratio;
  RealArray failure_ratios;
  if (compute_cov)
    failure_ratios.reserve(batch_size);

  // calculate the probability of failure using all samples relative
  // to each of the representative points
  for (i=0; i<batch_size; i++) {

    // if point is a failure, calculate recentered_pdf, pdf, and add to sum
    if ( ( fn_samples[i] < failThresh &&
	   ( (!invertProb &&  cdfFlag) || (invertProb && !cdfFlag) ) ) ||
	 ( fn_samples[i] > failThresh &&
	   ( (!invertProb && !cdfFlag) || (invertProb &&  cdfFlag) ) ) ) {

      const RealVector& sample_i = var_samples_u[i];

      // calculate ratio of pdf relative to origin to pdf relative to rep pt
      //pdf_ratio1
      //  = Pecos::NormalRandomVariable::std_pdf(sample_i.normFrobenius())
      //  / recentered_pdf;

      pdf_ratio = 1.;
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
	pdf_ratio *= natafTransform.u_pdf(sample_i[cntr], j);
      pdf_ratio /= recentered_density(sample_i);

      // add sample's contribution to sum_prob
      sum_prob += pdf_ratio;
      // if cov requested, store ratio data to avoid recalculating
      if (compute_cov)
	failure_ratios.push_back(pdf_ratio);
    }
  }

  /* Alternate approach computes probs for point sets only w.r.t. corresponding
     rep pt.  This does not appear to be correct/consistent w/ multimodal PDF.
  for (i=0, cntr=0; i<num_rep_pts; ++i) {
    // loop over all of sample batch, but associate with the correct rep point
    const RealVector& rep_pt = repPointsU[i];
    Real              rep_wt = repWeights[i];

    if (i == num_rep_pts - 1) // last set: include all remaining lhs samples
      rep_batch_size = (cntr < batch_size) ? batch_size - cntr : 0;
    else // apportion batch_size among repPointsU based on repWeights
      rep_batch_size = (num_rep_pts > 1) ?
	std::max(1, int(rep_wt * batch_size)) : batch_size;

    // recenter std normals around i-th rep point
    for (j=0; j<rep_batch_size && cntr<batch_size; ++j, ++cntr) {

      // if point is a failure, calculate pdf ratio and add to sum
      if ( ( fn_samples[cntr] < failThresh &&
	     ( cdfFlag || (invertProb && !cdfFlag) ) ) ||
	   ( fn_samples[cntr] > failThresh &&
	     (!cdfFlag || (invertProb &&  cdfFlag) ) ) ) {
	const RealVector& rep_sample = var_samples_u[cntr];

	// calculate recentered u-space pdf
        recentered_pdf = rep_wt *
	  Pecos::NormalRandomVariable::std_pdf(distance(rep_pt, rep_sample)
	                                       / n_std_devs);

	// calculate ratio of pdf relative to origin to pdf relative to rep pt
        pdf_ratio = Pecos::NormalRandomVariable::
	  std_pdf(rep_sample.normFrobenius()) / recentered_pdf;

	// add sample's contribution to sum
	sum_prob += pdf_ratio;
	// if cov requested, store failure data to avoid recalculating
	if (compute_cov)
	  failure_ratios.push_back(pdf_ratio);
      }
    }
  }
  */
  prob = sum_prob/double(total_samples);
  if (prob > 1.0) {
    prob = 1.0; 
    Cerr << "\nWarning: the probability calculated by importance sampling is " 
         << "greater than 1.0 due to numerical issues.  We set the probability "
         << "to 1.0 in this case.\n";
  }
  // compute the coeff of variation if requested
  if (compute_cov) {
    if (prob > 0.) {
      size_t num_failures = failure_ratios.size();
      for (i=0; i<num_failures; i++)
	sum_var += std::pow(failure_ratios[i] - prob, 2);
      Real var = sum_var/double(total_samples)/double(total_samples - 1);
      cov = std::sqrt(var) / prob;
    }
    else
      cov = 0.;
  }
}


Real NonDAdaptImpSampling::recentered_density(const RealVector& sample_point) 
{
  size_t i, j, k, num_rep_pts = repPointsU.size();

  // Previous code:
  //recentered_pdf =  0.;
  //for (j=0; j<num_rep_pts; ++j)
  //  recentered_pdf += repWeights[j] * Pecos::NormalRandomVariable::
  //    std_pdf(distance(repPointsU[j], sample_i) / n_std_devs);

  RealRealPairArray u_bnds = natafTransform.u_bounds(); // all active cv
  Real local_pdf = 0., rep_pdf, stdev = 1.;
  for (i=0; i<num_rep_pts; ++i) {
    rep_pdf = 1.;
    const RealVector& rep_pt_i = repPointsU[i];
    for (j=0, k=numContDesVars; j<numUncertainVars; ++j, ++k)
      rep_pdf *= Pecos::BoundedNormalRandomVariable::pdf(sample_point[j],
	rep_pt_i[j], stdev, u_bnds[k].first, u_bnds[k].second);
    local_pdf += repWeights[i] * rep_pdf;
  } 
  return local_pdf;
}


void NonDAdaptImpSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on the importance sampling calculations:\n";
    print_level_mappings(s);
  }
}

} // namespace Dakota
