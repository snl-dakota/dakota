/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonD
//- Description: Base class for NonDeterministic methods
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "RecastModel.hpp"
#include "DakotaResponse.hpp"
#include "DakotaNonD.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_tabular_io.hpp"
#include "pecos_stat_util.hpp"
#ifdef HAVE_DDACE
#include "Distribution.h"
#elif defined(DAKOTA_UTILIB)
#include <utilib/seconds.h>
#endif

//#define DEBUG

static const char rcsId[]="@(#) $Id: DakotaNonD.cpp 7024 2010-10-16 01:24:42Z mseldre $";

namespace Dakota {

// initialization of statics
NonD* NonD::nondInstance(NULL);


NonD::NonD(ProblemDescDB& problem_db, Model& model):
  Analyzer(problem_db, model), numContDesVars(0), numDiscIntDesVars(0),
  numDiscStringDesVars(0), numDiscRealDesVars(0), numDesignVars(0),
  numContStateVars(0), numDiscIntStateVars(0), numDiscStringStateVars(0),
  numDiscRealStateVars(0), numStateVars(0), numNormalVars(0),
  numLognormalVars(0), numUniformVars(0), numLoguniformVars(0),
  numTriangularVars(0), numExponentialVars(0), numBetaVars(0), numGammaVars(0),
  numGumbelVars(0), numFrechetVars(0), numWeibullVars(0),
  numHistogramBinVars(0), numPoissonVars(0), numBinomialVars(0),
  numNegBinomialVars(0), numGeometricVars(0), numHyperGeomVars(0),
  numHistogramPtIntVars(0), numHistogramPtStringVars(0),
  numHistogramPtRealVars(0), numContIntervalVars(0), numDiscIntervalVars(0),
  numDiscSetIntUncVars(0), numDiscSetStringUncVars(0), numDiscSetRealUncVars(0),
  numContAleatUncVars(0), numDiscIntAleatUncVars(0),
  numDiscStringAleatUncVars(0), numDiscRealAleatUncVars(0),
  numAleatoryUncVars(0), numContEpistUncVars(0), numDiscIntEpistUncVars(0),
  numDiscStringEpistUncVars(0), numDiscRealEpistUncVars(0),
  numEpistemicUncVars(0),
  respLevelTarget(probDescDB.get_short("method.nond.response_level_target")),
  respLevelTargetReduce(
    probDescDB.get_short("method.nond.response_level_target_reduce")),
  requestedRespLevels(probDescDB.get_rva("method.nond.response_levels")),
  requestedProbLevels(probDescDB.get_rva("method.nond.probability_levels")),
  requestedRelLevels(probDescDB.get_rva("method.nond.reliability_levels")),
  requestedGenRelLevels(
    probDescDB.get_rva("method.nond.gen_reliability_levels")),
  totalLevelRequests(0),
  cdfFlag(probDescDB.get_short("method.nond.distribution") != COMPLEMENTARY),
  pdfOutput(false), distParamDerivs(false)
{
  bool err_flag = false;
  const Variables& vars = iteratedModel.current_variables();
  short active_view = vars.view().first;
  const SharedVariablesData& svd = vars.shared_data();

  // initialize aleatory uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_ALEATORY_UNCERTAIN) {
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

    numPoissonVars = probDescDB.get_sizet("variables.poisson_uncertain");
    numBinomialVars = probDescDB.get_sizet("variables.binomial_uncertain");
    numNegBinomialVars
      = probDescDB.get_sizet("variables.negative_binomial_uncertain");
    numGeometricVars = probDescDB.get_sizet("variables.geometric_uncertain");
    numHyperGeomVars
      = probDescDB.get_sizet("variables.hypergeometric_uncertain");
    numHistogramPtIntVars
      = probDescDB.get_sizet("variables.histogram_uncertain.point_int");

    numHistogramPtStringVars
      = probDescDB.get_sizet("variables.histogram_uncertain.point_string");

    numHistogramPtRealVars
      = probDescDB.get_sizet("variables.histogram_uncertain.point_real");

    svd.aleatory_uncertain_counts(numContAleatUncVars, numDiscIntAleatUncVars,
				  numDiscStringAleatUncVars,
				  numDiscRealAleatUncVars);
    numAleatoryUncVars = numContAleatUncVars       + numDiscIntAleatUncVars
                       + numDiscStringAleatUncVars + numDiscRealAleatUncVars;
  }

  // initialize epistemic uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_EPISTEMIC_UNCERTAIN) { // epistemic or both
    numContIntervalVars
      = probDescDB.get_sizet("variables.continuous_interval_uncertain");

    numDiscIntervalVars
      = probDescDB.get_sizet("variables.discrete_interval_uncertain");
    numDiscSetIntUncVars
      = probDescDB.get_sizet("variables.discrete_uncertain_set_int");

    numDiscSetStringUncVars
      = probDescDB.get_sizet("variables.discrete_uncertain_set_string");

    numDiscSetRealUncVars
      = probDescDB.get_sizet("variables.discrete_uncertain_set_real");

    svd.epistemic_uncertain_counts(numContEpistUncVars, numDiscIntEpistUncVars,
				   numDiscStringEpistUncVars,
				   numDiscRealEpistUncVars);
    numEpistemicUncVars = numContEpistUncVars + numDiscIntEpistUncVars +
      numDiscStringEpistUncVars + numDiscRealEpistUncVars;
  }

  // default mode definition (can be overridden in derived classes, e.g.,
  // based on NonDSampling::samplingVarsMode):
  epistemicStats = (numEpistemicUncVars > 0);

  // initialize total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // initialize design variables (if active)
  if (active_view == RELAXED_ALL    || active_view == MIXED_ALL ||
      active_view == RELAXED_DESIGN || active_view == MIXED_DESIGN) {
    svd.design_counts(numContDesVars, numDiscIntDesVars, numDiscStringDesVars,
		      numDiscRealDesVars);
    numDesignVars = numContDesVars + numDiscIntDesVars + numDiscStringDesVars
                  + numDiscRealDesVars;
  }

  // initialize state variables (if active)
  if (active_view == RELAXED_ALL   || active_view == MIXED_ALL ||
      active_view == RELAXED_STATE || active_view == MIXED_STATE) {
    svd.state_counts(numContStateVars, numDiscIntStateVars,
		     numDiscStringStateVars, numDiscRealStateVars);
    numStateVars = numContStateVars       + numDiscIntStateVars
                 + numDiscStringStateVars + numDiscRealStateVars;
  }

  if ( !numUncertainVars && !numDesignVars && !numStateVars ) {
    Cerr << "\nError: number of active variables must be nonzero in Dakota::"
	 << "NonD." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
      numDiscreteRealVars != numDesignVars + numUncertainVars + numStateVars) {
    Cerr << "\nError: inconsistent active variable counts ("
	 << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
            numDiscreteRealVars << ", " << numDesignVars + numUncertainVars +
            numStateVars << ") in Dakota::" << "NonD(Model&) for method "
	 << method_enum_to_string(methodName) << '.' << std::endl;
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
  distribute_levels(requestedRespLevels);             // always ascending
  distribute_levels(requestedProbLevels,    cdfFlag); // ascending if cumulative
  distribute_levels(requestedRelLevels,    !cdfFlag);// descending if cumulative
  distribute_levels(requestedGenRelLevels, !cdfFlag);// descending if cumulative

  for (size_t i=0; i<numFunctions; i++)
    totalLevelRequests += requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();

  // simple PDF output control for now (suppressed if quiet/silent output level)
  if (totalLevelRequests && outputLevel >= NORMAL_OUTPUT)
    pdfOutput = true;

  if (err_flag)
    abort_handler(-1);
}


