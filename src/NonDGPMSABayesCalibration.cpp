/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	     NonDGPMSABayesCalibration
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

 * Normalization optional?  It's not clear what normalize means in
   context of functional data?  Do the config vars need to be normalized?

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
  approxImportFile(probDescDB.get_string("method.import_build_points_file")),
  approxImportFormat(probDescDB.get_ushort("method.import_build_format")),
  approxImportActiveOnly(
    probDescDB.get_bool("method.import_build_active_only")),
  buildSamples(probDescDB.get_int("method.build_samples")),
  optionsFile(probDescDB.get_string("method.queso_options_file"))
{   
  // quesoEnv: Base class calls init_queso_environment().  May need to
  // override this to provide additional power-user options to
  // override Dakota options.

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

  if (expData.num_config_vars() > 0 && !approxImportFile.empty() &&
      approxImportActiveOnly && outputLevel >= NORMAL_OUTPUT)
    Cout << "\nWarning: Experimental data presented to GPMSA has configuration variables, but\n         simulation data import specifies active_only, so nominal values of\n         configuration variables will be used." << std::endl;

  if (!optionsFile.empty()) {
    if (boost::filesystem::exists(optionsFile)) {
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "Any GPMSA options in file '" << optionsFile 
	     << "' will override Dakota options." << std::endl;
    } else {
      Cerr << "\nError: GPMSA options_file '" << optionsFile 
	   << "' specified, but file not found.\n";
      found_error = true;
    }
  }

  if (found_error)
    abort_handler(-1);

  init_queso_environment(optionsFile);

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
  lhsIter.assign_rep(new NonDLHSSampling(mcmcModel, sample_type, samples, 
					 randomSeed, rng), false);
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
  // BMA TODO: base class needs runtime update as well
  nonDQUESOInstance = this;
  nonDGPMSAInstance = this;

  // no emulators will be setup, but need to initialize the prob transforms
  initialize_model();

  // paramSpace, paramMins/paramMaxs, paramDomain, paramInitials, priorRV
  // Note: The priorRV is only defined over the calibration parameters
  // (and any Dakota-managed hyper-parameters)
  init_parameter_domain();

  // proposal may depend on the parameter space properties
  init_proposal_covariance();

  // In the following a "scenario" is a "configuration"
  unsigned int num_experiments = expData.num_experiments();
  //  unsigned int num_uncertain_vars = paramSpace->dimGlobal();
  unsigned int num_simulations = buildSamples; // TODO: generalize
  // GPMSA requires at least 1 configuration variable, set to 0.5 for
  // all scenarios if needed
  unsigned int gpmsa_config_vars = std::max(expData.num_config_vars(),
					    (size_t) 1);

  // BMA TODO: manage experiment size (each experiment need not have
  // the same size and one sim may inform multiple experiments through
  // DataTransform...)
  unsigned int experiment_size = expData.all_data(0).length();

  // BMA TODO: Would be helpful for user to be unaware of the eta
  // space.  Can it be sized differently than experiment size?  I
  // suppose for field data.

  // eta appears to be the size of the similation output space (numFunctions)
  unsigned int num_eta = numFunctions;

  // Step 3: Instantiate the 'scenario' and 'output' spaces for simulation
  configSpace.reset(new QUESO::VectorSpace<GslVector, GslMatrix>
		    (*quesoEnv, "scenario_", gpmsa_config_vars, NULL));
  nEtaSpace.reset(new QUESO::VectorSpace<GslVector, GslMatrix>
                  (*quesoEnv, "output_", num_eta, NULL));

  // Step 4: Instantiate the 'output' space for the experiments
  experimentSpace.reset(new QUESO::VectorSpace<GslVector, GslMatrix>
                        (*quesoEnv,"experimentspace_", experiment_size, NULL));

  // TODO: generalize this to multi-experiment of varying size, and fields

  // Step 5: Instantiate the Gaussian process emulator object
  //
  // Regarding simulation scenario input values, the user should standardise
  // them so that they exist inside a hypercube.
  //
  // Regarding simulation output data, the user should transform it so that the
  // mean is zero and the variance is one.
  //
  // Regarding experimental scenario input values, the user should standardize
  // them so that they exist inside a hypercube.
  //
  // Regarding experimental data, the user should transformed it so that it has
  // zero mean and variance one.

  // GPMSA stores all the information about our simulation
  // data and experimental data.  It also stores default information about the
  // hyperparameter distributions.

  // default constructed options will have recommended settings, then
  // we can override via C++ API or input file (parse)
  boost::scoped_ptr<QUESO::GPMSAOptions> gp_opts(new QUESO::GPMSAOptions());
  // insert Dakota parameters here: gp_opts.m_emulatorPrecisionShape()
  // now override with file-based power user parameters
  if (!optionsFile.empty()) {
    // if (outputLevel >= DEBUG_OUTPUT) {
    //   Cout << "\nGPMSAOptions:\n";
    //   gp_opts->print(Cout);
    // }
    gp_opts->parse(*quesoEnv, "");
    // if (outputLevel >= DEBUG_OUTPUT) {
    //   Cout << "\nGPMSAOptions:\n";
    //   gp_opts->print(Cout);
    // }
  }

  gpmsaFactory.reset(new QUESO::GPMSAFactory<GslVector, GslMatrix>
                     (*quesoEnv, gp_opts.get(), *priorRv, *configSpace, 
                      *paramSpace, *nEtaSpace, *experimentSpace, num_simulations,
                      num_experiments));

  // Load the simulation build data and experiment data
  fill_simulation_data();
  fill_experiment_data();

  // solver setup requires the configured factory
  init_queso_solver();

  // Override only the calibration parameter values with the ones the
  // user specified (or TODO the map point if we pre-solve)
  GslVector full_param_initials(
    gpmsaFactory->prior().imageSet().vectorSpace().zeroVector());
  // Initial condition of the chain: overlay user params on default GPMSA values
  overlay_initial_params(full_param_initials);

  GslMatrix full_proposal_cov(
    gpmsaFactory->prior().imageSet().vectorSpace().zeroVector());
  overlay_proposal_covariance(full_proposal_cov);

  inverseProb->solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(), 
                                                full_param_initials,
                                                &full_proposal_cov);

  acceptanceChain.shapeUninitialized(numContinuousVars + numHyperparams,
				     chainSamples * chainCycles);
  acceptedFnVals.shapeUninitialized(numFunctions, chainSamples * chainCycles);
  size_t cycle_num = 1;
  aggregate_acceptance_chain(cycle_num);

  compute_statistics();
}


