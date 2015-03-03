/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQUESOBayesCalibration
//- Description: Derived class for Bayesian inference using QUESO
//- Owner:       Laura Swiler, Brian Adams
//- Checked by:
//- Version:

// place Dakota headers first to minimize influence of QUESO defines
#include "NonDQUESOBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaModel.hpp"
#include "DakotaApproximation.hpp"
#include "ProbabilityTransformation.hpp"
// then list QUESO headers
#include "queso/StatisticalInverseProblem.h"
#include "queso/StatisticalInverseProblemOptions.h"
#include "queso/MetropolisHastingsSGOptions.h"
#include "queso/SequenceStatisticalOptions.h"
#include "queso/GslVector.h"
#include "queso/GslMatrix.h"
#include "queso/Environment.h"
#include "queso/EnvironmentOptions.h"
#include "queso/Defines.h"
#include "queso/ValidationCycle.h"
#include "queso/GenericScalarFunction.h"
#include "queso/UniformVectorRV.h"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

// initialization of statics
NonDQUESOBayesCalibration* NonDQUESOBayesCalibration::NonDQUESOInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDQUESOBayesCalibration::
NonDQUESOBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  quesoStandardizedSpace(false),
  mcmcType(probDescDB.get_string("method.mcmc_type")),
  rejectionType(probDescDB.get_string("method.rejection")),
  metropolisType(probDescDB.get_string("method.metropolis")),
  // these two deprecated:
  likelihoodScale(probDescDB.get_real("method.likelihood_scale")),
  calibrateSigmaFlag(probDescDB.get_bool("method.nond.calibrate_sigma"))
{ 

  ////////////////////////////////////////////////////////
  // Step 1 of 5: Instantiate the QUESO environment 
  ////////////////////////////////////////////////////////
  init_queso_environment();
 
  // Read in all of the experimental data:  any x configuration 
  // variables, y observations, and y_std if available 
  bool calc_sigma_from_data = true; // calculate sigma if not provided
  expData.load_data("QUESO Bayes Calibration", calc_sigma_from_data);
  
  // for now, assume that if you are reading in any experimental 
  // standard deviations, you do NOT want to calibrate sigma terms
  // BMA TODO: this is wrong logic: need to check for != "none"
  // Also, need to sync up with data available in the covariance class.
  if (!varianceTypesRead.empty() && !calibrateSigmaFlag)
    calibrateSigmaFlag = false;
  // For now, set calcSigmaFlag to true: this should be read from input
  //calibrateSigmaFlag = true;

  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  init_parameter_domain();

  // initialize or update the proposal covariance; default init must
  // be done after parameter domain is initialized
  // TODO: In general if user gives proposal covariance; must be
  // transformed to scaled space in same way as variables
  const String& covariance_type = 
    probDescDB.get_string("method.nond.proposal_cov_type");
  if (covariance_type.empty())
    default_proposal_covariance();
  else {
    // either filename OR data values will be non-empty
    const RealVector& covariance_data = 
      probDescDB.get_rv("method.nond.proposal_covariance_data");
    const String& covariance_filename = 
      probDescDB.get_string("method.nond.proposal_cov_filename");
    user_proposal_covariance(covariance_type, covariance_data, 
			     covariance_filename);
  }
}


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::quantify_uncertainty()
{
  // instantiate QUESO objects and execute
  NonDQUESOInstance = this;

  // construct emulatorModel and initialize tranformations, as needed
  initialize_model();

  // construct likelihoodFunctionObj, prior/posterior random vectors,
  // and inverse problem
  init_queso_solver();

  // For testing re-entrancy
  //for (size_t i=0; i<2; ++i) {
  //  Cout << "QUESO Major Iteration " << i << std::endl;
  //  quesoEnv->resetSeed(randomSeed);


  switch (adaptPosteriorRefine) {
  case false:
    if (emulatorType)
      precondition_proposal();
    // solve the inverse problem by MCMC sampling
    run_queso_solver();
    break;
  case true:
    if (!emulatorType) {
      Cerr << "Error: " << std::endl;
      abort_handler(-1);
    }
    Real conv_metric = DBL_MAX; int num_iter = 0;
    while (conv_metric > convergenceTol && num_iter < maxIterations) {
      // use emulator Hessian data to precondition the proposal covariance
      precondition_proposal();
      // solve the inverse problem by MCMC sampling on the emulator
      run_queso_solver();
      // Assess convergence of the posterior via sample-based K-L divergence:
      //conv_metric = assess_posterior_convergence();
      // filter chain -or- extract full chain and sort on likelihood values.
      // Evaluate these MCMC samples with truth evals
      filter_chain();
      // update the emulator surrogate data with new truth evals and
      // reconstruct surrogate (e.g., via PCE sparse recovery)
      update_model();
      // Assess convergence of the posterior via convergence of the PCE coeffs
      //onv_metric = assess_emulator_convergence();
      ++num_iter;
    }
    break;
  }

  // For testing re-entrancy
  //}
}