NonD::NonD(unsigned short method_name, Model& model):
  Analyzer(method_name, model), numContDesVars(0), numDiscIntDesVars(0),
  numDiscStringDesVars(0), numDiscRealDesVars(0), numDesignVars(0),
  numContStateVars(0), numDiscIntStateVars(0), numDiscStringStateVars(0),
  numDiscRealStateVars(0), numStateVars(0), numNormalVars(0),
  numLognormalVars(0), numUniformVars(0), numLoguniformVars(0),
  numTriangularVars(0), numExponentialVars(0), numBetaVars(0), numGammaVars(0),
  numGumbelVars(0), numFrechetVars(0), numWeibullVars(0),
  numHistogramBinVars(0), numPoissonVars(0), numBinomialVars(0),
  numNegBinomialVars(0), numGeometricVars(0), numHyperGeomVars(0),
  numHistogramPtIntVars(0), numHistogramPtStringVars(0),
  numHistogramPtRealVars(0), numContIntervalVars(0), numDiscIntervalVars(0),
  numDiscSetIntUncVars(0), numDiscSetStringUncVars(0), numDiscSetRealUncVars(0),
  numContAleatUncVars(0), numDiscIntAleatUncVars(0),
  numDiscStringAleatUncVars(0), numDiscRealAleatUncVars(0),
  numAleatoryUncVars(0), numContEpistUncVars(0), numDiscIntEpistUncVars(0),
  numDiscStringEpistUncVars(0), numDiscRealEpistUncVars(0),
  numEpistemicUncVars(0), totalLevelRequests(0), cdfFlag(true),
  pdfOutput(false), distParamDerivs(false)
{
  // NonDEvidence and NonDAdaptImpSampling use this ctor

  bool err_flag = false;
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  const SharedVariablesData& svd = vars.shared_data();

  // initialize aleatory uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_ALEATORY_UNCERTAIN) { // aleatory or both
    Pecos::AleatoryDistParams& adp = model.aleatory_distribution_parameters();
    numNormalVars       = adp.normal_means().length();
    numLognormalVars    = adp.lognormal_means().length();
    numUniformVars      = adp.uniform_lower_bounds().length();
    numLoguniformVars   = adp.loguniform_lower_bounds().length();
    numTriangularVars   = adp.triangular_modes().length();
    numExponentialVars  = adp.exponential_betas().length();
    numBetaVars         = adp.beta_alphas().length();
    numGammaVars        = adp.gamma_alphas().length();
    numGumbelVars       = adp.gumbel_alphas().length();
    numFrechetVars      = adp.frechet_alphas().length();
    numWeibullVars      = adp.weibull_alphas().length();
    numHistogramBinVars = adp.histogram_bin_pairs().size();

    numPoissonVars     = adp.poisson_lambdas().length();
    numBinomialVars    = adp.binomial_probability_per_trial().length();
    numNegBinomialVars = adp.negative_binomial_probability_per_trial().length();
    numGeometricVars   = adp.geometric_probability_per_trial().length();
    numHyperGeomVars   = adp.hypergeometric_num_drawn().length();
    numHistogramPtIntVars = adp.histogram_point_int_pairs().size();

    numHistogramPtStringVars = adp.histogram_point_string_pairs().size();

    numHistogramPtRealVars   = adp.histogram_point_real_pairs().size();

    svd.aleatory_uncertain_counts(numContAleatUncVars, numDiscIntAleatUncVars,
				  numDiscStringAleatUncVars,
				  numDiscRealAleatUncVars);
    numAleatoryUncVars = numContAleatUncVars       + numDiscIntAleatUncVars
                       + numDiscStringAleatUncVars + numDiscRealAleatUncVars;
  }

  // initialize epistemic uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_EPISTEMIC_UNCERTAIN) { // epistemic or both
    Pecos::EpistemicDistParams& edp = model.epistemic_distribution_parameters();
    numContIntervalVars  = edp.continuous_interval_basic_probabilities().size();

    numDiscIntervalVars  = edp.discrete_interval_basic_probabilities().size();
    numDiscSetIntUncVars = edp.discrete_set_int_values_probabilities().size();

    numDiscSetStringUncVars
      = edp.discrete_set_string_values_probabilities().size();

    numDiscSetRealUncVars = edp.discrete_set_real_values_probabilities().size();

    svd.epistemic_uncertain_counts(numContEpistUncVars, numDiscIntEpistUncVars,
				   numDiscStringEpistUncVars,
				   numDiscRealEpistUncVars);
    numEpistemicUncVars = numContEpistUncVars       + numDiscIntEpistUncVars
                        + numDiscStringEpistUncVars + numDiscRealEpistUncVars;
  }

  // default mode definition (can be overridden in derived classes, e.g.,
  // based on NonDSampling::samplingVarsMode):
  epistemicStats = (numEpistemicUncVars > 0);

  // initialize total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // initialize design variables (if active)
  if (active_view == RELAXED_ALL    || active_view == MIXED_ALL ||
      active_view == RELAXED_DESIGN || active_view == MIXED_DESIGN) {
    svd.design_counts(numContDesVars, numDiscIntDesVars, numDiscStringDesVars,
		      numDiscRealDesVars);
    numDesignVars = numContDesVars       + numDiscIntDesVars
                  + numDiscStringDesVars + numDiscRealDesVars;
  }

  // initialize state variables (if active)
  if (active_view == RELAXED_ALL   || active_view == MIXED_ALL ||
      active_view == RELAXED_STATE || active_view == MIXED_STATE) {
    svd.state_counts(numContStateVars, numDiscIntStateVars,
		     numDiscStringStateVars, numDiscRealStateVars);
    numStateVars = numContStateVars       + numDiscIntStateVars
                 + numDiscStringStateVars + numDiscRealStateVars;
  }

  if ( !numUncertainVars && !numDesignVars && !numStateVars ) {
    Cerr << "\nError: number of active variables must be nonzero in Dakota::"
	 << "NonD." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
      numDiscreteRealVars != numDesignVars + numUncertainVars + numStateVars) {
    Cout << "\nError: inconsistent active variable counts ("
	 << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
            numDiscreteRealVars << ", " << numDesignVars + numUncertainVars +
            numStateVars << ") in Dakota::NonD(Model&)." << std::endl;
    err_flag = true;
  }

  // current set of statistics is mean, standard deviation, and
  // probability of failure for each response function
  //ShortArray asv(3*numFunctions, 1);
  //finalStatistics = Response(numUncertainVars, asv);

  if (err_flag)
    abort_handler(-1);
}


NonD::NonD(unsigned short method_name, const RealVector& lower_bnds,
	   const RealVector& upper_bnds):
  Analyzer(method_name), numContDesVars(0), numDiscIntDesVars(0),
  numDiscStringDesVars(0), numDiscRealDesVars(0), numDesignVars(0),
  numContStateVars(0), numDiscIntStateVars(0), numDiscStringStateVars(0),
  numDiscRealStateVars(0), numStateVars(0), numNormalVars(0),
  numLognormalVars(0), numUniformVars(0), numLoguniformVars(0),
  numTriangularVars(0), numExponentialVars(0), numBetaVars(0), numGammaVars(0),
  numGumbelVars(0), numFrechetVars(0), numWeibullVars(0),
  numHistogramBinVars(0), numPoissonVars(0), numBinomialVars(0),
  numNegBinomialVars(0), numGeometricVars(0), numHyperGeomVars(0),
  numHistogramPtIntVars(0), numHistogramPtStringVars(0),
  numHistogramPtRealVars(0), numContIntervalVars(0), numDiscIntervalVars(0),
  numDiscSetIntUncVars(0), numDiscSetStringUncVars(0), numDiscSetRealUncVars(0),
  numContAleatUncVars(0), numDiscIntAleatUncVars(0),
  numDiscStringAleatUncVars(0), numDiscRealAleatUncVars(0),
  numAleatoryUncVars(0), numContEpistUncVars(0), numDiscIntEpistUncVars(0),
  numDiscStringEpistUncVars(0), numDiscRealEpistUncVars(0),
  numEpistemicUncVars(0), numUncertainVars(numUniformVars),
  epistemicStats(false), totalLevelRequests(0), cdfFlag(true), pdfOutput(false),
  distParamDerivs(false)
{
  // ConcurrentStrategy uses this ctor for design opt, either for multi-start
  // initial points or multibjective weight sets.

  numContinuousVars  = numUniformVars;
  numDiscreteIntVars = numDiscreteStringVars = numDiscreteRealVars = 0;
}


void NonD::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);
}


int NonD::generate_system_seed()
{
  // Generate initial seed from a system clock.  NOTE: the system clock
  // should not used for multiple LHS calls since (1) clock granularity can
  // be too coarse (can repeat on subsequent runs for inexpensive test fns)
  // and (2) seed progression can be highly structured, which could induce
  // correlation between sample sets.  Instead, the clock-generated case
  // varies the seed below using the same approach as the user-specified
  // case.  This has the additional benefit that a random run can be
  // recreated by specifying the clock-generated seed in the input file.
  int seed = 1;
#ifdef HAVE_DDACE
  seed += DistributionBase::timeSeed(); // microsecs, time of day
#elif defined(DAKOTA_UTILIB)
  seed += (int)CurrentTime();           // secs, time of day
#else
  seed += (int)clock();                 // clock ticks, exec time
#endif

  return seed;
}


void NonD::
requested_levels(const RealVectorArray& req_resp_levels,
		 const RealVectorArray& req_prob_levels,
		 const RealVectorArray& req_rel_levels,
		 const RealVectorArray& req_gen_rel_levels,
		 short resp_lev_tgt, short resp_lev_tgt_reduce, bool cdf_flag)
{
  respLevelTarget = resp_lev_tgt;
  respLevelTargetReduce = resp_lev_tgt_reduce;
  cdfFlag = cdf_flag;

  size_t i;
  totalLevelRequests = 0;
  if (req_resp_levels.empty())
    requestedRespLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedRespLevels = req_resp_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedRespLevels);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedRespLevels[i].length();
  }
  if (req_prob_levels.empty())
    requestedProbLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedProbLevels = req_prob_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedProbLevels, cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedProbLevels[i].length();
  }
  if (req_rel_levels.empty())
    requestedRelLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedRelLevels = req_rel_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedRelLevels, !cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedRelLevels[i].length();
  }
  if (req_gen_rel_levels.empty())
    requestedGenRelLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedGenRelLevels = req_gen_rel_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedGenRelLevels, !cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedGenRelLevels[i].length();
  }

  if (totalLevelRequests && outputLevel >= NORMAL_OUTPUT)
    pdfOutput = true;

  initialize_final_statistics();
  initialize_response_covariance();
}


void NonD::distribute_levels(RealVectorArray& levels, bool ascending)
{
  size_t i, j, num_level_arrays = levels.size();
  // when not already performed by the parser (i.e., a num_levels key
  // was not provided), distribute levels among an array of vectors
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
      size_t new_len = total_len/numFunctions;
      levels.resize(numFunctions);
      for (i=0; i<numFunctions; ++i) {
	RealVector& vec_i = levels[i];
	vec_i.sizeUninitialized(new_len);
        for (j=0; j<new_len; ++j)
          vec_i[j] = level_array0[i*new_len+j];
      }
    }
    else {
      Cerr << "\nError: num_levels specification differs from the number of "
           << "response functions." << std::endl;
      abort_handler(-1);
    }
  }
  // now ensure that each vector is sorted (ascending order is default)
  for (i=0; i<numFunctions; i++) {
    size_t len_i = levels[i].length();
    if (len_i > 1) {
      Real* vec_i = levels[i].values();
      if (ascending)
	std::sort(vec_i, vec_i + len_i); // ascending sort by default
      else
	std::sort(vec_i, vec_i + len_i, std::greater<Real>());// descending sort
    }
  }
}