void NonDGPMSABayesCalibration::init_queso_solver() 
{
  postRv.reset(new QUESO::GenericVectorRV<GslVector, GslMatrix>
               ("post_", gpmsaFactory->prior().imageSet().vectorSpace()));

  // TODO: consider what options are relevant for GPMSA vs. QUESO
  set_ip_options();
  set_mh_options();

  // TODO: This postRV appears to include GP-associated
  // hyper-parameters that most users won't care about...
  inverseProb.reset(new QUESO::StatisticalInverseProblem<GslVector, GslMatrix>
                    ("", calIpOptionsValues.get(), *gpmsaFactory, *postRv));
}



void NonDGPMSABayesCalibration::
overlay_proposal_covariance(GslMatrix& full_prop_cov) const
{
  // Start with the covariance matrix for the whole prior, including
  // hyper-parameters.
  gpmsaFactory->prior().pdf().distributionVariance(full_prop_cov);

  // Now override with user (or iterative algorithm-updated values)
  // BMA TODO: make sure the base class proposal update is getting called...
  unsigned int num_calib_params = paramSpace->dimGlobal();
  for (unsigned int i=0; i<num_calib_params; ++i)
    for (unsigned int j=0; j<num_calib_params; ++j)
      full_prop_cov(i,j) = (*proposalCovMatrix)(i,j);
}


void NonDGPMSABayesCalibration::
overlay_initial_params(GslVector& full_param_initials)
{
  // Start with the mean of the prior
  gpmsaFactory->prior().pdf().distributionMean(full_param_initials);

  // But override whatever we want.
  unsigned int num_calib_params = paramSpace->dimGlobal();
  for (unsigned int i=0; i<num_calib_params; ++i)
    full_param_initials[i] = (*paramInitials)[i];

  // paramInitials[5]  = 0;   // Emulator mean, unused but don't leave it NaN!
  //  full_param_initials[num_calib_params] = 0.4;

  // The rest of these we'll override, not because we have to, but
  // because the regression gold standard predates distributionMean()
  // paramInitials[6]  = 0.4; // emulator precision
  // paramInitials[7]  = 0.4; // weights0 precision
  // paramInitials[8]  = 0.4; // weights1 precision
  // paramInitials[9]  = 0.97; // emulator corr str
  // paramInitials[10] = 0.97; // emulator corr str
  // paramInitials[11] = 0.97; // emulator corr str
  // paramInitials[12]  = 0.97; // emulator corr str
  // paramInitials[13]  = 0.20; // emulator corr str
  // paramInitials[14]  = 0.80; // emulator corr str
  // paramInitials[15]  = 10.0; // discrepancy precision
  // paramInitials[16]  = 0.97; // discrepancy corr str
  // paramInitials[17]  = 8000.0; // emulator data precision
  // // paramInitials[18]  = 1.0;  // observation error precision
}

