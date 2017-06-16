/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DDACEDesignCompExp
//- Description: Implementation code for the DDACEDesignCompExp class
//- Owner:       Tony Giunta, Sandia National Laboratories

#include "DDACEDesignCompExp.hpp"
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DDaceSampler.h"
#include "DDaceRandomSampler.h"
#include "DDaceOASampler.h"
#include "DDaceOALHSampler.h"
#include "DDaceLHSampler.h"
#include "DDaceBoxBehnkenSampler.h"
#include "DDaceCentralCompositeSampler.h"
#include "DDaceFactorialSampler.h"
#include "UniformDistribution.h"
#include "OneWayANOVA.h"
#include "Factor.h"
#include "Response.h"

static const char rcsId[]="@(#) $Id: DDACEDesignCompExp.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard iterator built with data from
    probDescDB. */
DDACEDesignCompExp::DDACEDesignCompExp(ProblemDescDB& problem_db, Model& model):
  PStudyDACE(problem_db, model),
  daceMethod(probDescDB.get_ushort("method.sub_method")),
  samplesSpec(probDescDB.get_int("method.samples")), numSamples(samplesSpec),
  symbolsSpec(probDescDB.get_int("method.symbols")), numSymbols(symbolsSpec),
  seedSpec(probDescDB.get_int("method.random_seed")), randomSeed(seedSpec),
  allDataFlag(false), numDACERuns(0),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")),
  mainEffectsFlag(probDescDB.get_bool("method.main_effects"))
{
  if (numDiscreteIntVars > 0 || numDiscreteStringVars > 0 || 
      numDiscreteRealVars > 0) {
    Cerr << "\nError: dace methods do not support discrete variables.\n";
    abort_handler(-1);
  }

  if (daceMethod == SUBMETHOD_BOX_BEHNKEN)
    maxEvalConcurrency *= 1 + 4*numContinuousVars*(numContinuousVars-1)/2;
  else if (daceMethod == SUBMETHOD_CENTRAL_COMPOSITE)
    maxEvalConcurrency *= 1 + 2*numContinuousVars
                       +  (int)std::pow(2.,(double)numContinuousVars);
  else if (numSamples) // samples input is optional (default = 0)
    maxEvalConcurrency *= numSamples;

  if (mainEffectsFlag && (daceMethod == SUBMETHOD_RANDOM ||
			  daceMethod == SUBMETHOD_BOX_BEHNKEN ||
			  daceMethod == SUBMETHOD_CENTRAL_COMPOSITE ) ) {
    Cerr << "\nError: The Box-Behnken, central composite and random sampling" 
         << "\n       methods do not support calculation of main effects."
	 << std::endl;
    abort_handler(-1);
  }
}


/** This alternate constructor is used for instantiations on-the-fly,
    using only the incoming data.  No problem description database
    queries are used. */