void NonDQUESOBayesCalibration::init_queso_environment() {
  // NOTE:  for now we are assuming that DAKOTA will be run with 
  // mpiexec to call MPI_Init.  Eventually we need to generalize this 
  // and send QUESO the proper MPI subenvironments.

  // TODO: see if this can be a local, or if the env retains a pointer
  envOptionsValues.reset(new QUESO::EnvOptionsValues());
  envOptionsValues->m_subDisplayFileName   = "outputData/display";
  envOptionsValues->m_subDisplayAllowedSet.insert(0);
  envOptionsValues->m_subDisplayAllowedSet.insert(1);
  envOptionsValues->m_displayVerbosity     = 2;
  if (randomSeed) 
    envOptionsValues->m_seed                 = randomSeed;
  else
    envOptionsValues->m_seed                 = 1 + (int)clock(); 
 
  if (mcmcType== "dram")
    quesoEnv.reset(new QUESO::FullEnvironment(MPI_COMM_SELF,"","",
					      envOptionsValues.get()));
  else if (strends(mcmcType, "multilevel"))
    quesoEnv.reset(new QUESO::FullEnvironment(MPI_COMM_SELF,"ml.inp","",NULL));
  else {
    Cerr << "\nError (QUESO): Unknown MCMC type " << mcmcType << std::endl;
    abort_handler(-1);
  }
}

void NonDQUESOBayesCalibration::init_queso_solver()
{
  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  // routine computes [ln(function)]
  likelihoodFunctionObj.reset(new
    QUESO::GenericScalarFunction<QUESO::GslVector,QUESO::GslMatrix>("like_",
    *paramDomain, &dakotaLikelihoodRoutine, (void *) NULL, true));

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  // TODO: Map other Dakota uncertain types to QUESO priors in
  // set/update prior functions
  priorRv.reset(new QUESO::UniformVectorRV<QUESO::GslVector,QUESO::GslMatrix> 
		("prior_", *paramDomain));

  postRv.reset(new QUESO::GenericVectorRV<QUESO::GslVector,QUESO::GslMatrix>
	       ("post_", *paramSpace));
  // Q: are Prior/posterior RVs copied by StatInvProblem, such that these
  //    instances can be local and allowed to go out of scope?

  // define calIpOptionsValues
  set_inverse_problem_options();
  // set options specific to MH algorithm
  set_invpb_mh_options();
  // Inverse problem: instantiate it (posterior rv is instantiated internally)
  inverseProb.reset(new
    QUESO::StatisticalInverseProblem<QUESO::GslVector,QUESO::GslMatrix>("",
    calIpOptionsValues.get(), *priorRv, *likelihoodFunctionObj, *postRv));

  // To demonstrate retrieving the chain
  // TODO: Ask QUESO why the MH sequence generator decimates the
  // in-memory chain when generating final results
  //calIpMhOptionsValues->m_filteredChainGenerate = false; // **** HERE!
}


void NonDQUESOBayesCalibration::precondition_proposal()
{
  // TO DO: how are Hessians from multiple QoI combined?
  size_t i = 0; // for now
  std::vector<Approximation>& poly_approxs = emulatorModel.approximations();

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    const RealSymMatrix& hess_i
      = poly_approxs[i].hessian(emulatorModel.continuous_variables());
    proposal_covariance(hess_i);
    break;
  }
  }
}