void NonDGPMSABayesCalibration::fill_simulation_data()
{
  // this is wrong because it may include hyper-parameters
  //  unsigned int num_uncertain_vars = paramSpace->dimGlobal();
  unsigned int num_uncertain_vars = numContinuousVars;
  unsigned int num_simulations = buildSamples; // TODO: generalize
  unsigned int user_config_vars = expData.num_config_vars();
  // GPMSA requires at least 1 configuration variable, set to 0.5 for
  // all scenarios if needed
  bool no_config_vars = (user_config_vars < 1);
  unsigned int gpmsa_config_vars = std::max(user_config_vars, (unsigned int) 1);

  // simulations are described by configuration, parameters, output values
  std::vector<QUESO::SharedPtr<GslVector>::Type >
    sim_scenarios(num_simulations),  // config var values
    sim_params(num_simulations),     // theta var values
    sim_outputs(num_simulations);    // simulation output (response) values

  // Instantiate each of the simulation points/outputs
  for (unsigned int i = 0; i < num_simulations; i++) {
    sim_scenarios[i].reset(new GslVector(configSpace->zeroVector()));
    sim_params[i].reset(new GslVector(paramSpace->zeroVector()));
    sim_outputs[i].reset(new GslVector(nEtaSpace->zeroVector())); // eta
  }

  // Populate simulation data  (generate or load build data)
  if (approxImportFile.empty()) {

    // NOTE: Assumes the design is performed over the config vars
    lhsIter.run(methodPCIter->mi_parallel_level_iterator(miPLIndex));
    const RealMatrix&  all_samples = lhsIter.all_samples();
    const IntResponseMap& all_resp = lhsIter.all_responses();

    if (all_samples.numCols() != buildSamples ||
        all_resp.size() != buildSamples) {
      Cerr << "\nError: GPMSA has insufficient surrogate build data.\n";
      abort_handler(-1);
    }

    IntRespMCIter resp_it = all_resp.begin();
    for (unsigned int i = 0; i < num_simulations; i++, ++resp_it) {
      int bd_index = 0;
      for (int j=0; j<num_uncertain_vars; ++j, ++bd_index)
        (*sim_params[i])[j] = all_samples(bd_index, i);
      for (int j=0; j<gpmsa_config_vars; ++j, ++bd_index) {
        if (no_config_vars)
          (*sim_scenarios[i])[j] = 0.5;
        else
          (*sim_scenarios[i])[j] = all_samples(bd_index, i);
      }
      copy_gsl(resp_it->second.function_values(), *sim_outputs[i]);
    }

  }
  else {

    // Read surrogate build data ( theta, [x, ], fns ) depending on
    // active_only and number of user-specified config vars.  If
    // reading active_only, have to assume all simulations conducted
    // at nominal state variable values.
    RealMatrix build_data;
    size_t record_len = (approxImportActiveOnly) ?
      num_uncertain_vars + numFunctions :
      num_uncertain_vars + user_config_vars + numFunctions;
    bool verbose = (outputLevel > NORMAL_OUTPUT);
    TabularIO::read_data_tabular(approxImportFile, "GMPSA simulation data",
				 build_data, num_simulations, record_len,
				 approxImportFormat, verbose);

    for (unsigned int i = 0; i < num_simulations; i++) {
      int bd_index = 0;
      for (int j=0; j<num_uncertain_vars; ++j, ++bd_index)
        (*sim_params[i])[j] = build_data(i, bd_index);
      for (int j=0; j<gpmsa_config_vars; ++j, ++bd_index) {
        if (no_config_vars)
          (*sim_scenarios[i])[j] = 0.5;
        else
          (*sim_scenarios[i])[j] = build_data(i, bd_index);
      }
      for (int j=0; j<numFunctions; ++j, ++bd_index)
        (*sim_outputs[i])[j] = build_data(i, bd_index);

    }

  }

  gpmsaFactory->addSimulations(sim_scenarios, sim_params, sim_outputs);

}


void NonDGPMSABayesCalibration::fill_experiment_data()
{
  unsigned int num_experiments = expData.num_experiments();
  unsigned int user_config_vars = expData.num_config_vars();
  unsigned int experiment_size = expData.all_data(0).length();

  bool no_config_vars = (user_config_vars < 1);
  unsigned int gpmsa_config_vars = std::max(user_config_vars, (unsigned int) 1);

  // characterization of the experiment data
  std::vector<QUESO::SharedPtr<GslVector>::Type > 
   exp_scenarios(num_experiments),   // config var values
   exp_outputs(num_experiments);        // experiment (response) values
  
  // BMA: Why is sim_outputs based on nEtaSpace?  Is simulation allowed
  // to be differently sized from experiments?

  // Load the information on the experiments (scenarios and data)
  const RealVectorArray& exp_config_vars = expData.config_vars();
  for (unsigned int i = 0; i < num_experiments; i++) {

    exp_scenarios[i].reset(new GslVector(configSpace->zeroVector()));
    exp_outputs[i].reset(new GslVector(experimentSpace->zeroVector()));

    // NOTE: copy_gsl will resize the target objects rather than erroring.
    if (no_config_vars)
      (*exp_scenarios[i])[0] = 0.5;
    else
      copy_gsl(exp_config_vars[i], *exp_scenarios[i]);
    copy_gsl(expData.all_data(i), *exp_outputs[i]);

  }

  // Experimental observation error covariance (default = I)
  QUESO::VectorSpace<GslVector, GslMatrix> 
    total_exp_space(*quesoEnv, "experimentspace_", 
                    num_experiments * experiment_size, NULL);
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


//void NonDQUESOBayesCalibration::print_results(std::ostream& s)
//{
//  NonDBayesCalibration::print_results(s);
//
//  additional QUESO output
//}

} // namespace Dakota
