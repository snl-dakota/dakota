/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       FSUDesignCompExp
//- Description: Implementation code for the FSUDesignCompExp class
//- Owner:       Laura Swiler, Sandia National Laboratories

#include "FSUDesignCompExp.hpp"
#include "dakota_system_defs.hpp"
#include "fsu.H"
#include "ProblemDescDB.hpp"
#ifdef HAVE_DDACE
#include "Distribution.h"
#elif defined(DAKOTA_UTILIB)
#include <utilib/seconds.h>
#endif
#ifdef _MSC_VER
#undef min
#endif

static const char rcsId[]="@(#) $Id: FSUDesignCompExp.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard iterator built with data from
    probDescDB. */
FSUDesignCompExp::FSUDesignCompExp(ProblemDescDB& problem_db, Model& model):
  PStudyDACE(problem_db, model),
  samplesSpec(probDescDB.get_int("method.samples")), numSamples(samplesSpec),
  allDataFlag(false), numDACERuns(0),
  latinizeFlag(probDescDB.get_bool("method.latinize"))
{
  switch (methodName) {
  case FSU_CVT: {
    // CVT inputs
    randomSeed   = seedSpec =  probDescDB.get_int("method.random_seed");
    varyPattern  = !probDescDB.get_bool("method.fixed_seed");
    numCVTTrials =  probDescDB.get_int("method.fsu_cvt.num_trials");

    // Map sample_type string to trialType integer
    const String& trial_type = probDescDB.get_string("method.trial_type");
    if (trial_type == "grid")
      trialType = 2;
    else if (trial_type == "halton")
      trialType = 1;
    else
      trialType = -1; // default is "random"
    break;
  }
  case FSU_HALTON: case FSU_HAMMERSLEY: {
    // QMC inputs
    sequenceStart =  probDescDB.get_iv("method.fsu_quasi_mc.sequenceStart");
    sequenceLeap  =  probDescDB.get_iv("method.fsu_quasi_mc.sequenceLeap");
    primeBase     =  probDescDB.get_iv("method.fsu_quasi_mc.primeBase");
    varyPattern   = !probDescDB.get_bool("method.fsu_quasi_mc.fixed_sequence");
    // perform error checks and initialize defaults
    if (sequenceStart.empty()) {
      sequenceStart.resize(numContinuousVars);
      sequenceStart = 0;
    }
    else if (sequenceStart.length() != numContinuousVars) {
      Cerr << "\nError: wrong number of sequence_start inputs.\n";
      abort_handler(-1);
    }
    if (sequenceLeap.empty()) {
      sequenceLeap.resize(numContinuousVars);
      sequenceLeap = 1;
    }
    else if (sequenceLeap.length() != numContinuousVars) {
      Cerr << "\nError: wrong number of sequence_leap inputs.\n";
      abort_handler(-1);
    }
    if (primeBase.empty()) {
      primeBase.resize(numContinuousVars);
      if (methodName == FSU_HALTON) 
	for (size_t i=0; i<numContinuousVars; i++)
	  primeBase[i] = prime(i+1);
      else { // fsu_hammersley
	primeBase[0] = -numSamples;
	for (size_t i=1; i<numContinuousVars; i++)
	  primeBase[i] = prime(i);
      }
    }
    else if (methodName == FSU_HALTON) {
      if (primeBase.length() != numContinuousVars) {
	Cerr << "\nError: wrong number of prime_base inputs.\n";
	abort_handler(-1);
      }
    }
    else { // fsu_hammersley
      if (primeBase.length() != numContinuousVars-1) {
	Cerr << "\nError: wrong number of prime_base inputs.\n";
	abort_handler(-1);
      }
      // modify user input
      primeBase.resize(numContinuousVars);
      for (size_t i=numContinuousVars-1; i>0; i--)
	primeBase[i] = primeBase[i-1]; // move each input back one position
      primeBase[0] = -numSamples;
      //Cout << primeBase;
    }
    break;
  }
  default:
    Cerr << "Error: FSU DACE method \"" << methodName << "\" is not an option."
	 << std::endl;
    abort_handler(-1);
  }

  if (numDiscreteIntVars > 0 || numDiscreteStringVars > 0 || 
      numDiscreteRealVars > 0) {
    Cerr << "\nError: fsu_* methods do not support discrete variables.\n";
    abort_handler(-1);
  }

  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


/** This alternate constructor is used for instantiations on-the-fly,
    using only the incoming data.  No problem description database
    queries are used. */
FSUDesignCompExp::
FSUDesignCompExp(Model& model, int samples, int seed,
		 unsigned short sampling_method):
  PStudyDACE(sampling_method, model), samplesSpec(samples), numSamples(samples),
  allDataFlag(true), numDACERuns(0), latinizeFlag(false), varyPattern(true)
{
  switch (methodName) {
  case FSU_CVT:
    // CVT inputs and defaults
    randomSeed   = seedSpec = seed;
    numCVTTrials = 10000;
    trialType    = -1; // default is "random"
    break;
  case FSU_HALTON: case FSU_HAMMERSLEY:
    // QMC inputs and defaults
    // initialize defaults
    sequenceStart.resize(numContinuousVars);
    sequenceStart = 0;
    sequenceLeap.resize(numContinuousVars);
    sequenceLeap = 1;
    primeBase.resize(numContinuousVars);
    if (methodName == FSU_HALTON) 
      for (size_t i=0; i<numContinuousVars; i++)
	primeBase[i] = prime(i+1);
    else { // fsu_hammersley
      primeBase[0] = -numSamples;
      for (size_t i=1; i<numContinuousVars; i++)
	primeBase[i] = prime(i);
    }
    break;
  default:
    Cerr << "Error: FSU DACE method \"" << methodName << "\" is not an option."
	 << std::endl;
    abort_handler(-1);
  }

  if (numDiscreteIntVars > 0 || numDiscreteStringVars > 0 || 
      numDiscreteRealVars > 0) {
    Cerr << "\nError: fsu_* methods do not support discrete variables.\n";
    abort_handler(-1);
  }

  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


FSUDesignCompExp::~FSUDesignCompExp() { }

bool FSUDesignCompExp::resize()
{
  bool parent_reinit_comms = PStudyDACE::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void FSUDesignCompExp::pre_run()
{
  Analyzer::pre_run();

  // error check on input parameters
  enforce_input_rules();

  // If VBD has been selected, generate a series of replicate parameter sets
  // (each of the size specified by the user) in order to compute VBD metrics.
  if (varBasedDecompFlag)
    get_vbd_parameter_sets(iteratedModel, numSamples);
  else
    get_parameter_sets(iteratedModel);
}


void FSUDesignCompExp::core_run()
{
  bool compute_corr_flag = (!subIteratorFlag),
    log_resp_flag = (allDataFlag || compute_corr_flag),
    log_best_flag = (numObjFns || numLSqTerms); // opt or NLS data set
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void FSUDesignCompExp::post_input()
{
  // error check on input parameters (make sure numSamples is valid)
  enforce_input_rules();

  // call convenience function from Analyzer
  read_variables_responses(numSamples, numContinuousVars);
}


void FSUDesignCompExp::post_run(std::ostream& s)
{
  // error check on input parameters (make sure numSamples is valid)
  enforce_input_rules();

  // BMA TODO: always compute all stats, even in VBD mode (stats on
  // first two replicates)
  if (varBasedDecompFlag)
    compute_vbd_stats(numSamples, allResponses);
  else {
    // compute correlation statistics if (compute_corr_flag)
    bool compute_corr_flag = (!subIteratorFlag);
    if (compute_corr_flag)
      pStudyDACESensGlobal.compute_correlations(allSamples, allResponses);
  }

  Analyzer::post_run(s);
}


void FSUDesignCompExp::get_parameter_sets(Model& model)
{
  get_parameter_sets(model, numSamples, allSamples);
}


void FSUDesignCompExp::get_parameter_sets(Model& model, const int num_samples,
					  RealMatrix& design_matrix)
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
  const RealVector& c_l_bnds = model.continuous_lower_bounds();
  const RealVector& c_u_bnds = model.continuous_upper_bounds();
  if (c_l_bnds.length() != numContinuousVars || 
      c_u_bnds.length() != numContinuousVars) {
    Cerr << "\nError: Mismatch in number of active variables and length of"
         << "\n       bounds arrays in FSUDesignCompExp." << std::endl;
    abort_handler(-1);
  }
  size_t i, j;
  RealVector c_bnds_range(numContinuousVars);
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  for (i=0; i<numContinuousVars; i++) {
    if (c_l_bnds[i] == -dbl_inf || c_u_bnds[i] == dbl_inf) {
      Cerr << "\nError: FSUDesignCompExp requires specification of variable "
	   << "bounds for all active variables." << std::endl;
      abort_handler(-1);
    }
    c_bnds_range[i] = c_u_bnds[i] - c_l_bnds[i];
  }

  //Real* sample_points = new Real [numContinuousVars*num_samples];
  if (design_matrix.numRows() != numContinuousVars ||
      design_matrix.numCols() != num_samples)
    design_matrix.shapeUninitialized(numContinuousVars, num_samples);

  switch (methodName) {
  case FSU_HALTON: {
    int qmc_step = (varyPattern) ? (numDACERuns-1)*num_samples+1 : 1;
    fsu_halton(numContinuousVars, num_samples, qmc_step, sequenceStart.values(),
	       sequenceLeap.values(), primeBase.values(), design_matrix.values());
    break;
  }
  case FSU_HAMMERSLEY: {
    int qmc_step = (varyPattern) ? (numDACERuns-1)*num_samples+1 : 1;
    fsu_hammersley(numContinuousVars, num_samples, qmc_step,
		   sequenceStart.values(), sequenceLeap.values(),
		   primeBase.values(), design_matrix.values());
    break;
  }
  case FSU_CVT: {

    // # of initialization sampes = # samples
    // some initialization types (grid) may truncate a pattern if the number
    // of samples is not a power of the number of variables

    // We no longer allow the user to specify an initialization 
    // type although the FSU CVT code allows it.  The reason is that
    // the method is fairly insensitive to initialization type and we wanted 
    // to simplify the interface as much as possible.  If it needs to 
    // be added later, it can be done using the "initialization_type" 
    // keyword and allowing it to be one of four options:  init_grid, 
    // init_halton, init_uniform, and init_random. 
    int init_type = 0; // random initialization is giving problems with seed 
                       // handling; current default is now uniform 

    // Trial type can be one of four values: random, grid, halton, uniform
    // random  = system rand() (to be replaced w/ BoostRNG)
    // uniform = John's personal RNG (simple + portable/reproducible)
    // --> for simplicity, uniform has been removed.

    // trial_type --> could hardwire to random as well

    // batch_size is no longer part of the spec; we now use:
    int batch_size = std::min(10000, numCVTTrials);
    if (numCVTTrials < num_samples)
      numCVTTrials = num_samples *10;

    // assign default maxIterations (DataMethod default is -1)
    if (maxIterations < 0)
      maxIterations = 25;

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
    Cout << "\nFSU DACE method = " << methodName << " Samples = " << num_samples;
    if (numDACERuns == 1 || !varyPattern) {
      if (seedSpec) Cout << " Seed (user-specified) = ";
      else          Cout << " Seed (system-generated) = ";
    }
    else {
      if (seedSpec) Cout << " Seed (sequence from user-specified) = ";
      else          Cout << " Seed (sequence from system-generated) = ";
    }
    Cout << randomSeed << '\n';

    int* p_seed = &randomSeed;
    int* diag_num_iter = new int; // CVT returns actual number of iterations

    // Now generate the array of samples
    fsu_cvt(numContinuousVars, num_samples, batch_size, init_type, trialType,
	    numCVTTrials, maxIterations, p_seed, design_matrix.values(),
	    diag_num_iter);

    p_seed = NULL;
    delete diag_num_iter;
    break;
  }
  }

  if (latinizeFlag)
    fsu_latinize(numContinuousVars, num_samples, design_matrix.values());

  if (volQualityFlag)
    volumetric_quality(numContinuousVars, num_samples, design_matrix.values());

  // Convert from [0,1] to [lower,upper]
  for (i=0; i<num_samples; ++i) {
    Real* samples_i = design_matrix[i];
    for (j=0; j<numContinuousVars; ++j)
      samples_i[j] = c_l_bnds[j] + samples_i[j] * c_bnds_range[j];
  }

  //delete [] sample_points;
}


/** Users may input a variety of quantities, but this function must
    enforce any restrictions imposed by the sampling algorithms. */
void FSUDesignCompExp::enforce_input_rules()
{
  // error check
  if (numSamples <= 0) {
    Cerr << "Error: number of DACE samples must be greater than zero."
	 << std::endl;
    abort_handler(-1);
  }

  if (methodName == FSU_CVT) {
    // no input rules yet
  }
  else { // QMC
    // check for duplication in primeBase sequence
    IntVector temp_prime(primeBase);
    std::sort(temp_prime.values(), temp_prime.values() + temp_prime.length());
    bool primeEqual = false;
    for (size_t i=0; i<numContinuousVars-1; i++)
      if (temp_prime[i] == temp_prime[i+1])
	primeEqual = true;
    if (primeEqual) {
      Cerr << "\nError: please specify unique prime numbers for Quasi-MC "
	   << "methods." << std::endl;
      abort_handler(-1);
    }
  }
}

} // namespace Dakota
