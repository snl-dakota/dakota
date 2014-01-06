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
  useModelBounds(false)
{ 
  statsFlag = true;
  initialize_random_variables(STD_NORMAL_U);
  const String& int_refine
    = probDescDB.get_string("method.nond.integration_refinement");
  if (int_refine.empty() ||
      int_refine == "mmais")    importanceSamplingType = MMAIS;
  else if (int_refine ==  "is") importanceSamplingType =    IS;
  else if (int_refine == "ais") importanceSamplingType =   AIS;
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
NonDAdaptImpSampling::
NonDAdaptImpSampling(Model& model, const String& sample_type, int samples,
		     int seed, const String& rng, bool vary_pattern,
		     short is_type, bool cdf_flag, bool x_space_data,
		     bool x_space_model, bool bounded_model):
  NonDSampling(NoDBBaseConstructor(), model, sample_type, samples, seed, rng,
	       vary_pattern, ALEATORY_UNCERTAIN), // only sample aleatory vars
  initLHS(false), importanceSamplingType(is_type),
  transInitPoints(x_space_data), transPoints(x_space_model),
  useModelBounds(bounded_model)
{ cdfFlag = cdf_flag; }


/** Initializes data using a vector array of starting points. */
void NonDAdaptImpSampling::
initialize(const RealVectorArray& acv_points, int resp_fn,
	   const Real& initial_prob, const Real& failure_threshold)
{
  size_t i, j, cntr, num_points = acv_points.size();
  initPointsU.resize(num_points);
  sampleVals.resize(num_points);

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

  respFn     = resp_fn;
  initProb   = initial_prob;
  failThresh = failure_threshold;
}


/** Initializes data using a matrix of starting points. */
void NonDAdaptImpSampling::
initialize(const RealMatrix& acv_points, int resp_fn,
	   const Real& initial_prob, const Real& failure_threshold)
{
  size_t i, j, cntr, num_points = acv_points.numCols();
  initPointsU.resize(num_points);
  sampleVals.resize(num_points);

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

  respFn     = resp_fn;
  initProb   = initial_prob;
  failThresh = failure_threshold;
}


/** Initializes data using only one starting point. */
void NonDAdaptImpSampling::
initialize(const RealVector& acv_point, int resp_fn,
	   const Real& initial_prob, const Real& failure_threshold)
{
  size_t j, cntr;

  if (numContDesVars) { // store inactive variable values, if present
    designPoint.sizeUninitialized(numContDesVars);
    for (j=0; j<numContDesVars; ++j)
      designPoint[j] = acv_point[j];
  }

  initPointsU.resize(1);
  sampleVals.resize(1);
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

  respFn     = resp_fn;
  initProb   = initial_prob;
  failThresh = failure_threshold;
}


void NonDAdaptImpSampling::quantify_uncertainty()
{
  if (initLHS) {
    // Get initial set of LHS points.  I may need to do something 
    // fancier here, but as a default this is performing LHS sampling 
    // over the original distributions, evaluating the points, and 
    // computing statistics to get the initial probability estimates.
    get_parameter_sets(iteratedModel);
    bool log_resp_flag = (allDataFlag || statsFlag);
    evaluate_parameter_sets(iteratedModel, log_resp_flag, false);
    compute_statistics(allSamples, allResponses);

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
      size_t i, num_all = allResponses.size();
      sampleVals.resize(num_all);
      for (i=0; i<num_all; i++)
	sampleVals[i] = allResponses[i+1].function_value(resp_fn_count);
      
      // Loop over response/probability/reliability levels
      for (level_count=0; level_count<num_levels; level_count++) {
 
	Cout << "\n<<<<< Performing importance sampling for response function "
           << resp_fn_count+1 << " level " << level_count+1 << '\n';
 
	Real z = requestedRespLevels[resp_fn_count][level_count];
        Real p_first = computedProbLevels[resp_fn_count][level_count];
	Cout << "z " << z << " pfirst " << p_first << '\n';
	initialize(allSamples, resp_fn_count, p_first, z);
 
        // for now, hardcode to MMAIS
        // iteratively generate samples and select representative points
        //   until coefficient of variation converges

        //TMW: Only run converge_cov() for MMAIS
        if (importanceSamplingType == MMAIS)
          converge_cov();

        select_rep_points(initPointsU);

        if (numRepPoints)
	  // iteratively generate samples from final set of
	  // representative points until probability converges
	  converge_probability();
	else { // If no representative points were found, then p=0 (or 1)
	  // If p(success) calculated, set finalProb accordingly
	  if (invertProb) {
	    finalProb = 1.;
	    cdfFlag = !cdfFlag; // reset for next requested response level
	  }
	  else
	    finalProb = 0.;
        }

#ifdef DEBUG
        Cout << "Final Probability " << finalProb << '\n';
#endif
        computedProbLevels[resp_fn_count][level_count] = finalProb;
      }
    }
  }
  else if (importanceSamplingType == IS || importanceSamplingType == AIS) {

    evaluate_samples(initPointsU);

    // select representative points
    select_rep_points(initPointsU);

    // iteratively generate samples from input set of points until
    //   probability converges (IS only performs one iteration)
    converge_probability();
  }
  else if (importanceSamplingType == MMAIS) {
    // iteratively generate samples and select representative points
    //   until coefficient of variation converges
    converge_cov();
    
    if (numRepPoints)
      // iteratively generate samples from final set of representative points
      //   until probability converges
      converge_probability();
    else { // If no represenative points were found, then p=0 (or 1)
      // If p(success) calculated, set p accordingly
      if (invertProb) {
	finalProb = 1.;
	cdfFlag = !cdfFlag; // reset for next requested response level
      }
      else
	finalProb = 0.;
    }
  }
}


