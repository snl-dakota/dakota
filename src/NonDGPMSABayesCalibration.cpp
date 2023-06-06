/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPMSABayesCalibration
//- Description: Derived class for Bayesian inference using QUESO/GPMSA
//- Owner:       Laura Swiler, Brian Adams
//- Checked by:
//- Version:

// place Dakota headers first to minimize influence of QUESO defines
#include "NonDGPMSABayesCalibration.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaModel.hpp"

// then system-related headers
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

// then list QUESO headers
#include "queso/GPMSA.h"
#include "queso/StatisticalInverseProblem.h"
#include "queso/GslVector.h"
#include "queso/GslMatrix.h"

using QUESO::GslVector;
using QUESO::GslMatrix;

static const char rcsId[]="@(#) $Id$";


/* TODO / QUESTIONS:

 * Write tests that use DOE vs import 

    - Notify users what data we're trying to read from tabular

 * A number of improvements in DACE / imported build data

    - Decide on meaning of buildSamples for read vs. DACE iterator control

    - Read build data in base class?

    - Read using residualModel's DataTransformModel vars/resp?

    - Handle string variables (convert to indices), probably be
      reading into a Variables object

    - Instead of assuming config vars = 0.5, use initial_state?

    - Read the build data early so we know buildSamples; would be nice
      to integrate with populating the data below so we can avoid the
      temporary

    - Document: theta, x, f ordering; document allVariables on DACE
      iterator

 * Implement m_realizer? We implement only what's needed to compute
   log prior, not realize()

 * Can one disable inference of the hyper-parameters?  Option to
   calibrate theta vs. config vars, e.g.,
   mhVarOptions->m_parameterDisabledSet.insert(0);

 * Normalization: Is it truly optional?  It's not clear what normalize
   means in context of functional data?  Do the config vars need to be
   normalized?  Need to verify that normalization works in single
   config and single experiment cases.

 * Functional data omission of scenarios; set to 0.5?
   Need to be sure that no other code is assuming these params don't exist...
   Also need to make sure to send a valid domain

 * BJW: How to handle LHS design generation?  Over all variables or
        just theta?  Probably all variables; that's being assumed in
        some places.

 * Can't use MAP pre-solve since don't have access to the likelihood?
   Perhaps get access through GPMSAFactory?

 * Allow basic vs. advanced user options
   ??? Appears only GPMSA allows default construct with parse override? 
   ??? Passed prefix is ignored by some EnvOptions ctors ???
   ??? Can't construct QUESO env with C++ options, then override from file...

 * Terminate handler might be getting called recursively in serial, at
   least when run in debugger.  Perhaps because old_terminate_handler
   is NULL and causing a segfault when called.

 * Emulator mean must be set to something to avoid nan/nan issue.
   Probably want to be able to omit parameters that aren't being
   calibrated statistically...

 * Allow variable Experiment sizes

 * Experiment covariance: correlation or covariance;
   normalized by simulation std deviation as in example?  Does this
   need to be defined if identity?

 * Export chain to named file

 */