DDACEDesignCompExp::
DDACEDesignCompExp(Model& model, int samples, int symbols, int seed,
		   unsigned short sampling_method):
  PStudyDACE(DACE, model), daceMethod(sampling_method), samplesSpec(samples),
  numSamples(samples), symbolsSpec(symbols), numSymbols(symbols),
  seedSpec(seed), randomSeed(seed), allDataFlag(true), numDACERuns(0),
  varyPattern(true), mainEffectsFlag(false)
{
  if (numDiscreteIntVars > 0 || numDiscreteStringVars > 0 || 
      numDiscreteRealVars > 0) {
    Cerr << "\nError: dace methods do not support discrete variables.\n";
    abort_handler(-1);
  }
  // Verify symbol & sample input.  The experimental design may not use exactly
  // the requests passed in, but it always will use >= the incoming requests.
  resolve_samples_symbols();

  if (numSamples) // samples is now optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


DDACEDesignCompExp::~DDACEDesignCompExp() { }

bool DDACEDesignCompExp::resize()
{
  bool parent_reinit_comms = PStudyDACE::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void DDACEDesignCompExp::pre_run()
{
  Analyzer::pre_run();

  // Sanity check the user specification for samples/symbols
  resolve_samples_symbols();

  // If VBD has been selected, generate a series of replicate parameter sets
  // (each of the size specified by the user) in order to compute VBD metrics.
  if (varBasedDecompFlag)
    get_vbd_parameter_sets(iteratedModel, numSamples);
  else
    get_parameter_sets(iteratedModel);
}


void DDACEDesignCompExp::core_run()
{
  bool log_best_flag  = (numObjFns || numLSqTerms), // opt or NLS data set
    compute_corr_flag = (!subIteratorFlag),
    log_resp_flag     = (mainEffectsFlag || allDataFlag || compute_corr_flag);
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void DDACEDesignCompExp::post_input()
{
  if (parallelLib.command_line_user_modes() && 
      !parallelLib.command_line_post_run_input().empty()) {
    // apply any corrections to user spec to update numSamples before post input
    resolve_samples_symbols();
  }
  // call convenience function from Analyzer
  read_variables_responses(numSamples, numContinuousVars);
}


void DDACEDesignCompExp::post_run(std::ostream& s)
{
  // Main effects require the symbolMapping.  It should only be empty
  // if pre-run get_parameter_sets wasn't called in this execution, in
  // which case, we need a fixed seed to properly regenerate it.
  if (mainEffectsFlag && symbolMapping.empty()) {
    if (!seedSpec) {
      Cerr << "\nError (DACE): calculating main effects in post_run mode "
	   << "requires user-specified seed.\n";
      abort_handler(-1);
    }
    boost::shared_ptr<DDaceSamplerBase> ddace_sampler = 
      create_sampler(iteratedModel);
    symbolMapping = ddace_sampler->getP();
  }

  // BMA TODO: always compute all stats, even in VBD mode (stats on
  // first two replicates)
  if (varBasedDecompFlag) {
    compute_vbd_stats(numSamples, allResponses);
  }
  else {
    if (mainEffectsFlag) // need allResponses
      compute_main_effects();
    else {
      // compute correlation statistics if (compute_corr_flag)
      bool compute_corr_flag = (!subIteratorFlag);
      if (compute_corr_flag)
	pStudyDACESensGlobal.compute_correlations(allSamples, allResponses);
    }
  }

  Analyzer::post_run(s);
}


void DDACEDesignCompExp::get_parameter_sets(Model& model)
{
  get_parameter_sets(model, numSamples, allSamples);
}


void DDACEDesignCompExp::get_parameter_sets(Model& model, const int num_samples,
					    RealMatrix& design_matrix)
{
  // keep track of number of DACE executions for this object
  numDACERuns++;

  Cout << "\nDACE method = " << submethod_enum_to_string(daceMethod) 
       << " Samples = " << num_samples << " Symbols = " << numSymbols;

  // If a seed is specified, use it to get repeatable behavior, else allow
  // DDACE to generate different samples each time (seeded from a system clock).
  // For the case where core_run() may be called multiple times for the same
  // iterator object (e.g., SBO), the varyPattern flag manages whether or not
  // an old seed is reused.  This allows for repeatable studies in which the
  // sampling pattern varies from one core_run() call to the next.  The
  // implementation of this feature is more straightforward than in
  // NonDSampling due to the persistence of the DDACE data.
  if (numDACERuns == 1) {
    if (seedSpec) // user seed specification: repeatable behavior
      DistributionBase::setSeed(randomSeed);
    else // no user specification: nonrepeatable behavior
      randomSeed = DistributionBase::seed(); // set seed from system clock
  }
  else if (!varyPattern) // force same sample pattern: reset to previous value
    DistributionBase::setSeed(randomSeed);
  if (varyPattern && numDACERuns > 1)
    Cout << " Seed not reset from previous DACE execution\n";
  else if (seedSpec)
    Cout << " Seed (user-specified) = " << randomSeed << '\n';
  else
    Cout << " Seed (system-generated) = " << randomSeed << '\n';

  // vector used for DDace getSamples
  std::vector<DDaceSamplePoint> sample_points(num_samples);

  // in get_parameter_sets, generate the samples; could omit the symbolMapping
  boost::shared_ptr<DDaceSamplerBase> ddace_sampler = 
    create_sampler(iteratedModel);
  ddace_sampler->getSamples(sample_points);
  if (mainEffectsFlag)
    symbolMapping = ddace_sampler->getP();

  // copy the DDace sample array to design_matrix
  if (design_matrix.numRows() != numContinuousVars ||
      design_matrix.numCols() != num_samples)
    design_matrix.shapeUninitialized(numContinuousVars, num_samples);
  for (int i=0; i<num_samples; ++i) {
    Real* all_samp_i = design_matrix[i];
    const DDaceSamplePoint& sample_pt_i = sample_points[i];
    for (int j=0; j<numContinuousVars; ++j)
      all_samp_i[j] = sample_pt_i[j];
  }

  if (volQualityFlag) {
    double* dace_points = new double [numContinuousVars*num_samples];
    copy_data(sample_points, dace_points, numContinuousVars*num_samples);
    const RealVector& c_l_bnds = model.continuous_lower_bounds();
    const RealVector& c_u_bnds = model.continuous_upper_bounds();
    for (int i=0; i<numContinuousVars; i++) {
      const double& offset = c_l_bnds[i];
      double norm = 1. / (c_u_bnds[i] - c_l_bnds[i]);
      for (int j=0; j<num_samples; j++)
        dace_points[i+j*numContinuousVars]
	  = (dace_points[i+j*numContinuousVars] - offset) * norm;
    }
    volumetric_quality(numContinuousVars, num_samples, dace_points);
    delete [] dace_points;
  }
}

boost::shared_ptr<DDaceSamplerBase>
DDACEDesignCompExp::create_sampler(Model& model)
{
  // Get bounded region and check that (1) the lengths of bounds arrays are 
  // consistent with numContinuousVars, and (2) the bounds are not default 
  // bounds (upper/lower = +/-inf) since this results in Infinity in the 
  // sample_points returned.  Discrepancies can occur in the case of uncertain
  // variables, since they do not currently have global bounds specifications.
  // It would be nice to detect this and automatically delete any uncertain
  // variables (from numContinuousVars & local_vars).
  const RealVector& c_l_bnds = model.continuous_lower_bounds();
  const RealVector& c_u_bnds = model.continuous_upper_bounds();
  if (c_l_bnds.length() != numContinuousVars || 
      c_u_bnds.length() != numContinuousVars) {
    Cerr << "\nError: Mismatch in number of active variables and length of"
         << "\n       bounds arrays in DDACEDesignCompExp." << std::endl;
    abort_handler(-1);
  }
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  for (int i=0; i<numContinuousVars; i++) {
    if (c_l_bnds[i] == -dbl_inf || c_u_bnds[i] == dbl_inf) {
      Cerr << "\nError: DDACEDesignCompExp requires specification of variable "
	   << "bounds for all active variables." << std::endl;
      abort_handler(-1);
    }
  }

  // Construct a uniform distribution vector
  // An alternative option is a normal distribution 
  std::vector<Distribution> ddace_distribution(numContinuousVars);
  for (int i=0; i<numContinuousVars; i++)
    ddace_distribution[i]= UniformDistribution(c_l_bnds[i], c_u_bnds[i]);

  bool noise = true;
  switch (daceMethod) {
  case SUBMETHOD_OAS: {
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceOASampler(numSamples, noise, ddace_distribution));
    break;
  }
  case SUBMETHOD_OA_LHS: {
    int strength = 2;
    bool randomize = true;
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceOALHSampler(numSamples, numContinuousVars, strength, randomize,
			    ddace_distribution));
    break;
  }
  case SUBMETHOD_LHS: {
    int replications = numSamples/numSymbols;
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceLHSampler(numSamples, replications, noise, ddace_distribution));
    break;
  }
  case SUBMETHOD_RANDOM: {
      return boost::shared_ptr<DDaceSamplerBase>
	(new DDaceRandomSampler(numSamples, ddace_distribution));
    break;
  }
  case SUBMETHOD_GRID: {
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceFactorialSampler(numSamples, numSymbols, noise, 
				 ddace_distribution));
    break;
  }
  case SUBMETHOD_CENTRAL_COMPOSITE: {
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceCentralCompositeSampler(numSamples, numContinuousVars,
					ddace_distribution));
    break;
  }
  case SUBMETHOD_BOX_BEHNKEN: {
    return boost::shared_ptr<DDaceSamplerBase>
      (new DDaceBoxBehnkenSampler(numSamples, numContinuousVars,
				  ddace_distribution));
    break;
  }
  default:
    Cerr << "DDACE method \"" << submethod_enum_to_string(daceMethod) 
	 << "\" is not available at this time\n"
	 << "please choose another sampling method." << std::endl;
    abort_handler(-1);
    break;
  }

}