void NonDQUESOBayesCalibration::run_queso_solver()
{
  Cout << "Running Bayesian Calibration with QUESO" << std::endl; 
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "using the following settings:"
	 << "\nQUESO standardized space " << quesoStandardizedSpace
	 << "\nMCMC type "<< mcmcType << "\nRejection type "<< rejectionType
	 << "\nMetropolis type " << metropolisType
	 << "\nNumber of samples in the MCMC Chain " << numSamples
	 << "\nCalibrate Sigma Flag " << calibrateSigmaFlag  << '\n';

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  if (mcmcType == "dram")
    inverseProb->solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(),
						  *paramInitials, 
						  proposalCovMatrix.get());
  else if (mcmcType == "multilevel")
    inverseProb->solveWithBayesMLSampling();

  // TODO: move to print_results
  Cout << "\nThe results of QUESO are in the outputData directory.\nThe file "
       << "display_sub0.txt contains information regarding the MCMC process.\n"
       << "The Matlab files contain the chain values.  The files to " 
       << "load in Matlab are file_cal_ip_raw.m (the actual chain) " 
       << "or file_cal_ip_filt.m (the filtered chain, which contains " 
       << "every 20th step in the chain.\n"
       << "NOTE:  the chain values in these Matlab files are currently " 
       << "in scaled space. \n  You will have to transform them back to "
       << "original space by:\n"
       << "lower_bounds + chain_values * (upper_bounds - lower_bounds)\n"
       << "The rejection rate is in the tgaCalOutput file.\n"
       << "We hope to improve the postprocessing of the chains by the " 
       << "next Dakota release.\n";
}


void NonDQUESOBayesCalibration::filter_chain()
{
  if (outputLevel >= DEBUG_OUTPUT) {
    // TODO: Need to transform chain back to unscaled space for
    // reporting to user; possibly also in auxilliary data files for
    // user consumption.

    // To demonstrate retrieving the chain. Note that the QUESO
    // VectorSequence class has a number of helpful filtering and
    // statistics functions.
    const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& 
      mcmc_chain = inverseProb->chain();
    unsigned int num_mcmc = mcmc_chain.subSequenceSize();
    Cout << "Final MCMC Samples: " << num_mcmc << std::endl;
    QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
    for (size_t chain_pos = 0; chain_pos < num_mcmc; ++chain_pos) {
      mcmc_chain.getPositionValues(chain_pos, mcmc_sample);
      Cout << mcmc_sample << std::endl;
    }

    // from BMA: can get the full acceptance chain (if filter = false) 
    //           but can't yet get corresponding likelihood values

    // Ask QUESO: appears to be empty, at least after filtering
    // const QUESO::ScalarSequence<double>&
    //   loglikelihood_vals = inverseProb->logLikelihoodValues();
    //    unsigned int num_llhood = loglikelihood_vals.subSequenceSize();
  }

  // TO DO: perform truth evals for selected points (here or below)
  // May be simplest to combine this fn with update_model()
}


void NonDQUESOBayesCalibration::update_model()
{
  // TO DO: perform truth evals for selected points (here or above)
  // May be simplest to combine this fn with filter_chain()

  //emulatorModel.append_approximation(vars_array, resp_map, rebuild_flag);

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    stochExpIterator.run(pl_iter); break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR:
    emulatorModel.build_approximation(); break;
  }
}


