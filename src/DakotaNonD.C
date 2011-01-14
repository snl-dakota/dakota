/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonD
//- Description: Base class for NonDeterministic methods
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "NestedModel.H"
#include "DakotaResponse.H"
#include "DakotaNonD.H"
#include "ProblemDescDB.H"
#include "pecos_stat_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaNonD.C 7024 2010-10-16 01:24:42Z mseldre $";

namespace Dakota {

// initialization of statics
NonD* NonD::nondInstance(NULL);


NonD::NonD(Model& model): Analyzer(model), numContDesVars(0),
  numDiscIntDesVars(0), numDiscRealDesVars(0), numDesignVars(0),
  numContStateVars(0), numDiscIntStateVars(0), numDiscRealStateVars(0),
  numStateVars(0), numNormalVars(0), numLognormalVars(0), numUniformVars(0),
  numLoguniformVars(0), numTriangularVars(0), numExponentialVars(0),
  numBetaVars(0), numGammaVars(0), numGumbelVars(0), numFrechetVars(0),
  numWeibullVars(0), numHistogramBinVars(0), numPoissonVars(0),
  numBinomialVars(0), numNegBinomialVars(0), numGeometricVars(0),
  numHyperGeomVars(0), numHistogramPtVars(0), numIntervalVars(0),
  numContAleatUncVars(0), numDiscIntAleatUncVars(0), numDiscRealAleatUncVars(0),
  numAleatoryUncVars(0), numContEpistUncVars(0), numDiscIntEpistUncVars(0),
  numDiscRealEpistUncVars(0), numEpistemicUncVars(0),
  numResponseFunctions(
    probDescDB.get_sizet("responses.num_response_functions")),
  requestedRespLevels(probDescDB.get_rdva("method.nond.response_levels")),
  requestedProbLevels(probDescDB.get_rdva("method.nond.probability_levels")),
  requestedRelLevels(probDescDB.get_rdva("method.nond.reliability_levels")),
  requestedGenRelLevels(
    probDescDB.get_rdva("method.nond.gen_reliability_levels")),
  totalLevelRequests(0), distParamDerivs(false)
{
  bool err_flag = false;
  short active_view = iteratedModel.current_variables().view().first;

  // initialize aleatory uncertain variables
  if (active_view == MERGED_ALL || active_view == MERGED_DISTINCT_UNCERTAIN ||
      active_view == MERGED_DISTINCT_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_DISTINCT_UNCERTAIN ||
      active_view == MIXED_DISTINCT_ALEATORY_UNCERTAIN) {
    numNormalVars = probDescDB.get_sizet("variables.normal_uncertain");
    numLognormalVars = probDescDB.get_sizet("variables.lognormal_uncertain");
    numUniformVars = probDescDB.get_sizet("variables.uniform_uncertain");
    numLoguniformVars = probDescDB.get_sizet("variables.loguniform_uncertain");
    numTriangularVars = probDescDB.get_sizet("variables.triangular_uncertain");
    numExponentialVars
      = probDescDB.get_sizet("variables.exponential_uncertain");
    numBetaVars = probDescDB.get_sizet("variables.beta_uncertain");
    numGammaVars = probDescDB.get_sizet("variables.gamma_uncertain");
    numGumbelVars = probDescDB.get_sizet("variables.gumbel_uncertain");
    numFrechetVars = probDescDB.get_sizet("variables.frechet_uncertain");
    numWeibullVars = probDescDB.get_sizet("variables.weibull_uncertain");
    numHistogramBinVars
      = probDescDB.get_sizet("variables.histogram_uncertain.bin");
    numContAleatUncVars = numNormalVars + numLognormalVars + numUniformVars +
      numLoguniformVars + numTriangularVars + numExponentialVars + numBetaVars +
      numGammaVars + numGumbelVars + numFrechetVars + numWeibullVars +
      numHistogramBinVars;

    numPoissonVars = probDescDB.get_sizet("variables.poisson_uncertain");
    numBinomialVars = probDescDB.get_sizet("variables.binomial_uncertain");
    numNegBinomialVars
      = probDescDB.get_sizet("variables.negative_binomial_uncertain");
    numGeometricVars = probDescDB.get_sizet("variables.geometric_uncertain");
    numHyperGeomVars
      = probDescDB.get_sizet("variables.hypergeometric_uncertain");
    if (active_view == MIXED_ALL || active_view >= MIXED_DISTINCT_DESIGN)
      numDiscIntAleatUncVars = numPoissonVars + numBinomialVars +
	numNegBinomialVars + numGeometricVars + numHyperGeomVars;
    else
      numContAleatUncVars += numPoissonVars + numBinomialVars +
	numNegBinomialVars + numGeometricVars + numHyperGeomVars;

    numHistogramPtVars
      = probDescDB.get_sizet("variables.histogram_uncertain.point");
    if (active_view == MIXED_ALL || active_view >= MIXED_DISTINCT_DESIGN)
      numDiscRealAleatUncVars = numHistogramPtVars;
    else
      numContAleatUncVars += numHistogramPtVars;

    numAleatoryUncVars = numContAleatUncVars + numDiscIntAleatUncVars +
      numDiscRealAleatUncVars;
  }
  // initialize epistemic uncertain variables
  if (active_view == MERGED_ALL || active_view == MERGED_DISTINCT_UNCERTAIN ||
      active_view == MERGED_DISTINCT_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_DISTINCT_UNCERTAIN ||
      active_view == MIXED_DISTINCT_EPISTEMIC_UNCERTAIN) // epistemic or both
    numEpistemicUncVars = numContEpistUncVars = numIntervalVars
      = probDescDB.get_sizet("variables.interval_uncertain");

  // initialize total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // initialize design/state variables for all_variables mode
  if (active_view == MERGED_ALL || active_view == MIXED_ALL) {
    numContDesVars   = probDescDB.get_sizet("variables.continuous_design");
    numContStateVars = probDescDB.get_sizet("variables.continuous_state");
    if (active_view == MERGED_ALL) {
      numContDesVars += probDescDB.get_sizet("variables.discrete_design_range")
	+ probDescDB.get_sizet("variables.discrete_design_set_int")
	+ probDescDB.get_sizet("variables.discrete_design_set_real");
      numContStateVars += probDescDB.get_sizet("variables.discrete_state_range")
	+ probDescDB.get_sizet("variables.discrete_state_set_int")
	+ probDescDB.get_sizet("variables.discrete_state_set_real");
    }
    else {
      numDiscIntDesVars
	= probDescDB.get_sizet("variables.discrete_design_range")
	+ probDescDB.get_sizet("variables.discrete_design_set_int");
      numDiscRealDesVars
	= probDescDB.get_sizet("variables.discrete_design_set_real");
      numDiscIntStateVars
	= probDescDB.get_sizet("variables.discrete_state_range")
	+ probDescDB.get_sizet("variables.discrete_state_set_int");
      numDiscRealStateVars
	= probDescDB.get_sizet("variables.discrete_state_set_real");
    }
    numDesignVars = numContDesVars + numDiscIntDesVars + numDiscRealDesVars;
    numStateVars
      = numContStateVars + numDiscIntStateVars + numDiscRealStateVars;
  }
  else if (!numUncertainVars || !numResponseFunctions) {
    // if !all_variables, then response function type should be generic
    // (numResponseFunctions implies UQ usage; numFunctions has a broader
    // interpretation, e.g., general DACE usage of NonDLHSSampling iterator).
    Cerr << "\nError: number of uncertain variables and number of response "
	 << "functions must be nonzero in Dakota::NonD." << std::endl;
    err_flag = true;
  }

  if (numContinuousVars + numDiscreteIntVars + numDiscreteRealVars !=
      numDesignVars + numUncertainVars + numStateVars) {
    Cerr << "\nError: bad number of active variables in Dakota::NonD."
	 << std::endl;
    err_flag = true;
  }

  // initialize convenience flags & enums
  cdfFlag = (probDescDB.get_string("method.nond.distribution") ==
             "complementary") ? false : true;
  const String& resp_mapping
    = probDescDB.get_string("method.nond.response_level_mapping_type");
  if (resp_mapping == "probabilities")
    respLevelTarget = PROBABILITIES;
  else if (resp_mapping == "reliabilities")
    respLevelTarget = RELIABILITIES;
  else if (resp_mapping == "gen_reliabilities")
    respLevelTarget = GEN_RELIABILITIES;
  else {
    Cerr << "Error: bad response level mapping type in DakotaNonD constructor."
	 << std::endl;
    err_flag = true;
  }

  // When specifying z/p/beta/beta* levels, a spec with an index key (number of
  // levels = list of ints) will result in multiple vectors of levels, one for
  // each response fn.  A spec without an index key will result in a single
  // vector of levels.  This is as much logic as the parser can support since it
  // cannot access the number of response fns.  Here, support the shorthand spec
  // where there are multiple response fns but only one vector of levels (no
  // index key provided), which are to be evenly distributed among the response
  // fns.  This provides some measure of backwards compatibility.
  distribute_levels(requestedRespLevels);
  distribute_levels(requestedProbLevels);
  distribute_levels(requestedRelLevels);
  distribute_levels(requestedGenRelLevels);

  for (size_t i=0; i<numFunctions; i++)
    totalLevelRequests += requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();

  if (err_flag)
    abort_handler(-1);
}


NonD::NonD(NoDBBaseConstructor, Model& model):
  Analyzer(NoDBBaseConstructor(), model), numContDesVars(0),
  numDiscIntDesVars(0), numDiscRealDesVars(0), numDesignVars(0),
  numContStateVars(0), numDiscIntStateVars(0), numDiscRealStateVars(0),
  numStateVars(0), numNormalVars(0), numLognormalVars(0), numUniformVars(0),
  numLoguniformVars(0), numTriangularVars(0), numExponentialVars(0),
  numBetaVars(0), numGammaVars(0), numGumbelVars(0), numFrechetVars(0),
  numWeibullVars(0), numHistogramBinVars(0), numPoissonVars(0),
  numBinomialVars(0), numNegBinomialVars(0), numGeometricVars(0),
  numHyperGeomVars(0), numHistogramPtVars(0), numIntervalVars(0),
  numContAleatUncVars(0), numDiscIntAleatUncVars(0), numDiscRealAleatUncVars(0),
  numAleatoryUncVars(0), numContEpistUncVars(0), numDiscIntEpistUncVars(0),
  numDiscRealEpistUncVars(0), numEpistemicUncVars(0),
  numResponseFunctions(numFunctions), totalLevelRequests(0), cdfFlag(true),
  distParamDerivs(false)
{
  // NonDEvidence and NonDAdaptImpSampling use this ctor

  bool err_flag = false;
  short active_view = model.current_variables().view().first;

  // initialize aleatory uncertain variables
  if (active_view == MERGED_ALL || active_view == MERGED_DISTINCT_UNCERTAIN ||
      active_view == MERGED_DISTINCT_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_DISTINCT_UNCERTAIN ||
      active_view == MIXED_DISTINCT_ALEATORY_UNCERTAIN) { // aleatory or both
    Pecos::DistributionParams& dp = model.distribution_parameters();
    numNormalVars = dp.normal_means().length();
    numLognormalVars = dp.lognormal_means().length();
    numUniformVars = dp.uniform_lower_bounds().length();
    numLoguniformVars = dp.loguniform_lower_bounds().length();
    numTriangularVars = dp.triangular_modes().length();
    numExponentialVars = dp.exponential_betas().length();
    numBetaVars = dp.beta_alphas().length();
    numGammaVars = dp.gamma_alphas().length();
    numGumbelVars = dp.gumbel_alphas().length();
    numFrechetVars = dp.frechet_alphas().length();
    numWeibullVars = dp.weibull_alphas().length();
    numHistogramBinVars = dp.histogram_bin_pairs().size();
    numContAleatUncVars = numNormalVars + numLognormalVars + numUniformVars +
      numLoguniformVars + numTriangularVars + numExponentialVars + numBetaVars +
      numGammaVars + numGumbelVars + numFrechetVars + numWeibullVars +
      numHistogramBinVars;

    numPoissonVars = dp.poisson_lambdas().length();
    numBinomialVars = dp.binomial_probabilities_per_trial().length();
    numNegBinomialVars
      = dp.negative_binomial_probabilities_per_trial().length();
    numGeometricVars = dp.geometric_probabilities_per_trial().length();
    numHyperGeomVars = dp.hypergeometric_num_drawn().length();
    if (active_view == MIXED_ALL || active_view >= MIXED_DISTINCT_DESIGN)
      numDiscIntAleatUncVars = numPoissonVars + numBinomialVars +
	numNegBinomialVars + numGeometricVars + numHyperGeomVars;
    else
      numContAleatUncVars += numPoissonVars + numBinomialVars +
	numNegBinomialVars + numGeometricVars + numHyperGeomVars;

    numHistogramPtVars = dp.histogram_point_pairs().size();
    if (active_view == MIXED_ALL || active_view >= MIXED_DISTINCT_DESIGN)
      numDiscRealAleatUncVars = numHistogramPtVars;
    else
      numContAleatUncVars += numHistogramPtVars;

    numAleatoryUncVars = numContAleatUncVars + numDiscIntAleatUncVars +
      numDiscRealAleatUncVars;
  }

  // initialize epistemic uncertain variables
  if (active_view == MERGED_ALL || active_view == MERGED_DISTINCT_UNCERTAIN ||
      active_view == MERGED_DISTINCT_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_DISTINCT_UNCERTAIN ||
      active_view == MIXED_DISTINCT_EPISTEMIC_UNCERTAIN) // epistemic or both
    numEpistemicUncVars = numContEpistUncVars = numIntervalVars
      = model.distribution_parameters().interval_probabilities().size();

  // initialize total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // initialize design/state variables for all_variables mode
  if (active_view == MERGED_ALL || active_view == MIXED_ALL) {
    UShortMultiArrayConstView acv_types = model.all_continuous_variable_types();
    numContDesVars   = std::count(acv_types.begin(), acv_types.end(),
				  (unsigned short)CONTINUOUS_DESIGN);
    numContStateVars = std::count(acv_types.begin(), acv_types.end(),
				  (unsigned short)CONTINUOUS_STATE);
    if (active_view == MERGED_ALL) {
      numContDesVars   += std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_DESIGN_RANGE) +
	                  std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_DESIGN_SET_INT) +
	                  std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_DESIGN_SET_REAL);
      numContStateVars += std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_STATE_RANGE) +
	                  std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_STATE_SET_INT) +
	                  std::count(acv_types.begin(), acv_types.end(),
				     (unsigned short)DISCRETE_STATE_SET_REAL);
    }
    else {
      UShortMultiArrayConstView adiv_types
	= model.all_discrete_int_variable_types();
      UShortMultiArrayConstView adrv_types
	= model.all_discrete_real_variable_types();
      numDiscIntDesVars = std::count(adiv_types.begin(), adiv_types.end(),
				     (unsigned short)DISCRETE_DESIGN_RANGE) +
	                  std::count(adiv_types.begin(), adiv_types.end(),
				     (unsigned short)DISCRETE_DESIGN_SET_INT);
      numDiscRealDesVars = std::count(adrv_types.begin(), adrv_types.end(),
				      (unsigned short)DISCRETE_DESIGN_SET_REAL);
      numDiscIntStateVars = std::count(adiv_types.begin(), adiv_types.end(),
				       (unsigned short)DISCRETE_STATE_RANGE) +
	                    std::count(adiv_types.begin(), adiv_types.end(),
				       (unsigned short)DISCRETE_STATE_SET_INT);
      numDiscRealStateVars = std::count(adrv_types.begin(), adrv_types.end(),
				       (unsigned short)DISCRETE_STATE_SET_REAL);
    }
    numDesignVars = numContDesVars + numDiscIntDesVars + numDiscRealDesVars;
    numStateVars
      = numContStateVars + numDiscIntStateVars + numDiscRealStateVars;
  }
  else if (active_view == MERGED_DISTINCT_DESIGN ||
	   active_view == MIXED_DISTINCT_DESIGN) {
    // can happen with EGO usage of on-the-fly NonDLHSSampling construction.
    // TO DO: would selecting samplingVarsMode=ALL_UNIFORM from EGO be OK?
    numContDesVars     = model.cv();
    numDiscIntDesVars  = model.div();
    numDiscRealDesVars = model.drv();
    numDesignVars = numContDesVars + numDiscIntDesVars + numDiscRealDesVars;
  }
  else if (!numUncertainVars || !numResponseFunctions) {
    // if !all_variables, then response function type should be generic
    // (numResponseFunctions implies UQ usage; numFunctions has a broader
    // interpretation, e.g., general DACE usage of NonDLHSSampling iterator).
    Cerr << "\nError: number of uncertain variables and number of response "
	 << "functions must be nonzero in Dakota::NonD." << std::endl;
    err_flag = true;
  }

  if (numContinuousVars + numDiscreteIntVars + numDiscreteRealVars !=
      numDesignVars + numUncertainVars + numStateVars) {
    Cerr << "\nError: bad number of active variables in Dakota::NonD."
	 << std::endl;
    err_flag = true;
  }

  // current set of statistics is mean, standard deviation, and
  // probability of failure for each response function
  //ShortArray asv(3*numFunctions, 1);
  //finalStatistics = Response(numUncertainVars, asv);

  if (err_flag)
    abort_handler(-1);
}