/** This function must define a combination of samples and symbols
    that is acceptable for a particular sampling algorithm.  Users
    provide requests for these quantities, but this function must
    enforce any restrictions imposed by the sampling algorithms. */
void DDACEDesignCompExp::resolve_samples_symbols()
{
  // precedence given to numSamples request over any numSymbols request, 
  // and numSamples may not be decreased.  NOTE: a better spec might be the 
  // integer multiple (optional, default = 1) which would automatically define
  // the number of symbols for each method.

  // numSamples is an optional specification with a default of zero.  In some
  // cases (i.e., data fit approximations), the number of required samples can
  // be automatically determined and passed via sampling_reset().  Since
  // resolve_samples_symbols() is called at run time, it can be assumed that
  // numSamples is up-to-date, containing either a user-specified requirement
  // or a minimum surrogate-build requirement.

  // Error check: CCD/BB have a default number of samples, but 
  // OAS/LHS/OA_LHS/RANDOM/GRID have no reasonable default and must have a
  // samples specification to proceed.
  if (numSamples <= 0 && daceMethod != SUBMETHOD_CENTRAL_COMPOSITE &&
      daceMethod != SUBMETHOD_BOX_BEHNKEN ) {
    Cerr << "\nError: number of DACE samples must be greater than zero."
	 << std::endl;
    abort_handler(-1);
  }

  int num_samples_orig = numSamples, num_symbols_orig = numSymbols;
  switch (daceMethod) {
  case SUBMETHOD_OAS: case SUBMETHOD_OA_LHS: {  
    // numSamples is an integer multiple of numSymbols^r where r=2 for DDACE
    // numSymbols must equal 4, or it must be a prime number.
    int i;
    bool prime_flag = 1;
    for (i=2; i<numSymbols; i++) {
      if ( numSymbols%i == 0 && numSymbols != 4 ) {
	prime_flag = 0;
	break;
      }
    }
    // if user input is imperfect, recalculate samples and symbols
    if (numSymbols <= 0 || numSamples%(numSymbols*numSymbols) || !prime_flag) {
      Cout << "\n\nWarning: For orthogonal array sampling, the number of "
	   << "samples should be an\n         integer multiple of "
	   << "(num_symbols)^2, and num_symbols should be\n         either 4 "
	   << "or a prime number.\n";
      numSymbols = (int)std::ceil(std::sqrt((double)numSamples)); // round up

      // Verify that numSymbols equals 4, or is prime.  If not, increase
      // numSymbols until it is prime.
      prime_flag = 0;
      while( !prime_flag ) {
 	prime_flag = 1;
	for ( i=2; i<numSymbols; i++ ) {
	  if ( numSymbols%i == 0 && numSymbols != 4 ) {
	    prime_flag = 0;
	    break;
	  }
	}
	if ( !prime_flag )
          numSymbols++;
      }
      numSamples = numSymbols*numSymbols;
    }
    break;
  }
  case SUBMETHOD_LHS: {
    // numSamples must be an integer multiple of the numSymbols
    if ( numSymbols <= 0 )
      numSymbols = numSamples; // default if no specification
    else if ( numSamples%numSymbols ) {
      Cout << "\n\nWarning: For Latin hypercube sampling the number of samples"
	   << " should be an\n        integer multiple of the number of "
	   << "symbols.\n";
      int replications = (int)std::ceil((double)numSamples/(double)numSymbols);
      numSamples = replications*numSymbols;
    }
    break;
  }
  case SUBMETHOD_RANDOM:
    // numSymbols ignored for this method
    break;
  case SUBMETHOD_BOX_BEHNKEN: {
    // BBD is inflexible in the number of samples that can be used
    int num_samples_bb = 1 + 4*numContinuousVars*(numContinuousVars-1)/2;
    if ( num_samples_bb >= numSamples )
      // For a quadratic polynomial, BBD is sufficient only for #vars >= 3
      numSymbols = numSamples = num_samples_bb;
    else { // numSamples > num_samples_bb
      // In this case, the BBD cannot produce enough samples to meet either the
      // user request (which may not be decreased) or the minimum required by a
      // surrogate.
      Cerr << "\nError: DDACE method box_behnken generates " << num_samples_bb
	   << " samples for " << numContinuousVars << " variables.\n       "
	   << "This is insufficient to meet/exceed the user/surrogate "
	   << "requirement of " << numSamples << " samples.\n       Either "
	   << "reduce the requirement or select another sampling method.\n"
	   << std::endl;
      abort_handler(-1);
    }
    break;
  }
  case SUBMETHOD_CENTRAL_COMPOSITE: {
    // CCD is inflexible in the number of samples that can be used
    int num_samples_ccd
      = 1 + 2*numContinuousVars + (int)std::pow(2.0,(double)numContinuousVars);
    if ( num_samples_ccd >= numSamples )
      // num_samples_ccd should be greater than the minimum number of samples
      // required for a surface-fitting method.  E.g., a quadratic polynomial
      // needs (n+1)*(n+2)/2 samples and CCD always generates more than this.
      numSymbols = numSamples = num_samples_ccd;
    else { // numSamples > num_samples_ccd
      // In this case, the CCD cannot produce enough samples to meet either the
      // user request (which may not be decreased) or the minimum required by a
      // surrogate.
      Cerr << "\nError: DDACE method central_composite generates "
	   << num_samples_ccd << " samples for " << numContinuousVars
	   << " variables.\n       This is insufficient to meet/exceed the "
	   << "user/surrogate requirement of " << numSamples << " samples.\n"
	   << "       Either reduce the requirement or select another sampling "
	   << "method.\n" << std::endl;
      abort_handler(-1);
    }
    break;
  }
  case SUBMETHOD_GRID: {
    // 'grid' sampling method sets up an n-dimensional grid around the current
    // variables: number of samples = number of symbols^(number of variables)
    if (numSamples != std::pow((double)numSymbols,(double)numContinuousVars)) {
      Cout << "\n\nWarning: For grid sampling the number of samples should "
    	   << "be\n         (num_symbols)^(num_variables).\n";
      numSymbols = (int)std::ceil(std::pow((double)numSamples,
					   1./(double)numContinuousVars));
      numSamples = (int)std::pow((double)numSymbols,(double)numContinuousVars);
    }
    break;
  }
  default:
    Cerr << "Error: DDACE method \"" << submethod_enum_to_string(daceMethod) 
	 << "\" is not an option." << std::endl;
    abort_handler(-1); break;
  }

  if (numSamples != num_samples_orig || numSymbols != num_symbols_orig)
    Cout << "\nAdjusting the number of symbols and samples...."
         << "\n  num_variables   = " << numContinuousVars
	 << "\n  OLD num_samples = " << num_samples_orig
	 << "    OLD num_symbols = " << num_symbols_orig
	 << "\n  NEW num_samples = " << numSamples
	 << "    NEW num_symbols = " << numSymbols << '\n';
}


