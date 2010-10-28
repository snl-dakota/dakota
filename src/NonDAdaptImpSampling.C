/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDAdaptImpSampling - Adaptive Importance Sampling
//- Description: Implementation code for NonDAdaptImpSampling class
//- Owner:       Barron Bichon and Laura Swiler
//- Checked by:
//- Version:

#include "NonDAdaptImpSampling.H"
#include "system_defs.h"
#include "data_types.h"
#include "RecastModel.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "NonDLHSSampling.H"
#include "pecos_stat_util.hpp"

static const char rcsId[] = "@(#) $Id: NonDAdaptImpSampling.C 4058 2006-10-26 01:39:40Z lpswile $";

//#define DEBUG

namespace Dakota {


/** This is the primary constructor.  It accepts a Model reference. */
NonDAdaptImpSampling::NonDAdaptImpSampling(Model& model):
  NonDSampling(model), importanceSamplingType(MMAIS), initLHS(true),
  // if initial points in x-space, they must be transformed because method
  //   expects all points in u-space
  transInitPoints(true),
  // if model is in x-space then future points generated in u-space will
  //   need to be transformed prior to evaluation
  transPoints(true),
  // should the model bounds be respected?
  useModelBounds(false)
{ 
  samplingVarsMode = ACTIVE;
  initialize_random_variables(STD_NORMAL_U);
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
NonDAdaptImpSampling::
NonDAdaptImpSampling(Model& model, int samples, int seed, const String& rng,
		     short sampling_type, bool cdf_flag, bool x_space_data,
		     bool x_space_model, bool bounded_model): 
  NonDSampling(NoDBBaseConstructor(), model, samples, seed, rng),
  initLHS(false), importanceSamplingType(sampling_type),
  // if initial points in x-space, they must be transformed because method
  //   expects all points in u-space
  transInitPoints(x_space_data),
  // if model is in x-space then future points generated in u-space will
  //   need to be transformed prior to evaluation
  transPoints(x_space_model),
  // should the model bounds be respected?
  useModelBounds(bounded_model)
{
  samplingVarsMode = ACTIVE;
  cdfFlag          = cdf_flag;
}


/** Initializes data using a set of starting points. */
void NonDAdaptImpSampling::
initialize(const RealVectorArray& initial_points, int resp_fn,
	   const Real& initial_prob, const Real& failure_threshold)
{
  size_t i, j, cntr, num_points = initial_points.size();
  initPoints.resize(num_points);

  designPoint.sizeUninitialized(numContDesVars);
  for (i=0; i<numContDesVars; i++)
    designPoint[i] = initial_points[0][i];

  RealVector point, u_point(numUncertainVars);
  for (i=0; i<num_points; i++) {
    if (transInitPoints)
      natafTransform.trans_X_to_U(initial_points[i],point);
    else
      point = initial_points[i];
    cntr = 0;
    for (j=numContDesVars; j<numContinuousVars; j++)
      u_point[cntr++] = point[j];
    initPoints[i] = u_point;
  }
#ifdef DEBUG  
  for (i=0; i<num_points; i++) {
    Cout << "Initial Point " << i << initPoints[i] << '\n';
  }
#endif
  respFn     = resp_fn;
  initProb   = initial_prob;
  failThresh = failure_threshold;
  Cout << "RespFn " << resp_fn << "  InitProb " << initProb
       << "  FailThres " << failure_threshold << '\n';
}


/** Initializes data using only one starting point. */
void NonDAdaptImpSampling::
initialize(const RealVector& initial_point, int resp_fn,
	   const Real& initial_prob, const Real& failure_threshold)
{
  size_t i, j, cntr;
  initPoints.resize(1);

  designPoint.sizeUninitialized(numContDesVars);
  for (i=0; i<numContDesVars; i++)
    designPoint[i] = initial_point[i];

  RealVector point, u_point(numUncertainVars);
  if (transInitPoints)
    natafTransform.trans_X_to_U(initial_point,point);
  else
    point = initial_point;
  cntr = 0;
  for (i=numContDesVars; i<numContinuousVars; i++)
    u_point[cntr++] = point[i];
  initPoints[0] = u_point;

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
    //Cout << "computed respprob" << computedProbLevels[0][0] << "\n";

    RealVectorArray lhs_inputs(numSamples);
    size_t i,j;
    for (i=0; i<numSamples; i++)
      copy_data(allSamples[i], (int)numContinuousVars, lhs_inputs[i]);
#ifdef DEBUG
    for (i=0; i<numSamples; i++) { 
      for (j=0; j<numContinuousVars; j++)
	Cout << "LHS input " << i << " " << j << lhs_inputs[i][j];        
      Cout << '\n';
    }
#endif

    size_t level_count, resp_fn_count;
    for (resp_fn_count=0; resp_fn_count<numFunctions; resp_fn_count++) {
      size_t rl_len = requestedRespLevels[resp_fn_count].length(),
             pl_len = requestedProbLevels[resp_fn_count].length(),
             gl_len = requestedGenRelLevels[resp_fn_count].length(),
             num_levels = rl_len + pl_len + gl_len;
    
      // Loop over response/probability/reliability levels
      for (level_count=0; level_count<num_levels; level_count++) {
 
	Cout << "\n<<<<< Performing importance sampling for response function "
           << resp_fn_count+1 << " level " << level_count+1 << '\n';
 
	Real z = requestedRespLevels[resp_fn_count][level_count];
        Real p_first = computedProbLevels[resp_fn_count][level_count];
	Cout << "z " << z << " pfirst " << p_first << '\n';
	initialize(lhs_inputs, resp_fn_count, p_first, z);
        
        // for now, hardcode to MMAIS
        // iteratively generate samples and select representative points
        //   until coefficient of variation converges
        converge_cov();

        // If no representative points were found, then p=0 (or 1)
        if (numRepPoints==0) {
	  // If p(success) calculated, set p accordingly
	  if (invertProb) {
	    finalProb = 1.;
	    // reset cdfFlag for next requested response level
	    if (cdfFlag)
	      cdfFlag = false;
	    else
	      cdfFlag = true;
	  }
	  else
	    finalProb = 0.;
        }
        else
	  // iteratively generate samples from final set of
	  // representative points until probability converges
	  converge_probability();
        Cout << "Final Probability " << finalProb << '\n';
      }
    }
  }

  else {
    if (importanceSamplingType == IS || importanceSamplingType == AIS) {

      // select representative points
      select_init_rep_points(initPoints);

      // iteratively generate samples from input set of points until
      //   probability converges (IS only performs one iteration)
      converge_probability();
    }
    else if (importanceSamplingType == MMAIS) {
      // iteratively generate samples and select representative points
      //   until coefficient of variation converges
      converge_cov();

      // If no represenative points were found, then p=0 (or 1)
      if (numRepPoints==0) {
	// If p(success) calculated, set p accordingly
	if (invertProb) {
	  finalProb = 1.;
	  // reset cdfFlag for next requested response level
	  if (cdfFlag)
	    cdfFlag = false;
	  else
	    cdfFlag = true;
	}
	else
	  finalProb = 0.;
      }
      else {
	// iteratively generate samples from final set of representative points
	//   until probability converges
	converge_probability();
      }
    }
  }
}


void NonDAdaptImpSampling::converge_cov()
{
  // select intial set of representative points from initPoints
  // representative points & weights stored in repPoints/repWeights
  select_init_rep_points(initPoints);

  // If no representative points were found, quit
  if (numRepPoints==0) return;

  // iteratively sample and select representative points until COV converges
  Real converge_tol = .0001;
  Real p, sum_p = 0., sum_var = 0.;
  Real cov, delta_cov, old_cov = DBL_MAX;
  RealVectorArray samples(numSamples);

  size_t total_samples = 0, max_iterations = 10,
    max_samples = numSamples*max_iterations;
  bool converged = false;
  while (!converged && (total_samples < max_samples) ) {
    // generate samples based on multimodal sampling density - the set
    //   of samples is output
    generate_samples(samples);
    total_samples += numSamples; 

    // calculate probability and coeff of variation
    calculate_statistics(samples, total_samples, sum_p, p, true, sum_var, cov);

    // check for convergence of cov
    delta_cov = std::abs(cov - old_cov);
    if (delta_cov < converge_tol)
      converged = true;
    else {
      old_cov = cov;

      // select rep points from among new samples & current repPoints
      RealVectorArray reps_and_samples(numRepPoints+numSamples);
      for (size_t i=0; i<numRepPoints; i++)
	reps_and_samples[i] = repPoints[i];
      for (size_t i=0; i<numSamples; i++)
	reps_and_samples[numRepPoints+i] = samples[i];

      select_rep_points(reps_and_samples);
     } 
  }
}


void NonDAdaptImpSampling::converge_probability()
{
  // iteratively sample until probability converges
  Real converge_tol = .000001;
  Real sum_var, cov, p, sum_p = 0., old_p = initProb;
  RealVectorArray samples(numSamples);
  size_t total_samples = 0, max_iterations = 1000,
    max_samples = numSamples*max_iterations;
  bool converged = false;
  while (!converged && (total_samples < max_samples) ) {
    // generate samples based on multimodal sampling density - the set
    //   of samples is output
    generate_samples(samples);
    total_samples += numSamples;
    // calculate probability with this set of samples
    calculate_statistics(samples, total_samples, sum_p, p, false, sum_var, cov);
    // check for convergence of probability
    // for IS, only perform one iteration
    if ( (std::abs(p-old_p) < converge_tol) || importanceSamplingType==IS)
      converged = true;
    old_p = p;
  }
  // If p(success) calculated, return 1-p 
  if (invertProb) { 
    finalProb = 1.-p;
    // reset cdfFlag for next requested response level
    if (cdfFlag)
      cdfFlag = false;
    else
      cdfFlag = true;
  }
  else
    finalProb = p;
}


void NonDAdaptImpSampling::
select_init_rep_points(const RealVectorArray& samples)
{
  // This differs from 'select_rep_points' because it allows a tolerance on
  //   the initial samples when determining failure to ensure that meaningful
  //   points from NonDGlobalReliability that are near the limit state (but on 
  //   the wrong side of it) are not discarded

  // pick out failures from input samples
  // calculate beta for failures only
  size_t i, j, cntr, num_samples = samples.size(), fail_count = 0;
  RealVector sample_fns(num_samples);
  RealVector whole_sample(numContinuousVars, false);

  // store designPoint in whole_sample, append uncertain samples later
  for (i=0; i<numContDesVars; i++)
    whole_sample[i] = designPoint[i];

  // Calculate failure tolerance = min(1% of range, 0.5)
  // If only one sample present (MPP), then
  //   skip this and use MPP as only initial sample
  Real fail_tol;
  if (num_samples > 1) {
    Real min_fn = DBL_MAX, max_fn = -DBL_MAX;
    for (i=0; i<num_samples; i++) {
      if (transPoints) { // u-space points -> x-space Model (NonDLocal)
	// append uncertain sample to designPoint before evaluation
	// this must be done before the transformation because trans_U_to_X
	//   expects numContinuousVars variables
	cntr = 0;
	for (j=numContDesVars; j<numContinuousVars; j++)
	  whole_sample[j] = samples[i][cntr++];
	RealVector sample_x;
#ifdef DEBUG
	for (j=numContDesVars; j<numContinuousVars; j++) 
           Cout << "whole sample j" << whole_sample[j] << '\n';
#endif
	natafTransform.trans_U_to_X(whole_sample, sample_x);
	iteratedModel.continuous_variables(sample_x);
      }
      else { // u-space points -> u-space Model (NonDGlobal)
	// append uncertain sample to designPoint before evaluation
	cntr = 0;
	for (j=numContDesVars; j<numContinuousVars; j++)
	  whole_sample[j] = samples[i][cntr++];
	iteratedModel.continuous_variables(whole_sample);
      }
#ifdef DEBUG
	for (j=numContDesVars; j<numContinuousVars; j++) 
           Cout << "whole sample j" << whole_sample[j] << '\n';
#endif
      iteratedModel.compute_response();
      const Response&   limit_state_resp = iteratedModel.current_response();
      const RealVector& limit_state_fns  = limit_state_resp.function_values();
      Real fn = limit_state_fns[respFn];
      if (fn < min_fn)  min_fn = fn;
      if (fn > max_fn)  max_fn = fn;
#ifdef DEBUG
      Cout << "fn " << fn << "min_fn " << min_fn << "max_fn" << max_fn << '\n';
#endif
      // store response
      sample_fns[i] = fn;
    }
    fail_tol = std::min(.01*(max_fn-min_fn),0.5); 
  }
  else
    fail_tol = 0.5;

  // If center point is a failure, calculate P_s and return 1-p
  size_t len = samples[0].length();
  RealVector center(len, true);
  
  if (transPoints) { // u-space points -> x-space Model (NonDLocal)
    // append uncertain sample to designPoint before evaluation
    // this must be done before the transformation because trans_U_to_X
    //   expects numContinuousVars variables
    cntr = 0;
    for (i=numContDesVars; i<numContinuousVars; i++)
      whole_sample[i] = center[cntr++];
    RealVector center_x;
    natafTransform.trans_U_to_X(whole_sample, center_x);
    iteratedModel.continuous_variables(center_x);
  }
  else { // u-space points -> u-space Model (NonDGlobal)
    // append uncertain sample to designPoint before evaluation
    cntr = 0;
    for (i=numContDesVars; i<numContinuousVars; i++)
      whole_sample[i] = center[cntr++];
    iteratedModel.continuous_variables(whole_sample);
  }
  
  iteratedModel.compute_response();
  const Response&   limit_state_resp = iteratedModel.current_response();
  const RealVector& limit_state_fns  = limit_state_resp.function_values();
  Real fn = limit_state_fns[respFn];
  invertProb = false; 
  if ( ( cdfFlag && fn < failThresh) || (!cdfFlag && fn > failThresh) ) {
    // center is a failure - calculate prob. of success instead
    invertProb = true; 
    // reverse the sense of failure by switching cdfFlag
    if (cdfFlag) 
      cdfFlag = false;
    else
      cdfFlag = true;
  }

#ifdef DEBUG
  Cout << "Failure threshold " << failThresh << '\n';
  Cout << "Failure tolerance " << fail_tol << '\n';
#endif
  // Find failure points among initial samples
  RealVector temp_betas(num_samples);
  RealVectorArray temp_samples(num_samples);
  for (i=0; i<num_samples; i++) {

    // pull from storage instead of recalculating
    Real fn = sample_fns[i];
#ifdef DEBUG
    Cout << "fn value near end " << fn << '\n';
#endif
    if ( ( cdfFlag && (fn < failThresh+fail_tol)) ||
	 (!cdfFlag && (fn > failThresh-fail_tol)) ) {
      temp_samples[fail_count] = samples[i];
      Real beta = 0.;
      for (j=0; j<numUncertainVars; j++) {
	beta += std::pow(samples[i][j],2);
      }
      beta = std::sqrt(beta);
      temp_betas[fail_count] = beta;
      fail_count++;
    }
  }

#ifdef DEBUG 
  Cout << "Number of failure points " << fail_count << '\n';
#endif
  // if there are no failure points, quit
  if (fail_count==0) { 
    // no failures, so p=0 (or 1)
    numRepPoints = 0;
    return;
  }

  // now the first 'fail_count' entries in temp_samples & temp_betas contain
  //   the failure samples & their beta values
  // copy these into a new data structure of this exact size
  RealVector fail_betas(fail_count);
  RealVectorArray fail_samples(fail_count);
  for (i=0; i<fail_count; i++) {
    fail_betas[i]   = temp_betas[i];
    fail_samples[i] = temp_samples[i];
  }

  // remove points too close together
  // start with most probable failure point (smallest beta)
  // remove all points within the cutoff distance of this point
  // pick most probable point from remaining failure points
  // remove all points within the cutoff distance of this point
  // repeat until all failure points exhausted
  Real cutoff_distance = 1.;
  IntArray min_indx(fail_count);
  size_t num_rep_pts = 0;
  bool exhausted = false;
  while (!exhausted) {
    Real min_beta = 100.;
    for (i=0; i<fail_count; i++) {
      Real beta = fail_betas[i];
      if (beta < min_beta) {
	min_beta = beta;
	min_indx[num_rep_pts] = i;
      }
    }
    if (min_beta == 100.)  
      exhausted = true;
    else {
      size_t idx = min_indx[num_rep_pts];
      for (i=0; i<fail_count; i++) {
	if (fail_betas[i] < 99.) {
	  Real distance = 0.;
	  for (j=0; j<numUncertainVars; j++)
	    distance += std::pow(fail_samples[i][j]-fail_samples[idx][j],2);
	  distance = std::sqrt(distance);
	  if (distance < cutoff_distance)
	    fail_betas[i] = 100.;
	}
      }
      num_rep_pts++;
    }
  }

  // set the number of representative points
  numRepPoints = num_rep_pts;

  // store samples in min_indx in repPoints
  repPoints.resize(numRepPoints);
  for (i=0; i<numRepPoints; i++)
    repPoints[i] = fail_samples[min_indx[i]];

  calculate_rep_weights();
}


void NonDAdaptImpSampling::
select_rep_points(const RealVectorArray& samples)
{
  // pick out failures from input samples
  // calculate beta for failures only
  size_t num_samples = samples.size();
  RealVector temp_betas(num_samples);
  RealVector whole_sample(numContinuousVars, false);
  RealVectorArray temp_samples(num_samples);
  size_t i, j, cntr, fail_count = 0;

  // store designPoint in whole_sample, append uncertain samples later
  for (i=0; i<numContDesVars; i++)
    whole_sample[i] = designPoint[i];

  for (i=0; i<num_samples; i++) {

    if (transPoints) { // u-space points -> x-space Model (NonDLocal)
      // append uncertain sample to designPoint before evaluation
      // this must be done before the transformation because trans_U_to_X
      //   expects numContinuousVars variables
      cntr = 0;
      for (j=numContDesVars; j<numContinuousVars; j++)
	whole_sample[j] = samples[i][cntr++];
      RealVector sample_x;
      natafTransform.trans_U_to_X(whole_sample, sample_x);
      iteratedModel.continuous_variables(sample_x);
    }
    else { // u-space points -> u-space Model (NonDGlobal)
      // append uncertain sample to designPoint before evaluation
      cntr = 0;
      for (j=numContDesVars; j<numContinuousVars; j++)
	whole_sample[j] = samples[i][cntr++];
      iteratedModel.continuous_variables(whole_sample);
    }

    iteratedModel.compute_response();
    const Response&   limit_state_resp = iteratedModel.current_response();
    const RealVector& limit_state_fns  = limit_state_resp.function_values();
    if ( ( cdfFlag && limit_state_fns[respFn] < failThresh) ||
	 (!cdfFlag && limit_state_fns[respFn] > failThresh) ) {
      temp_samples[fail_count] = samples[i];
      Real beta = 0.;
      for (j=0; j<numUncertainVars; j++) {
	beta += std::pow(samples[i][j],2);
      }
      beta = std::sqrt(beta);
      temp_betas[fail_count] = beta;
      fail_count++;
    }
  }

  // now the first 'fail_count' entries in temp_samples & temp_betas contain
  //   the failure samples & their beta values
  // copy these into a new data structure of this exact size
  RealVector fail_betas(fail_count);
  RealVectorArray fail_samples(fail_count);
  for (i=0; i<fail_count; i++) {
    fail_betas[i]   = temp_betas[i];
    fail_samples[i] = temp_samples[i];
  }

  // remove points too close together
  // start with most probable failure point (smallest beta)
  // remove all points within the cutoff distance of this point
  // pick most probable point from remaining failure points
  // remove all points within the cutoff distance of this point
  // repeat until all failure points exhausted
  Real cutoff_distance = 1.5;
  IntArray min_indx(fail_count);
  size_t num_rep_pts = 0;
  bool exhausted = false;
  while (!exhausted) {
    Real min_beta = 100.;
    for (i=0; i<fail_count; i++) {
      Real beta = fail_betas[i];
      if (beta < min_beta) {
	min_beta = beta;
	min_indx[num_rep_pts] = i;
      }
    }
    if (min_beta == 100.)  
      exhausted = true;
    else {
      size_t idx = min_indx[num_rep_pts];
      for (i=0; i<fail_count; i++) {
	if (fail_betas[i] < 99.) {
	  Real distance = 0.;
	  for (j=0; j<numUncertainVars; j++)
	    distance += std::pow(fail_samples[i][j]-fail_samples[idx][j],2);
	  distance = std::sqrt(distance);
	  if (distance < cutoff_distance)
	    fail_betas[i] = 100.;
	}
      }
      num_rep_pts++;
    }
  }

  // set the number of representative points
  numRepPoints = num_rep_pts;

  // store samples in min_indx in repPoints
  repPoints.resize(numRepPoints);
  for (i=0; i<numRepPoints; ++i)
    repPoints[i] = fail_samples[min_indx[i]];

  calculate_rep_weights();
}


void NonDAdaptImpSampling::calculate_rep_weights()
{
  // calculate repWeights
  RealVector rep_density(numRepPoints);
  Real sum_density = 0.;
  for (size_t i=0; i<numRepPoints; i++) {
    Real beta = 0.;
    for (size_t j=0; j<numUncertainVars; j++)
      beta += std::pow(repPoints[i][j],2);
    beta = std::sqrt(beta);
    rep_density[i] = Pecos::phi(beta);
    sum_density   += Pecos::phi(beta);
  }
  repWeights.sizeUninitialized(numRepPoints);
  for (size_t i=0; i<numRepPoints; i++)
    repWeights[i] = rep_density[i]/sum_density;
}


void NonDAdaptImpSampling::generate_samples(RealVectorArray& samples)
{
  size_t i, j, k, cntr = 0, sample_counter = 0;
  RealVector n_means(numUncertainVars),
    n_std_devs(numUncertainVars, false), n_l_bnds(numUncertainVars, false),
    n_u_bnds(numUncertainVars, false);
  n_std_devs = 1.;
  n_l_bnds = -DBL_MAX;
  n_u_bnds =  DBL_MAX;

  // Bound the sampler if necessary - this is needed for NonDGlobalReliability
  // because the Gaussian Process model is only accurate within these bounds
  if (useModelBounds) {
    const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
    for (i=numContDesVars; i<numContinuousVars; i++, cntr++) {
      n_l_bnds[cntr] = c_l_bnds[i];
      n_u_bnds[cntr] = c_u_bnds[i];
    }
  }

  varyPattern = true;
  initialize_lhs(false);
  RealMatrix lhs_samples_array;
#ifdef DAKOTA_PECOS
  lhsDriver.generate_normal_samples(n_means, n_std_devs, n_l_bnds, n_u_bnds,
				    numSamples, lhs_samples_array);
#endif // DAKOTA_PECOS

  for (i=0; i<numRepPoints; i++) {
    int num_samples = int(repWeights[i]*numSamples);
    if (sample_counter < numSamples && num_samples == 0)
      num_samples = 1;
    const RealVector& rep_pt_i = repPoints[i];
    for (j=0; j<num_samples; j++) {
      //const Real* c_vars = allSamples[sample_counter+j];
      // WJB -- ToDo: use NumericalType vector ops
      Real* lhs_samples_col_j = lhs_samples_array[sample_counter+j];
      samples[sample_counter+j].sizeUninitialized(numUncertainVars);
      for (k=0; k<numUncertainVars; k++)
	samples[sample_counter+j][k] = lhs_samples_col_j[k] + rep_pt_i[k];
	//samples[sample_counter+j][k] = c_vars[k] + rep_pt_i[k];
    }
    sample_counter += num_samples;
    if (sample_counter >= numSamples) break;
  }
}


void NonDAdaptImpSampling::
calculate_statistics(const RealVectorArray& samples, 
		     const size_t& total_samples, Real& sum_prob, Real& prob,
		     bool compute_cov, Real& sum_var, Real& cov)
{
  // Note: The current beta calculation assumes samples input in u-space
  size_t i, j, k, cntr, num_failures;
  RealVector failure_mmpdf, failure_pdf;
  RealVector whole_sample(numContinuousVars, false);

  // store designPoint in whole_sample, append uncertain samples later
  for (i=0; i<numContDesVars; i++)
    whole_sample[i] = designPoint[i];

  if (compute_cov) {
    num_failures = 0;
    failure_mmpdf.resize(numSamples);
    failure_pdf.resize(numSamples);
  }
  // calculate the probability of failure
  for (i=0; i<numSamples; i++) {

    if (transPoints) { // u-space points -> x-space Model (NonDLocal)
      // append uncertain sample to designPoint before evaluation
      // this must be done before the transformation because trans_U_to_X
      //   expectes numContinuousVars variables
      cntr = 0;
      for (j=numContDesVars; j<numContinuousVars; j++)
	whole_sample[j] = samples[i][cntr++];
      RealVector sample_x;
      natafTransform.trans_U_to_X(whole_sample, sample_x);
      iteratedModel.continuous_variables(sample_x);
    }
    else { // u-space points -> u-space Model (NonDGlobal)
      // append uncertain sample to designPoint before evaluation
      cntr = 0;
      for (j=numContDesVars; j<numContinuousVars; j++)
	whole_sample[j] = samples[i][cntr++];
      iteratedModel.continuous_variables(whole_sample);
    }

    // get response value at the sample point
    iteratedModel.compute_response();
    const Response&   limit_state_resp = iteratedModel.current_response();
    const RealVector& limit_state_fns  = limit_state_resp.function_values();

    // if point is a failure, calculate mmpdf, pdf, and add to sum
    if (( cdfFlag && (limit_state_fns[respFn] < failThresh)) ||
	(!cdfFlag && (limit_state_fns[respFn] > failThresh)) ) {

      // calculate mmpdf
      Real mmpdf = 0.;
      for (j=0; j<numRepPoints; j++) {
	Real beta  = 0.;
	for (k=0; k<numUncertainVars; k++) 
	  beta += std::pow((repPoints[j][k]-samples[i][k]),2);
	beta = std::sqrt(beta);
	mmpdf += repWeights[j]*Pecos::phi(beta);
      }

      // calculate pdf
      Real beta = 0.;
      for (j=0; j<numUncertainVars; j++)
	beta += std::pow(samples[i][j],2);
      Real pdf = Pecos::phi(std::sqrt(beta));

      // add sample's contribution to sum_prob
      sum_prob += pdf/mmpdf;

      // if cov requested, store failure sample data in local structures 
      //   to avoid recalculating
      if (compute_cov) {
	failure_mmpdf[num_failures] = mmpdf;
	failure_pdf[num_failures]   = pdf;
	num_failures++;
      }
    }
  }
  prob = sum_prob/double(total_samples);

  // compute the coeff of variation if requested
  if (compute_cov) {
    for (i=0; i<num_failures; i++)
      sum_var += std::pow((failure_pdf[i]/failure_mmpdf[i]) - prob, 2);
    Real var = sum_var/double(total_samples)/double(total_samples - 1);
    cov = std::sqrt(var)/prob;
  }
}

} // namespace Dakota