void NonDQUESOBayesCalibration::init_parameter_domain()
{
  int total_num_params = (calibrateSigmaFlag) ?
    numContinuousVars + numFunctions : numContinuousVars; 
  
  paramSpace.reset(new QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix>
		   (*quesoEnv, "param_", total_num_params, NULL));

  QUESO::GslVector paramMins(paramSpace->zeroVector());
  QUESO::GslVector paramMaxs(paramSpace->zeroVector());
  const RealVector& lower_bounds = emulatorModel.continuous_lower_bounds();
  const RealVector& upper_bounds = emulatorModel.continuous_upper_bounds();
  const RealVector& init_point = emulatorModel.continuous_variables();
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "\nInitial Point in original, unscaled space\n" << init_point
	 << "\nLower bounds of variables in original, unscaled space\n"
	 << lower_bounds << "\nUpper bounds of variables in original, "
	 << "unscaled space\n" << upper_bounds << '\n';

  if (emulatorType == GP_EMULATOR || emulatorType == KRIGING_EMULATOR ||
      emulatorType == NO_EMULATOR) {
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = lower_bounds[i];
      paramMaxs[i] = upper_bounds[i];
    }
  }
  else { // case PCE_EMULATOR: case SC_EMULATOR:
    Iterator* se_iter = NonDQUESOInstance->stochExpIterator.iterator_rep();
    // TODO: This transformation shouldn't be necessary, but need to
    // verify that ALL variable types are mapped to new bounds and
    // initial point when the PCE is constructed in u-space.
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = lower_bounds[i];
      paramMaxs[i] = upper_bounds[i];
    }
  }
  // the parameter domain will now be expanded by sigma terms if 
  // calibrateSigmaFlag is true
  if (calibrateSigmaFlag) {
    for (int j=0; j<numFunctions; j++){
      // TODO: Need to be able to get sigma from new covariance class.
      // Also need to sync up this with the logic in the ctor.  Also
      // need a default if there's no experimental data (may not be
      // sensible)
      Real std_0_j = expData.scalar_sigma(j, 0);
      paramMins[numContinuousVars+j] = 0.01*std_0_j;
      paramMaxs[numContinuousVars+j] = 2.0*std_0_j;
    }
  }
 
  // TODO: Update this after we decide how to manage the
  // transformation to standardized space.
  if (quesoStandardizedSpace){
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = 0.0;
      paramMaxs[i] = 1.0;
    }
  }
    
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "calibrateSigmaFlag  " << calibrateSigmaFlag
	 << "\nParameter bounds sent to QUESO (may be scaled):\nparamMins  "
	 << paramMins << "\nparamMaxs  " << paramMaxs << '\n';

  // instantiate QUESO parameters and likelihood
  paramDomain.reset(new QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix>
		    ("param_",*paramSpace,paramMins,paramMaxs));

  paramInitials.reset(new QUESO::GslVector(paramSpace->zeroVector()));
  for (int i=0; i<numContinuousVars; i++) {
    if (init_point[i]) 
      if (quesoStandardizedSpace)
        (*paramInitials)[i] = (init_point[i]-lower_bounds[i])/(upper_bounds[i]-lower_bounds[i]);
      else
        (*paramInitials)[i] = init_point[i];
    else 
      (*paramInitials)[i] = (paramMaxs[i]+paramMins[i])/2.0;
  }
  //for (int i=numContinuousVars;i<total_num_params;i++) {
  //  paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  //}
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Initial Parameter values sent to QUESO " 
	 << "(may be in scaled space) \n"  << *paramInitials << std::endl;
}


/** Must be called after paramMins/paramMaxs set above */
void NonDQUESOBayesCalibration::default_proposal_covariance()
{
  int total_num_params = paramSpace->dimGlobal();
  QUESO::GslVector covDiag(paramSpace->zeroVector());

  // default to covariance of independent uniforms (diagonal)
  const QUESO::GslVector& param_min = paramDomain->minValues();
  const QUESO::GslVector& param_max = paramDomain->maxValues();
  for (int i=0; i<total_num_params; i++)
    covDiag[i] = (param_max[i]-param_min[i])*(param_max[i]-param_min[i])/12.0;
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Diagonal elements of the proposal covariance " 
	 << "sent to QUESO (may be in scaled space) \n" << covDiag << '\n';

  proposalCovMatrix.reset(new QUESO::GslMatrix(covDiag));
  
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "QUESO ProposalCovMatrix " << '\n'; 
    for (size_t i=0; i<total_num_params; i++) {
      for (size_t j=0; j<total_num_params; j++) 
	Cout <<  (*proposalCovMatrix)(i,j) << "  "; 
      Cout << '\n'; 
    }
  }
}