NonD::NonD(NoDBBaseConstructor, const RealVector& lower_bnds,
	   const RealVector& upper_bnds):
  Analyzer(NoDBBaseConstructor()), numContDesVars(0), numDiscIntDesVars(0),
  numDiscRealDesVars(0), numDesignVars(0), numContStateVars(0),
  numDiscIntStateVars(0), numDiscRealStateVars(0), numStateVars(0),
  numNormalVars(0), numLognormalVars(0), numUniformVars(lower_bnds.length()),
  numLoguniformVars(0), numTriangularVars(0), numExponentialVars(0),
  numBetaVars(0), numGammaVars(0), numGumbelVars(0), numFrechetVars(0),
  numWeibullVars(0), numHistogramBinVars(0), numPoissonVars(0),
  numBinomialVars(0), numNegBinomialVars(0), numGeometricVars(0),
  numHyperGeomVars(0), numHistogramPtVars(0), numIntervalVars(0),
  numContAleatUncVars(numUniformVars), numDiscIntAleatUncVars(0),
  numDiscRealAleatUncVars(0), numAleatoryUncVars(numUniformVars),
  numContEpistUncVars(0), numDiscIntEpistUncVars(0), numDiscRealEpistUncVars(0),
  numEpistemicUncVars(0), numUncertainVars(numUniformVars),
  numResponseFunctions(0), totalLevelRequests(0), cdfFlag(true),
  distParamDerivs(false)
{
  // ConcurrentStrategy uses this ctor for design opt, either for multi-start
  // initial points or multibjective weight sets.

  numContinuousVars  = numUniformVars;
  numDiscreteIntVars = numDiscreteRealVars = 0;
}