namespace Dakota {

// initialization of statics
NonDGPMSABayesCalibration* NonDGPMSABayesCalibration::nonDGPMSAInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGPMSABayesCalibration::
NonDGPMSABayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDQUESOBayesCalibration(problem_db, model),
  buildSamples(probDescDB.get_int("method.build_samples")),
  approxImportFile(probDescDB.get_string("method.import_build_points_file")),
  approxImportFormat(probDescDB.get_ushort("method.import_build_format")),
  approxImportActiveOnly(
    probDescDB.get_bool("method.import_build_active_only")),
  userConfigVars(expData.num_config_vars()),
  gpmsaConfigVars(std::max(userConfigVars, (unsigned int) 1)),
  gpmsaNormalize(probDescDB.get_bool("method.nond.gpmsa_normalize"))
{   
  bool found_error = false;

  // Input spec should prevent this, but be sure.  It's possible
  // through user input that the inbound model is of type surrogate
  // and that's okay.
  if (emulatorType != NO_EMULATOR) {
    Cerr << "\nError: Dakota emulators not supported with GPMSA\n";
    found_error = true;
  }

  // TODO: Do we want to allow a single field group to allow the full
  // multi-variate case?
  const SharedResponseData& srd = model.current_response().shared_data();
  if (srd.num_field_response_groups() > 0 && outputLevel >= NORMAL_OUTPUT)
    Cout << "\nWarning: GPMSA does not yet treat field_responses; they will be "
	 << "treated as a\n         single multivariate response set."
	 << std::endl;

  // REQUIRE experiment data
  if (expData.num_experiments() < 1) {
    Cerr << "\nError: GPMSA requires experimental data\n";
    found_error = true;
  }

  if (userConfigVars > 0 && !approxImportFile.empty() &&
      approxImportActiveOnly && outputLevel >= NORMAL_OUTPUT)
    Cout << "\nWarning: Experimental data presented to GPMSA has configuration variables, but\n         simulation data import specifies active_only, so nominal values of\n         configuration variables will be used." << std::endl;

  if (found_error)
    abort_handler(METHOD_ERROR);

  // TODO: use base class to manage any problem transformations and
  // probably the surrogate build data management

  // TODO: conditionally enable sampler only if needed to augment
  // samples and allow both to be specified
  // if (approxImportFile.empty())
  //   buildSamples = probDescDB.get_int("method.build_samples");
  // else buildSamples will get set after reading the file at run-time

  // BMA TODO: should we always instantiate this or not?  Allow augmentation?
  int samples = approxImportFile.empty() ? buildSamples : 0;
  const String& rng = probDescDB.get_string("method.random_number_generator");
  unsigned short sample_type = SUBMETHOD_DEFAULT;
  lhsIter.assign_rep(std::make_shared<NonDLHSSampling>
		     (mcmcModel, sample_type, samples, randomSeed, rng));
}


NonDGPMSABayesCalibration::~NonDGPMSABayesCalibration()
{ }


void NonDGPMSABayesCalibration::derived_init_communicators(ParLevLIter pl_iter)
{
  // lhsIter uses NoDBBaseConstructor, so no need to manage DB list nodes
  // at this level
  lhsIter.init_communicators(pl_iter);
  NonDBayesCalibration::derived_init_communicators(pl_iter);
}


void NonDGPMSABayesCalibration::derived_set_communicators(ParLevLIter pl_iter)
{
  // lhsIter uses NoDBBaseConstructor, so no need to manage DB list nodes
  // at this level
  lhsIter.set_communicators(pl_iter);
  NonDBayesCalibration::derived_set_communicators(pl_iter);
}


void NonDGPMSABayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  NonDBayesCalibration::derived_free_communicators(pl_iter);
  lhsIter.free_communicators(pl_iter);
}


