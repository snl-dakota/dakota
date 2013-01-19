/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DDACEDesignCompExp
//- Description: Implementation code for the DDACEDesignCompExp class
//- Owner:       Tony Giunta, Sandia National Laboratories

#include "DDACEDesignCompExp.hpp"
#include "system_defs.hpp"
#include "ProblemDescDB.hpp"
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
DDACEDesignCompExp::DDACEDesignCompExp(Model& model): PStudyDACE(model),
  daceMethod(probDescDB.get_string("method.sub_method_name")),
  samplesSpec(probDescDB.get_int("method.samples")), numSamples(samplesSpec),
  symbolsSpec(probDescDB.get_int("method.symbols")), numSymbols(symbolsSpec),
  seedSpec(probDescDB.get_int("method.random_seed")), randomSeed(seedSpec),
  allDataFlag(false), numDACERuns(0),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")),
  mainEffectsFlag(probDescDB.get_bool("method.main_effects"))
{
  if (daceMethod == "box_behnken")
    maxConcurrency *= 1 + 4*numContinuousVars*(numContinuousVars-1)/2;
  else if (daceMethod == "central_composite")
    maxConcurrency *= 1 + 2*numContinuousVars
                   +  (int)std::pow(2.,(double)numContinuousVars);
  else if (numSamples) // samples input is optional (default = 0)
    maxConcurrency *= numSamples;

  if (mainEffectsFlag && (daceMethod == "random" ||
      daceMethod == "box_behnken" || daceMethod == "central_composite" ) ) {
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
		   const String& sampling_method):
  PStudyDACE(NoDBBaseConstructor(), model), daceMethod(sampling_method),
  samplesSpec(samples), numSamples(samples), symbolsSpec(symbols),
  numSymbols(symbols), seedSpec(seed), randomSeed(seed), allDataFlag(true),
  numDACERuns(0), varyPattern(true), mainEffectsFlag(false)
{
  // Verify symbol & sample input.  The experimental design may not use exactly
  // the requests passed in, but it always will use >= the incoming requests.
  resolve_samples_symbols();

  if (numSamples) // samples is now optional (default = 0)
    maxConcurrency *= numSamples;
}


DDACEDesignCompExp::~DDACEDesignCompExp() { }


void DDACEDesignCompExp::pre_run()
{
  // obtain a set of samples for evaluation; if VBD, defer to run phase
  if (!varBasedDecompFlag)
    get_parameter_sets(iteratedModel);
}


void DDACEDesignCompExp::extract_trends()
{
  // If VBD has been selected, evaluate a series of parameter sets
  // (each of the size specified by the user) in order to compute VBD metrics.
  // If there are active discrete variables, DDACE currently ignores them.
  if (varBasedDecompFlag)
    variance_based_decomp(numContinuousVars, 0, 0, numSamples);
  // if VBD has not been selected, evaluate a single parameter set of the size
  // specified by the user and stored in allSamples
  else {
    bool log_best_flag  = (numObjFns || numLSqTerms), // opt or NLS data set
      compute_corr_flag = (!subIteratorFlag),
      log_resp_flag     = (mainEffectsFlag || allDataFlag || compute_corr_flag);
    evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
  }
}


void DDACEDesignCompExp::post_input()
{
  // call convenience function from Analyzer
  read_variables_responses(numSamples, numContinuousVars);
}


void DDACEDesignCompExp::post_run(std::ostream& s)
{
  // In VBD case, stats are managed in the run phase
  if (!varBasedDecompFlag) {
    if (mainEffectsFlag) // need allResponses
      compute_main_effects();
    else {
      // compute correlation statistics if (compute_corr_flag)
      bool compute_corr_flag = (!subIteratorFlag);
      if (compute_corr_flag)
	pStudyDACESensGlobal.compute_correlations(allSamples, allResponses);
    }
  }

  Iterator::post_run(s);
}


void DDACEDesignCompExp::get_parameter_sets(Model& model)
{
  // Sanity check the user specification
  resolve_samples_symbols();

  // keep track of number of DACE executions for this object
  numDACERuns++;

  // If a seed is specified, use it to get repeatable behavior, else allow DDACE
  // to generate different samples each time (seeded from a system clock).  For
  // the case where extract_trends() may be called multiple times for the same
  // iterator object (e.g., SBO), the varyPattern flag manages whether or not
  // an old seed is reused.  This allows for repeatable studies in which the
  // sampling pattern varies from one extract_trends() call to the next.  The
  // implementation of this feature is more straightforward than in NonDSampling
  // due to the persistence of the DDACE data.
  if (numDACERuns == 1) {
    if (seedSpec) // user seed specification: repeatable behavior
      DistributionBase::setSeed(randomSeed);
    else // no user specification: nonrepeatable behavior
      randomSeed = DistributionBase::seed(); // set seed from system clock
  }
  else if (!varyPattern) // force same sample pattern: reset to previous value
    DistributionBase::setSeed(randomSeed);
  Cout << "\nDACE method = " << daceMethod << " Samples = " << numSamples
       << " Symbols = " << numSymbols;
  if (varyPattern && numDACERuns > 1)
    Cout << " Seed not reset from previous DACE execution\n";
  else if (seedSpec)
    Cout << " Seed (user-specified) = " << randomSeed << '\n';
  else
    Cout << " Seed (system-generated) = " << randomSeed << '\n';

  // Get bounded region and check that (1) the lengths of bounds arrays are 
  // consistent with numContinuousVars, and (2) the bounds are not default 
  // bounds (upper/lower = +/-DBL_MAX) since this results in Infinity in the 
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
  int i, j;
  for (i=0; i<numContinuousVars; i++) {
    if (c_l_bnds[i] <= -DBL_MAX || c_u_bnds[i] >= DBL_MAX) {
      Cerr << "\nError: DDACEDesignCompExp requires specification of variable "
	   << "bounds for all active variables." << std::endl;
      abort_handler(-1);
    }
  }

  // Construct a uniform distribution vector
  // An alternative option is a normal distribution 
  std::vector<Distribution> ddace_distribution(numContinuousVars);
  for (i=0; i<numContinuousVars; i++)
    ddace_distribution[i]= UniformDistribution(c_l_bnds[i], c_u_bnds[i]);

  // vector used for DDace getSamples
  std::vector<DDaceSamplePoint> sample_points(numSamples);

  bool noise = true;
  if (daceMethod == "oas") {  
    DDaceOASampler ddace_sampler(numSamples, noise, ddace_distribution);
    ddace_sampler.getSamples(sample_points);
    if (mainEffectsFlag)
      symbolMapping = ddace_sampler.getP();
  }
  else if (daceMethod == "oa_lhs") {
    int strength = 2;
    bool randomize = true;
    DDaceOALHSampler ddace_sampler(numSamples, numContinuousVars, strength,
				   randomize, ddace_distribution);
    ddace_sampler.getSamples(sample_points);
    if (mainEffectsFlag)
      symbolMapping = ddace_sampler.getP();
  }
  else if (daceMethod == "lhs") {
    int replications = numSamples/numSymbols;
    DDaceLHSampler ddace_sampler(numSamples, replications, noise,
				 ddace_distribution);
    ddace_sampler.getSamples(sample_points);
    if (mainEffectsFlag)
      symbolMapping = ddace_sampler.getP();
  }
  else if (daceMethod == "random") {
    DDaceRandomSampler ddace_sampler(numSamples, ddace_distribution);
    ddace_sampler.getSamples(sample_points);
  }
  else if (daceMethod == "grid") {
    DDaceFactorialSampler ddace_sampler(numSamples, numSymbols, noise,
					ddace_distribution);
    ddace_sampler.getSamples(sample_points);
    if (mainEffectsFlag)
      symbolMapping = ddace_sampler.getP();
  }
  else if (daceMethod == "central_composite") {
    DDaceCentralCompositeSampler ddace_sampler(numSamples, numContinuousVars,
					       ddace_distribution);
    ddace_sampler.getSamples(sample_points);
  }
  else if (daceMethod == "box_behnken") {
    DDaceBoxBehnkenSampler ddace_sampler(numSamples, numContinuousVars,
					 ddace_distribution);
    ddace_sampler.getSamples(sample_points);
  }
  else {
    Cerr << "DDACE method \"" << daceMethod << "\" is not available at this "
    	 << "time\nplease choose another sampling method." << std::endl;
    abort_handler(-1);
  }

  // copy the DDace sample array to allSamples
  if (allSamples.numRows() != numContinuousVars ||
      allSamples.numCols() != numSamples)
    allSamples.shapeUninitialized(numContinuousVars, numSamples);
  for (i=0; i<numSamples; ++i) {
    Real* all_samp_i = allSamples[i];
    const DDaceSamplePoint& sample_pt_i = sample_points[i];
    for (j=0; j<numContinuousVars; ++j)
      all_samp_i[j] = sample_pt_i[j];
  }

  if (volQualityFlag) {
    double* dace_points = new double [numContinuousVars*numSamples];
    copy_data(sample_points, dace_points, numContinuousVars*numSamples);
    for (i=0; i<numContinuousVars; i++) {
      const double& offset = c_l_bnds[i];
      double norm = 1. / (c_u_bnds[i] - c_l_bnds[i]);
      for (j=0; j<numSamples; j++)
        dace_points[i+j*numContinuousVars]
	  = (dace_points[i+j*numContinuousVars] - offset) * norm;
    }
    volumetric_quality(numContinuousVars, numSamples, dace_points);
    delete [] dace_points;
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
  if (numSamples <= 0 && daceMethod != "central_composite" &&
      daceMethod != "box_behnken" ) {
    Cerr << "\nError: number of DACE samples must be greater than zero."
	 << std::endl;
    abort_handler(-1);
  }

  int num_samples_orig = numSamples, num_symbols_orig = numSymbols;
  if (daceMethod == "oas" || daceMethod == "oa_lhs") {  
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
  }
  else if (daceMethod == "lhs") {
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
  }
  else if (daceMethod == "random") {
    // numSymbols ignored for this method
  }
  else if (daceMethod == "box_behnken") {
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
  }
  else if (daceMethod == "central_composite") {
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
  }
  else if (daceMethod == "grid") {
    // 'grid' sampling method sets up an n-dimensional grid around the current
    // variables: number of samples = number of symbols^(number of variables)
    if ( numSamples != std::pow((double)numSymbols,(double)numContinuousVars) ) {
      Cout << "\n\nWarning: For grid sampling the number of samples should "
    	   << "be\n         (num_symbols)^(num_variables).\n";
      numSymbols
	= (int)std::ceil(std::pow((double)numSamples, 1./(double)numContinuousVars));
      numSamples = (int)std::pow((double)numSymbols,(double)numContinuousVars);
    }
  }
  else {
    Cerr << "Error: DDACE method \"" << daceMethod << "\" is not an option."
	 << std::endl;
    abort_handler(-1);
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
	 << setw(14) << fn_labels[f] << ":\n--------------------------------";
    DDaceMainEffects::OneWayANOVA main_effects(ddace_factors);	
    main_effects.printANOVATables();
  }
}


  // inactive copy functions -- remove?

// copy DDACE point to RealVector
//void copy_data(const DDaceSamplePoint& dsp, RealVector& rv)
// {
//   int j, vec_len = dsp.length();
//   if (rv.length() != vec_len)
//     rv.sizeUninitialized(vec_len);
//   for (j=0; j<vec_len; ++j)
//     rv[j] = dsp[j];
// }

// copy DDACE point array to RealVectorArray
// void copy_data(const std::vector<DDaceSamplePoint>& dspa,
// 	       RealVectorArray& rva)
// {
//   int i, num_vec = dspa.size();
//   if (rva.size() != num_vec)
//     rva.resize(num_vec);
//   for (i=0; i<num_vec; ++i)
//     copy_data(dspa[i], rva[i]);
// }

// copy DDACE point array to Real*
void DDACEDesignCompExp::
copy_data(const std::vector<DDaceSamplePoint>& dspa, Real* ptr,
	  const int ptr_len)
{
  int i, j, num_vec = dspa.size(), total_len = 0, cntr = 0;
  for (i=0; i<num_vec; ++i)
    total_len += dspa[i].length();
  if (total_len != ptr_len) {
    Cerr << "Error: pointer allocation (" << ptr_len << ") does not equal "
	 << "total Array<DDaceSamplePoint> length (" << total_len << ") in "
	 << "copy_data(Array<DDaceSamplePoint>, Real*)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_vec; ++i) {
    int vec_len = dspa[i].length();
    for (j=0; j<vec_len; ++j)
      ptr[cntr++] = dspa[i][j];
  }
}

} // namespace Dakota