void NonD::
requested_levels(const RealVectorArray& req_resp_levels,
		 const RealVectorArray& req_prob_levels,
		 const RealVectorArray& req_rel_levels,
		 const RealVectorArray& req_gen_rel_levels,
		 short resp_lev_target, bool cdf_flag)
{
  requestedRespLevels   = req_resp_levels;
  requestedProbLevels   = req_prob_levels;
  requestedRelLevels    = req_rel_levels;
  requestedGenRelLevels = req_gen_rel_levels;
  respLevelTarget       = resp_lev_target;
  cdfFlag               = cdf_flag;

  // In current usage, incoming levels are already distributed.
  // But if they're not, distribute them now.
  distribute_levels(requestedRespLevels);
  distribute_levels(requestedProbLevels);
  distribute_levels(requestedRelLevels);
  distribute_levels(requestedGenRelLevels);

  for (size_t i=0; i<numFunctions; i++)
    totalLevelRequests += requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();

  initialize_final_statistics();
}


void NonD::distribute_levels(RealVectorArray& levels)
{
  size_t i, j, num_level_arrays = levels.size();
  if (num_level_arrays != numFunctions) {
    if (num_level_arrays == 0) // create array of empty vectors
      levels.resize(numFunctions);
      // NOTE: old response_thresholds default was a single 0 per resp fn.  New
      // default is no output level calculations if no requested input levels.
    else if (num_level_arrays == 1) { // evenly distribute among all resp fns
      RealVector level_array0 = levels[0];
      size_t total_len = level_array0.length();
      // check for divisibility
      if (total_len%numFunctions) {
        Cerr << "\nError: number of levels not evenly divisible by the number "
             << "of response functions." << std::endl;
        abort_handler(-1);
      }
      levels.resize(numFunctions);
      size_t new_len = total_len/numFunctions;
      RealVector new_vec(new_len);
      for (i=0; i<numFunctions; i++) {
        for (j=0; j<new_len; j++)
          new_vec[j] = level_array0[i*new_len+j];
        levels[i] = new_vec;
      }
    }
    else {
      Cerr << "\nError: num_levels specification differs from the number of "
           << "response functions." << std::endl;
      abort_handler(-1);
    }
  }
}


/** Default definition of virtual function (used by sampling,
    reliability, and polynomial chaos) defines the set of statistical
    results to include means, standard deviations, and level mappings. */
