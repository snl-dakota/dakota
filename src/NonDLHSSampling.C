/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Implementation code for NonDSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "system_defs.h"
#include "data_io.h"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "NonDLHSSampling.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: NonDLHSSampling.C 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDLHSSampling::NonDLHSSampling(Model& model): NonDSampling(model),
  numResponseFunctions(
    probDescDB.get_sizet("responses.num_response_functions")),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp"))
{ samplingVarsMode = ACTIVE; }


/** This alternate constructor is used for generation and evaluation
    of Model-based sample sets.  A set_db_list_nodes has not been
    performed so required data must be passed through the constructor.
    It's purpose is to avoid the need for a separate LHS specification
    within methods that use LHS sampling. */
NonDLHSSampling::
NonDLHSSampling(Model& model, const String& sample_type, int samples,
		int seed, const String& rng, bool vary_pattern,
		short sampling_vars_mode): 
  NonDSampling(NoDBBaseConstructor(), model, sample_type, samples, seed, rng,
	       vary_pattern),
  numResponseFunctions(numFunctions), varBasedDecompFlag(false)
{ samplingVarsMode = sampling_vars_mode; }


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets.  It is _not_ a
    letter-envelope instantiation and a set_db_list_nodes has not been
    performed.  It is called with all needed data passed through the
    constructor and is designed to allow more flexibility in variables
    set definition (i.e., relax connection to a variables
    specification and allow sampling over parameter sets such as
    multiobjective weights).  In this case, a Model is not used and 
    the object must only be used for sample generation (no evaluation). */
NonDLHSSampling::
NonDLHSSampling(const String& sample_type, int samples, int seed,
		const String& rng, const RealVector& lower_bnds,
		const RealVector& upper_bnds): 
  NonDSampling(NoDBBaseConstructor(), sample_type, samples, seed, rng,
	       lower_bnds, upper_bnds),
  numResponseFunctions(0), varBasedDecompFlag(false)
{
  samplingVarsMode = ACTIVE_UNIFORM; // not used but included for completeness

  // since there will be no late data updates to capture in this case
  // (no sampling_reset()), go ahead and get the parameter sets.
  get_parameter_sets(lower_bnds, upper_bnds);
}


NonDLHSSampling::~NonDLHSSampling()
{ }


void NonDLHSSampling::pre_run()
{
  // run LHS to generate parameter sets
  if (!varBasedDecompFlag)
    get_parameter_sets(iteratedModel);
}

void NonDLHSSampling::post_input()
{
  size_t cv_start, num_cv, div_start, num_div, drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div, drv_start, 
	      num_drv);
  size_t num_vars = num_cv + num_div + num_drv;
  // call convenience function from Analyzer
  read_variables_responses(numSamples, num_vars);
}


/** Loop over the set of samples and compute responses.  Compute
    statistics on the set of responses if statsFlag is set. */
void NonDLHSSampling::quantify_uncertainty()
{
  // If VBD has been selected, evaluate a series of parameter sets
  // (each of the size specified by the user) in order to compute VBD metrics.
  // If there are active discrete vars, they are included within allSamples.
  if (varBasedDecompFlag)
    variance_based_decomp(numContinuousVars, numDiscreteIntVars,
			  numDiscreteRealVars, numSamples);
  // if VBD has not been selected, evaluate a single parameter set of the size
  // specified by the user and stored in allSamples
  else {
    bool log_resp_flag = (allDataFlag || statsFlag);
    bool log_best_flag = !numResponseFunctions; // DACE mode w/ opt or NLS
    evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
  }
}

void NonDLHSSampling::post_run(std::ostream& s)
{
  //Statistics are generated here and output in NonDLHSSampling's
  // redefinition of print_results().
  if (statsFlag && !varBasedDecompFlag) // calculate statistics on allResponses
    compute_statistics(allSamples, allResponses);

  Iterator::post_run(s);
}


void NonDLHSSampling::print_results(std::ostream& s)
{
  if (!numResponseFunctions) // DACE mode w/ opt or NLS
    Analyzer::print_results(s);

  if (varBasedDecompFlag)
    print_sobol_indices(s);
  else if (statsFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota
