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
#include "ParallelLibrary.hpp"

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
  pdfOutput(false),
  finalMomentsType(probDescDB.get_short("method.nond.final_moments")),
  distParamDerivs(false)
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
  pdfOutput(false), finalMomentsType(STANDARD_MOMENTS), distParamDerivs(false)
{
  // NonDEvidence and NonDAdaptImpSampling use this ctor

  size();

  // current set of statistics is mean, standard deviation, and 
  // probability of failure for each response function 
  //ShortArray asv(3*numFunctions, 1); 
  //finalStatistics = Response(numUncertainVars, asv);
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
  finalMomentsType(STANDARD_MOMENTS), distParamDerivs(false)
{
  // ConcurrentStrategy uses this ctor for design opt, either for multi-start
  // initial points or multibjective weight sets.

  numContinuousVars  = numUniformVars;
  numDiscreteIntVars = numDiscreteStringVars = numDiscreteRealVars = 0;
}


void NonD::size()
{
  bool err_flag = false;
  const Variables& vars = iteratedModel.current_variables();
  short active_view = vars.view().first;
  const SharedVariablesData& svd = vars.shared_data();

  // update sizes for aleatory uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_ALEATORY_UNCERTAIN) { // aleatory or both
    const Pecos::AleatoryDistParams& adp
      = iteratedModel.aleatory_distribution_parameters();
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

  // update sizes for epistemic uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_EPISTEMIC_UNCERTAIN) { // epistemic or both
    const Pecos::EpistemicDistParams& edp
      = iteratedModel.epistemic_distribution_parameters();
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

  // update total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // update sizes for design variables (if active)
  if (active_view == RELAXED_ALL    || active_view == MIXED_ALL ||
      active_view == RELAXED_DESIGN || active_view == MIXED_DESIGN) {
    svd.design_counts(numContDesVars, numDiscIntDesVars, numDiscStringDesVars,
		      numDiscRealDesVars);
    numDesignVars = numContDesVars       + numDiscIntDesVars
                  + numDiscStringDesVars + numDiscRealDesVars;
  }

  // update sizes for state variables (if active)
  if (active_view == RELAXED_ALL   || active_view == MIXED_ALL ||
      active_view == RELAXED_STATE || active_view == MIXED_STATE) {
    svd.state_counts(numContStateVars, numDiscIntStateVars,
		     numDiscStringStateVars, numDiscRealStateVars);
    numStateVars = numContStateVars       + numDiscIntStateVars
                 + numDiscStringStateVars + numDiscRealStateVars;
  }

  if ( !numUncertainVars && !numDesignVars && !numStateVars ) {
    Cerr << "\nError: number of active variables must be nonzero in Dakota::"
	 << "NonD::resize()." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
      numDiscreteRealVars != numDesignVars + numUncertainVars + numStateVars) {
    Cout << "\nError: inconsistent active variable counts ("
	 << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
            numDiscreteRealVars << ", " << numDesignVars + numUncertainVars +
            numStateVars << ") in Dakota::NonD::resize()." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}


bool NonD::resize()
{
  bool parent_reinit_comms = Analyzer::resize();

  bool err_flag = false;
  const Variables& vars = iteratedModel.current_variables();
  short active_view = vars.view().first;
  const SharedVariablesData& svd = vars.shared_data();

  // update sizes for aleatory uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_ALEATORY_UNCERTAIN) { // aleatory or both
    const Pecos::AleatoryDistParams& adp
      = iteratedModel.aleatory_distribution_parameters();
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

  // update sizes for epistemic uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_EPISTEMIC_UNCERTAIN) { // epistemic or both
    const Pecos::EpistemicDistParams& edp
      = iteratedModel.epistemic_distribution_parameters();
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

  // update total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // update sizes for design variables (if active)
  if (active_view == RELAXED_ALL    || active_view == MIXED_ALL ||
      active_view == RELAXED_DESIGN || active_view == MIXED_DESIGN) {
    svd.design_counts(numContDesVars, numDiscIntDesVars, numDiscStringDesVars,
		      numDiscRealDesVars);
    numDesignVars = numContDesVars       + numDiscIntDesVars
                  + numDiscStringDesVars + numDiscRealDesVars;
  }

  // update sizes for state variables (if active)
  if (active_view == RELAXED_ALL   || active_view == MIXED_ALL ||
      active_view == RELAXED_STATE || active_view == MIXED_STATE) {
    svd.state_counts(numContStateVars, numDiscIntStateVars,
		     numDiscStringStateVars, numDiscRealStateVars);
    numStateVars = numContStateVars       + numDiscIntStateVars
                 + numDiscStringStateVars + numDiscRealStateVars;
  }

  if ( !numUncertainVars && !numDesignVars && !numStateVars ) {
    Cerr << "\nError: number of active variables must be nonzero in Dakota::"
	 << "NonD::resize()." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
      numDiscreteRealVars != numDesignVars + numUncertainVars + numStateVars) {
    Cout << "\nError: inconsistent active variable counts ("
	 << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
            numDiscreteRealVars << ", " << numDesignVars + numUncertainVars +
            numStateVars << ") in Dakota::NonD::resize()." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  return parent_reinit_comms;
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
		 short resp_lev_tgt, short resp_lev_tgt_reduce,
		 bool cdf_flag, bool pdf_output)
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

  if (totalLevelRequests && pdf_output)
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
transform_model(Model& x_model, Model& u_model,
		bool truncated_bounds, Real bound)
{
  ///////////////////////
  // Perform recasting //
  ///////////////////////

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

  /////////////////////////////////////////////////////////////////
  // Populate aleatory distribution params for transformed space //
  /////////////////////////////////////////////////////////////////

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
    size_t num_total_nuv = num_u_nuv + num_u_bnuv;
    // u_adp was lightweight constructed; must size before entry assign
    u_adp.nuv(num_total_nuv);
    if (num_u_bnuv) {
      size_t u_n_cntr = 0, x_n_cntr = 0;
      for (i=numContDesVars; i<num_cdv_cauv; ++i) {
	x_type = x_ran_vars[i].type(); u_type = u_types[i];
	if (u_type == Pecos::BOUNDED_NORMAL) {
	  u_adp.normal_mean(x_adp.normal_mean(x_n_cntr), u_n_cntr);
	  u_adp.normal_std_deviation(
	    x_adp.normal_std_deviation(x_n_cntr), u_n_cntr);
	  u_adp.normal_lower_bound(x_adp.normal_lower_bound(x_n_cntr),u_n_cntr);
	  u_adp.normal_upper_bound(x_adp.normal_upper_bound(x_n_cntr),u_n_cntr);
	  ++u_n_cntr;
	}
	else if (u_type == Pecos::STD_NORMAL) {
	  u_adp.normal_mean(0., u_n_cntr);
	  u_adp.normal_std_deviation(1., u_n_cntr);
	  u_adp.normal_lower_bound(-dbl_inf, u_n_cntr);
	  u_adp.normal_upper_bound( dbl_inf, u_n_cntr);
	  ++u_n_cntr;
	}
	if (x_type == Pecos::NORMAL || x_type == Pecos::BOUNDED_NORMAL)
	  ++x_n_cntr;
      }
    }
    else {
      for (i=0; i<num_total_nuv; ++i) {
	u_adp.normal_mean(0., i);
	u_adp.normal_std_deviation(1., i);
	u_adp.normal_lower_bound(-dbl_inf, i);
	u_adp.normal_upper_bound( dbl_inf, i);
      }
    }
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
    u_adp.uuv(num_u_uuv); // size before entry assign
    for (i=0; i<num_u_uuv; ++i)
      { u_adp.uniform_lower_bound(-1., i); u_adp.uniform_upper_bound(1., i); }
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
    u_adp.euv(num_u_euv); // size before entry assign
    for (i=0; i<num_u_euv; ++i)
      u_adp.exponential_beta(1., i);
  }
  if (num_u_buv) {
    u_adp.buv(num_u_buv); // size before entry assign
    for (i=0; i<num_u_buv; ++i) {
      u_adp.beta_alpha(x_adp.beta_alpha(i), i);
      u_adp.beta_beta(x_adp.beta_beta(i), i);
      u_adp.beta_lower_bound(-1., i);
      u_adp.beta_upper_bound( 1., i);
    }
  }
  if (num_u_gauv) {
    u_adp.gauv(num_u_gauv); // size before entry assign
    for (i=0; i<num_u_gauv; ++i) {
      u_adp.gamma_alpha(x_adp.gamma_alpha(i), i);
      u_adp.gamma_beta(1., i);
    }
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

  //////////////////////////////////////////////////////////////////
  // Populate epistemic distribution params for transformed space //
  //////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////
  // Populate model bounds for transformed space //
  /////////////////////////////////////////////////

  // [-1,1] are standard bounds for design, state, epistemic, uniform, & beta
  RealVector c_l_bnds(numContinuousVars, false); c_l_bnds = -1.;
  RealVector c_u_bnds(numContinuousVars, false); c_u_bnds =  1.;
  if (truncated_bounds) {
    // truncate unbounded distributions for approaches requiring bounds:
    //   standard sampling modes: model bounds only used for design/state
    //   *_UNIFORM modes: model bounds are used for all active variables
    size_t nuv_cntr = 0, lnuv_cntr = 0, gauv_cntr = 0, guuv_cntr = 0,
      fuv_cntr = 0, wuv_cntr = 0;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:      // mean +/- bound std devs
	c_l_bnds[i] = -bound; c_u_bnds[i] =    bound; break;
      case Pecos::STD_EXPONENTIAL: // [0, mean + bound std devs] for beta=1
	c_l_bnds[i] =     0.; c_u_bnds[i] = 1.+bound; break;
      case Pecos::STD_GAMMA: {
	Real mean, stdev;
	Pecos::GammaRandomVariable::
	  moments_from_params(x_adp.gamma_alpha(gauv_cntr), 1., mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound * stdev; break;
      }
      case Pecos::BOUNDED_NORMAL: {
	// Note: as for NIDR initialization, we use the gauss{Mean,StdDev}
	// parameters rather than computing the actual mean,std_dev of the 
	// bounded distribution
	Real l_bnd = x_adp.normal_lower_bound(nuv_cntr),
             u_bnd = x_adp.normal_upper_bound(nuv_cntr);
	c_l_bnds[i] = (l_bnd > -dbl_inf) ? l_bnd : // use specified bound
	  x_adp.normal_mean(nuv_cntr)              // infer bound
	  - bound * x_adp.normal_std_deviation(nuv_cntr);
	c_u_bnds[i] = (u_bnd <  dbl_inf) ? u_bnd : // use specified bound
	  x_adp.normal_mean(nuv_cntr)              // infer bound
	  + bound * x_adp.normal_std_deviation(nuv_cntr);
	break;
      }
      case Pecos::BOUNDED_LOGNORMAL: {
	c_l_bnds[i] = x_adp.lognormal_lower_bound(lnuv_cntr); // specified or 0
	Real u_bnd  = x_adp.lognormal_upper_bound(lnuv_cntr);
	if (u_bnd < dbl_inf)
	  c_u_bnds[i] = u_bnd; // use specified bound
	else {                 // infer bound
	  // Note: as for NIDR initialization, we use the mean,std_dev
	  // parameters rather than computing the actual mean,std_dev of the 
	  // bounded distribution
	  Real mean, stdev;
	  Pecos::moments_from_lognormal_spec(x_adp.lognormal_means(),
	    x_adp.lognormal_std_deviations(), x_adp.lognormal_lambdas(),
	    x_adp.lognormal_zetas(), x_adp.lognormal_error_factors(),
	    lnuv_cntr, mean, stdev);
	  c_u_bnds[i] = mean + bound * stdev;
	}
	break;
      }
      case Pecos::LOGUNIFORM:     case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:
	// bounded distributions: x-space has desired bounds
	c_l_bnds[i] = x_model.continuous_lower_bound(i);
	c_u_bnds[i] = x_model.continuous_upper_bound(i); break;
      // Note: Could use x_model bounds for the following cases as well except
      // that NIDR uses +/-3 sigma, whereas here we're using +/-10 sigma
      case Pecos::LOGNORMAL: { // semi-bounded distribution
	Real mean, stdev;
	Pecos::moments_from_lognormal_spec(x_adp.lognormal_means(),
	  x_adp.lognormal_std_deviations(), x_adp.lognormal_lambdas(),
	  x_adp.lognormal_zetas(), x_adp.lognormal_error_factors(),
	  lnuv_cntr, mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound * stdev; break;
      }
      case Pecos::GUMBEL: { // unbounded distribution
	Real mean, stdev;
	Pecos::GumbelRandomVariable::
	  moments_from_params(x_adp.gumbel_alpha(guuv_cntr),
			      x_adp.gumbel_beta(guuv_cntr), mean, stdev);
	c_l_bnds[i] = mean - bound * stdev; c_u_bnds[i] = mean + bound * stdev;
	break;
      }
      case Pecos::FRECHET: { // semibounded distribution
	Real mean, stdev;
	Pecos::FrechetRandomVariable::
	  moments_from_params(x_adp.frechet_alpha(fuv_cntr),
			      x_adp.frechet_beta(fuv_cntr), mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound * stdev; break;
      }
      case Pecos::WEIBULL: { // semibounded distribution
	Real mean, stdev;
	Pecos::WeibullRandomVariable::
	  moments_from_params(x_adp.weibull_alpha(wuv_cntr),
			      x_adp.weibull_beta(wuv_cntr), mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + bound * stdev; break;
      }
      }
      switch (x_ran_vars[i].type()) {
      case Pecos::NORMAL:    case Pecos::BOUNDED_NORMAL:     ++nuv_cntr; break;
      case Pecos::LOGNORMAL: case Pecos::BOUNDED_LOGNORMAL: ++lnuv_cntr; break;
      case Pecos::GAMMA:                                    ++gauv_cntr; break;
      case Pecos::GUMBEL:                                   ++guuv_cntr; break;
      case Pecos::FRECHET:                                   ++fuv_cntr; break;
      case Pecos::WEIBULL:                                   ++wuv_cntr; break;
      }
    }
  }
  else { // retain infinite model bounds where distributions are unbounded
    size_t nuv_cntr = 0, lnuv_cntr = 0;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL: case Pecos::GUMBEL: // unbounded distributions
	c_l_bnds[i] = -dbl_inf; c_u_bnds[i] = dbl_inf; break;
      case Pecos::LOGNORMAL:  case Pecos::STD_EXPONENTIAL:
      case Pecos::STD_GAMMA:  case Pecos::FRECHET:
      case Pecos::WEIBULL:                      // semibounded distributions
	c_l_bnds[i] = 0.; c_u_bnds[i] = dbl_inf; break;
      case Pecos::BOUNDED_NORMAL:
	// can't rely on x_model bounds since could be 1-sided
	c_l_bnds[i] = x_adp.normal_lower_bound(nuv_cntr);
	c_u_bnds[i] = x_adp.normal_upper_bound(nuv_cntr); break;
      case Pecos::BOUNDED_LOGNORMAL:
	// can't rely on x_model bounds since could be 1-sided
	c_l_bnds[i] = x_adp.lognormal_lower_bound(lnuv_cntr);
	c_u_bnds[i] = x_adp.lognormal_upper_bound(lnuv_cntr); break;
      case Pecos::LOGUNIFORM: case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:                    // bounded distributions
	// 2-sided: can rely on x_model bounds
	c_l_bnds[i] = x_model.continuous_lower_bound(i);
	c_u_bnds[i] = x_model.continuous_upper_bound(i); break;
      }
      switch (x_ran_vars[i].type()) {
      case Pecos::NORMAL:    case Pecos::BOUNDED_NORMAL:     ++nuv_cntr; break;
      case Pecos::LOGNORMAL: case Pecos::BOUNDED_LOGNORMAL: ++lnuv_cntr; break;
      }
    }
  }
  u_model.continuous_lower_bounds(c_l_bnds);
  u_model.continuous_upper_bounds(c_u_bnds);
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

  size_t i, av_cntr = 0, num_active_vars = iteratedModel.cv() + 
    iteratedModel.div() + iteratedModel.dsv() + iteratedModel.drv();
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
  
  // discrete int aleatory uncertain
  for (i=0; i<numPoissonVars; ++i, ++av_cntr){
    x_types[av_cntr] = Pecos::POISSON;
    switch (u_space_type) {
    case STD_NORMAL_U: case STD_UNIFORM_U:
      err_flag = true; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::POISSON; break;
    }
  }
  for (i=0; i<numBinomialVars; ++i, ++av_cntr){
    x_types[av_cntr] = Pecos::BINOMIAL;
    switch (u_space_type) {
    case STD_NORMAL_U: case STD_UNIFORM_U:
      err_flag = true; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::BINOMIAL; break;
    }
  }
  for (i=0; i<numNegBinomialVars; ++i, ++av_cntr){
    x_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
    switch (u_space_type) {
    case STD_NORMAL_U: case STD_UNIFORM_U:
      err_flag = true; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL; break;
    }
  }
  for (i=0; i<numGeometricVars; ++i, ++av_cntr){
    x_types[av_cntr] = u_types[av_cntr] = Pecos::GEOMETRIC;
    switch (u_space_type) {
    case STD_NORMAL_U: case STD_UNIFORM_U:
      err_flag = true; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::GEOMETRIC; break;
    }
  }
  for (i=0; i<numHyperGeomVars; ++i, ++av_cntr){
    x_types[av_cntr] = Pecos::HYPERGEOMETRIC;
    switch (u_space_type) {
    case STD_NORMAL_U: case STD_UNIFORM_U:
      err_flag = true; break;
    case ASKEY_U: case EXTENDED_U:
      u_types[av_cntr] = Pecos::HYPERGEOMETRIC; break;
    }
  }
  for (i=0; i<numHistogramPtIntVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_INT;
  
  // discrete string aleatory uncertain
  for (i=0; i<numHistogramPtStringVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_STRING;

  // discrete real aleatory uncertain
  for (i=0; i<numHistogramPtRealVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_REAL;

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
  /*
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
  size_t i, j, num_levels, cntr = 0, rl_len = 0, num_final_stats,
    num_active_vars = iteratedModel.cv();
  if (epistemicStats)
    num_final_stats = 2*numFunctions;
  else { // aleatory UQ
    num_final_stats  = (finalMomentsType) ? 2*numFunctions : 0;
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
      if (finalMomentsType) {
	stats_labels[cntr++] = String("mean") + String(resp_tag);
	stats_labels[cntr++] = (finalMomentsType == CENTRAL_MOMENTS) ?
	  String("variance") + String(resp_tag) :
	  String("std_dev")  + String(resp_tag);
      }
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
  if (finalStatistics.is_null()) // some ctor chains do not track final stats
    return;  

  // this default implementation gets overridden/augmented in derived classes
  update_aleatory_final_statistics();

  if (respLevelTargetReduce) {
    update_system_final_statistics();
    update_system_final_statistics_gradients();
  }
}


void NonD::update_aleatory_final_statistics()
{
  // update finalStatistics from computed{Resp,Prob,Rel,GenRel}Levels
  size_t i, j, cntr = 0, rl_len, pl_bl_gl_len;
  for (i=0; i<numFunctions; ++i) {
    // final stats from compute_moments()
    if (finalMomentsType) {
      if (finalMomentStats.empty())
	cntr += 2;
      else {
	const Real* mom_i = finalMomentStats[i];
	finalStatistics.function_value(mom_i[0], cntr++); // mean
	finalStatistics.function_value(mom_i[1], cntr++); // stdev or var
      }
    }
    // final stats from compute_level_mappings()
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
  // same rl_len enforced for all resp fns in initialize_final_statistics()
  size_t i, j, rl_len = requestedRespLevels[0].length(),
    cntr = totalLevelRequests;
  if (finalMomentsType) cntr += 2*numFunctions;
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


void NonD::update_system_final_statistics_gradients()
{
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  // same rl_len enforced for all resp fns in initialize_final_statistics()
  size_t l, v, s, p, rl_len = requestedRespLevels[0].length(),
    num_deriv_vars = final_dvv.size(), cntr = totalLevelRequests,
    moment_offset = (finalMomentsType) ? 2 : 0;
  if (finalMomentsType) cntr += 2*numFunctions;
  RealVector final_stat_grad(num_deriv_vars, false);
  RealVectorArray component_grad(numFunctions);
  Real prod;
  for (l=0; l<rl_len; ++l, ++cntr) {
    if (final_asv[cntr] & 2) {
      // Retrieve component probability gradients from finalStatistics
      size_t index = 0;
      for (s=0; s<numFunctions; ++s) {
	index += moment_offset;
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


void NonD::initialize_level_mappings()
{
  // Default sizing assumes no distinction between requested and achieved
  // levels for the same measure (the request is always achieved) and assumes
  // probability (e.g., computed by binning) and reliability (e.g., computed
  // by moment projection) are not collapsible.
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


/** This function infers PDFs from the CDF/CCDF level mappings, in order
    to enable PDF computation after CDF/CCDF probability level refinement
    (e.g., from importance sampling).  

    prob_refinement alerts the routine to exclude inverse mappings from
    the PDF, since refinement only applies to z->p forward mappings and
    mixing refined and unrefined probability mappings results in an
    inconsistency (potentially manifesting as negative density values).

    all_levels_computed is an option used by reliability methods where 
    computed*Levels are defined across the union of all requested levels. */
void NonD::
compute_densities(const RealRealPairArray& min_max_fns,
		  bool prob_refinement, bool all_levels_computed)
{
  if (!pdfOutput)
    return;

  computedPDFAbscissas.resize(numFunctions);
  computedPDFOrdinates.resize(numFunctions);
  archive_allocate_pdf();

  size_t i, j, cntr, core_pdf_bins, pdf_size, offset;
  Real z, min, max, prev_r, prev_p, new_r, new_p, last_r;
  std::map<Real, Real> cdf_map;
  std::map<Real, Real>::iterator it, it_last;
  for (i=0; i<numFunctions; ++i) {

    // CDF/CCDF mappings: z -> p/beta/beta* and p/beta/beta* -> z
    const RealVector&  req_rlev_i = requestedRespLevels[i];
    const RealVector&  req_plev_i = requestedProbLevels[i];
    const RealVector&  req_glev_i = requestedGenRelLevels[i];
    const RealVector& comp_rlev_i = computedRespLevels[i];
    const RealVector& comp_plev_i = computedProbLevels[i];
    const RealVector& comp_glev_i = computedGenRelLevels[i];
    size_t rl_len = req_rlev_i.length(), pl_len = req_plev_i.length(),
      bl_len = requestedRelLevels[i].length(), gl_len = req_glev_i.length();

    // Define a unique sorted map of response levels -> cdf probabilities.
    // (refer to NonD::initialize_level_mappings() for default indexing;
    cdf_map.clear(); min = min_max_fns[i].first; max = min_max_fns[i].second;
    if (all_levels_computed) {
      for (j=0; j<rl_len; ++j) {
        z = comp_rlev_i[j]; // request may be outside extreme values
	cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
      }
      if (!prob_refinement || !rl_len) { // don't combine refined/unrefined
	size_t total_len = rl_len + pl_len + bl_len + gl_len;
	for (j=rl_len; j<total_len; ++j) {
	  z = comp_rlev_i[j];
	  //if (z >= min && z <= max) // exclude any extrapolations outside bnds
	    cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
	}
      }
    }
    else {
      switch (respLevelTarget) {
      case PROBABILITIES:
	for (j=0; j<rl_len; ++j) {
	  z = req_rlev_i[j]; // request may be outside extreme values
	  cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
	}
	break;
    //case RELIABILITIES: // exclude reliability level mappings from cdf_map
      case GEN_RELIABILITIES:
	for (j=0; j<rl_len; ++j) {
	  z = req_rlev_i[j]; // request may be outside extreme values
	  Real g_cdf = (cdfFlag) ? comp_glev_i[j] : -comp_glev_i[j];
	  cdf_map[z] = Pecos::NormalRandomVariable::std_cdf(-g_cdf);
	}
	break;
      }
      if (!prob_refinement || !rl_len) { // don't combine refined/unrefined
	for (j=0; j<pl_len; ++j) {
	  z = comp_rlev_i[j];
	  //if (z >= min && z <= max) // exclude any extrapolations outside bnds
	    cdf_map[z] = (cdfFlag) ? req_plev_i[j] : 1.-req_plev_i[j];
	}
	// exclude reliability level mappings from cdf_map
	for (j=0, cntr=pl_len+bl_len; j<gl_len; ++j, ++cntr) {
	  z = comp_rlev_i[cntr];
	  //if (z >= min && z <= max) {//exclude any extrapolations outside bnds
	    Real g_cdf = (cdfFlag) ? req_glev_i[j] : -req_glev_i[j];
	    cdf_map[z] = Pecos::NormalRandomVariable::std_cdf(-g_cdf);
	  //}
	}
      }
    }

    if (!cdf_map.empty()) {
      it      =   cdf_map.begin(); prev_r = it->first;
      it_last = --cdf_map.end();   last_r = it_last->first;
      // compute computedPDF{Abscissas,Ordinates} from bin counts and widths
      core_pdf_bins = cdf_map.size()-1; pdf_size = core_pdf_bins;
      if (min < prev_r) ++pdf_size;
      if (max > last_r) ++pdf_size;
      RealVector& abs_i = computedPDFAbscissas[i]; abs_i.resize(pdf_size+1);
      RealVector& ord_i = computedPDFOrdinates[i]; ord_i.resize(pdf_size);
      if (min < prev_r) { // first bin accumulates p0 over [min,lev0]
	offset   = 1;   prev_p   = it->second;
	abs_i[0] = min;	ord_i[0] = prev_p/(prev_r - min);
      }
      else { // first bin accumulates p0+p1 over [lev0,lev1]
	offset   = 0;   prev_p   = 0.;
      }
      for (j=0; j<core_pdf_bins; ++j) {
	++it; new_r = it->first; new_p = it->second;
	abs_i[j+offset] = prev_r;
	ord_i[j+offset] = (new_p - prev_p) / (new_r - prev_r);
	prev_r = new_r; prev_p = new_p;
      }
      if (max > last_r) {
	abs_i[pdf_size-1] = last_r;
	ord_i[pdf_size-1] = (1. - it_last->second)/(max - last_r);
	abs_i[pdf_size]   = max;  // no ordinate for final abscissa
      }
      else
	abs_i[pdf_size] = last_r; // no ordinate for final abscissa

      archive_pdf(i);
    }
  }
}


/** Print distribution mappings, including to file per response. */
void NonD::
print_level_mappings(std::ostream& s, String qoi_type,
		     const StringArray& qoi_labels) const
{
  print_densities(s, qoi_type, qoi_labels);

  // output CDF/CCDF probabilities resulting from binning or CDF/CCDF
  // reliabilities resulting from number of std devs separating mean & target
  s << std::scientific << std::setprecision(write_precision)
    << "\nLevel mappings for each " << qoi_type << ":\n";
  size_t i;
  for (i=0; i<numFunctions; ++i)
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty()) {
      print_level_map(s, i, qoi_labels[i]);
      // optionally write the distribution mapping to a .dist file
      if (outputLevel >= VERBOSE_OUTPUT)
	level_mappings_file(i, qoi_labels[i]);
    }
}


void NonD::
print_densities(std::ostream& s, String qoi_type,
		const StringArray& pdf_labels) const
{
  if (!pdfOutput)
    return;

  size_t i, j, wpp7 = write_precision+7, num_qoi = computedPDFOrdinates.size();
  if (num_qoi)
    s << std::scientific << std::setprecision(write_precision)
      << "\nProbability Density Function (PDF) histograms for each "
      << qoi_type << ":\n";
  for (i=0; i<num_qoi; ++i) {
    const RealVector& ord_i = computedPDFOrdinates[i];
    const RealVector& abs_i = computedPDFAbscissas[i];
    size_t pdf_len = ord_i.length();
    if (pdf_len) {
      s << "PDF for " << pdf_labels[i] << ":\n"
	<< "          Bin Lower          Bin Upper      Density Value\n"
	<< "          ---------          ---------      -------------\n";
      for (j=0; j<pdf_len; ++j)
	s << "  " << std::setw(wpp7) << abs_i[j]
	  << "  " << std::setw(wpp7) << abs_i[j+1]
	  << "  " << std::setw(wpp7) << ord_i[j] << '\n';
    }
  }
}


/// Write distribution mappings to a file for a single response
void NonD::level_mappings_file(size_t fn_index, const String& qoi_label) const
{
  std::string dist_filename = qoi_label + ".dist";
  std::ofstream dist_file;
  TabularIO::open_file(dist_file, dist_filename, "Distribution Map Output"); 
  dist_file << std::scientific << std::setprecision(write_precision);
  print_level_map(dist_file, fn_index, qoi_label);
}


/** Print the distribution mapping for a single response function to
    the passed output stream.  This base class version maps from one
    requested level type to one computed level type; some derived
    class implementations (e.g., local and global reliability) output
    multiple computed level types. */
void NonD::
print_level_map(std::ostream& s, size_t fn_index, const String& qoi_label) const
{
  size_t j, width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;

  if (cdfFlag)
    s << "Cumulative Distribution Function (CDF) for ";
  else
    s << "Complementary Cumulative Distribution Function (CCDF) for ";
  s << qoi_label << ":\n     Response Level  Probability Level  "
    << "Reliability Index  General Rel Index\n     --------------  "
    << "-----------------  -----------------  -----------------\n";
  size_t num_resp_levels = requestedRespLevels[fn_index].length();
  for (j=0; j<num_resp_levels; j++) { // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << requestedRespLevels[fn_index][j] << "  ";
    switch (respLevelTarget) {
    case PROBABILITIES:
      s << std::setw(width) << computedProbLevels[fn_index][j]   << '\n'; break;
    case RELIABILITIES:
      s << std::setw(w2p2)  << computedRelLevels[fn_index][j]    << '\n'; break;
    case GEN_RELIABILITIES:
      s << std::setw(w3p4)  << computedGenRelLevels[fn_index][j] << '\n'; break;
    }
  }
  size_t num_prob_levels = requestedProbLevels[fn_index].length();
  for (j=0; j<num_prob_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j]
      << "  " << std::setw(width) << requestedProbLevels[fn_index][j] << '\n';
  size_t num_rel_levels = requestedRelLevels[fn_index].length(),
    offset = num_prob_levels;
  for (j=0; j<num_rel_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j+offset]
      << "  " << std::setw(w2p2)  << requestedRelLevels[fn_index][j] << '\n';
  size_t num_gen_rel_levels = requestedGenRelLevels[fn_index].length();
  offset += num_rel_levels;
  for (j=0; j<num_gen_rel_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j+offset]
      << "  " << std::setw(w3p4)  << requestedGenRelLevels[fn_index][j] << '\n';
}


void NonD::print_system_mappings(std::ostream& s) const
{
  size_t rl_len = requestedRespLevels[0].length();
  if (respLevelTargetReduce && rl_len) {
    size_t i, width = write_precision+7, g_width = 2*width+2,
      cntr = totalLevelRequests;
    if (finalMomentsType) cntr += 2*numFunctions;
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


void NonD::
print_multilevel_evaluation_summary(std::ostream& s, const SizetArray& N_samp)
{
  size_t j, width = write_precision+7, num_lev = N_samp.size();
  for (j=0; j<num_lev; ++j)
    s << "                     " << std::setw(width) << N_samp[j] << '\n';
}


void NonD::
print_multilevel_evaluation_summary(std::ostream& s, const Sizet2DArray& N_samp)
{
  size_t j, width = write_precision+7, num_lev = N_samp.size();
  for (j=0; j<num_lev; ++j) {
    const SizetArray& N_j = N_samp[j];
    s << "                     " << std::setw(width) << N_j[0];
    if (!homogeneous(N_j)) // print all qoi counts
      for (size_t q=1; q<numFunctions; ++q)
	s << ' ' << N_j[q];
    s << '\n';
  }
}


void NonD::
print_multilevel_evaluation_summary(std::ostream& s, const Sizet3DArray& N_samp)
{
  size_t i, j, num_mf = N_samp.size(), width = write_precision+7;
  if (num_mf == 1)  s << "<<<<< Final samples per level:\n";
  else              s << "<<<<< Final samples per model form:\n";
  for (i=0; i<num_mf; ++i) {
    if (num_mf > 1) s << "      Model Form " << i+1 << ":\n";
    print_multilevel_evaluation_summary(s, N_samp[i]);
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


void NonD::archive_allocate_pdf() // const
{
  if (!resultsDB.active())  return;

  // pdf per function, possibly empty
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Response Functions");
  md["Row Labels"] = 
    make_metadatavalue("Bin Lower", "Bin Upper", "Density Value");
  resultsDB.array_allocate<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, numFunctions, md);
}


void NonD::archive_pdf(size_t i) // const
{
  if (!resultsDB.active()) return;

  size_t pdf_len = computedPDFOrdinates[i].length();
  RealMatrix pdf(3, pdf_len);
  for (size_t j=0; j<pdf_len; ++j) {
    pdf(0, j) = computedPDFAbscissas[i][j];
    pdf(1, j) = computedPDFAbscissas[i][j+1];
    pdf(2, j) = computedPDFOrdinates[i][j];
  }
  
  resultsDB.array_insert<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, i, pdf);
}

} // namespace Dakota