void DDACEDesignCompExp::compute_main_effects() 
{
  if (allResponses.size() != numSamples) {
    Cerr << "\nError in DDACEDesignCompExp::compute_main_effects(): expected "
	 << numSamples << " responses; received " << allResponses.size()
	 << std::endl;
    abort_handler(-1);
  }

  const StringArray& fn_labels = iteratedModel.response_labels();
  IntRespMCIter r_it; size_t f, s, v;
  std::vector<double> resp_fn_samples(numSamples);
  std::vector<int> symbols_map_factor(numSamples);
  for (f=0; f<numFunctions; ++f) {

    for (s=0, r_it=allResponses.begin(); s<numSamples; ++s, ++r_it)
      resp_fn_samples[s] = r_it->second.function_value(f);

    // Create a DDACE Response object
    DDaceMainEffects::Response ddace_response(resp_fn_samples);

    // Create a vector of factors 
    std::vector<DDaceMainEffects::Factor> ddace_factors(numContinuousVars);

    for (v=0; v<numContinuousVars; ++v) {
      for (s=0; s<numSamples; ++s) 
	symbols_map_factor[s] = symbolMapping[s][v];

      // Create a DDACE Factor object
      ddace_factors[v] = DDaceMainEffects::Factor(symbols_map_factor,
						  numSymbols, ddace_response);
    }

    // Perform the ANOVA computations and display screen output
    Cout << "\n--------------------------------\nMain effects for "
	 << std::setw(14) << fn_labels[f] << ":\n--------------------------------";
    DDaceMainEffects::OneWayANOVA main_effects(ddace_factors);	
    // BMA TODO: should this instead be printed at print_results time?
    main_effects.printANOVATables();
  }
}

} // namespace Dakota