/** Perform the uncertainty quantification */
void NonDGPMSABayesCalibration::calibrate()
{
  // TODO: base class needs runtime update as well; decouple these:
  nonDQUESOInstance = this;
  nonDGPMSAInstance = this;

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << ">>>>> GPMSA: Setting up calibration." << std::endl;

  // GPMSA scenario space = configuration space. GPMSA requires at least 1
  // configuration variable, set to 0.5 for all scenarios if needed
  configSpace = std::make_shared<QUESO::VectorSpace<GslVector, GslMatrix>>
    (*quesoEnv, "scenario_", gpmsaConfigVars, nullptr);

  // GPMSA output space.  TODO: The simulation output space eta won't
  // always have same size as experiment space.  Moreover, each
  // experiment might have different size. Generalize this to
  // multi-experiment of varying size, and fields.
  unsigned int num_eta = numFunctions;
  nEtaSpace = std::make_shared<QUESO::VectorSpace<GslVector, GslMatrix>>
    (*quesoEnv, "output_", num_eta, nullptr);

  // BMA TODO: manage experiment size (each experiment need not have
  // the same size and one sim may inform multiple experiments through
  // DataTransform...)
  unsigned int experiment_size = expData.all_data(0).length();
  experimentSpace = std::make_shared<QUESO::VectorSpace<GslVector, GslMatrix>>
    (*quesoEnv,"experimentspace_", experiment_size, nullptr);

  // Construct with default options
  // default constructed options will have recommended settings, then
  // we can override via C++ API or input file (parse)
  gpmsaOptions = std::make_shared<QUESO::GPMSAOptions>();

  // C++ API options may override defaults
  // insert Dakota parameters here: gpmsaOptions.m_emulatorPrecisionShape

  // TODO: user option for min/max vs. mu/sigma?  Also, when user
  // gives bounds on distro or bounds on configs instead of data,
  // should we use those?  They might not be finite... For that
  // matter, autoscale may break on infinite domains?
  if (gpmsaNormalize) {
    for (unsigned int i = 0; i < (numContinuousVars + numHyperparams); ++i)
      gpmsaOptions->set_autoscale_minmax_uncertain_parameter(i);
    for (unsigned int i = 0; i < gpmsaConfigVars; ++i)
      gpmsaOptions->set_autoscale_minmax_scenario_parameter(i);
    for (unsigned int i = 0; i < num_eta; i++)
      gpmsaOptions->set_autoscale_meanvar_output(i);
  }

  // File-based power user parameters have the final say
  if (!advancedOptionsFile.empty())
    gpmsaOptions->parse(*quesoEnv, "");

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nGPMSA Final Options:" << *gpmsaOptions << std::endl;

  gpmsaFactory = std::make_shared<QUESO::GPMSAFactory<GslVector, GslMatrix>>
    (*quesoEnv, gpmsaOptions.get(), *priorRv, *configSpace,
     *paramSpace, *nEtaSpace, buildSamples,
     expData.num_experiments());

  // Populate simulation build data and experiment data
  fill_simulation_data();
  fill_experiment_data();

  // solver setup must follow factory instantiation
  init_queso_solver();

  // Override only the calibration parameter values with the ones the
  // user specified (or TODO: the map point if we pre-solve)
  GslVector full_param_initials(
    gpmsaFactory->prior().imageSet().vectorSpace().zeroVector());
  // Initial condition of the chain: overlay user params on default GPMSA values
  overlay_initial_params(full_param_initials);

  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "INFO (GPMSA): Final initial point\n [ "
	 << full_param_initials << " ]" << std::endl;

  GslMatrix full_proposal_cov(
    gpmsaFactory->prior().imageSet().vectorSpace().zeroVector());
  overlay_proposal_covariance(full_proposal_cov);

  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "INFO (GPMSA): Final proposal covariance matrix\n [ "
	 << full_proposal_cov << " ]" << std::endl;

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << ">>>>> GPMSA: Performing calibration with " << mcmcType << " using "
       << calIpMhOptionsValues->m_rawChainSize << " MCMC samples." << std::endl;
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "\n  Calibrating " << numHyperparams << " error hyperparameters."
	   << std::endl;
  }

  inverseProb->solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(), 
                                                full_param_initials,
                                                &full_proposal_cov);

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << ">>>>> GPMSA: Calibration complete. Generating statistics and ouput.\n";

    Cout << "  Info: MCMC details are in the QuesoDiagnostics directory:\n"
	 << "          display_sub0.txt contains MCMC diagnostics\n";
    if (standardizedSpace)
      Cout << "          Matlab files contain chain values (in "
	   << "standardized probability space)\n";
    else
      Cout << "          Matlab files contain chain values\n";

    Cout << "  Info: GPMSA cannot currently retrieve response function statistics."
	 << std::endl;
  }

  cache_acceptance_chain();
  //compute_statistics();
}


void NonDGPMSABayesCalibration::init_queso_solver() 
{
  // Note: The postRv includes GP-associated hyper-parameters
  postRv = std::make_shared<QUESO::GenericVectorRV<GslVector, GslMatrix>>
    ("post_", gpmsaFactory->prior().imageSet().vectorSpace());

  // TODO: consider what options are relevant for GPMSA vs. QUESO
  set_ip_options();
  set_mh_options();

  inverseProb = 
    std::make_shared<QUESO::StatisticalInverseProblem<GslVector, GslMatrix>>
    ("", calIpOptionsValues.get(), *gpmsaFactory, *postRv);
}



