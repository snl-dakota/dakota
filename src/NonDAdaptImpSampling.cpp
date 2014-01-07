/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

static const char rcsId[] = "@(#) $Id: NonDAdaptImpSampling.cpp 4058 2006-10-26 01:39:40Z lpswile $";

//#define DEBUG

namespace Dakota {


/** This is the primary constructor.  It accepts a Model reference. */
NonDAdaptImpSampling::NonDAdaptImpSampling(Model& model):
  NonDSampling(model), initLHS(true),
  // if initial points in x-space, they must be transformed because method
  //   expects all points in u-space
  transInitPoints(true),
  // if model is in x-space then future points generated in u-space will
  //   need to be transformed prior to evaluation
  transPoints(true),
  // should the model bounds be respected?
  useModelBounds(false),
  // invert the sense of the refinement
  invertProb(false),
  // size of refinement batches is separate from initial LHS size (numSamples)
  refineSamples(probDescDB.get_int("method.nond.refinement_samples"))
{
  // if separate refinement batch size not provided, reuse initial LHS size
  if (!refineSamples) refineSamples = numSamples;

  statsFlag = true;
  initialize_random_variables(STD_NORMAL_U);
  const String& int_refine
    = probDescDB.get_string("method.nond.integration_refinement");
  if (int_refine.empty() ||
      int_refine == "mmais")    importanceSamplingType = MMAIS;
  else if (int_refine ==  "is") importanceSamplingType =    IS;
  else if (int_refine == "ais") importanceSamplingType =   AIS;

  // maxConcurrency defined from initial LHS size (numSamples)
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
NonDAdaptImpSampling::
NonDAdaptImpSampling(Model& model, const String& sample_type,
		     int refine_samples, int refine_seed, const String& rng,
		     bool vary_pattern, short is_type, bool cdf_flag,
		     bool x_space_data, bool x_space_model, bool bounded_model):
  NonDSampling(NoDBBaseConstructor(), model, sample_type, 0, refine_seed, rng,
	       vary_pattern, ALEATORY_UNCERTAIN), // only sample aleatory vars
  initLHS(false), importanceSamplingType(is_type),
  transInitPoints(x_space_data), transPoints(x_space_model),
  useModelBounds(bounded_model), invertProb(false),
  refineSamples(refine_samples)
{
  cdfFlag = cdf_flag;
  if (refineSamples)
    maxConcurrency *= refineSamples;
}


/** Initializes data using a vector array of starting points. */
void NonDAdaptImpSampling::
initialize(const RealVectorArray& acv_points, size_t resp_index,
	   const Real& initial_prob, const Real& failure_threshold)
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
    if (transInitPoints) {
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
initialize(const RealMatrix& acv_points, size_t resp_index,
	   const Real& initial_prob, const Real& failure_threshold)
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
    if (transInitPoints) {
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
initialize(const RealVector& acv_point, size_t resp_index,
	   const Real& initial_prob, const Real& failure_threshold)
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
  if (transInitPoints) {
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


void NonDAdaptImpSampling::quantify_uncertainty()
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
    get_parameter_sets(iteratedModel); // generates numSamples points
    evaluate_parameter_sets(iteratedModel, true, false);
    compute_statistics(allSamples, allResponses);

    init_fns.sizeUninitialized(numSamples);
    size_t level_count, resp_fn_count;
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
	initialize(allSamples, resp_fn_count, p_first, z);
 
	// select initial set of representative points
        select_rep_points(initPointsU, init_fns);
	// perform refinement iterations
	converge_statistics(cov_flag);
	// update bookkeeping
        computedProbLevels[resp_fn_count][level_count] = probEstimate;
      }
    }
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
  size_t i, j, cntr, fail_count = 0, num_samples = var_samples_u.size();
  RealArray  fail_betas;   fail_betas.reserve(num_samples);
  SizetArray fail_indices; fail_indices.reserve(num_samples);

  //TMW: Additional variables needed if no failures are found
  size_t closestpt;
  Real mindist = DBL_MAX, closestbeta, ddist;

  // store designPoint in acv_sample, append uncertain samples later
  RealVector acv_sample, acv_x_sample;
  if (transPoints) {
    acv_sample.sizeUninitialized(numContinuousVars);
    for (j=0; j<numContDesVars; ++j)
      acv_sample[j] = designPoint[j];
    acv_x_sample.sizeUninitialized(numContinuousVars);
  }
  else
    for (j=0; j<numContDesVars; ++j)
      iteratedModel.continuous_variable(designPoint[j], j);

  for (i=0; i<num_samples; i++) {
    //TMW: Modified this to exclude the previous repPointsU
    //const RealVector& sample_i = (i<numRepPoints) ? repPointsU[i] :
    //  var_samples_u[i-numRepPoints];
    const RealVector& sample_i = var_samples_u[i];
    //TMW: We now use the stored values rather than recomputing
    Real limit_state_fn = fn_samples[i];
    if ( ( limit_state_fn < failThresh &&
	   ( cdfFlag || (invertProb && !cdfFlag) ) ) ||
	 ( limit_state_fn > failThresh &&
	   (!cdfFlag || (invertProb &&  cdfFlag) ) ) ) {
      fail_indices.push_back(i);
      fail_betas.push_back(sample_i.normFrobenius());
      ++fail_count;
    }
    else { //TMW: Determine if this point is closest to the failThresh
      ddist = std::abs(limit_state_fn - failThresh);
      if (ddist < mindist) {
	mindist = ddist;
	closestpt = i;
	closestbeta = sample_i.normFrobenius();
      }
    }
  }

  //TMW: If no points are found in the failure domain, take the closest one
  if (!fail_count) {
    fail_count = 1;
    fail_indices.push_back(closestpt);
    fail_betas.push_back(closestbeta);
  }

  // remove points too close together
  // start with most probable failure point (smallest beta)
  // remove all points within the cutoff distance of this point
  // pick most probable point from remaining failure points
  // remove all points within the cutoff distance of this point
  // repeat until all failure points exhausted
  Real cutoff_distance = 1.5;
  IntArray min_indx(fail_count);
  size_t new_rep_pts = 0;
  bool exhausted = false;
  while (!exhausted) {
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

  // store samples in min_indx in repPointsU
  //RealVectorArray prev_rep_pts = repPointsU;

  // define repPointsU and calculate repWeights
  repPointsU.resize(new_rep_pts);
  repWeights.sizeUninitialized(new_rep_pts);
  Real sum_density = 0.;
  for (i=0; i<new_rep_pts; ++i) {
    size_t idx = min_indx[i], fail_idx = fail_indices[idx];
    //TMW: Modified this to exclude the previous repPointsU
    //repPointsU[i] = (fail_idx < numRepPoints) ? prev_rep_pts[fail_idx] :
      //var_samples_u[fail_idx - numRepPoints];
    repPointsU[i] = var_samples_u[fail_idx];

    Real phi_beta = Pecos::phi(repPointsU[i].normFrobenius());
    repWeights[i] = phi_beta;
    sum_density  += phi_beta;
  }
  repWeights.scale(1./sum_density);

#ifdef DEBUG //TMW: Debug output to monitor the repPointsU
  Cout << "select_rep_point(): #Points = " << repPointsU.size()
       << " Point =  " << repPointsU[0] << std::endl;
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
  size_t total_samples = 0, max_samples = refineSamples*maxIterations;
  Real sum_var = 0., cov, old_cov = DBL_MAX, p, sum_p = 0.,
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
	if (std::abs(old_cov) > 0. &&
	    std::abs(cov/old_cov - 1.) < convergenceTol) // relative cov
	  cov_converged = true;
	//else if (std::abs(cov - old_cov) < cov_abs_tol) // absolute cov
	//  cov_converged = true;
      }

      // probability convergence metrics:
      // enforce both probabilities to be non-0/1 and employ a relative tol
      // (during refinement, this enforces non-0/1 p for 2 consecutive
      // iterations).  If the true p is 0 or 1, then we will incur the
      // overhead of maxIterations in seeking separation.
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

  size_t i, j, k, cntr, num_rep_pts = repPointsU.size();
  RealVector n_means(numUncertainVars), // init to 0
    n_std_devs(numUncertainVars, false), n_l_bnds(numUncertainVars, false),
    n_u_bnds(numUncertainVars, false);
  n_std_devs = 1.;//1.;
  // Bound the sampler if necessary - this is needed for NonDGlobalReliability
  // because the Gaussian Process model is only accurate within these bounds
  if (useModelBounds) {
    // TO DO: enforce that iteratedModel is a u-space model?
    const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
    for (i=numContDesVars, cntr=0; i<numContinuousVars; ++i, ++cntr) {
      n_l_bnds[cntr] = c_l_bnds[i];
      n_u_bnds[cntr] = c_u_bnds[i];
    }
  }
  else {
    n_l_bnds = -DBL_MAX;
    n_u_bnds =  DBL_MAX;
  }
  initialize_lhs(false);
  RealMatrix lhs_samples_array;
  lhsDriver.generate_normal_samples(n_means, n_std_devs, n_l_bnds, n_u_bnds,
				    refineSamples, lhs_samples_array);

  // generate u-space samples by adding std normals to rep points

  int num_rep_samples;
  for (i=0, cntr=0; i<num_rep_pts; ++i) {

    if (i == num_rep_pts - 1) // last set: include all remaining lhs samples
      num_rep_samples = (refineSamples > cntr) ? refineSamples - cntr : 0;
    else // apportion refineSamples among repPointsU based on repWeights
      num_rep_samples = (num_rep_pts > 1) ?
	std::max(1, int(repWeights[i]*refineSamples)) : refineSamples;

    // recenter std normals around i-th rep point
    const RealVector& rep_pt_i = repPointsU[i];
    for (j=0; j<num_rep_samples && cntr<refineSamples; ++j, ++cntr) {
      Real* lhs_sample = lhs_samples_array[cntr];
      RealVector& rep_sample = var_samples_u[cntr];
      rep_sample.sizeUninitialized(numUncertainVars);
      for (k=0; k<numUncertainVars; ++k)
	rep_sample[k] = lhs_sample[k] + rep_pt_i[k];
    }
  }
}


void NonDAdaptImpSampling::
evaluate_samples(const RealVectorArray& var_samples_u, RealVector& fn_samples)
{
  size_t i, j, k, cntr, num_samples = var_samples_u.size();
  if (fn_samples.length() != num_samples)
    fn_samples.sizeUninitialized(num_samples);

  // store designPoint in acv_sample, append uncertain samples later
  RealVector acv_sample, acv_x_sample;
  if (transPoints) {
    acv_sample.sizeUninitialized(numContinuousVars);
    for (j=0; j<numContDesVars; ++j)
      acv_sample[j] = designPoint[j];
    acv_x_sample.sizeUninitialized(numContinuousVars);
  }
  else
    for (j=0; j<numContDesVars; ++j)
      iteratedModel.continuous_variable(designPoint[j], j);

  // calculate the probability of failure
  ActiveSet set = iteratedModel.current_response().active_set(); // copy
  set.request_values(0); set.request_value(1, respFnIndex);
  for (i=0; i<num_samples; i++) {
    const RealVector& sample_i = var_samples_u[i];

    if (transPoints) { // u-space points -> x-space Model (NonDLocal)
      // append uncertain sample to designPoint before evaluation
      // this must be done before the transformation because trans_U_to_X
      //   expectes numContinuousVars variables
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
        acv_sample[j] = sample_i[cntr];
      natafTransform.trans_U_to_X(acv_sample, acv_x_sample);
      iteratedModel.continuous_variables(acv_x_sample);
    }
    else // u-space points -> u-space Model (NonDGlobal)
      for (j=numContDesVars, cntr=0; cntr<numUncertainVars; ++j, ++cntr)
        iteratedModel.continuous_variable(sample_i[cntr], j);

    // get response value at the sample point
    iteratedModel.compute_response(set);
    fn_samples[i]
      = iteratedModel.current_response().function_value(respFnIndex);
  }
}


void NonDAdaptImpSampling::
calculate_statistics(const RealVectorArray& var_samples_u,
		     const RealVector& fn_samples, size_t total_samples,
		     Real& sum_prob, Real& prob, bool compute_cov,
		     Real& sum_var, Real& cov)
{
  // Note: The current beta calculation assumes samples input in u-space
  size_t i, j, k, cntr, num_samples = var_samples_u.size(),
    num_rep_pts = repPointsU.size();
  Real n_std_devs = 1.0;
  RealArray failure_ratios;
  if (compute_cov)
    failure_ratios.reserve(num_samples);

  // calculate the probability of failure
  for (i=0; i<num_samples; i++) {

    // if point is a failure, calculate mmpdf, pdf, and add to sum
    if ( ( fn_samples[i] < failThresh &&
	   ( cdfFlag || (invertProb && !cdfFlag) ) ) ||
	 ( fn_samples[i] > failThresh &&
	   (!cdfFlag || (invertProb &&  cdfFlag) ) ) ) {

      const RealVector& sample_i = var_samples_u[i];

      // calculate mmpdf
      Real mmpdf = 0.;
      for (j=0; j<num_rep_pts; ++j)
	mmpdf += repWeights[j] *
	  Pecos::phi(distance(repPointsU[j], sample_i) / n_std_devs);

      // calculate pdf
      Real pdf = Pecos::phi(sample_i.normFrobenius()), ratio = pdf/mmpdf;
      // add sample's contribution to sum_prob
      sum_prob += ratio;
      // if cov requested, store failure data to avoid recalculating
      if (compute_cov)
	failure_ratios.push_back(ratio);
    }
  }
  prob = sum_prob/double(total_samples);

  // compute the coeff of variation if requested
  if (compute_cov) {
    size_t num_failures = failure_ratios.size();
    for (i=0; i<num_failures; i++)
      sum_var += std::pow(failure_ratios[i] - prob, 2);
    Real var = sum_var/double(total_samples)/double(total_samples - 1);
    cov = std::sqrt(var)/prob; // MSE: possible division by zero ?
  }
}


void NonDAdaptImpSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on the importance sampling calculations:\n";
    print_distribution_mappings(s);
  }
}

} // namespace Dakota