void NonD::initialize_final_statistics()
{
  size_t num_final_stats = 2*numFunctions, num_active_vars = iteratedModel.cv();
  if (!numEpistemicUncVars) // aleatory UQ
    num_final_stats += totalLevelRequests;
  ActiveSet stats_set(num_final_stats, num_active_vars);
  finalStatistics = Response(stats_set);

  // Assign meaningful labels to finalStatistics (appear in NestedModel output)
  size_t i, j, num_levels, cntr = 0;
  char tag_string[10], lev_string[10];
  StringArray stats_labels(num_final_stats);
  for (i=0; i<numFunctions; i++) {
    std::sprintf(tag_string, "_r%i", i+1);
    if (numEpistemicUncVars) { // epistemic & mixed aleatory/epistemic
      stats_labels[cntr++] = String("z_lo") + String(tag_string);
      stats_labels[cntr++] = String("z_up") + String(tag_string);
    }
    else {                  // aleatory
      stats_labels[cntr++] = String("mean")    + String(tag_string);
      stats_labels[cntr++] = String("std_dev") + String(tag_string);
      num_levels = requestedRespLevels[i].length();
      for (j=0; j<num_levels; j++, cntr++) {
	stats_labels[cntr] = (cdfFlag) ? String("cdf") : String("ccdf");
	switch (respLevelTarget) {
	case PROBABILITIES:
	  std::sprintf(lev_string, "_plev%i",  j+1); break;
	case RELIABILITIES:
	  std::sprintf(lev_string, "_blev%i",  j+1); break;
	case GEN_RELIABILITIES:
	  std::sprintf(lev_string, "_b*lev%i", j+1); break;
	}
	stats_labels[cntr] += String(lev_string) + String(tag_string);
      }
      num_levels = requestedProbLevels[i].length() +
	requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
      for (j=0; j<num_levels; j++, cntr++) {
	stats_labels[cntr] = (cdfFlag) ? String("cdf") : String("ccdf");
	std::sprintf(lev_string, "_zlev%i", j+1);
	stats_labels[cntr] += String(lev_string) + String(tag_string);
      }
    }
  }
  finalStatistics.function_labels(stats_labels);
}


void NonD::initialize_final_statistics_gradients()
{
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, num_final_stats     = final_asv.size(),
            num_final_grad_vars = final_dvv.size();
  bool final_grad_flag = false;
  for (i=0; i<num_final_stats; i++)
    if (final_asv[i] & 2)
      { final_grad_flag = true; break; }
  finalStatistics.reshape(num_final_stats, num_final_grad_vars,
			  final_grad_flag, false);
}


/** Map the variables from iterator space (u) to simulation space (x). */
void NonD::vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars)
{
  RealVector x;
  nondInstance->natafTransform.trans_U_to_X(u_vars.continuous_variables(), x);
  x_vars.continuous_variables(x);
}


/** Define the DVV for x-space derivative evaluations by augmenting
    the iterator requests to account for correlations. */
void NonD::set_u_to_x_mapping(const ActiveSet& u_set, ActiveSet& x_set)
{
  //if (nondInstance->distParamDerivs) {
  //}
  //else
  if (nondInstance->natafTransform.x_correlation()) {
    const SizetArray& u_dvv = u_set.derivative_vector();
    SizetMultiArrayConstView cv_ids
      = nondInstance->iteratedModel.continuous_variable_ids();
    SizetMultiArrayConstView icv_ids
      = nondInstance->iteratedModel.inactive_continuous_variable_ids();
    bool std_dvv = (u_dvv == cv_ids || u_dvv == icv_ids);
    if (!std_dvv) { // partial random variable derivatives: check correlations
      SizetMultiArrayConstView acv_ids
	= nondInstance->iteratedModel.all_continuous_variable_ids();
      size_t i, j, num_cv = cv_ids.size(), num_acv = acv_ids.size();
      SizetArray x_dvv;
      const RealSymMatrix& corr_x
	= nondInstance->natafTransform.x_correlation_matrix();
      for (i=0; i<num_acv; ++i) { // insert in ascending order
	size_t acv_id = acv_ids[i];
	if (contains(u_dvv, acv_id))
	  x_dvv.push_back(acv_id);
	else {
	  size_t cv_index = find_index(cv_ids, acv_id);
	  if (cv_index != _NPOS) { // random var: check correlation
	    for (j=0; j<num_cv; ++j) {
	      if (cv_index != j && std::fabs(corr_x(cv_index, j)) > 1.e-25 &&
		  contains(u_dvv, cv_ids[j])) {
		x_dvv.push_back(acv_id);
		break;
	      }
	    }
	  }
	}
      }
      x_set.derivative_vector(x_dvv);
    }
  }
}