void NonD::
transform_model(Model& x_model, Model& u_model, bool global_bounds, Real bound)
{
  size_t i, num_cdv_cauv = numContDesVars+numContAleatUncVars;
  Sizet2DArray vars_map, primary_resp_map, secondary_resp_map;
  SizetArray recast_vars_comps_total; // default: no change in cauv total
  // we do not reorder the u-space variable types such that we preserve a
  // 1-to-1 mapping with consistent ordering
  vars_map.resize(numContinuousVars);
  for (i=0; i<numContinuousVars; ++i) {
    vars_map[i].resize(1);
    vars_map[i][0] = i;
  }
  primary_resp_map.resize(numFunctions);
  for (i=0; i<numFunctions; ++i) {
    primary_resp_map[i].resize(1);
    primary_resp_map[i][0] = i;
  }
  // Nataf is a nonlinear tranformation for all variables except Normals.
  // Nonlinear mappings require special ASV logic for transforming Hessians.
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  const Pecos::ShortArray& u_types = natafTransform.u_types();
  bool nonlinear_vars_map = false; short x_type, u_type;
  for (i=numContDesVars; i<num_cdv_cauv; ++i) {
    x_type = x_ran_vars[i].type(); u_type = u_types[i];
    if ( x_type != u_type &&
	 !( x_type == Pecos::NORMAL && u_type == Pecos::STD_NORMAL ) &&
	 !( ( x_type == Pecos::UNIFORM || x_type == Pecos::HISTOGRAM_BIN ||
	      x_type == Pecos::CONTINUOUS_DESIGN   ||
	      x_type == Pecos::CONTINUOUS_INTERVAL ||
	      x_type == Pecos::CONTINUOUS_STATE ) &&
	    u_type == Pecos::STD_UNIFORM ) &&
	 !( x_type == Pecos::EXPONENTIAL && u_type == Pecos::STD_EXPONENTIAL) &&
	 !( x_type == Pecos::BETA   && u_type == Pecos::STD_BETA ) &&
	 !( x_type == Pecos::GAMMA  && u_type == Pecos::STD_GAMMA ) )
      { nonlinear_vars_map = true; break; }
  }

  // There is no additional response mapping beyond that required by the
  // nonlinear variables mapping.
  BoolDequeArray nonlinear_resp_map(numFunctions, BoolDeque(1, false));
  const Response& x_resp = x_model.current_response();
  const SharedVariablesData& svd = x_model.current_variables().shared_data();
  const BitArray& all_relax_di = svd.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = svd.all_relaxed_discrete_real();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!x_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!x_resp.function_hessians().empty())  recast_resp_order |= 4;
  RecastModel* recast_model = new RecastModel(x_model, vars_map,
    recast_vars_comps_total, all_relax_di, all_relax_dr, nonlinear_vars_map,
    vars_u_to_x_mapping, set_u_to_x_mapping, primary_resp_map,
    secondary_resp_map, 0, recast_resp_order, nonlinear_resp_map,
    resp_x_to_u_mapping, NULL);
  u_model.assign_rep(recast_model, false);
  // publish inverse mappings for use in data imports.  Since derivatives are
  // not imported and response values are not transformed, an inverse variables
  // transformation is sufficient for this purpose.
  recast_model->inverse_mappings(vars_x_to_u_mapping, NULL, NULL, NULL);

  // Update continuous aleatory variable types (needed for Model::
  // continuous_{probability_density,distribution_bounds,distribution_moment}())
  for (i=numContDesVars; i<num_cdv_cauv; ++i)
    recast_model->continuous_variable_type(
      pecos_to_dakota_variable_type(u_types[i]), i);

  // Populate aleatory random var distribution params for transformed u-space.
  // *** Note ***: For use with REGRESSION approaches, variable ordering in
  // get_parameter_sets() does not use x_types/u_types as in NonDQuadrature/
  // NonDSparseGrid and thus a possibility for future ordering errors exists.
  // Currently, Design/State -> Uniform is handled separately via global bounds
  // and no current x->u selection in NonD::initialize_random_variable_types()
  // causes an ordering discrepancy.  If a future mapping causes an ordering
  // inconsistency, this could be handled above via the RecastModel vars_map.
  size_t num_u_nuv = 0, num_u_bnuv = 0, num_u_lnuv = 0, num_u_blnuv = 0,
    num_u_uuv = 0, num_u_luuv = 0, num_u_tuv = 0, num_u_euv = 0,
    num_u_buv = 0, num_u_gauv = 0, num_u_guuv = 0, num_u_fuv = 0,
    num_u_wuv = 0, num_u_hbuv = 0;
  for (i=numContDesVars; i<num_cdv_cauv; ++i)
    switch (u_types[i]) {
    case Pecos::STD_NORMAL:        ++num_u_nuv;   break;
    case Pecos::BOUNDED_NORMAL:    ++num_u_bnuv;  break;
    case Pecos::LOGNORMAL:         ++num_u_lnuv;  break;
    case Pecos::BOUNDED_LOGNORMAL: ++num_u_blnuv; break;
    case Pecos::STD_UNIFORM:       ++num_u_uuv;   break;
    case Pecos::LOGUNIFORM:        ++num_u_luuv;  break;
    case Pecos::TRIANGULAR:        ++num_u_tuv;   break;
    case Pecos::STD_EXPONENTIAL:   ++num_u_euv;   break;
    case Pecos::STD_BETA:          ++num_u_buv;   break;
    case Pecos::STD_GAMMA:         ++num_u_gauv;  break;
    case Pecos::GUMBEL:            ++num_u_guuv;  break;
    case Pecos::FRECHET:           ++num_u_fuv;   break;
    case Pecos::WEIBULL:           ++num_u_wuv;   break;
    case Pecos::HISTOGRAM_BIN:     ++num_u_hbuv;  break;
    }
  const Pecos::AleatoryDistParams& x_adp
    = x_model.aleatory_distribution_parameters();
  Pecos::AleatoryDistParams&       u_adp
    = u_model.aleatory_distribution_parameters();
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  if (num_u_nuv || num_u_bnuv) {
    size_t num_total_nuv = num_u_nuv+num_u_bnuv;
    RealVector nuv_means(num_total_nuv, false),
      nuv_std_devs(num_total_nuv, false), nuv_l_bnds(num_total_nuv, false),
      nuv_u_bnds(num_total_nuv, false);
    if (num_u_bnuv) {
      const Pecos::RealVector& x_nuv_means    = x_adp.normal_means();
      const Pecos::RealVector& x_nuv_std_devs = x_adp.normal_std_deviations();
      const Pecos::RealVector& x_nuv_l_bnds   = x_adp.normal_lower_bounds();
      const Pecos::RealVector& x_nuv_u_bnds   = x_adp.normal_upper_bounds();
      size_t n_cntr = 0, x_n_cntr = 0;;
      for (i=numContDesVars; i<num_cdv_cauv; ++i) {
	x_type = x_ran_vars[i].type(); u_type = u_types[i];
	if (u_type == Pecos::BOUNDED_NORMAL) {
	  nuv_means[n_cntr]    = x_nuv_means[x_n_cntr];
	  nuv_std_devs[n_cntr] = x_nuv_std_devs[x_n_cntr];
	  nuv_l_bnds[n_cntr]   = x_nuv_l_bnds[x_n_cntr];
	  nuv_u_bnds[n_cntr]   = x_nuv_u_bnds[x_n_cntr];
	  ++n_cntr;
	}
	else if (u_type == Pecos::STD_NORMAL) {
	  nuv_means[n_cntr]  = 0.;	 nuv_std_devs[n_cntr] = 1.;
	  nuv_l_bnds[n_cntr] = -dbl_inf; nuv_u_bnds[n_cntr]   = dbl_inf;
	  ++n_cntr;
	}
	if (x_type == Pecos::NORMAL || x_type == Pecos::BOUNDED_NORMAL)
	  ++x_n_cntr;
      }
    }
    else {
      nuv_means  = 0.;       nuv_std_devs = 1.;
      nuv_l_bnds = -dbl_inf; nuv_u_bnds   = dbl_inf;
    }
    u_adp.normal_means(nuv_means);
    u_adp.normal_std_deviations(nuv_std_devs);
    u_adp.normal_lower_bounds(nuv_l_bnds);
    u_adp.normal_upper_bounds(nuv_u_bnds);
  }
  if (num_u_lnuv || num_u_blnuv) {
    u_adp.lognormal_means(x_adp.lognormal_means());
    u_adp.lognormal_std_deviations(x_adp.lognormal_std_deviations());
    u_adp.lognormal_lambdas(x_adp.lognormal_lambdas());
    u_adp.lognormal_zetas(x_adp.lognormal_zetas());
    u_adp.lognormal_error_factors(x_adp.lognormal_error_factors());
    if (num_u_blnuv) {
      u_adp.lognormal_lower_bounds(x_adp.lognormal_lower_bounds());
      u_adp.lognormal_upper_bounds(x_adp.lognormal_upper_bounds());
    }
  }
  if (num_u_uuv) {
    RealVector uuv_l_bnds(num_u_uuv, false); uuv_l_bnds = -1.;
    RealVector uuv_u_bnds(num_u_uuv, false); uuv_u_bnds =  1.;
    u_adp.uniform_lower_bounds(uuv_l_bnds);
    u_adp.uniform_upper_bounds(uuv_u_bnds);
  }
  if (num_u_luuv) {
    u_adp.loguniform_lower_bounds(x_adp.loguniform_lower_bounds());
    u_adp.loguniform_upper_bounds(x_adp.loguniform_upper_bounds());
  }
  if (num_u_tuv) {
    u_adp.triangular_modes(x_adp.triangular_modes());
    u_adp.triangular_lower_bounds(x_adp.triangular_lower_bounds());
    u_adp.triangular_upper_bounds(x_adp.triangular_upper_bounds());
  }
  if (num_u_euv) {
    RealVector euv_betas(num_u_euv, false); euv_betas = 1.;
    u_adp.exponential_betas(euv_betas);
  }
  if (num_u_buv) {
    RealVector buv_l_bnds(num_u_buv, false); buv_l_bnds = -1.;
    RealVector buv_u_bnds(num_u_buv, false); buv_u_bnds =  1.;
    u_adp.beta_alphas(x_adp.beta_alphas());
    u_adp.beta_betas(x_adp.beta_betas());
    u_adp.beta_lower_bounds(buv_l_bnds);
    u_adp.beta_upper_bounds(buv_u_bnds);
  }
  if (num_u_gauv) {
    u_adp.gamma_alphas(x_adp.gamma_alphas());
    RealVector gauv_betas(num_u_gauv, false); gauv_betas = 1.;
    u_adp.gamma_betas(gauv_betas);
  }
  if (num_u_guuv) {
    u_adp.gumbel_alphas(x_adp.gumbel_alphas());
    u_adp.gumbel_betas(x_adp.gumbel_betas());
  }
  if (num_u_fuv) {
    u_adp.frechet_alphas(x_adp.frechet_alphas());
    u_adp.frechet_betas(x_adp.frechet_betas());
  }
  if (num_u_wuv) {
    u_adp.weibull_alphas(x_adp.weibull_alphas());
    u_adp.weibull_betas(x_adp.weibull_betas());
  }
  if (num_u_hbuv)
    u_adp.histogram_bin_pairs(x_adp.histogram_bin_pairs());

  // Populate epistemic random var distribution params for transformed u-space
  const Pecos::EpistemicDistParams& x_edp
    = x_model.epistemic_distribution_parameters();
  size_t num_ciuv = x_edp.ceuv();
  if (num_ciuv) {
    RealRealPairRealMapArray ciuv_bpa(num_ciuv);
    RealRealPair bnds(-1., 1.);
    for (i=0; i<num_ciuv; ++i) // one standard cell per variable in u-space
      ciuv_bpa[i][bnds] = 1.;
    Pecos::EpistemicDistParams& u_edp
      = u_model.epistemic_distribution_parameters();
    u_edp.continuous_interval_basic_probabilities(ciuv_bpa);
  }

  if (global_bounds) {
    // [-1,1] are standard bounds for design, state, epistemic, uniform, & beta
    RealVector c_l_bnds(numContinuousVars, false); c_l_bnds = -1.;
    RealVector c_u_bnds(numContinuousVars, false); c_u_bnds =  1.;
    // handle nonstandard bounds
    const RealVector& x_c_l_bnds = x_model.continuous_lower_bounds();
    const RealVector& x_c_u_bnds = x_model.continuous_upper_bounds();
    const RealVector& lnuv_means     = x_adp.lognormal_means();
    const RealVector& lnuv_std_devs  = x_adp.lognormal_std_deviations();
    const RealVector& lnuv_lambdas   = x_adp.lognormal_lambdas();
    const RealVector& lnuv_zetas     = x_adp.lognormal_zetas();
    const RealVector& lnuv_err_facts = x_adp.lognormal_error_factors();
    const RealVector& gauv_alphas = x_adp.gamma_alphas();
    const RealVector& guuv_alphas = x_adp.gumbel_alphas();
    const RealVector& guuv_betas  = x_adp.gumbel_betas();
    const RealVector& fuv_alphas  = x_adp.frechet_alphas();
    const RealVector& fuv_betas   = x_adp.frechet_betas();
    const RealVector& wuv_alphas  = x_adp.weibull_alphas();
    const RealVector& wuv_betas   = x_adp.weibull_betas();
    size_t i, lnuv_cntr = 0, gauv_cntr = 0, guuv_cntr = 0, fuv_cntr = 0,
      wuv_cntr = 0;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:      // mean +/- bound std devs
	c_l_bnds[i] = -bound; c_u_bnds[i] =    bound; break;
      case Pecos::STD_EXPONENTIAL: // [0, mean + bound std devs] for beta=1
	c_l_bnds[i] =     0.; c_u_bnds[i] = 1.+bound; break;
      case Pecos::STD_GAMMA: {
	Real mean, stdev;
	Pecos::GammaRandomVariable::
	  moments_from_params(gauv_alphas[guuv_cntr], 1., mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound*stdev; ++gauv_cntr; break;
      }
      case Pecos::BOUNDED_NORMAL: case Pecos::BOUNDED_LOGNORMAL:
      case Pecos::LOGUNIFORM:     case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:
	// bounded distributions: x-space has desired bounds
	c_l_bnds[i] = x_c_l_bnds[i]; c_u_bnds[i] = x_c_u_bnds[i]; break;
      // Note: Could use x_c_l_bnds/x_c_u_bnds for the following cases as well
      // except that NIDR uses +/-3 sigma, whereas here we're using +/-10 sigma
      case Pecos::LOGNORMAL: { // semi-bounded distribution
	Real mean, stdev;
	Pecos::moments_from_lognormal_spec(lnuv_means, lnuv_std_devs,
					   lnuv_lambdas, lnuv_zetas,
					   lnuv_err_facts, lnuv_cntr,
					   mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound*stdev; ++lnuv_cntr; break;
      }
      case Pecos::GUMBEL: { // unbounded distribution
	Real mean, stdev;
	Pecos::GumbelRandomVariable::
	  moments_from_params(guuv_alphas[guuv_cntr], guuv_betas[guuv_cntr],
			      mean, stdev);
	c_l_bnds[i] = mean - bound*stdev; c_u_bnds[i] = mean + bound*stdev;
	++guuv_cntr; break;
      }
      case Pecos::FRECHET: { // semibounded distribution
	Real mean, stdev;
	Pecos::FrechetRandomVariable::
	  moments_from_params(fuv_alphas[fuv_cntr], fuv_betas[fuv_cntr],
			      mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound*stdev; ++fuv_cntr; break;
      }
      case Pecos::WEIBULL: { // semibounded distribution
	Real mean, stdev;
	Pecos::WeibullRandomVariable::
	  moments_from_params(wuv_alphas[wuv_cntr], wuv_betas[wuv_cntr],
			      mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound*stdev; ++wuv_cntr; break;
      }
      }
    }
    // uncertain bounds not currently used, since ACTIVE, not ACTIVE_UNIFORM
    u_model.continuous_lower_bounds(c_l_bnds);
    u_model.continuous_upper_bounds(c_u_bnds);
  }
}


unsigned short NonD::
pecos_to_dakota_variable_type(unsigned short pecos_var_type)
{
  switch (pecos_var_type) {
  case Pecos::CONTINUOUS_DESIGN:           return CONTINUOUS_DESIGN;      break;
  case Pecos::STD_NORMAL: case Pecos::NORMAL: case Pecos::BOUNDED_NORMAL:
    return NORMAL_UNCERTAIN; break;
  case Pecos::LOGNORMAL: case Pecos::BOUNDED_LOGNORMAL:
    return LOGNORMAL_UNCERTAIN; break;
  case Pecos::STD_UNIFORM: case Pecos::UNIFORM:
    return UNIFORM_UNCERTAIN; break;
  case Pecos::LOGUNIFORM:                  return LOGUNIFORM_UNCERTAIN;   break;
  case Pecos::TRIANGULAR:                  return TRIANGULAR_UNCERTAIN;   break;
  case Pecos::STD_EXPONENTIAL: case Pecos::EXPONENTIAL:
    return EXPONENTIAL_UNCERTAIN; break;
  case Pecos::STD_BETA:  case Pecos::BETA:      return BETA_UNCERTAIN;    break;
  case Pecos::STD_GAMMA: case Pecos::GAMMA:     return GAMMA_UNCERTAIN;   break;
  case Pecos::GUMBEL:                           return GUMBEL_UNCERTAIN;  break;
  case Pecos::FRECHET:                          return FRECHET_UNCERTAIN; break;
  case Pecos::WEIBULL:                          return WEIBULL_UNCERTAIN; break;
  case Pecos::HISTOGRAM_BIN:     return HISTOGRAM_BIN_UNCERTAIN;          break;
  case Pecos::POISSON:           return POISSON_UNCERTAIN;                break;
  case Pecos::BINOMIAL:          return BINOMIAL_UNCERTAIN;               break;
  case Pecos::NEGATIVE_BINOMIAL: return NEGATIVE_BINOMIAL_UNCERTAIN;      break;
  case Pecos::GEOMETRIC:         return GEOMETRIC_UNCERTAIN;              break;
  case Pecos::HYPERGEOMETRIC:    return HYPERGEOMETRIC_UNCERTAIN;         break;
  case Pecos::HISTOGRAM_PT_INT:  return HISTOGRAM_POINT_UNCERTAIN_INT;    break;
  case Pecos::HISTOGRAM_PT_STRING:
    return HISTOGRAM_POINT_UNCERTAIN_STRING; break;
  case Pecos::HISTOGRAM_PT_REAL:   return HISTOGRAM_POINT_UNCERTAIN_REAL; break;
  case Pecos::CONTINUOUS_INTERVAL: return CONTINUOUS_INTERVAL_UNCERTAIN;  break;
  case Pecos::CONTINUOUS_STATE:    return CONTINUOUS_STATE;               break;
  default:
    Cerr << "Error: unsupported Pecos distribution type in "
	 << "pecos_to_dakota_variable_type()." << std::endl;
    abort_handler(-1);
    return 0; break;
  }
}


void NonD::
construct_lhs(Iterator& u_space_sampler, Model& u_model,
	      unsigned short sample_type, int num_samples, int seed,
	      const String& rng, bool vary_pattern, short sampling_vars_mode)
{
  // sanity checks
  if (num_samples <= 0) {
    Cerr << "Error: bad samples specification (" << num_samples << ") in "
	 << "NonD::construct_lhs()." << std::endl;
    abort_handler(-1);
  }

  // construct NonDLHSSampling with default sampling_vars_mode (ACTIVE)
  u_space_sampler.assign_rep(new NonDLHSSampling(u_model, sample_type,
    num_samples, seed, rng, vary_pattern, sampling_vars_mode), false);
}


/** Define the DVV for x-space derivative evaluations by augmenting
    the iterator requests to account for correlations. */
void NonD::set_u_to_x_mapping(const Variables& u_vars, const ActiveSet& u_set,
			      ActiveSet& x_set)
{
  //if (nondInstance->distParamDerivs) {
  //}
  //else
  if (nondInstance->natafTransform.x_correlation()) {
    const SizetArray& u_dvv = u_set.derivative_vector();
    SizetMultiArrayConstView cv_ids = u_vars.continuous_variable_ids();
    SizetMultiArrayConstView icv_ids
      = u_vars.inactive_continuous_variable_ids();
    bool std_dvv = (u_dvv == cv_ids || u_dvv == icv_ids);
    if (!std_dvv) { // partial random variable derivatives: check correlations
      SizetMultiArrayConstView acv_ids = u_vars.all_continuous_variable_ids();
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
	      if (cv_index != j &&
		  std::fabs(corr_x(cv_index, j)) > Pecos::SMALL_NUMBER &&
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
  const RealVector&         x_cv      = x_vars.continuous_variables();
  SizetMultiArrayConstView  x_cv_ids  = x_vars.continuous_variable_ids();
  SizetMultiArrayConstView  x_acv_ids = x_vars.all_continuous_variable_ids();
  const RealVector&         x_fns     = x_response.function_values();

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
	 << "resp_x_to_u_mapping().\nx-space response size =\n" << num_fns
	 << "u-space response size =\n" << u_asv.size() << std::endl;
    abort_handler(-1);
  }
  if (!nondInstance->natafTransform.x_correlation() && u_dvv != x_dvv) {
    Cerr << "Error: inconsistent derivative component definition in NonD::"
	 << "resp_x_to_u_mapping().\nx-space DVV =\n" << x_dvv
	 << "u-space DVV =\n" << u_dvv << std::endl;
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
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = nondInstance->natafTransform.x_random_variables();
  const Pecos::ShortArray& u_types = nondInstance->natafTransform.u_types();
  size_t num_types = u_types.size();
  bool nonlinear_vars_map = false; short x_type, u_type;
  for (i=0; i<num_types; ++i) {
    x_type = x_ran_vars[i].type(); u_type = u_types[i];
    if ( x_type != u_type &&
	 !( x_type == Pecos::NORMAL && u_type == Pecos::STD_NORMAL ) &&
	 !( ( x_type == Pecos::UNIFORM || x_type == Pecos::HISTOGRAM_BIN ||
	      x_type == Pecos::CONTINUOUS_DESIGN   ||
	      x_type == Pecos::CONTINUOUS_INTERVAL ||
	      x_type == Pecos::CONTINUOUS_STATE ) &&
	    u_type == Pecos::STD_UNIFORM ) &&
	 !( x_type == Pecos::EXPONENTIAL && u_type == Pecos::STD_EXPONENTIAL) &&
	 !( x_type == Pecos::BETA        && u_type == Pecos::STD_BETA ) &&
	 !( x_type == Pecos::GAMMA       && u_type == Pecos::STD_GAMMA ) )
      { nonlinear_vars_map = true; break; }
  }

  RealVector   fn_grad_x,  fn_grad_us; RealSymMatrix      fn_hess_us;
  RealMatrix jacobian_xu, jacobian_xs; RealSymMatrixArray hessian_xu;

  if (map_derivs) {
    // The following transformation data is invariant w.r.t. the response fns
    // and is computed outside of the num_fns loop
    if (nondInstance->distParamDerivs)
      nondInstance->natafTransform.jacobian_dX_dS(x_cv, jacobian_xs,
	x_cv_ids, x_acv_ids, nondInstance->primaryACVarMapIndices,
	nondInstance->secondaryACVarMapTargets);
    else {
      if (u_grad_flag || u_hess_flag)
	nondInstance->natafTransform.jacobian_dX_dU(x_cv, jacobian_xu);
      if (u_hess_flag && nonlinear_vars_map)
	nondInstance->natafTransform.hessian_d2X_dU2(x_cv, hessian_xu);
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

    // manage data requirements for derivative transformations: if fn_grad_x
    // is needed for Hessian x-form (nonlinear I/O mapping), then x_asv has been
    // augmented to include the gradient in RecastModel::asv_mapping().
    if (x_asv_val & 2)
      fn_grad_x = x_response.function_gradient_view(i);

    // map gradient dg/dx to dG/du
    if (u_asv_val & 2) {
      if ( !(x_asv_val & 2) ) {
	Cerr << "Error: missing required gradient sub-model data in NonD::"
	     << "resp_x_to_u_mapping()" << std::endl;
	abort_handler(-1);
      }
      if (map_derivs) { // perform transformation
	fn_grad_us = u_response.function_gradient_view(i);
	if (nondInstance->distParamDerivs) // transform subset of components
	  nondInstance->natafTransform.trans_grad_X_to_S(fn_grad_x,
	    fn_grad_us, jacobian_xs, x_dvv, x_cv_ids, x_acv_ids,
	    nondInstance->primaryACVarMapIndices,
	    nondInstance->secondaryACVarMapTargets);
	else   // transform subset of components
	  nondInstance->natafTransform.trans_grad_X_to_U(fn_grad_x,
	    fn_grad_us, jacobian_xu, x_dvv, x_cv_ids);
      }
      else // no transformation: dg/dx = dG/du
	u_response.function_gradient(fn_grad_x, i);
    }

    // map Hessian d^2g/dx^2 to d^2G/du^2
    if (u_asv_val & 4) {
      if ( !(x_asv_val & 4) ||
	   ( map_derivs && nonlinear_vars_map && !(x_asv_val & 2) ) ) {
	Cerr << "Error: missing required sub-model data in NonD::"
	     << "resp_x_to_u_mapping()" << std::endl;
	abort_handler(-1);
      }
      const RealSymMatrix& fn_hess_x = x_response.function_hessian(i);
      if (map_derivs) { // perform transformation
	fn_hess_us = u_response.function_hessian_view(i);
	if (nondInstance->distParamDerivs) { // transform subset of components
	  Cerr << "Error: Hessians with respect to inserted variables not yet "
	       << "supported." << std::endl;
	  abort_handler(-1);
	  //nondInstance->natafTransform.trans_hess_X_to_S(fn_hess_x,
	  //  fn_hess_us, jacobian_xs, hessian_xs, fn_grad_s, x_dvv,
	  //  x_cv_ids, x_vars.all_continuous_variable_ids(),
	  //  nondInstance->primaryACVarMapIndices,
	  //  nondInstance->secondaryACVarMapTargets);
	}
	else // transform subset of components
	  nondInstance->natafTransform.trans_hess_X_to_U(fn_hess_x, fn_hess_us,
	    jacobian_xu, hessian_xu, fn_grad_x, x_dvv, x_cv_ids);
      }
      else // no transformation: d^2g/dx^2 = d^2G/du^2
	u_response.function_hessian(fn_hess_x, i);
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
  initialize_random_variable_transformation();
  initialize_random_variable_types(u_space_type);
  initialize_random_variable_parameters();
  initialize_random_variable_correlations();
  verify_correlation_support(u_space_type);
}


/** This function is commonly used to publish tranformation data when
    the Model variables are in a transformed space (e.g., u-space) and
    ProbabilityTransformation::ranVarTypes et al. may not be generated
    directly.  This allows for the use of inverse transformations to
    return the transformed space variables to their original states. */
void NonD::
initialize_random_variables(const Pecos::ProbabilityTransformation& transform,
			    bool deep_copy)
{
  if (deep_copy) {
    initialize_random_variable_transformation();
    natafTransform.copy(transform);
    // TO DO: deep copy of randomVarsX not yet implemented in
    // Pecos::ProbabilityTransformation::copy()
  }
  else
    natafTransform = transform; // shared rep

  // infer numCont{Des,Interval,State}Vars, but don't update continuous
  // aleatory uncertain counts (these may be u-space counts).
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  numContDesVars = numContIntervalVars = numContStateVars = 0;
  size_t i, num_v = x_ran_vars.size(); short x_type;
  for (i=0; i<num_v; ++i) {
    x_type = x_ran_vars[i].type();
    if      (x_type == Pecos::CONTINUOUS_DESIGN)   ++numContDesVars;
    else if (x_type == Pecos::CONTINUOUS_INTERVAL) ++numContIntervalVars;
    else if (x_type == Pecos::CONTINUOUS_STATE)    ++numContStateVars;
  }
  numContEpistUncVars = numContIntervalVars;
}


void NonD::initialize_random_variable_transformation()
{
  if (natafTransform.is_null())
    natafTransform = Pecos::ProbabilityTransformation("nataf"); // for now
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void NonD::initialize_random_variable_types(short u_space_type)
{
  // u_space_type is an enumeration for type of u-space transformation:
  // > if STD_NORMAL_U (reliability, AIS, and Wiener PCE/SC), then u-space is
  //   defined exclusively with std normals;
  // > if STD_UNIFORM_U (SC with local & global Hermite basis polynomials),
  //   then u-space is defined exclusively with std uniforms.
  // > if ASKEY_U (PCE/SC using Askey polynomials), then u-space is defined by
  //   std normals, std uniforms, std exponentials, std betas, and std gammas
  // > if EXTENDED_U (PCE/SC with Askey plus numerically-generated polynomials),
  //   then u-space involves at most linear scaling to std distributions.

  size_t i, av_cntr = 0, num_active_vars = iteratedModel.cv();
  ShortArray x_types(num_active_vars), u_types(num_active_vars);
  bool err_flag = false;

  for (i=0; i<numContDesVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_DESIGN;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }
  Pecos::AleatoryDistParams& adp
    = iteratedModel.aleatory_distribution_parameters();
  const RealVector& n_l_bnds = adp.normal_lower_bounds();
  const RealVector& n_u_bnds = adp.normal_upper_bounds();
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  for (i=0; i<numNormalVars; ++i, ++av_cntr) {
    if (n_l_bnds[i] > -dbl_inf || n_u_bnds[i] < dbl_inf) {
      x_types[av_cntr] = Pecos::BOUNDED_NORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U: case ASKEY_U:
	u_types[av_cntr] = Pecos::STD_NORMAL;     break;
      case STD_UNIFORM_U:
	u_types[av_cntr] = Pecos::STD_UNIFORM;    break;
      case EXTENDED_U:
	u_types[av_cntr] = Pecos::BOUNDED_NORMAL; break;
      }
    }
    else {
      x_types[av_cntr] = Pecos::NORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U: case ASKEY_U: case EXTENDED_U:
	u_types[av_cntr] = Pecos::STD_NORMAL;  break;
      case STD_UNIFORM_U:
	u_types[av_cntr] = Pecos::STD_UNIFORM; break;
      }
    }
  }
  const RealVector& ln_l_bnds = adp.lognormal_lower_bounds();
  const RealVector& ln_u_bnds = adp.lognormal_upper_bounds();
  for (i=0; i<numLognormalVars; ++i, ++av_cntr) {
    if (ln_l_bnds[i] > 0. || ln_u_bnds[i] < dbl_inf) {
      x_types[av_cntr] = Pecos::BOUNDED_LOGNORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U: case ASKEY_U:
	u_types[av_cntr] = Pecos::STD_NORMAL;        break;
      case STD_UNIFORM_U:
	u_types[av_cntr] = Pecos::STD_UNIFORM;       break;
      case EXTENDED_U:
	u_types[av_cntr] = Pecos::BOUNDED_LOGNORMAL; break;
      }
    }
    else {
      x_types[av_cntr] = Pecos::LOGNORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U: case ASKEY_U:
	u_types[av_cntr] = Pecos::STD_NORMAL;  break;
      case STD_UNIFORM_U:
	u_types[av_cntr] = Pecos::STD_UNIFORM; break;
      case EXTENDED_U:
	u_types[av_cntr] = Pecos::LOGNORMAL;   break;
      }
    }
  }
  for (i=0; i<numUniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::UNIFORM;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case ASKEY_U: case EXTENDED_U: case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    }
  }
  for (i=0; i<numLoguniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::LOGUNIFORM;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case ASKEY_U: case STD_UNIFORM_U:
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
    case ASKEY_U: case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::TRIANGULAR;  break;
    }
  }
  for (i=0; i<numExponentialVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::EXPONENTIAL;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;     break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_EXPONENTIAL; break;
    }
  }
  for (i=0; i<numBetaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::BETA;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_BETA;    break;
    }
  }
  for (i=0; i<numGammaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::GAMMA;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_GAMMA;   break;
    }
  }
  for (i=0; i<numGumbelVars; ++i, ++av_cntr) { // 2-sided distribution
    x_types[av_cntr] = Pecos::GUMBEL;
    switch (u_space_type) {
    case STD_NORMAL_U: case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;            break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::GUMBEL;                break;
    }
  }
  for (i=0; i<numFrechetVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::FRECHET;
    switch (u_space_type) {
    case STD_NORMAL_U: case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::FRECHET;     break;
    }
  }
  for (i=0; i<numWeibullVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::WEIBULL;
    switch (u_space_type) {
    case STD_NORMAL_U: case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;  break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM; break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::WEIBULL;     break;
    }
  }
  for (i=0; i<numHistogramBinVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::HISTOGRAM_BIN;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;    break;
    case ASKEY_U: case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;   break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::HISTOGRAM_BIN; break;
    }
  }

  /*
  // discrete int aleatory uncertain
  for (i=0; i<numPoissonVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::POISSON;
  for (i=0; i<numBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::BINOMIAL;
  for (i=0; i<numNegBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
  for (i=0; i<numGeometricVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::GEOMETRIC;
  for (i=0; i<numHyperGeomVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HYPERGEOMETRIC;
  for (i=0; i<numHistogramPtIntVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_INT;

  // discrete string aleatory uncertain
  for (i=0; i<numHistogramPtStringVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_STRING;

  // discrete real aleatory uncertain
  for (i=0; i<numHistogramPtRealVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_REAL;
  */

  // continuous epistemic uncertain
  for (i=0; i<numContIntervalVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_INTERVAL;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }

  // discrete int epistemic uncertain

  // discrete string epistemic uncertain

  // discrete real epistemic uncertain

  for (i=0; i<numContStateVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_STATE;
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
void NonD::initialize_random_variable_types()
{
  size_t i, av_cntr = 0, num_active_vars = iteratedModel.cv();
  ShortArray x_types(num_active_vars);
  Pecos::AleatoryDistParams& adp
    = iteratedModel.aleatory_distribution_parameters();
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  for (i=0; i<numContDesVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_DESIGN;

  const RealVector& n_l_bnds = adp.normal_lower_bounds();
  const RealVector& n_u_bnds = adp.normal_upper_bounds();
  for (i=0; i<numNormalVars; ++i, ++av_cntr)
    x_types[av_cntr] = (n_l_bnds[i] > -dbl_inf || n_u_bnds[i] < dbl_inf) ?
      Pecos::BOUNDED_NORMAL : Pecos::NORMAL;

  const RealVector& ln_l_bnds = adp.lognormal_lower_bounds();
  const RealVector& ln_u_bnds = adp.lognormal_upper_bounds();
  for (i=0; i<numLognormalVars; ++i, ++av_cntr)
    x_types[av_cntr] = (ln_l_bnds[i] > 0. || ln_u_bnds[i] < dbl_inf) ?
      Pecos::BOUNDED_LOGNORMAL : Pecos::LOGNORMAL;

  for (i=0; i<numUniformVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::UNIFORM;

  for (i=0; i<numLoguniformVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::LOGUNIFORM;

  for (i=0; i<numTriangularVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::TRIANGULAR;

  for (i=0; i<numExponentialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::EXPONENTIAL;

  for (i=0; i<numBetaVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::BETA;

  for (i=0; i<numGammaVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GAMMA;

  for (i=0; i<numGumbelVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GUMBEL;

  for (i=0; i<numFrechetVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::FRECHET;

  for (i=0; i<numWeibullVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::WEIBULL;

  for (i=0; i<numHistogramBinVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_BIN;

  /*
  // discrete int aleatory uncertain
  for (i=0; i<numPoissonVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::POISSON;
  for (i=0; i<numBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::BINOMIAL;
  for (i=0; i<numNegBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
  for (i=0; i<numGeometricVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GEOMETRIC;
  for (i=0; i<numHyperGeomVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HYPERGEOMETRIC;
  for (i=0; i<numHistogramPtIntVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_INT;

  // discrete string aleatory uncertain
  for (i=0; i<numHistogramPtStringVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_STRING;

  // discrete real aleatory uncertain
  for (i=0; i<numHistogramPtRealVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_REAL;
  */

  // continuous epistemic uncertain
  for (i=0; i<numContIntervalVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_INTERVAL;

  // discrete int epistemic uncertain

  // discrete string epistemic uncertain

  // discrete real epistemic uncertain

  for (i=0; i<numContStateVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_STATE;

  natafTransform.initialize_random_variable_types(x_types);
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void NonD::initialize_random_variable_parameters()
{
  // Be consistent with NonD active view logic.  Active counts are sufficient
  // for now, but could formalize with active view check as in NonD ctors.
  //short active_view = iteratedModel.current_variables().view().first;
  // Note: {Aleatory,Epistemic}DistParams handles always have reps, so
  // default constructed objects are safe to interrogate for empty arrays.
  const Pecos::AleatoryDistParams&  adp = (numAleatoryUncVars)  ?
    iteratedModel.aleatory_distribution_parameters()  :
    Pecos::AleatoryDistParams();
  const Pecos::EpistemicDistParams& edp = (numEpistemicUncVars) ?
    iteratedModel.epistemic_distribution_parameters() :
    Pecos::EpistemicDistParams();

  RealVector cd_l_bnds, cd_u_bnds, cs_l_bnds, cs_u_bnds;
  const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
  if (numContDesVars) {
    cd_l_bnds = RealVector(Teuchos::View, c_l_bnds.values(), numContDesVars);
    cd_u_bnds = RealVector(Teuchos::View, c_u_bnds.values(), numContDesVars);
  }
  if (numContStateVars) {
    size_t csv_start = numContinuousVars - numContStateVars;
    cs_l_bnds	= RealVector(Teuchos::View,
      const_cast<Real*>(&c_l_bnds[csv_start]), numContStateVars);
    cs_u_bnds = RealVector(Teuchos::View,
      const_cast<Real*>(&c_u_bnds[csv_start]), numContStateVars);
  }
  natafTransform.initialize_random_variable_parameters(cd_l_bnds, cd_u_bnds,
						       adp, edp,
						       cs_l_bnds, cs_u_bnds);
}


void NonD::initialize_random_variable_correlations()
{
  const RealSymMatrix& uncertain_corr
    = iteratedModel.aleatory_distribution_parameters().uncertain_correlations();
  if (!uncertain_corr.empty()) {
    natafTransform.initialize_random_variable_correlations(uncertain_corr);
    if (numContDesVars || numContEpistUncVars || numContStateVars)
      // expand ProbabilityTransformation::corrMatrixX to include design,
      // epistemic, state vars.  TO DO: propagate through model recursion?
      natafTransform.reshape_correlation_matrix(numContDesVars,
	numContAleatUncVars, numContEpistUncVars+numContStateVars);
  }
}


void NonD::verify_correlation_support(short u_space_type)
{
  if (natafTransform.x_correlation()) {
    const std::vector<Pecos::RandomVariable>& x_ran_vars
      = natafTransform.x_random_variables();
    const Pecos::ShortArray& u_types   = natafTransform.u_types();
    const Pecos::RealSymMatrix& x_corr = natafTransform.x_correlation_matrix();
    size_t i, j, num_cdv_cauv = numContDesVars+numContAleatUncVars;

    // We can only decorrelate in std normal space; therefore, if a variable
    // with a u_type other than STD_NORMAL is correlated with anything, revert
    // its u_type to STD_NORMAL.
    if (u_space_type != STD_NORMAL_U) {
      for (i=numContDesVars; i<num_cdv_cauv; ++i)
	if (u_types[i] != Pecos::STD_NORMAL)
	  // since we don't check all rows, check *all* columns despite symmetry
	  for (j=numContDesVars; j<num_cdv_cauv; ++j)
	    if (i != j && std::fabs(x_corr(i, j)) > Pecos::SMALL_NUMBER) {
	      Cerr << "\nWarning: u-space type for random variable "
		   << i-numContDesVars+1 << " changed to\n         "
		   << "STD_NORMAL due to decorrelation requirements.\n";
	      natafTransform.u_type(Pecos::STD_NORMAL, i);
	      break; // out of inner loop
	    }
    }

    // Check for correlations among variable types (bounded normal, bounded
    // lognormal, loguniform, triangular, beta, and histogram) that are not
    // supported by Der Kiureghian & Liu for correlation warping estimation
    // when transforming to std normals.
    bool err_flag = false;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      bool distribution_error = false;
      short x_type = x_ran_vars[i].type();
      if ( x_type == Pecos::BOUNDED_NORMAL    || x_type == Pecos::LOGUNIFORM ||
	   x_type == Pecos::BOUNDED_LOGNORMAL || x_type == Pecos::TRIANGULAR ||
	   x_type == Pecos::BETA || x_type == Pecos::HISTOGRAM_BIN )
	// since we don't check all rows, check *all* columns despite symmetry
	for (j=numContDesVars; j<num_cdv_cauv; ++j)
	  if (i != j && std::fabs(x_corr(i, j)) > Pecos::SMALL_NUMBER)
	    { distribution_error = true; break; }
      if (distribution_error) {
	Cerr << "Error: correlation warping for Nataf variable transformation "
	     << "of bounded normal,\n       bounded lognormal, loguniform, "
	     << "triangular, beta, and histogram bin\n       distributions is "
	     << "not currently supported.  Error detected for variable " << i+1
	     << "." << std::endl;
	err_flag = true;
      }
    }
    if (err_flag)
      abort_handler(-1);
  }
}


void NonD::initialize_response_covariance()
{ } // default is no-op


/** Default definition of virtual function (used by sampling, reliability,
    and stochastic expansion methods) defines the set of statistical
    results to include means, standard deviations, and level mappings. */
void NonD::initialize_final_statistics()
{
  size_t i, j, num_levels, cntr = 0, rl_len = 0,
    num_final_stats = 2*numFunctions, num_active_vars = iteratedModel.cv();
  if (!epistemicStats) { // aleatory UQ
    num_final_stats += totalLevelRequests;
    if (respLevelTargetReduce) {
      rl_len = requestedRespLevels[0].length();
      for (i=1; i<numFunctions; ++i)
	if (requestedRespLevels[i].length() != rl_len) {
	  Cerr << "Error: system metric aggregation from component metrics "
	       << "requires\n       consistency in length of response_levels "
	       << "among response function set." << std::endl;
	  abort_handler(-1);
	}
      num_final_stats += rl_len; // 1 aggregated system metric per resp level
    }
  }
  // default response ASV/DVV may be overridden by NestedModel update
  // in subIterator.response_results_active_set(sub_iterator_set)
  ActiveSet stats_set(num_final_stats);//, num_active_vars);
  stats_set.derivative_vector(iteratedModel.inactive_continuous_variable_ids());
  finalStatistics = Response(SIMULATION_RESPONSE, stats_set);

  // Assign meaningful labels to finalStatistics (appear in NestedModel output)
  char resp_tag[10];
  StringArray stats_labels(num_final_stats);
  if (epistemicStats) { // epistemic & mixed aleatory/epistemic
    for (i=0; i<numFunctions; ++i) {
      std::sprintf(resp_tag, "_r%i", i+1);
      stats_labels[cntr++] = String("z_lo") + String(resp_tag);
      stats_labels[cntr++] = String("z_up") + String(resp_tag);
    }
  }
  else {                     // aleatory
    char lev_tag[10];
    String dist_tag = (cdfFlag) ? String("cdf") : String("ccdf");
    for (i=0; i<numFunctions; ++i) {
      std::sprintf(resp_tag, "_r%i", i+1);
      stats_labels[cntr++] = String("mean")    + String(resp_tag);
      stats_labels[cntr++] = String("std_dev") + String(resp_tag);
      num_levels = requestedRespLevels[i].length();
      for (j=0; j<num_levels; ++j, ++cntr) {
	switch (respLevelTarget) {
	case PROBABILITIES:     std::sprintf(lev_tag, "_plev%i",  j+1); break;
	case RELIABILITIES:     std::sprintf(lev_tag, "_blev%i",  j+1); break;
	case GEN_RELIABILITIES: std::sprintf(lev_tag, "_b*lev%i", j+1); break;
	}
	stats_labels[cntr] = dist_tag + String(lev_tag) + String(resp_tag);
      }
      num_levels = requestedProbLevels[i].length() +
	requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
      for (j=0; j<num_levels; ++j, ++cntr) {
	std::sprintf(lev_tag, "_zlev%i", j+1);
	stats_labels[cntr] = dist_tag + String(lev_tag) + String(resp_tag);
      }
    }
    if (respLevelTargetReduce) {
      String sys_tag("_sys");
      for (j=0; j<rl_len; ++j, ++cntr) {
	switch (respLevelTarget) {
	case PROBABILITIES:     std::sprintf(lev_tag, "_plev%i",  j+1); break;
	case RELIABILITIES:     std::sprintf(lev_tag, "_blev%i",  j+1); break;
	case GEN_RELIABILITIES: std::sprintf(lev_tag, "_b*lev%i", j+1); break;
	}
	stats_labels[cntr] = dist_tag + String(lev_tag) + sys_tag;
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


void NonD::update_final_statistics()
{
  //if (finalStatistics.is_null())
  //  initialize_final_statistics();

  // this default implementation gets overridden/augmented in derived classes
  update_aleatory_final_statistics();
  update_system_final_statistics();
  update_system_final_statistics_gradients();
}


void NonD::update_aleatory_final_statistics()
{
  // update finalStatistics from computed{Resp,Prob,Rel,GenRel}Levels
  size_t i, j, cntr = 0, rl_len, pl_bl_gl_len;
  for (i=0; i<numFunctions; ++i) {
    // final stats from compute_moments()
    if (!momentStats.empty()) {
      finalStatistics.function_value(momentStats(0,i), cntr++); // mean
      finalStatistics.function_value(momentStats(1,i), cntr++); // std dev
    }
    else
      cntr += 2;
    // final stats from compute_distribution_mappings()
    rl_len = requestedRespLevels[i].length();
    switch (respLevelTarget) { // individual component p/beta/beta*
    case PROBABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedProbLevels[i][j], cntr);
      break;
    case RELIABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedRelLevels[i][j], cntr);
      break;
    case GEN_RELIABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedGenRelLevels[i][j], cntr);
      break;
    }
    pl_bl_gl_len = requestedProbLevels[i].length()
      + requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
    for (j=0; j<pl_bl_gl_len; ++j, ++cntr)
      finalStatistics.function_value(computedRespLevels[i][j], cntr);
  }
}


void NonD::update_system_final_statistics()
{
  if (respLevelTargetReduce) {
    // same rl_len enforced for all resp fns in initialize_final_statistics()
    size_t i, j, rl_len = requestedRespLevels[0].length(),
      cntr = 2*numFunctions + totalLevelRequests;
    for (j=0; j<rl_len; ++j, ++cntr) {
      // compute system probability
      Real system_p = 1.;
      switch (respLevelTargetReduce) {
      case SYSTEM_SERIES: // system p_success = product of component p_success
	for (i=0; i<numFunctions; ++i)
	  system_p *= (1.-computedProbLevels[i][j]);
	system_p = 1. - system_p; // convert back to p_fail
	break;
      case SYSTEM_PARALLEL: // system p_fail = product of component p_fail
	for (i=0; i<numFunctions; ++i)
	  system_p *= computedProbLevels[i][j];
	break;
      }
#ifdef DEBUG
      Cout << "\nSystem p = " << system_p << " from component p:\n";
      for (i=0; i<numFunctions; ++i)
	Cout << "  " << computedProbLevels[i][j] << '\n';
      Cout << '\n';
#endif // DEBUG
      // convert system probability to desired system metric
      switch (respLevelTarget) {
      case PROBABILITIES:
	finalStatistics.function_value(system_p, cntr); break;
      case RELIABILITIES: case GEN_RELIABILITIES:
	finalStatistics.function_value(
	  -Pecos::NormalRandomVariable::inverse_std_cdf(system_p), cntr);
	break;
      }
    }
  }
}


void NonD::update_system_final_statistics_gradients()
{
  if (respLevelTargetReduce) {
    const ShortArray& final_asv = finalStatistics.active_set_request_vector();
    const SizetArray& final_dvv
      = finalStatistics.active_set_derivative_vector();
    // same rl_len enforced for all resp fns in initialize_final_statistics()
    size_t l, v, s, p, rl_len = requestedRespLevels[0].length(),
      num_deriv_vars = final_dvv.size(),
      cntr = 2*numFunctions + totalLevelRequests;
    RealVector final_stat_grad(num_deriv_vars, false);
    RealVectorArray component_grad(numFunctions);
    Real prod;
    for (l=0; l<rl_len; ++l, ++cntr) {
      if (final_asv[cntr] & 2) {
	// Retrieve component probability gradients from finalStatistics
	size_t index = 0;
	for (s=0; s<numFunctions; ++s) {
	  index += 2;
	  if (respLevelTarget == PROBABILITIES)
	    component_grad[s] = finalStatistics.function_gradient_view(index+l);
	  else {
	    component_grad[s] = finalStatistics.function_gradient_copy(index+l);
	    Real component_beta = finalStatistics.function_value(index+l);
	    component_grad[s].scale(
	      -Pecos::NormalRandomVariable::std_pdf(-component_beta));
	  }
	  index += rl_len + requestedProbLevels[s].length() +
	    requestedRelLevels[s].length() + requestedGenRelLevels[s].length();
#ifdef DEBUG
	  Cout << "component gradient " << s+1 << ":\n";
	  write_data(Cout, component_grad[s]); Cout << '\n';
#endif // DEBUG
	}
	// Compute system probability
	for (v=0; v<num_deriv_vars; ++v) {
	  // apply product rule over n factors
	  Real& sum = final_stat_grad[v]; sum = 0.;
	  for (s=0; s<numFunctions; ++s) {
	    prod = 1.;
	    switch (respLevelTargetReduce) {
	    case SYSTEM_SERIES:// system p_success = prod of component p_success
	      for (p=0; p<numFunctions; ++p)
		prod *= (p == s) ? -component_grad[p][v] :
		  1.-computedProbLevels[p][l];
	      break;
	    case SYSTEM_PARALLEL: // system p_fail = product of component p_fail
	      for (p=0; p<numFunctions; ++p)
		prod *= (p == s) ? component_grad[p][v] :
		  computedProbLevels[p][l];
	      break;
	    }
	    sum += prod;
	  }
	}
	Real factor = 1.; bool scale = false;
	// negate gradient if converting system p_success to system p_fail
	if (respLevelTargetReduce == SYSTEM_SERIES)
	  { factor *= -1.; scale = true; }
	// define any scaling for system metric type
	if (respLevelTarget != PROBABILITIES) {
	  Real sys_beta = finalStatistics.function_value(cntr);
	  factor /= -Pecos::NormalRandomVariable::std_pdf(-sys_beta);
	  scale = true;
	}
	if (scale) final_stat_grad.scale(factor);
	finalStatistics.function_gradient(final_stat_grad, cntr);
      }
    }
  }
}


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
      switch (respLevelTarget) {
      case PROBABILITIES:
	computedProbLevels[i].resize(requestedRespLevels[i].length());   break;
      case RELIABILITIES:
	computedRelLevels[i].resize(requestedRespLevels[i].length());    break;
      case GEN_RELIABILITIES:
	computedGenRelLevels[i].resize(requestedRespLevels[i].length()); break;
      }
      computedRespLevels[i].resize(requestedProbLevels[i].length() +
				   requestedRelLevels[i].length()  +
				   requestedGenRelLevels[i].length());
    }
  }
}

/// Print distribution mappings, including to file per response
void NonD::print_distribution_mappings(std::ostream& s) const
{
  // output CDF/CCDF probabilities resulting from binning or CDF/CCDF
  // reliabilities resulting from number of std devs separating mean & target
  s << std::scientific << std::setprecision(write_precision)
    << "\nLevel mappings for each response function:\n";
  size_t i;
  for (i=0; i<numFunctions; ++i)
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty()) {
      print_distribution_map(i, s);
      // optionally write the distribution mapping to a .dist file
      if (outputLevel >= VERBOSE_OUTPUT)
	distribution_mappings_file(i);
    }
}

/// Write distribution mappings to a file for a single response
void NonD::distribution_mappings_file(size_t fn_index) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();
  std::string dist_filename(resp_labels[fn_index]);
  dist_filename += ".dist";
  std::ofstream dist_file;
  TabularIO::open_file(dist_file, dist_filename, "Distribution Map Output"); 
  dist_file << std::scientific << std::setprecision(write_precision);
  print_distribution_map(fn_index, dist_file);
}


/** Print the distribution mapping for a single response function to
    the passed output stream */
void NonD::print_distribution_map(size_t fn_index, std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  size_t i = fn_index, j;
  size_t width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;

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


void NonD::print_system_mappings(std::ostream& s) const
{
  size_t rl_len = requestedRespLevels[0].length();
  if (respLevelTargetReduce && rl_len) {
    size_t i, width = write_precision+7, g_width = 2*width+2,
      cntr = 2*numFunctions + totalLevelRequests;
    const RealVector& final_stats = finalStatistics.function_values();
    s << std::scientific << std::setprecision(write_precision)
      << "\nSystem response level mappings:\n";
    if (cdfFlag) s << "Cumulative distribution metrics ";
    else         s << "Complementary cumulative distribution metrics ";
    if (respLevelTargetReduce == SYSTEM_SERIES)        s << "for series ";
    else if (respLevelTargetReduce == SYSTEM_PARALLEL) s << "for parallel ";
    s << "system:\n     Resp Level Set  Probability Level  Reliability Index  "
      << "General Rel Index\n     --------------  -----------------  "
      << "-----------------  -----------------\n";
    Real prob, gen_rel;
    for (i=0; i<rl_len; ++i, ++cntr) {
      switch (respLevelTarget) {
      case PROBABILITIES:
	prob    =  final_stats[cntr];
	gen_rel = -Pecos::NormalRandomVariable::inverse_std_cdf(prob); break;
      default:
	gen_rel =  final_stats[cntr];
	prob    =  Pecos::NormalRandomVariable::std_cdf(-gen_rel);     break;
      }
      s << "  " << std::setw(width) << i+1 << "  " << std::setw(width) << prob
	<< "  " << std::setw(g_width) << gen_rel << '\n';
    }
  }
}

void NonD::archive_allocate_mappings()
{
  if (!resultsDB.active())  return;

  bool req_resp = false, req_prob = false, req_rel = false, req_gen = false;
  for (size_t i=0; i<numFunctions; ++i) {
    if (requestedRespLevels[i].length() > 0)   req_resp = true;
    if (requestedProbLevels[i].length() > 0)   req_prob = true;
    if (requestedRelLevels[i].length() > 0)    req_rel  = true;
    if (requestedGenRelLevels[i].length() > 0) req_gen  = true;
  }

  if (req_resp) {
    std::string resp_target, data_name;
    switch (respLevelTarget) { 
    case PROBABILITIES:
      resp_target = "Probability";
      data_name = resultsNames.map_resp_prob;
      break;
    case RELIABILITIES:
      resp_target = "Reliability";
      data_name = resultsNames.map_resp_rel;
      break;
    case GEN_RELIABILITIES: 
      resp_target = "Generalized Reliability";
      data_name = resultsNames.map_resp_genrel;
      break;
    }

    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Response Level", resp_target + " Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), data_name, numFunctions, md);
  }
  if (req_prob) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Probability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_prob_resp, numFunctions, md);
  }
  if (req_rel) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Reliability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_rel_resp, numFunctions, md);
  }
  if (req_gen) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Generalized Reliability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_genrel_resp, numFunctions, md);
  }
}

// archive the mappings from response levels
void NonD::archive_from_resp(size_t i)
{
  // only insert if active and response levels specified
  size_t num_resp_levels = requestedRespLevels[i].length(); 
  if (!resultsDB.active() || num_resp_levels == 0)  return;

  size_t j;
  std::string data_name;

  RealMatrix mapping(num_resp_levels, 2);
  
  // TODO: could use SetCol?
  switch (respLevelTarget) { 
  case PROBABILITIES:
    data_name = resultsNames.map_resp_prob;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedProbLevels[i][j];
    }
    break;
  case RELIABILITIES:
    data_name = resultsNames.map_resp_rel;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedRelLevels[i][j];
    }
    break;
  case GEN_RELIABILITIES: 
    data_name = resultsNames.map_resp_genrel;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedGenRelLevels[i][j];
    }
    break;
  }

  resultsDB.array_insert<RealMatrix>(run_identifier(), data_name, i, mapping);
}


// archive the mappings to response levels
void NonD::archive_to_resp(size_t i)
{
  if (!resultsDB.active())  return;

  size_t j;
  size_t num_prob_levels = requestedProbLevels[i].length();
  if (num_prob_levels > 0) {
    RealMatrix mapping(num_prob_levels, 2);
    for (j=0; j<num_prob_levels; j++) {
      mapping(j, 0) = requestedProbLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_prob_resp, i, mapping);
  } 
  size_t num_rel_levels = requestedRelLevels[i].length();
  size_t offset = num_prob_levels; 
  if (num_rel_levels > 0) {
    RealMatrix mapping(num_rel_levels, 2);
    for (j=0; j<num_rel_levels; j++) {
      mapping(j, 0) = requestedRelLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j+offset];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_rel_resp, i, mapping);
  } 
  size_t num_gen_rel_levels = requestedGenRelLevels[i].length();
  offset += num_rel_levels; 
  if (num_gen_rel_levels > 0) {
    RealMatrix mapping(num_gen_rel_levels, 2);
    for (j=0; j<num_gen_rel_levels; j++) {
      mapping(j, 0) = requestedGenRelLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j+offset];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_genrel_resp, i, mapping);
  } 
}


} // namespace Dakota