void NonDAdaptImpSampling::converge_cov()
{
  // select intial set of representative points from initPointsU
  // representative points & weights stored in repPointsU/repWeights
  select_rep_points(initPointsU);

  // If no representative points were found, quit
  if (numRepPoints==0) return;

  // iteratively sample and select representative points until COV converges
  //Real abs_tol = .0001; // absolute tolerance
  Real p, sum_p = 0., sum_var = 0.;
  Real cov, old_cov = DBL_MAX;
  RealVectorArray samples(numSamples);

  size_t total_samples = 0, max_samples = numSamples*maxIterations;
  bool converged = false;
  while (!converged && total_samples < max_samples) {
    // generate samples based on multimodal sampling density - the set
    //   of samples is output
    generate_samples(samples);
    total_samples += numSamples; 

    // calculate probability and coeff of variation
    calculate_statistics(samples, total_samples, sum_p, p, true, sum_var, cov);

    // check for convergence of cov
    if (std::abs(old_cov) > 0. &&
	std::abs(cov/old_cov - 1.) < convergenceTol) // relative tol
      converged = true;
    //else if (std::abs(cov - old_cov) < abs_tol)
    //  converged = true;
    else {
      old_cov = cov;
      select_rep_points(samples); // define new rep pts from new samples
    }
  }
}


void NonDAdaptImpSampling::converge_probability()
{
  // iteratively sample until probability converges
  //Real abs_tol = .000001; // absolute tolerance
  Real sum_var, cov, p, sum_p = 0., old_p = initProb;
  RealVectorArray samples(numSamples);
  size_t total_samples = 0, max_samples = numSamples*maxIterations;
  bool converged = false;

  while (!converged && total_samples < max_samples) {
    // generate samples based on multimodal sampling density - the set
    //   of samples is output
    generate_samples(samples);
    total_samples += numSamples;

    // calculate probability with this set of samples
    calculate_statistics(samples, total_samples, sum_p, p, false, sum_var, cov);
    // check for convergence of probability
    // MSE: enforce both probabilities to be non-0/1 and employ a relative tol
    //   (during refinement, this enforces non-0/1 p for 2 consecutive iters).
    //   If the true p is 0 or 1, then we will incur the overhead of maxIter.
    if (importanceSamplingType == IS) // for IS, only perform one iteration
      converged = true;
    else if (p > 0. && p < 1. && old_p > 0. && old_p < 1. &&
	     std::abs(p/old_p - 1.) < convergenceTol) // relative tol
      converged = true;
    else
      select_rep_points(samples); //TMW: Select a new repPoint
    //else if (p > 0. && p < 1. && std::abs(p - old_p) < abs_tol)// absolute tol
    //  converged = true;
#ifdef DEBUG
    Cout << "converge_probability(): old_p = " << old_p << " p = " << p
	 << std::endl;
#endif // DEBUG
    old_p = p;
  }
  // If p(success) calculated, return 1-p 
  if (invertProb) { 
    finalProb = 1.-p;
    cdfFlag = !cdfFlag; // reset for next requested response level
  }
  else
    finalProb = p;
}