void NonD::
resp_x_to_u_mapping(const Variables& x_vars,     const Variables& u_vars,
		    const Response&  x_response, Response&        u_response)
{
  const RealVector&        x_rdv   = x_vars.continuous_variables();
  SizetMultiArrayConstView cv_ids  = x_vars.continuous_variable_ids();
  SizetMultiArrayConstView acv_ids = x_vars.all_continuous_variable_ids();
  const RealVector&        x_fns   = x_response.function_values();
  //const RealMatrix&  x_grads = x_response.function_gradients();
  const RealSymMatrixArray& x_hessians = x_response.function_hessians();

  // In this recasting, the inputs and outputs are mapped one-to-one, with no
  // reordering.  However, the x-space ASV may be augmented from the original
  // u-space ASV due to nonlinear mapping logic in RecastModel::asv_mapping().
  const ShortArray& u_asv = u_response.active_set_request_vector();
  const SizetArray& u_dvv = u_response.active_set_derivative_vector();
  const ShortArray& x_asv = x_response.active_set_request_vector();
  const SizetArray& x_dvv = x_response.active_set_derivative_vector();
  size_t i, j, num_fns = x_asv.size(), num_deriv_vars = x_dvv.size();
  if (u_asv.size() != num_fns) {
    Cerr << "Error: inconsistent response function definition in NonD::"
	 << "resp_x_to_u_mapping()" << std::endl;
    abort_handler(-1);
  }
  if (!nondInstance->natafTransform.x_correlation() && u_dvv != x_dvv) {
    Cerr << "Error: inconsistent derivative component definition in NonD::"
	 << "resp_x_to_u_mapping()" << std::endl;
    abort_handler(-1);
  }
  bool u_grad_flag = false, u_hess_flag = false;
  for (i=0; i<num_fns; ++i) {
    if (u_asv[i] & 2)
      u_grad_flag = true;
    if (u_asv[i] & 4)
      u_hess_flag = true;
  }

  bool map_derivs = ( (u_grad_flag || u_hess_flag) &&
		      u_dvv != u_vars.inactive_continuous_variable_ids() );
  const Pecos::ShortArray& x_types = nondInstance->natafTransform.x_types();
  const Pecos::ShortArray& u_types = nondInstance->natafTransform.u_types();
  size_t num_types = x_types.size();
  bool nonlinear_vars_map = false;
  for (i=0; i<num_types; ++i)
    if ( x_types[i] != u_types[i] &&
	 !( x_types[i] == Pecos::NORMAL && u_types[i] == Pecos::STD_NORMAL ) &&
	 !( ( x_types[i] == Pecos::DESIGN || x_types[i] == Pecos::UNIFORM ||
	      x_types[i] == Pecos::HISTOGRAM_BIN ||
	      x_types[i] == Pecos::INTERVAL || x_types[i] == Pecos::STATE ) &&
	    u_types[i] == Pecos::STD_UNIFORM ) &&
	 !( x_types[i] == Pecos::EXPONENTIAL &&
	    u_types[i] == Pecos::STD_EXPONENTIAL ) &&
	 !( x_types[i] == Pecos::BETA   && u_types[i] == Pecos::STD_BETA ) &&
	 !( x_types[i] == Pecos::GAMMA  && u_types[i] == Pecos::STD_GAMMA ) )
      { nonlinear_vars_map = true; break; }

  RealVector         fn_grad_x_rv, fn_grad_us_rv;
  RealSymMatrix      fn_hess_u_rsdm, fn_hess_u_rm;
  RealMatrix         jacobian_xu, jacobian_xs;
  RealSymMatrixArray hessian_xu;

  if (map_derivs) {
    // The following transformation data is invariant w.r.t. the response fns
    // and is computed outside of the num_fns loop
    if (nondInstance->distParamDerivs)
      nondInstance->natafTransform.jacobian_dX_dS(x_rdv, jacobian_xs,
	cv_ids, acv_ids, nondInstance->primaryACVarMapIndices,
	nondInstance->secondaryACVarMapTargets);
    else {
      if (u_grad_flag || u_hess_flag)
	nondInstance->natafTransform.jacobian_dX_dU(x_rdv, jacobian_xu);
      if (u_hess_flag && nonlinear_vars_map)
	nondInstance->natafTransform.hessian_d2X_dU2(x_rdv, hessian_xu);
    }
  }

  for (i=0; i<num_fns; ++i) {
    short u_asv_val = u_asv[i]; // original request from iterator
    short x_asv_val = x_asv[i]; // mapped request for sub-model

    // map value g(x) to G(u)
    if (u_asv_val & 1) {
      if ( !(x_asv_val & 1) ) {
	Cerr << "Error: missing required sub-model data in NonD::"
	     << "resp_x_to_u_mapping()" << std::endl;
	abort_handler(-1);
      }
      // no transformation: g(x) = G(u) by definition
      u_response.function_value(x_fns[i], i);
    }

    // manage data requirements for derivative transformations: if fn_grad_x_rv
    // is needed for Hessian x-form (nonlinear I/O mapping), then x_asv has been
    // augmented to include the gradient in RecastModel::asv_mapping().
    if (map_derivs && (x_asv_val & 2)) {
      //copy_data(x_response.function_gradient(i), fn_grad_x_rv);
      // WJB: looks easy (except VIEW vs COPY choice) copy_data(x_grads[i], x_grads.numRows(), fn_grad_x_rdv);
      fn_grad_x_rv = x_response.function_gradient_copy(i);
    }

    // map gradient dg/dx to dG/du
    if (u_asv_val & 2) {
      if ( !(x_asv_val & 2) ) {
	Cerr << "Error: missing required sub-model data in NonD::"
	     << "resp_x_to_u_mapping()" << std::endl;
	abort_handler(-1);
      }
      if (map_derivs) { // perform transformation

	if (nondInstance->distParamDerivs) // transform subset of components
	  nondInstance->natafTransform.trans_grad_X_to_S(fn_grad_x_rv,
	    fn_grad_us_rv, jacobian_xs, x_dvv, cv_ids, acv_ids,
	    nondInstance->primaryACVarMapIndices,
	    nondInstance->secondaryACVarMapTargets);
	else   // transform subset of components
	  nondInstance->natafTransform.trans_grad_X_to_U(fn_grad_x_rv,
	    fn_grad_us_rv, jacobian_xu, x_dvv, cv_ids);
	u_response.function_gradient(fn_grad_us_rv, i);
      }
      else { // no transformation: dg/dx = dG/du
	u_response.function_gradient(x_response.function_gradient(i), i);
        /* WJB: above line work?? RealVector x_grad_i(Teuchos::View, (Real*)x_grads[i],
			    x_grads.numRows());
	u_response.function_gradient(x_grad_i, i); */
      }
    }

    // map Hessian d^2g/dx^2 to d^2G/du^2
    if (u_asv_val & 4) {
      if ( !(x_asv_val & 4) ||
	   ( map_derivs && nonlinear_vars_map && !(x_asv_val & 2) ) ) {
	Cerr << "Error: missing required sub-model data in NonD::"
	     << "resp_x_to_u_mapping()" << std::endl;
	abort_handler(-1);
      }
      if (map_derivs) { // perform transformation
        const RealSymMatrix& fn_hess_x_rsdm = x_hessians[i];
	if (nondInstance->distParamDerivs) { // transform subset of components
	  Cerr << "Error: Hessians with respect to inserted variables not yet "
	       << "supported." << std::endl;
	  abort_handler(-1);;
	  //nondInstance->natafTransform.trans_hess_X_to_S(fn_hess_x_rsdm,
	  //  fn_hess_s_rsdm, jacobian_xs, hessian_xs, fn_grad_s_rdv, x_dvv,
	  //  cv_ids, x_vars.all_continuous_variable_ids(),
	  //  nondInstance->primaryACVarMapIndices,
	  //  nondInstance->secondaryACVarMapTargets);
	}
	else // transform subset of components
	  nondInstance->natafTransform.trans_hess_X_to_U(fn_hess_x_rsdm,
            fn_hess_u_rsdm, jacobian_xu, hessian_xu, fn_grad_x_rv, x_dvv,
	    cv_ids);
	fn_hess_u_rm = fn_hess_u_rsdm;
	u_response.function_hessian(fn_hess_u_rm, i);
      }
      else // no transformation: d^2g/dx^2 = d^2G/du^2
	u_response.function_hessian(x_hessians[i], i);
    }
  }

#ifdef DEBUG
  Cout << "\nx_response:\n" << x_response
       << "\nu_response:\n" << u_response << std::endl;
#endif
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void NonD::initialize_random_variables(short u_space_type)
{
  if (natafTransform.is_null())
    natafTransform = Pecos::ProbabilityTransformation("nataf"); // for now

  initialize_random_variable_types(u_space_type);
  initialize_random_variable_parameters();
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void NonD::initialize_random_variable_types(short u_space_type)
{
  // u_space_type is an enumeration for type of u-space transformation:
  // > if STD_NORMAL_U (reliability, AIS), then u-space is defined exclusively
  //   with std normals;
  // > if ASKEY_U (PCE/SC using Askey polynomials), then u-space is defined by
  //   std normals, std uniforms, std exponentials, std betas, and std gammas
  // > if EXTENDED_U (PCE/SC with Askey plus numerically-generated polynomials),
  //   then u-space involves at most linear scaling to std distributions.

  size_t i, av_cntr = 0, num_active_vars = iteratedModel.cv();
  ShortArray x_types(num_active_vars), u_types(num_active_vars);
  bool err_flag = false;

  for (i=0; i<numContDesVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::DESIGN;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }
  Pecos::DistributionParams& dp = iteratedModel.distribution_parameters();
  const RealVector& n_l_bnds = dp.normal_lower_bounds();
  const RealVector& n_u_bnds = dp.normal_upper_bounds();
  for (i=0; i<numNormalVars; ++i, ++av_cntr) {
    if (n_l_bnds[i] > -DBL_MAX || n_u_bnds[i] < DBL_MAX) {
      x_types[av_cntr] = Pecos::BOUNDED_NORMAL;
      u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
	Pecos::BOUNDED_NORMAL : Pecos::STD_NORMAL;
    }
    else {
      x_types[av_cntr] = Pecos::NORMAL;
      u_types[av_cntr] = Pecos::STD_NORMAL;
    }
  }
  const RealVector& ln_l_bnds = dp.lognormal_lower_bounds();
  const RealVector& ln_u_bnds = dp.lognormal_upper_bounds();
  for (i=0; i<numLognormalVars; ++i, ++av_cntr) {
    if (ln_l_bnds[i] > 0. || ln_u_bnds[i] < DBL_MAX) {
      x_types[av_cntr] = Pecos::BOUNDED_LOGNORMAL;
      u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
	Pecos::BOUNDED_LOGNORMAL : Pecos::STD_NORMAL;
    }
    else {
      x_types[av_cntr] = Pecos::LOGNORMAL;
      u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
	Pecos::LOGNORMAL : Pecos::STD_NORMAL;
    }
  }
  for (i=0; i<numUniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::UNIFORM;
    u_types[av_cntr] = (u_space_type == STD_NORMAL_U) ?
      Pecos::STD_NORMAL : Pecos::STD_UNIFORM;
  }
  for (i=0; i<numLoguniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::LOGUNIFORM;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::LOGUNIFORM;  break;
    }
  }
  for (i=0; i<numTriangularVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::TRIANGULAR;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::TRIANGULAR;  break;
    }
  }
  for (i=0; i<numExponentialVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::EXPONENTIAL;
    u_types[av_cntr] = (u_space_type == STD_NORMAL_U) ?
      Pecos::STD_NORMAL : Pecos::STD_EXPONENTIAL;
  }
  for (i=0; i<numBetaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::BETA;
    u_types[av_cntr] = (u_space_type == STD_NORMAL_U) ?
      Pecos::STD_NORMAL : Pecos::STD_BETA;
  }
  for (i=0; i<numGammaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::GAMMA;
    u_types[av_cntr] = (u_space_type == STD_NORMAL_U) ?
      Pecos::STD_NORMAL : Pecos::STD_GAMMA;
  }
  for (i=0; i<numGumbelVars; ++i, ++av_cntr) { // 2-sided distribution
    x_types[av_cntr] = Pecos::GUMBEL;
    u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
      Pecos::GUMBEL  : Pecos::STD_NORMAL;
  }
  for (i=0; i<numFrechetVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::FRECHET;
    u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
      Pecos::FRECHET : Pecos::STD_NORMAL;
  }
  for (i=0; i<numWeibullVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::WEIBULL;
    u_types[av_cntr] = (u_space_type == EXTENDED_U) ?
      Pecos::WEIBULL : Pecos::STD_NORMAL;
  }
  for (i=0; i<numHistogramBinVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::HISTOGRAM_BIN;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::HISTOGRAM_BIN;   break;
    }
  }
  for (i=0; i<numHistogramPtVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::HISTOGRAM_POINT;
    switch (u_space_type) {
    case STD_NORMAL_U: case ASKEY_U:
      err_flag = true;                           break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::HISTOGRAM_POINT; break;
    }
  }
  for (i=0; i<numIntervalVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::INTERVAL;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }
  for (i=0; i<numContStateVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::STATE;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }

  if (err_flag) {
    Cerr << "Error: unsupported mapping in NonD::"
	 << "initialize_random_variable_types()." << std::endl;
    abort_handler(-1);
  }
  else
    natafTransform.initialize_random_variable_types(x_types, u_types);
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void NonD::initialize_random_variable_parameters()
{
  size_t num_active_vars = iteratedModel.cv();
  RealVector x_means(num_active_vars, false),
    x_std_devs(num_active_vars, false), x_l_bnds(num_active_vars, false),
    x_u_bnds(num_active_vars, false);
  RealVectorArray x_addtl(num_active_vars);

  const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();

  size_t i, av_cntr = 0;
  for (i=0; i<numContDesVars; ++i, ++av_cntr) {
    const Real& lwr = x_l_bnds[av_cntr] = c_l_bnds[av_cntr];
    const Real& upr = x_u_bnds[av_cntr] = c_u_bnds[av_cntr];
    // unlike uniform/loguniform/beta/triangular, design bounds are optional
    if (lwr == -DBL_MAX || upr == DBL_MAX) {
      Cerr << "Error: bounds specification required for design variable "
	   << "transformation to standard uniform." << std::endl;
      abort_handler(-1);
    }
    else
      Pecos::moments_from_uniform_params(lwr, upr, x_means[av_cntr],
					 x_std_devs[av_cntr]);
  }
  Pecos::DistributionParams& dp = iteratedModel.distribution_parameters();
  if (numNormalVars) {
    const Pecos::RealVector& n_means    = dp.normal_means();
    const Pecos::RealVector& n_std_devs = dp.normal_std_deviations();
    const Pecos::RealVector& n_l_bnds   = dp.normal_lower_bounds();
    const Pecos::RealVector& n_u_bnds   = dp.normal_upper_bounds();
    for (i=0; i<numNormalVars; ++i, ++av_cntr) {
      x_means[av_cntr]    = n_means[i];
      x_std_devs[av_cntr] = n_std_devs[i];
      x_l_bnds[av_cntr]   = n_l_bnds[i];
      x_u_bnds[av_cntr]   = n_u_bnds[i];
    }
  }
  if (numLognormalVars) {
    const Pecos::RealVector& ln_means     = dp.lognormal_means();
    const Pecos::RealVector& ln_std_devs  = dp.lognormal_std_deviations();
    const Pecos::RealVector& ln_lambdas   = dp.lognormal_lambdas();
    const Pecos::RealVector& ln_zetas     = dp.lognormal_zetas();
    const Pecos::RealVector& ln_err_facts = dp.lognormal_error_factors();
    const Pecos::RealVector& ln_l_bnds    = dp.lognormal_lower_bounds();
    const Pecos::RealVector& ln_u_bnds    = dp.lognormal_upper_bounds();
    // DAKOTA/UQ uses the mean/std dev or mean/error factor of the actual
    // lognormal distribution or the mean/std dev (lambda/zeta) of the
    // corresponding normal distribution
    bool ln_l_z = !ln_lambdas.empty(), ln_m_s = !ln_std_devs.empty();
    Real mean, stdev, lambda, zeta, err_fact;
    for (i=0; i<numLognormalVars; ++i, ++av_cntr) {
      Pecos::all_from_lognormal_spec(ln_means, ln_std_devs, ln_lambdas,
				     ln_zetas, ln_err_facts, i, mean, stdev,
				     lambda, zeta, err_fact);
      if (!ln_err_facts.empty()) {
	x_addtl[av_cntr].sizeUninitialized(3);
	x_addtl[av_cntr][2] = err_fact;
      }
      else
	x_addtl[av_cntr].sizeUninitialized(2);
      x_means[av_cntr]    = mean;
      x_std_devs[av_cntr] = stdev;
      x_l_bnds[av_cntr]   = ln_l_bnds[i];
      x_u_bnds[av_cntr]   = ln_u_bnds[i];
      x_addtl[av_cntr][0] = lambda;
      x_addtl[av_cntr][1] = zeta;
    }
  }
  if (numUniformVars) {
    const Pecos::RealVector& u_l_bnds = dp.uniform_lower_bounds();
    const Pecos::RealVector& u_u_bnds = dp.uniform_upper_bounds();
    for (i=0; i<numUniformVars; ++i, ++av_cntr) {
      const Real& lwr = x_l_bnds[av_cntr] = u_l_bnds[i];
      const Real& upr = x_u_bnds[av_cntr] = u_u_bnds[i];
      Pecos::moments_from_uniform_params(lwr, upr, x_means[av_cntr],
					 x_std_devs[av_cntr]);
    }
  }
  if (numLoguniformVars) {
    // see SAND98-0210 LHS manual, pp. 43-44
    const Pecos::RealVector& lu_l_bnds = dp.loguniform_lower_bounds();
    const Pecos::RealVector& lu_u_bnds = dp.loguniform_upper_bounds();
    for (i=0; i<numLoguniformVars; ++i, ++av_cntr) {
      const Real& lwr = x_l_bnds[av_cntr] = lu_l_bnds[i];
      const Real& upr = x_u_bnds[av_cntr] = lu_u_bnds[i];
      Pecos::moments_from_loguniform_params(lwr, upr, x_means[av_cntr],
					    x_std_devs[av_cntr]);
    }
  }
  if (numTriangularVars) {
    // See Haldar and Mahadevan, p. 99
    const Pecos::RealVector& t_modes  = dp.triangular_modes();
    const Pecos::RealVector& t_l_bnds = dp.triangular_lower_bounds();
    const Pecos::RealVector& t_u_bnds = dp.triangular_upper_bounds();
    for (i=0; i<numTriangularVars; ++i, ++av_cntr) {
      const Real& mode = t_modes[i];
      const Real& lwr  = x_l_bnds[av_cntr] = t_l_bnds[i];
      const Real& upr  = x_u_bnds[av_cntr] = t_u_bnds[i];
      Pecos::moments_from_triangular_params(lwr, upr, mode, x_means[av_cntr],
					    x_std_devs[av_cntr]);
      x_addtl[av_cntr].sizeUninitialized(1);
      x_addtl[av_cntr][0] = mode;
    }
  }
  if (numExponentialVars) {
    // DAKOTA employs the 1/beta exp(-x/beta) definition, which differs from
    // the lambda exp(-lambda x) LHS definition (lambda_LHS = 1/beta_DAKOTA).
    const Pecos::RealVector& e_betas = dp.exponential_betas();
    for (i=0; i<numExponentialVars; ++i, ++av_cntr) {
      const Real& beta = e_betas[i];
      Pecos::moments_from_exponential_params(beta, x_means[av_cntr],
					     x_std_devs[av_cntr]);
      // use default distribution bounds, not default global DACE bounds
      x_l_bnds[av_cntr] = 0.;
      x_u_bnds[av_cntr] = DBL_MAX;
      x_addtl[av_cntr].sizeUninitialized(1);
      x_addtl[av_cntr][0] = beta;
    }
  }
  if (numBetaVars) {
    // See Haldar and Mahadevan, p. 72
    const Pecos::RealVector& b_alphas = dp.beta_alphas();
    const Pecos::RealVector& b_betas  = dp.beta_betas();
    const Pecos::RealVector& b_l_bnds = dp.beta_lower_bounds();
    const Pecos::RealVector& b_u_bnds = dp.beta_upper_bounds();
    for (i=0; i<numBetaVars; ++i, ++av_cntr) {
      const Real& alpha = b_alphas[i];
      const Real& beta  = b_betas[i];
      const Real& lwr   = x_l_bnds[av_cntr] = b_l_bnds[i];
      const Real& upr   = x_u_bnds[av_cntr] = b_u_bnds[i];
      Pecos::moments_from_beta_params(lwr, upr, alpha, beta, x_means[av_cntr],
				      x_std_devs[av_cntr]);
      x_addtl[av_cntr].sizeUninitialized(2);
      x_addtl[av_cntr][0] = alpha;
      x_addtl[av_cntr][1] = beta;
    }
  }
  if (numGammaVars) {
    // This follows the Gamma(alpha,beta) definition in Law & Kelton, which
    // differs from the LHS definition (beta_LK = 1/beta_LHS).
    const Pecos::RealVector& ga_alphas = dp.gamma_alphas();
    const Pecos::RealVector& ga_betas  = dp.gamma_betas();
    for (i=0; i<numGammaVars; ++i, ++av_cntr) {
      const Real& alpha = ga_alphas[i]; const Real& beta = ga_betas[i];
      Pecos::moments_from_gamma_params(alpha, beta, x_means[av_cntr],
				       x_std_devs[av_cntr]);
      // use default distribution bounds, not default global DACE bounds
      x_l_bnds[av_cntr] = 0.;
      x_u_bnds[av_cntr] = DBL_MAX;
      x_addtl[av_cntr].sizeUninitialized(2);
      x_addtl[av_cntr][0] = alpha;
      x_addtl[av_cntr][1] = beta;
    }
  }
  if (numGumbelVars) {
    // See Haldar and Mahadevan, p. 90
    const Pecos::RealVector& gu_alphas = dp.gumbel_alphas();
    const Pecos::RealVector& gu_betas  = dp.gumbel_betas();
    for (i=0; i<numGumbelVars; ++i, ++av_cntr) {
      const Real& alpha = gu_alphas[i]; const Real& beta = gu_betas[i];
      Pecos::moments_from_gumbel_params(alpha, beta, x_means[av_cntr],
					x_std_devs[av_cntr]);
      // use default distribution bounds, not default global DACE bounds
      x_l_bnds[av_cntr] = -DBL_MAX;
      x_u_bnds[av_cntr] =  DBL_MAX;
      x_addtl[av_cntr].sizeUninitialized(2);
      x_addtl[av_cntr][0] = alpha;
      x_addtl[av_cntr][1] = beta;
    }
  }
  if (numFrechetVars) {
    const Pecos::RealVector& f_alphas = dp.frechet_alphas();
    const Pecos::RealVector& f_betas  = dp.frechet_betas();
    for (i=0; i<numFrechetVars; ++i, ++av_cntr) {
      const Real& alpha = f_alphas[i]; const Real& beta = f_betas[i];
      Pecos::moments_from_frechet_params(alpha, beta, x_means[av_cntr],
					 x_std_devs[av_cntr]);
      // use default distribution bounds, not default global DACE bounds
      x_l_bnds[av_cntr] = 0.;
      x_u_bnds[av_cntr] = DBL_MAX;
      x_addtl[av_cntr].sizeUninitialized(2);
      x_addtl[av_cntr][0] = alpha;
      x_addtl[av_cntr][1] = beta;
    }
  }
  if (numWeibullVars) {
    const Pecos::RealVector& w_alphas = dp.weibull_alphas();
    const Pecos::RealVector& w_betas  = dp.weibull_betas();
    for (i=0; i<numWeibullVars; ++i, ++av_cntr) {
      const Real& alpha = w_alphas[i]; const Real& beta = w_betas[i];
      Pecos::moments_from_weibull_params(alpha, beta, x_means[av_cntr],
					 x_std_devs[av_cntr]);
      // use default distribution bounds, not default global DACE bounds
      x_l_bnds[av_cntr] = 0.;
      x_u_bnds[av_cntr] = DBL_MAX;
      x_addtl[av_cntr].sizeUninitialized(2);
      x_addtl[av_cntr][0] = alpha;
      x_addtl[av_cntr][1] = beta;
    }
  }
  if (numHistogramBinVars) {
    const Pecos::RealVectorArray& hist_bin_prs = dp.histogram_bin_pairs();
    for (i=0; i<numHistogramBinVars; ++i, ++av_cntr) {
      // histogram bnds already processed for global bnds in NIDRProblemDescDB
      x_l_bnds[av_cntr] = c_l_bnds[av_cntr];
      x_u_bnds[av_cntr] = c_u_bnds[av_cntr];
      Pecos::moments_from_histogram_bin_params(hist_bin_prs[i],
					       x_means[av_cntr],
					       x_std_devs[av_cntr]);
      x_addtl[av_cntr] = hist_bin_prs[i];
    }
  }
  if (numHistogramPtVars) {
    const Pecos::RealVectorArray& hist_pt_prs = dp.histogram_point_pairs();
    for (i=0; i<numHistogramPtVars; ++i, ++av_cntr) {
      // histogram bnds already processed for global bnds in NIDRProblemDescDB
      x_l_bnds[av_cntr] = c_l_bnds[av_cntr];
      x_u_bnds[av_cntr] = c_u_bnds[av_cntr];
      Pecos::moments_from_histogram_pt_params(hist_pt_prs[i], x_means[av_cntr],
					      x_std_devs[av_cntr]);
      x_addtl[av_cntr] = hist_pt_prs[i];
    }
  }
  for (i=0; i<numIntervalVars; ++i, ++av_cntr) {
    // interval bnds already processed for global bnds in NIDRProblemDescDB
    const Real& lwr = x_l_bnds[av_cntr] = c_l_bnds[av_cntr];
    const Real& upr = x_u_bnds[av_cntr] = c_u_bnds[av_cntr];
    Pecos::moments_from_uniform_params(lwr, upr, x_means[av_cntr],
				       x_std_devs[av_cntr]);
  }
  for (i=0; i<numContStateVars; ++i, ++av_cntr) {
    const Real& lwr = x_l_bnds[av_cntr] = c_l_bnds[av_cntr];
    const Real& upr = x_u_bnds[av_cntr] = c_u_bnds[av_cntr];
    // unlike uniform/loguniform/beta/triangular, state bounds are optional
    if (lwr == -DBL_MAX || upr == DBL_MAX) {
      Cerr << "Error: bounds specification required for state variable "
	   << "transformation to standard uniform." << std::endl;
      abort_handler(-1);
    }
    else
      Pecos::moments_from_uniform_params(lwr, upr, x_means[av_cntr],
					 x_std_devs[av_cntr]);
  }

  natafTransform.initialize_random_variable_parameters(x_means, x_std_devs,
    x_l_bnds, x_u_bnds, x_addtl);
}