void NonDGPMSABayesCalibration::
overlay_proposal_covariance(GslMatrix& full_prop_cov) const
{
  // Start with the covariance matrix for the whole prior, including
  // GPMSA hyper-parameters.
  gpmsaFactory->prior().pdf().distributionVariance(full_prop_cov);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "INFO (GPMSA): Proposal covariance matrix from GPMSA prior:\n [ "
	 << full_prop_cov << " ]" << std::endl;

  // Now override with user (or iterative algorithm-updated values)
  unsigned int num_calib_params = numContinuousVars + numHyperparams;
  for (unsigned int i=0; i<num_calib_params; ++i)
    for (unsigned int j=0; j<num_calib_params; ++j)
      full_prop_cov(i,j) = (*proposalCovMatrix)(i,j);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "INFO (GPMSA): Proposal covariance matrix after overlay: [ \n"
	 << full_prop_cov << " ]" << std::endl;

  // Power users can override the covariance values with a file
  std::string initial_cov_filename =
    "initial_proposal_covariance_sub" + quesoEnv->subIdString();
  if (bfs::exists(initial_cov_filename + ".m")) {
    std::set<unsigned int> sid_set;
    sid_set.insert(quesoEnv->subId());
    full_prop_cov.subReadContents(initial_cov_filename, "m", sid_set);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "INFO (GPMSA): Initial proposal covariance overridden with values"
	   << " from " << (initial_cov_filename + ".m") << std::endl;
  }
}


void NonDGPMSABayesCalibration::
overlay_initial_params(GslVector& full_param_initials)
{
  // Start with the mean of the prior
  gpmsaFactory->prior().pdf().distributionMean(full_param_initials);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "INFO (GPMSA): Initial point from GPMSA prior:\n [ "
	 << full_param_initials << " ]" << std::endl;

  // But override whatever we want, e.g., with user-specified values:
  unsigned int num_calib_params = numContinuousVars + numHyperparams;;
  for (unsigned int i=0; i<num_calib_params; ++i)
    full_param_initials[i] = (*paramInitials)[i];

  // Example of manually adjusting initial values:
  // NOTE: This can also be done via file read in QUESO input file

  // full_param_initials[num_calib_params + 0]  = 0.4; // Emulator mean (unused, but set to avoid NaN!)
  // full_param_initials[num_calib_params + 1]  = 0.4; // emulator precision
  // full_param_initials[num_calib_params + 2]  = 0.4; // weights0 precision
  // full_param_initials[num_calib_params + 3]  = 0.4; // weights1 precision
  // full_param_initials[num_calib_params + 4]  = 0.97; // emulator corr str
  // full_param_initials[num_calib_params + 5]  = 0.97; // emulator corr str
  // full_param_initials[num_calib_params + 6]  = 0.97; // emulator corr str
  // full_param_initials[num_calib_params + 7]  = 0.97; // emulator corr str
  // full_param_initials[num_calib_params + 8]  = 0.20; // emulator corr str
  // full_param_initials[num_calib_params + 9]  = 0.80; // emulator corr str
  // full_param_initials[num_calib_params + 10] = 10.0; // discrepancy precision
  // full_param_initials[num_calib_params + 11] = 0.97; // discrepancy corr str
  // full_param_initials[num_calib_params + 12] = 8000.0; // emulator data precision
  // full_param_initials[num_calib_params + 13] = 1.0;  // observation error precision

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "INFO (GPMSA): Initial point after overlay:\n [ "
	 << full_param_initials << " ]" << std::endl;

  // Power users can override the initial values with a file
  std::string initial_point_filename =
    "initial_point_sub" + quesoEnv->subIdString();
  if (bfs::exists(initial_point_filename + ".m")) {
    std::set<unsigned int> sid_set;
    sid_set.insert(quesoEnv->subId());
    full_param_initials.subReadContents(initial_point_filename, "m", sid_set);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "INFO (GPMSA): Initial point overridden with values from " 
	   << (initial_point_filename + ".m") << std::endl;
  }
}