void NonDQUESOBayesCalibration::
user_proposal_covariance(const String& cov_type, 
			 const RealVector& cov_data, 
			 const String& cov_filename)
{
  // TODO: Will need to scale user covariance if this method is
  // applying its own scaling.

  // this function will convert user-specified cov_type = "diagonal" |
  // "matrix" data from either cov_data or cov_filename and populate a
  // full QUESO::GslMatrix* in proposalCovMatrix with the covariance

  bool use_file = !cov_filename.empty();

  // Sanity check
  if( ("diagonal" != cov_type) &&
      ("matrix"   != cov_type) )
    throw std::runtime_error("User-specified covariance must have type of either \"diagonal\" of \"matrix\".  You have \""+cov_type+"\".");

  // Sanity check
  if( cov_data.length() && use_file )
    throw std::runtime_error("You cannot provide both covariance values and a covariance data filename.");

  // Size our Queso covariance matrix
  proposalCovMatrix.reset(new QUESO::GslMatrix(paramSpace->zeroVector()));

  // Sanity check
  int total_num_params = paramSpace->dimGlobal();
  if( (proposalCovMatrix->numRowsLocal()  != total_num_params) || 
      (proposalCovMatrix->numRowsGlobal() != total_num_params) || 
      (proposalCovMatrix->numCols()       != total_num_params)   )
    throw std::runtime_error("Queso vector space is not consistent with parameter dimension.");
        
  // Read in a general way and then check that the data is consistent
  RealVectorArray values_from_file;
  if( use_file )
  {
    std::ifstream s;
    TabularIO::open_file(s, cov_filename, "read_queso_covariance_data");
    bool row_major = false;
    read_unsized_data(s, values_from_file, row_major);
  }

  if( "diagonal" == cov_type )
  {
    if( use_file ) {
      // Sanity checks
      if( values_from_file.size() != 1 ) 
        throw std::runtime_error("\"diagonal\" Queso covariance file data should have 1 column and "
                                 +convert_to_string(total_num_params)+" rows.");
      if( values_from_file[0].length() != total_num_params )
        throw std::runtime_error("\"diagonal\" Queso covariance file data should have "
                                 +convert_to_string(total_num_params)+" rows.  Found "
                                 +convert_to_string(values_from_file[0].length())+" rows.");
      for( int i=0; i<total_num_params; ++i )
        (*proposalCovMatrix)(i,i) = values_from_file[0](i);
    }
    else {
      // Sanity check
      if( total_num_params != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(total_num_params)
                                 +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      for( int i=0; i<total_num_params; ++i )
        (*proposalCovMatrix)(i,i) = cov_data(i);
    }
  }
  else // "matrix" == cov_type
  {
    if( use_file ) {
      // Sanity checks
      if( values_from_file.size() != total_num_params ) 
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(total_num_params)+" columns.  Found "
                                 +convert_to_string(values_from_file.size())+" columns.");
      if( values_from_file[0].length() != total_num_params )
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(total_num_params)+" rows.  Found "
                                 +convert_to_string(values_from_file[0].length())+" rows.");
      for( int i=0; i<total_num_params; ++i )
        for( int j=0; j<total_num_params; ++j )
          (*proposalCovMatrix)(i,j) = values_from_file[i](j);
    }
    else {
      // Sanity check
      if( total_num_params*total_num_params != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(total_num_params*total_num_params)
            +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      int count = 0;
      for( int i=0; i<total_num_params; ++i )
        for( int j=0; j<total_num_params; ++j )
          (*proposalCovMatrix)(i,j) = cov_data[count++];
    }
  }

  // validate that provided data is a valid covariance - test symmetry
  //    Note: I had hoped to have this check occur in the call to chol() below, 
  //    but chol doesn't seem to mind that matrices are not symmetric... RWH
  //proposalCovMatrix->print(std::cout);
  //std::cout << std::endl;
  QUESO::GslMatrix test_mat = proposalCovMatrix->transpose();
  test_mat -= *proposalCovMatrix;
  if( test_mat.normMax() > 1.e-14 )
    throw std::runtime_error("Queso covariance matrix is not symmetric.");

  // validate that provided data is a valid covariance matrix - test PD part of SPD
  test_mat = *proposalCovMatrix;
  int ierr = test_mat.chol();
  if( ierr == QUESO::UQ_MATRIX_IS_NOT_POS_DEFINITE_RC)
    throw std::runtime_error("Queso covariance data is not SPD.");

  //proposalCovMatrix->print(std::cout);
  //std::cout << std::endl;
  //cov_data.print(std::cout);
}