/** This function is commonly used to publish tranformation data when
    the Model variables are in a transformed space (e.g., u-space) and
    ProbabilityTransformation::ranVarTypes et al. may not be generated
    directly.  This allows for the use of inverse transformations to
    return the transformed space variables to their original states. */
#ifdef HAVE_PECOS
void NonD::
initialize_random_variables(const Pecos::ProbabilityTransformation& transform)
{
  if (natafTransform.is_null())
    natafTransform = Pecos::ProbabilityTransformation("nataf"); // for now

  natafTransform.initialize_random_variables(transform); // or could use copy()

  // infer numContDesVars and numContStateVars, but don't update continuous
  // aleatory/epistemic uncertain counts (these may be u-space counts).
  const Pecos::ShortArray& x_types = transform.x_types();
  numContDesVars
    = std::count(x_types.begin(), x_types.end(), (short)Pecos::DESIGN);
  numContStateVars
    = std::count(x_types.begin(), x_types.end(), (short)Pecos::STATE);
}
#endif // HAVE_PECOS


void NonD::initialize_distribution_mappings()
{
  // Default sizing assumes no distinction between requested and achieved levels
  // for the same measure (the request is always achieved) and assumes
  // probability (e.g., computed by binning) and reliability (e.g., computed by
  // moment projection) are not collapsible.
  if (computedRespLevels.empty() || computedProbLevels.empty() ||
      computedRelLevels.empty()  || computedGenRelLevels.empty()) {
    computedRespLevels.resize(numFunctions);
    computedProbLevels.resize(numFunctions);
    computedRelLevels.resize(numFunctions);
    computedGenRelLevels.resize(numFunctions);
    for (size_t i=0; i<numFunctions; ++i) {
      size_t num_levels = requestedRespLevels[i].length();
      switch (respLevelTarget) {
      case PROBABILITIES:
	computedProbLevels[i].resize(num_levels);   break;
      case RELIABILITIES:
	computedRelLevels[i].resize(num_levels);    break;
      case GEN_RELIABILITIES:
	computedGenRelLevels[i].resize(num_levels); break;
      }
      num_levels = requestedProbLevels[i].length() +
	requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
      computedRespLevels[i].resize(num_levels);
    }
  }
}