void NonDAdaptImpSampling::select_rep_points(const RealVectorArray& samples_u)
{
  // pick out failures from input samples_u
  // calculate beta for failures only
  size_t i, j, cntr, fail_count = 0,

  //TMW: Modified this to exclude the previous repPointsU
  num_samples = samples_u.size();
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
    //  samples_u[i-numRepPoints];
    const RealVector& sample_i = samples_u[i];
    //TMW: We now use the stored values rather than recomputing
    Real limit_state_fn = sampleVals[i];
    if ( ( cdfFlag && limit_state_fn < failThresh ) ||
	 (!cdfFlag && limit_state_fn > failThresh ) ) {
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
	//repPointsU[fail_idx] : samples_u[fail_idx - numRepPoints];
      const RealVector& ref = samples_u[fail_idx];
      for (i=0; i<fail_count; ++i)
	if (fail_betas[i] < 99.) {
	  fail_idx = fail_indices[i];
          //TMW: Modified this to exclude the previous repPointsU
	  //const RealVector& pt = (fail_idx < numRepPoints) ?
	    //repPointsU[fail_idx] : samples_u[fail_idx - numRepPoints];
	  const RealVector& pt = samples_u[fail_idx];
	  if (distance(pt, ref) < cutoff_distance)
	    fail_betas[i] = 100.;
	}
      ++new_rep_pts;
    }
  }

  // store samples in min_indx in repPointsU
  //RealVectorArray prev_rep_pts = repPointsU;

  repPointsU.resize(new_rep_pts);
  for (i=0; i<new_rep_pts; ++i) {
    size_t idx = min_indx[i], fail_idx = fail_indices[idx];
    //TMW: Modified this to exclude the previous repPointsU
    //repPointsU[i] = (fail_idx < numRepPoints) ? prev_rep_pts[fail_idx] :
      //samples_u[fail_idx - numRepPoints];
    repPointsU[i] = samples_u[fail_idx];
  }
  numRepPoints = new_rep_pts;
#ifdef DEBUG //TMW: Debug output to monitor the repPointsU
  Cout << "select_rep_point(): #Points = " << repPointsU.size()
       << " Point =  " << repPointsU[0] << std::endl;
#endif
  calculate_rep_weights();
}


void NonDAdaptImpSampling::generate_samples(RealVectorArray& samples_u)
{
  // generate std normal samples

  size_t i, j, k, cntr;
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
				    numSamples, lhs_samples_array);

  // generate u-space samples by adding std normals to rep points

  int num_rep_samples;
  for (i=0, cntr=0; i<numRepPoints; ++i) {

    if (i == numRepPoints - 1) // last set: include all remaining lhs samples
      num_rep_samples = (numSamples > cntr) ? numSamples - cntr : 0;
    else // apportion numSamples among repPointsU based on repWeights
      num_rep_samples = (numRepPoints > 1) ?
	std::max(1, int(repWeights[i]*numSamples)) : numSamples;

    // recenter std normals around i-th rep point
    const RealVector& rep_pt_i = repPointsU[i];
    for (j=0; j<num_rep_samples && cntr<numSamples; ++j, ++cntr) {
      Real* lhs_sample = lhs_samples_array[cntr];
      RealVector& rep_sample = samples_u[cntr];
      rep_sample.sizeUninitialized(numUncertainVars);
      for (k=0; k<numUncertainVars; ++k)
	rep_sample[k] = lhs_sample[k] + rep_pt_i[k];
    }
  }

  sampleVals.resize(numSamples);
}


void NonDAdaptImpSampling::evaluate_samples(const RealVectorArray& samples_u)
{
  // store designPoint in acv_sample, append uncertain samples later
  size_t i, j, k, cntr;
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
  set.request_values(0); set.request_value(1, respFn);
  for (i=0; i<numSamples; i++) {
    const RealVector& sample_i = samples_u[i];

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
    sampleVals[i] = iteratedModel.current_response().function_values()[respFn];
  }
}


void NonDAdaptImpSampling::
calculate_statistics(const RealVectorArray& samples_u, 
		     const size_t& total_samples, Real& sum_prob, Real& prob,
		     bool compute_cov, Real& sum_var, Real& cov)
{
  // Note: The current beta calculation assumes samples input in u-space
  size_t i, j, k, cntr;
  RealArray failure_ratios;
  Real n_std_devs = 1.0;
  if (compute_cov)
    failure_ratios.reserve(numSamples);

  evaluate_samples(samples_u);

  // calculate the probability of failure
  for (i=0; i<numSamples; i++) {
    
    const RealVector& sample_i = samples_u[i];
    Real limit_state_fn = sampleVals[i];

    // if point is a failure, calculate mmpdf, pdf, and add to sum
    if ( ( cdfFlag && limit_state_fn < failThresh) ||
	 (!cdfFlag && limit_state_fn > failThresh) ) {
      // calculate mmpdf
      Real mmpdf = 0.;

      for (j=0; j<numRepPoints; ++j)
        //Cout << "repPoint = " << repPoints[j] << std::endl;

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


void NonDAdaptImpSampling::calculate_rep_weights()
{
  // calculate repWeights
  RealVector rep_density(numRepPoints);
  Real sum_density = 0.;
  for (size_t i=0; i<numRepPoints; i++) {
    Real phi_beta = Pecos::phi(repPointsU[i].normFrobenius());
    rep_density[i] = phi_beta;
    sum_density   += phi_beta;
  }
  repWeights.sizeUninitialized(numRepPoints);
  for (size_t i=0; i<numRepPoints; i++)
    repWeights[i] = rep_density[i]/sum_density;
}


void NonDAdaptImpSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on the importance sampling calculations:\n";
    print_distribution_mappings(s);
  }
}

} // namespace Dakota