void NonDGPMSABayesCalibration::acquire_simulation_data(RealMatrix& sim_data)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << ">>>>> GPMSA: Acquiring simulation data." << std::endl;

  // Rationale: Trying to keep this agnostic to mocked up config vars...

  sim_data.shape(buildSamples,
		 numContinuousVars + userConfigVars + numFunctions);

  if (approxImportFile.empty()) {

    // NOTE: Assumes the design is performed over the config vars
    // TODO: make this modular on the dimensions of config vars...
    lhsIter.run(methodPCIter->mi_parallel_level_iterator(miPLIndex));
    const RealMatrix&  all_samples = lhsIter.all_samples();
    const IntResponseMap& all_resp = lhsIter.all_responses();

    if (all_samples.numCols() != buildSamples ||
        all_resp.size() != buildSamples) {
      Cerr << "\nError: GPMSA has insufficient surrogate build data.\n";
      abort_handler(-1);
    }

    IntRespMCIter resp_it = all_resp.begin();
    for (unsigned int i = 0; i < buildSamples; i++, ++resp_it) {
      for (int j=0; j<numContinuousVars; ++j)
	sim_data(i, j) = all_samples(j, i);
      for (int j=0; j<userConfigVars; ++j)
	sim_data(i, numContinuousVars + j) =
	  all_samples(numContinuousVars + j, i);
      for (int j=0; j<numFunctions; ++j)
	sim_data(i, numContinuousVars + userConfigVars + j) =
	  resp_it->second.function_values()[j];
    }

  }
  else {

    // Read surrogate build data ( theta, [x, ], fns ) depending on
    // active_only and number of user-specified config vars.  If
    // reading active_only, have to assume all simulations conducted
    // at nominal state variable values.
    size_t record_len = (approxImportActiveOnly) ?
      numContinuousVars + numFunctions :
      numContinuousVars + userConfigVars + numFunctions;
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "GPMSA: Importing simulation data from '" << approxImportFile
	   << "'\n       with " << numContinuousVars
	   << " calibration variable(s), " << userConfigVars
	   << " configuration variable(s),\n       and " << numFunctions
	   << " simulation output(s)." << std::endl;
    bool verbose = (outputLevel > NORMAL_OUTPUT);
    TabularIO::read_data_tabular(approxImportFile, "GMPSA simulation data",
				 sim_data, buildSamples, record_len,
				 approxImportFormat, verbose);
    // TODO: Have to fill in configuration variable values for
    // active_only, or error and move the function data over if so...
  }

}


void NonDGPMSABayesCalibration::fill_simulation_data()
{
  // simulations are described by configuration, parameters, output values
  std::vector<QUESO::SharedPtr<GslVector>::Type >
    sim_scenarios(buildSamples),  // config var values
    sim_params(buildSamples),     // theta var values
    sim_outputs(buildSamples);    // simulation output (response) values

  // Instantiate each of the simulation points/outputs
  for (unsigned int i = 0; i < buildSamples; i++) {
    sim_scenarios[i] = std::make_shared<GslVector>(configSpace->zeroVector());
    sim_params[i] = std::make_shared<GslVector>(paramSpace->zeroVector());
    sim_outputs[i] = std::make_shared<GslVector>(nEtaSpace->zeroVector()); // eta
  }

  /// simulation data, one row per simulation build sample, columns
  /// for calibration variables, configuration variables, function
  /// values (duplicates storage, but unifies import vs. DOE cases)
  RealMatrix sim_data;
  acquire_simulation_data(sim_data);

  for (int i=0; i<buildSamples; ++i) {

    for (int j=0; j<numContinuousVars; ++j)
       (*sim_params[i])[j] = sim_data(i, j);

    for (int j=0; j<gpmsaConfigVars; ++j)
       if (userConfigVars > 0)
	 (*sim_scenarios[i])[j] = sim_data(i, numContinuousVars + j);
       else
	 (*sim_scenarios[i])[j] = 0.5;

    for (int j=0; j<numFunctions; ++j)
      (*sim_outputs[i])[j] = 
	sim_data(i, numContinuousVars + userConfigVars + j);

  }

  gpmsaFactory->addSimulations(sim_scenarios, sim_params, sim_outputs);
}