void NonD::print_distribution_mappings(std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  // output CDF/CCDF probabilities resulting from binning or CDF/CCDF
  // reliabilities resulting from number of std devs separating mean & target
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision)
    << "\nLevel mappings for each response function:\n";
  size_t i, j, width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;
  for (i=0; i<numFunctions; ++i) {
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty()) {
      if (cdfFlag)
	s << "Cumulative Distribution Function (CDF) for ";
      else
	s << "Complementary Cumulative Distribution Function (CCDF) for ";
      s << resp_labels[i] << ":\n     Response Level  Probability Level  "
	<< "Reliability Index  General Rel Index\n     --------------  "
	<< "-----------------  -----------------  -----------------\n";
      size_t num_resp_levels = requestedRespLevels[i].length();
      for (j=0; j<num_resp_levels; j++) {
	s << "  " << std::setw(width) << requestedRespLevels[i][j] << "  ";
	switch (respLevelTarget) {
	case PROBABILITIES:
	  s << std::setw(width) << computedProbLevels[i][j]   << '\n'; break;
	case RELIABILITIES:
	  s << std::setw(w2p2)  << computedRelLevels[i][j]    << '\n'; break;
	case GEN_RELIABILITIES:
	  s << std::setw(w3p4)  << computedGenRelLevels[i][j] << '\n'; break;
	}
      }
      size_t num_prob_levels = requestedProbLevels[i].length();
      for (j=0; j<num_prob_levels; j++)
	s << "  " << std::setw(width) << computedRespLevels[i][j]
	  << "  " << std::setw(width) << requestedProbLevels[i][j] << '\n';
      size_t num_rel_levels = requestedRelLevels[i].length(),
	     offset = num_prob_levels;
      for (j=0; j<num_rel_levels; j++)
	s << "  " << std::setw(width) << computedRespLevels[i][j+offset]
	  << "  " << std::setw(w2p2)  << requestedRelLevels[i][j] << '\n';
      size_t num_gen_rel_levels = requestedGenRelLevels[i].length();
      offset += num_rel_levels;
      for (j=0; j<num_gen_rel_levels; j++)
	s << "  " << std::setw(width) << computedRespLevels[i][j+offset]
	  << "  " << std::setw(w3p4)  << requestedGenRelLevels[i][j] << '\n';
    }
  }
}

} // namespace Dakota