void NonDQUESOBayesCalibration::
proposal_covariance(const RealSymMatrix& hessian)
{
  // set proposalCovMatrix to inv(hessian), validating that it is a
  // covariance matrix
}


/// set inverse problem options common to all solvers
void NonDQUESOBayesCalibration::set_inverse_problem_options() 
{
  calIpOptionsValues.reset(new QUESO::SipOptionsValues());
  //definitely want to retain computeSolution
  calIpOptionsValues->m_computeSolution      = true;
  calIpOptionsValues->m_dataOutputFileName   = "outputData/invpb_output";
  calIpOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpOptionsValues->m_dataOutputAllowedSet.insert(1);
}


void NonDQUESOBayesCalibration::set_invpb_mh_options() 
{

  calIpMhOptionsValues.reset(new QUESO::MhOptionsValues());

  calIpMhOptionsValues->m_dataOutputFileName   = "outputData/mh_output";
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(1);

  calIpMhOptionsValues->m_rawChainDataInputFileName     = ".";
  if (numSamples == 0)
    calIpMhOptionsValues->m_rawChainSize                  = 48576;
  else 
    calIpMhOptionsValues->m_rawChainSize                  = numSamples;
  //calIpMhOptionsValues->m_rawChainGenerateExtra         = false;
  //calIpMhOptionsValues->m_rawChainDisplayPeriod         = 20000;
  //calIpMhOptionsValues->m_rawChainMeasureRunTimes       = true;
  calIpMhOptionsValues->m_rawChainDataOutputFileName    = "outputData/raw_chain";
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(1);
  // NO LONGER SUPPORTED.  calIpMhOptionsValues->m_rawChainComputeStats          = true;

  //calIpMhOptionsValues->m_displayCandidates         = false;
  calIpMhOptionsValues->m_putOutOfBoundsInChain       = true;
  //calIpMhOptionsValues->m_tkUseLocalHessian         = false;
  //calIpMhOptionsValues->m_tkUseNewtonComponent      = true;
  if (strends(rejectionType, "standard"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 0;
  else if (strends(rejectionType, "delayed"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 1;
  calIpMhOptionsValues->m_drScalesForExtraStages.resize(1);
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 5;
  //calIpMhOptionsValues->m_drScalesForExtraStages[1] = 10;
  //calIpMhOptionsValues->m_drScalesForExtraStages[2] = 20;
  if (strends(metropolisType, "hastings"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 0;
  else if (strends(metropolisType, "adaptive"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 1;
  calIpMhOptionsValues->m_amAdaptInterval           = 100;
  calIpMhOptionsValues->m_amEta                     = 2.88;
  calIpMhOptionsValues->m_amEpsilon                 = 1.e-8;

  calIpMhOptionsValues->m_filteredChainGenerate              = true; // **** HERE!
  calIpMhOptionsValues->m_filteredChainDiscardedPortion      = 0.;
  calIpMhOptionsValues->m_filteredChainLag                   = 20; // **** HERE!
  calIpMhOptionsValues->m_filteredChainDataOutputFileName    = "outputData/filtered_chain";
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(1);
  //calIpMhOptionsValues->m_filteredChainComputeStats          = true;
}



//void NonDQUESOBayesCalibration::print_results(std::ostream& s)
//{
//  NonDBayesCalibration::print_results(s);
//
//  additional QUESO output
//}


double NonDQUESOBayesCalibration::dakotaLikelihoodRoutine(
  const QUESO::GslVector& paramValues,
  const QUESO::GslVector* paramDirection,
  const void*  functionDataPtr,
  QUESO::GslVector*       gradVector,
  QUESO::GslMatrix*       hessianMatrix,
  QUESO::GslVector*       hessianEffect)
{
  
  double result = 0.;
  size_t i,j;
  int num_exp = NonDQUESOInstance->numExperiments;
  int num_funcs = NonDQUESOInstance->numFunctions;
  int num_cont = NonDQUESOInstance->numContinuousVars; 
  RealVector x(num_cont);
  
  for (i=0; i<num_cont; i++) 
    x(i)=paramValues[i];
  
  if (NonDQUESOInstance->quesoStandardizedSpace){
    // TODO: need to transform sigmas back to unscaled space...
    const RealVector& xLow = NonDQUESOInstance->emulatorModel.continuous_lower_bounds();
    const RealVector& xHigh = NonDQUESOInstance->emulatorModel.continuous_upper_bounds();
    if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
      Cout << "Values of theta parameter QUESO is seeing" << x << '\n';
    for (i=0; i<num_cont; i++) 
      x(i)=xLow(i)+x(i)*(xHigh(i)-xLow(i));
    if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
      Cout << "Values of theta parameters DAKOTA uses" << x << '\n';
  }

  // The GP/KRIGING/NO EMULATOR case use an unstandardized space
  // (original) and the PCE or SC cases use a more general
  // standardized space.
  //
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).
  if (NonDQUESOInstance->emulatorType == GP_EMULATOR ||
      NonDQUESOInstance->emulatorType == KRIGING_EMULATOR ||
      NonDQUESOInstance->emulatorType == NO_EMULATOR)
    NonDQUESOInstance->emulatorModel.continuous_variables(x);
  else  //case PCE_EMULATOR: case SC_EMULATOR: 
    NonDQUESOInstance->emulatorModel.continuous_variables(x); 

  // Compute simulation response to use in likelihood 
  NonDQUESOInstance->emulatorModel.compute_response();
  const RealVector& fn_vals = 
    NonDQUESOInstance->emulatorModel.current_response().function_values();
  if (NonDQUESOInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "input is " << x << '\n';
    Cout << "output is " << fn_vals << '\n';
  } 

  // TODO: Update treatment of standard deviations as inference
  // vs. fixed parameters; also advanced use cases of calibrated
  // scalar sigma against user-provided covariance structure.

  // Calculate the likelihood depending on what information is
  // available for the standard deviations NOTE: If the calibration of
  // the sigma terms is included, we assume ONE sigma term per
  // function is calibrated.  Otherwise, we assume that yStdData has
  // already had the correct values placed depending if there is zero,
  // one, num_funcs, or a full num_exp*num_func matrix of standard
  // deviations.  Thus, we just have to iterate over this to calculate
  // the likelihood.
  if (NonDQUESOInstance->calibrateSigmaFlag) {
    for (i=0; i<num_exp; i++) {
      const RealVector& exp_data = NonDQUESOInstance->expData.all_data(i);
      for (j=0; j<num_funcs; j++)
        result += pow((fn_vals(j)-exp_data[j])/paramValues[num_cont+j],2.0);
    }
  }
  else {	
    for (i=0; i<num_exp; i++) {
      RealVector residuals;
      NonDQUESOInstance->expData.form_residuals(NonDQUESOInstance->emulatorModel.current_response(),i,residuals);
      result += NonDQUESOInstance->expData.apply_covariance(residuals, i);
    }
  }

  result = (result/(NonDQUESOInstance->likelihoodScale));
  result = -0.5*result;
  Cout << "Log likelihood is " << result << '\n';
  if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
    Cout << "Likelihood is " << exp(result) << '\n';
  
  // TODO: open file once and append here, or rely on QUESO to output
  if (NonDQUESOInstance->outputLevel > NORMAL_OUTPUT) {
    std::ofstream QuesoOutput;
    QuesoOutput.open("QuesoOutput.txt", std::ios::out | std::ios::app);
    for (i=0; i<num_cont; i++) 
      QuesoOutput << x(i) << ' ' ;
    for (j=0; j<num_funcs; j++)
      QuesoOutput << fn_vals(j) << ' ' ;
    QuesoOutput << result << '\n';
    QuesoOutput.close();
  }
  return result;
}

} // namespace Dakota