void NonDGPMSABayesCalibration::fill_experiment_data()
{
  unsigned int num_experiments = expData.num_experiments();
  unsigned int experiment_size = expData.all_data(0).length();

  // characterization of the experiment data
  std::vector<QUESO::SharedPtr<GslVector>::Type > 
   exp_scenarios(num_experiments),   // config var values
   exp_outputs(num_experiments);        // experiment (response) values
  
  // BMA: Why is sim_outputs based on nEtaSpace?  Is simulation allowed
  // to be differently sized from experiments?

  // Load the information on the experiments (scenarios and data)
  // Scenarios omit string variables in this:
  std::vector<RealVector> exp_config_vars = expData.config_vars_as_real();
  for (unsigned int i = 0; i < num_experiments; i++) {

    exp_scenarios[i] = std::make_shared<GslVector>(configSpace->zeroVector());
    exp_outputs[i] = std::make_shared<GslVector>(experimentSpace->zeroVector());

    // NOTE: copy_gsl will resize the target objects rather than erroring.
    if (userConfigVars > 0)
      copy_gsl(exp_config_vars[i], *exp_scenarios[i]);
    else
      (*exp_scenarios[i])[0] = 0.5;

    copy_gsl(expData.all_data(i), *exp_outputs[i]);

  }
  // TODO: experiment space may not be numFunctions in field case

  // Experimental observation error covariance (default = I)
  QUESO::VectorSpace<GslVector, GslMatrix> 
    total_exp_space(*quesoEnv, "experimentspace_", 
                    num_experiments * experiment_size, nullptr);
  QUESO::SharedPtr<GslMatrix>::Type exp_covariance
    (new GslMatrix(total_exp_space.zeroVector(), 1.0));

  if (expData.variance_active()) {
    for (unsigned int i = 0; i < num_experiments; i++) {
      RealSymMatrix exp_cov;
      expData.covariance(i, exp_cov);
      for (unsigned int j = 0; j < experiment_size; j++) {
        for (unsigned int k = 0; k < experiment_size; k++) {
          (*exp_covariance)(experiment_size*i+j, experiment_size*i+k) =
            exp_cov(j,k);
        }
      }
    }
  }

  gpmsaFactory->addExperiments(exp_scenarios, exp_outputs, exp_covariance);

}


/** This is a subset of the base class retrieval, but we can't do the
    fn value lookups.  Eventually should be able to retrieve them from
    GPMSA. */
void NonDGPMSABayesCalibration::cache_acceptance_chain()
{
  int num_params = numContinuousVars + numHyperparams;

  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>&
    mcmc_chain = inverseProb->chain();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();

  if (num_mcmc != chainSamples && outputLevel >= NORMAL_OUTPUT) {
    Cout << "GPMSA Warning: Final chain is length " << num_mcmc 
	 << ", not expected length " << chainSamples << std::endl;
  }

  acceptanceChain.shapeUninitialized(numContinuousVars + numHyperparams,
				     chainSamples);
  acceptedFnVals.shapeUninitialized(numFunctions, chainSamples);

  // The posterior includes GPMSA hyper-parameters, so use the postRv space
  QUESO::GslVector qv(postRv->imageSet().vectorSpace().zeroVector());
  RealVector nan_fn_vals(numFunctions);
  nan_fn_vals = std::numeric_limits<double>::quiet_NaN();

  for (int i=0; i<chainSamples; ++i) {

    // translate the QUESO vector into x-space acceptanceChain
    mcmc_chain.getPositionValues(i, qv); // extract GSLVector from sequence
    if (standardizedSpace) {
      // u_rv and x_rv omit any hyper-parameters
      RealVector u_rv(numContinuousVars, false);
      copy_gsl_partial(qv, 0, u_rv);
      Real* acc_chain_i = acceptanceChain[i];
      RealVector x_rv(Teuchos::View, acc_chain_i, numContinuousVars);
      mcmcModel.trans_U_to_X(u_rv, x_rv);
      for (int j=numContinuousVars; j<num_params; ++j)
	acc_chain_i[j] = qv[j]; // trailing hyperparams are not transformed
    }
    else {
      // A view that includes calibration params and Dakota-managed
      // hyper-parameters, to facilitate copying from the longer qv
      // into acceptanceChain:
      RealVector theta_hp(Teuchos::View, acceptanceChain[i], num_params);
      copy_gsl_partial(qv, 0, theta_hp);
    }

    // TODO: Find a way to set meaningful function values
    Teuchos::setCol(nan_fn_vals, i, acceptedFnVals);
  }
}

void NonDGPMSABayesCalibration::
print_results(std::ostream& s, short results_state)
{
  //  TODO: additional GPMSA output

  NonDBayesCalibration::print_results(s, results_state);
}

} // namespace Dakota
