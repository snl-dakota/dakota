/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       PSUADEDesignCompExp
//- Description: Implementation code for the PSUADEDesignCompExp class
//- Owner:       Brian M. Adams, Sandia National Laboratories

#include "PSUADEDesignCompExp.hpp"
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#ifdef HAVE_DDACE
#include "Distribution.h"
#elif defined(DAKOTA_UTILIB)
#include <utilib/seconds.h>
#endif
// PSUADE specific headers
#include "MOATSampling.h"
#include "sData.h"
#include "MOATAnalyzer.h"
#include "aData.h"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

/** This constructor is called for a standard iterator built with data from
    probDescDB. */
PSUADEDesignCompExp::
PSUADEDesignCompExp(ProblemDescDB& problem_db, Model& model):
  PStudyDACE(problem_db, model),
  samplesSpec(probDescDB.get_int("method.samples")), numSamples(samplesSpec),
  varPartitionsSpec(probDescDB.get_usa("method.partitions")),
  numPartitions(0), allDataFlag(false), numDACERuns(0), varyPattern(true), 
  seedSpec(probDescDB.get_int("method.random_seed")), randomSeed(seedSpec)
{
  if (methodName != PSUADE_MOAT) {
    Cerr << "\nError: PSUADE method \"" << method_string() 
	 << "\" is not an option." << std::endl;
    abort_handler(-1);
  }
  if (numDiscreteIntVars > 0 || numDiscreteStringVars > 0 || 
      numDiscreteRealVars > 0) {
    Cerr << "\nError: psuade_* methods do not support discrete variables.\n";
    abort_handler(-1);
  }

  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


PSUADEDesignCompExp::~PSUADEDesignCompExp()
{ }

bool PSUADEDesignCompExp::resize()
{
  bool parent_reinit_comms = PStudyDACE::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void PSUADEDesignCompExp::pre_run()
{
  Analyzer::pre_run();

  // error check on input parameters; also adjusts numSamples if needed
  enforce_input_rules();

  // obtain a set of samples for evaluation
  get_parameter_sets(iteratedModel);
}


void PSUADEDesignCompExp::post_input()
{
  // error check on input parameters; also adjusts numSamples if needed
  enforce_input_rules();

  // call convenience function from Analyzer
  read_variables_responses(numSamples, numContinuousVars);
}


void PSUADEDesignCompExp::core_run()
{
  // evaluate the sample set to obtain the corresponding set of results
  //evaluate_parameter_sets(iteratedModel, allDataFlag, true);
  bool log_resp_flag = true, // allResponses required below
       log_best_flag = (numObjFns || numLSqTerms); // opt or NLS data set
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void PSUADEDesignCompExp::post_run(std::ostream& s)
{
  // error check on input parameters; also adjusts numSamples if needed
  enforce_input_rules();

  // Perform post-processing with MOAT Analyzer
  
  // define the data structure necessary for analyzing a PSUADE MOAT sample
  aData psuade_adata;
  psuade_adata.nInputs_ = numContinuousVars;
  psuade_adata.nOutputs_ = numFunctions;
  psuade_adata.nSamples_ = numSamples;

  // since MOAT uses pointers, make copies of the data
  const RealVector& lb = iteratedModel.continuous_lower_bounds();
  const RealVector& ub = iteratedModel.continuous_upper_bounds();
  psuade_adata.iLowerB_ = new double [numContinuousVars];
  psuade_adata.iUpperB_ = new double [numContinuousVars];
  for (int i=0; i<numContinuousVars; i++) {
    psuade_adata.iLowerB_[i] = lb[i];
    psuade_adata.iUpperB_[i] = ub[i];
  }

  psuade_adata.sampleInputs_ = new double [numSamples*numContinuousVars];
  for (int i=0; i<numSamples; i++) {
    const Real* c_vars = allSamples[i];
    for (int j=0; j<numContinuousVars; j++)
      psuade_adata.sampleInputs_[i*numContinuousVars+j]	= c_vars[j];
  }

  psuade_adata.sampleOutputs_ = new double [numSamples*numFunctions];
 
  if (allResponses.size() != numSamples) {
    Cerr << "\nError in PSUADEDesignCompExp::post_run(): expected "
	 << numSamples << " responses; received " << allResponses.size()
	 << std::endl;
    abort_handler(-1);
  }

  IntRespMCIter r_it; int i, j;
  for (r_it=allResponses.begin(), i=0; i<numSamples; ++r_it, ++i) {
    const RealVector& fn_vals = r_it->second.function_values();
    for (j=0; j<numFunctions; ++j)
      psuade_adata.sampleOutputs_[i*numFunctions+j] = fn_vals[j];
  }

  MOATAnalyzer *psuadeAnalyzer = new MOATAnalyzer();
  for (i=0; i<numFunctions; ++i) {
    Cout << "\n>>>>>> PSUADE MOAT output for function " << i << ":\n";
    psuade_adata.outputID_ = i;
    psuadeAnalyzer->analyze(psuade_adata);
  }

  delete [] psuade_adata.iLowerB_;
  delete [] psuade_adata.iUpperB_;
  delete [] psuade_adata.sampleInputs_;
  delete [] psuade_adata.sampleOutputs_;
  delete psuadeAnalyzer;
  psuadeAnalyzer = NULL;

  Analyzer::post_run(s);
}


void PSUADEDesignCompExp::get_parameter_sets(Model& model)
{
  // keep track of number of DACE executions for this object
  numDACERuns++;

  // Get bounded region and check that (1) the lengths of bounds arrays are 
  // consistent with numContinuousVars, and (2) the bounds are not default 
  // bounds (upper/lower = +/-inf) since this results in Infinity in the 
  // sample_points returned.  Discrepancies can occur in the case of uncertain
  // variables, since they do not currently have global bounds specifications.
  // It would be nice to detect this and automatically delete any uncertain
  // variables (from numContinuousVars & local_vars).

  // make copies since we'll pass pointers to MOAT
  const RealVector& c_l_bnds = model.continuous_lower_bounds();
  const RealVector& c_u_bnds = model.continuous_upper_bounds();
  if (c_l_bnds.length() != numContinuousVars || 
      c_u_bnds.length() != numContinuousVars) {
    Cerr << "\nError: Mismatch in number of active variables and length of"
         << "\n       bounds arrays in PSUADEDesignCompExp." << std::endl;
    abort_handler(-1);
  }
  size_t i, j;
  RealVector c_bnds_range(numContinuousVars);
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  for (i=0; i<numContinuousVars; i++) {
    if (c_l_bnds[i] == -dbl_inf || c_u_bnds[i] == dbl_inf) {
      Cerr << "\nError: PSUADEDesignCompExp requires specification of variable "
	   << "bounds for all active variables." << std::endl;
      abort_handler(-1);
    }
    c_bnds_range[i] = c_u_bnds[i] - c_l_bnds[i];
  }

  if( methodName == PSUADE_MOAT ) {

    // Set seed value for input to CVT.  A user-specified seed gives you
    // repeatable behavior but no specification gives you random behavior (seed
    // generated from a system clock).  For the cases where core_run() may be
    // called multiple times for the same DACE object (e.g., SBO), a
    // deterministic sequence of seed values is used (unless fixed_seed has
    // been specified).  This renders the study repeatable but the sampling
    // pattern varies from one run to the next.
    if (numDACERuns == 1) { // set initial seed
      if (!seedSpec) // no user specification: random behavior
	// Generate initial seed from a system clock.  NOTE: the system clock
	// is not reused on subsequent invocations since (1) clock granularity
	// can be too coarse (can repeat on subsequent runs for inexpensive test
	// fns) and (2) seed progression can be highly structured, which could
	// induce correlation between sample sets.  Instead, the clock-generated
	// case uses the same seed progression procedure below as the
	// user-specified case.  This has the additional benefit that a random
	// run can be recreated by specifying the clock-generated seed in the
	// input file.
#ifdef HAVE_DDACE
	randomSeed = 1 + DistributionBase::timeSeed(); // microsecs, time of day
#elif defined(DAKOTA_UTILIB)
        randomSeed = 1 + (int)CurrentTime(); // secs, time of day
#else
        randomSeed = 1 + (int)clock(); // last resort: clock ticks, exec time
#endif
    }
    else if (varyPattern) { // define sequence of seed values for numLHSRuns > 1
      // It would be preferable to call srand() only once and then call rand()
      // for each execution (the intended usage model), but possible interaction
      // with other uses of rand() in ANN, SGOPT, APPS, etc. is a concern. E.g.,
      // an srand(clock()) executed elsewhere would ruin the repeatability of
      // the NonDSampling seed sequence.  The only way to insure isolation is to
      // reseed each time.  Any induced correlation should be inconsequential
      // for the intended use.
      std::srand(randomSeed);
      randomSeed = 1 + std::rand(); // from 1 to RANDMAX+1
    }
    Cout << "\nPSUADE DACE method = " << methodName << " Samples = " 
	 << numSamples;
    if (numDACERuns == 1 || !varyPattern) {
      if (seedSpec) Cout << " Seed (user-specified) = ";
      else          Cout << " Seed (system-generated) = ";
    }
    else {
      if (seedSpec) Cout << " Seed (sequence from user-specified) = ";
      else          Cout << " Seed (sequence from system-generated) = ";
    }
    Cout << randomSeed << "\n            Partitions = " << numPartitions
	 << " (Levels = " << numPartitions+1 << ")\n";

    // define the data structure necessary for generating a PSUADE MOAT sample
    // may need some more care here with passing pointers to the model data
    sData psuade_sdata;
    psuade_sdata.nInputs_  = numContinuousVars;
    psuade_sdata.nSamples_ = numSamples;

    psuade_sdata.iLowerB_ = new double [numContinuousVars];
    psuade_sdata.iUpperB_ = new double [numContinuousVars];
    for (i=0; i<numContinuousVars; i++) {
      psuade_sdata.iLowerB_[i] = c_l_bnds[i];
      psuade_sdata.iUpperB_[i] = c_u_bnds[i];
    }

    // instantiate and initialize the sampler
    // MOAT works with (even) number of levels instead of partitions
    MOATSampling *psuadeSampler = new MOATSampling(randomSeed, numPartitions+1);
    psuadeSampler->initialize(psuade_sdata);

    // extract samples from MOAT (could move this into a copy_data function)
    if (allSamples.numRows() != numContinuousVars ||
	allSamples.numCols() != numSamples)
      allSamples.shapeUninitialized(numContinuousVars, numSamples);
    for (i=0; i<numSamples; ++i) {
      Real* all_samp_i = allSamples[i];
      for (j=0; j<numContinuousVars; ++j)
	all_samp_i[j] = psuadeSampler->sample_matrix(i,j);
    }

    // no longer need the sampler
    delete [] psuade_sdata.iLowerB_;
    delete [] psuade_sdata.iUpperB_;
    delete psuadeSampler;
    psuadeSampler = NULL;

#ifdef DAKOTA_DEBUG
    Cout << "PSUADE MOAT Sample Points \n";
    for (i=0; i<numSamples; i++) {
      if (i>0 && i%(numContinuousVars+1) == 0)
	Cout << "   new replicate --------------------------\n";
      write_data(Cout, allSamples[i], numContinuousVars); Cout << '\n';
    }
#endif
  }
}


/** Users may input a variety of quantities, but this function must
    enforce any restrictions imposed by the sampling algorithms. */
void PSUADEDesignCompExp::enforce_input_rules()
{
  // if no samples specified, make at 10*number of inputs
  // otherwise round up to next largest permissible number
  if (numSamples <= 0) {
    numSamples = 10*(numContinuousVars+1);
    Cout << "\nWarning: Number of samples not specified for PSUADE MOAT.\n"
	 << "         Resetting samples to " << numSamples 
	 << " 10*(num_cdv+1).\n";
  }
  else if (numSamples/(numContinuousVars+1)*(numContinuousVars+1) != 
	   numSamples) {
    int num_reps = numSamples/(numContinuousVars+1);
    numSamples = (num_reps+1)*(numContinuousVars+1);
    Cout << "\nWarning: PSUADE MOAT requires number of samples to be a "
	 << "multiple of num_cdv+1.\n         Resetting samples to " 
	 << numSamples <<".\n";
  }
  if (varPartitionsSpec.size() >= 1) {
    numPartitions = varPartitionsSpec[0];
    if (varPartitionsSpec.size() > 1)
      Cout << "\nWarning: PSUADE MOAT accepts one partition specification "
	   << "(which applies to all\n         variables). Taking first "
	   << "component.\n";
  }
  if (numPartitions <= 0) {
    numPartitions = 3;
    Cout << "\nWarning: PSUADE MOAT partitions must be positive.\n"
	 << "         Setting to" << " default partitions = 3 (levels = 4)."
	 << "\n";
  }
  else if (numPartitions % 2 == 0) {
    numPartitions++;
    Cout << "\nWarning: PSUADE MOAT partitions must be odd (even number of "
	 << "levels).\n         Setting to partitions = "
	 << numPartitions << " (levels = " << numPartitions+1 << ").\n";
  }
}

} // namespace Dakota
