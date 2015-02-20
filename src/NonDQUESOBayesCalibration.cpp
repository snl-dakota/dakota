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
#include "DakotaModel.hpp"
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
  quesoStandardizedSpace(true),
  mcmcType(probDescDB.get_string("method.mcmc_type")),
  rejectionType(probDescDB.get_string("method.rejection")),
  metropolisType(probDescDB.get_string("method.metropolis")),
  // these two deprecated:
  proposalCovScale(probDescDB.get_rv("method.nond.proposal_covariance_scale")),
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
  // construct emulatorModel, if needed
  // TODO: consider a separate helper function for this 
  NonDBayesCalibration::quantify_uncertainty();
  // instantiate QUESO objects and execute
  NonDQUESOInstance = this;

  Cout << "Running Bayesian Calibration with QUESO" << std::endl; 
  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "using the following settings: " << '\n';
    Cout << "QUESO standardized space " << quesoStandardizedSpace << '\n';
    Cout << "MCMC type "<< mcmcType << '\n';
    Cout << "Rejection type "<< rejectionType << '\n';
    Cout << "Metropolis type " << metropolisType << '\n';
    Cout << "Number of samples in the MCMC Chain " << numSamples << '\n';
    Cout << "Calibrate Sigma Flag " << calibrateSigmaFlag  << '\n';
  } 

  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  // BMA: Left this in quantify_uncertainty since low overhead
  QUESO::GenericScalarFunction<QUESO::GslVector,QUESO::GslMatrix>
    likelihoodFunctionObj("like_", *paramDomain, &dakotaLikelihoodRoutine,
                          (void *) NULL, true); // routine computes [ln(function)]

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  // For testing re-entrancy
  // for (size_t i=0; i<2; ++i) {
  //   Cout << "QUESO Major Iteration " << i << std::endl;
  //   quesoEnv->resetSeed(randomSeed);
    
  // BMA: Left this in quantify_uncertainty, since may be iteratively updated
  // TODO: Map other Dakota uncertain types to QUESO priors in
  // set/update prior functions
  QUESO::UniformVectorRV<QUESO::GslVector,QUESO::GslMatrix> 
    priorRv("prior_", *paramDomain);

  // TODO: update with PCE Hessian
  //  RealMatrix hessian;
  //  proposal_covariance(hessian);

  QUESO::GenericVectorRV<QUESO::GslVector,QUESO::GslMatrix>
    postRv("post_", *paramSpace);

  // define calIpOptionsValues
  set_inverse_problem_options();
  // set options specific to MH algorithm
  set_invpb_mh_options();
  // Inverse problem: instantiate it (posterior rv is instantiated internally)
  QUESO::StatisticalInverseProblem<QUESO::GslVector,QUESO::GslMatrix>
    inv_pb("", calIpOptionsValues.get(), priorRv, likelihoodFunctionObj, postRv);
  
  // To demonstrate retrieving the chain
  // TODO: Ask QUESO why the MH sequence generator decimates the
  // in-memory chain when generating final results
  //calIpMhOptionsValues->m_filteredChainGenerate              = false;

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  if (mcmcType == "dram")
    inv_pb.solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(),
					    *paramInitials, 
					    proposalCovMatrix.get());
  else if (mcmcType == "multilevel")
    inv_pb.solveWithBayesMLSampling();

  if (outputLevel >= DEBUG_OUTPUT) {
    // To demonstrate retrieving the chain. Note that the QUESO
    // VectorSequence class has a number of helpful filtering and
    // statistics functions.
    const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& 
      mcmc_chain = inv_pb.chain();
    unsigned int num_mcmc = mcmc_chain.subSequenceSize();
    Cout << "Final MCMC Samples: " << num_mcmc << std::endl;
    QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
    for (size_t chain_pos = 0; chain_pos < num_mcmc; ++chain_pos) {
      mcmc_chain.getPositionValues(chain_pos, mcmc_sample);
      Cout << mcmc_sample << std::endl;
    }

    // Ask QUESO: appears to be empty, at least after filtering
    // const QUESO::ScalarSequence<double>&
    //   loglikelihood_vals = inv_pb.logLikelihoodValues();
    //    unsigned int num_llhood = loglikelihood_vals.subSequenceSize();
  }

  // For testing re-entrancy
  //}

  // TODO: move to print_results
  Cout << "\nThe results of QUESO are in the outputData directory.\n" 
       <<  "The file display_sub0.txt contains information regarding"
       <<  " the MCMC process. " << '\n';
  Cout << "The Matlab files contain the chain values.  The files to " 
       << "load in Matlab are file_cal_ip_raw.m (the actual chain) " 
       << "or file_cal_ip_filt.m (the filtered chain, which contains " 
       << "every 20th step in the chain." << '\n';
  Cout << "NOTE:  the chain values in these Matlab files are currently " 
       << "in scaled space. \n  You will have to transform them back to "
       << "original space by:" << '\n';
  Cout << "lower_bounds + chain_values * (upper_bounds - lower_bounds)" <<'\n'; 
  Cout << "The rejection rate is in the tgaCalOutput file. " << '\n';
  Cout << "We hope to improve the postprocessing of the chains by the " 
       << "next Dakota release. " << '\n';
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

void NonDQUESOBayesCalibration::init_parameter_domain()
{
  int total_num_params;
  if (calibrateSigmaFlag) 
    total_num_params = numContinuousVars + numFunctions;
  else 
    total_num_params = numContinuousVars; 
  
  paramSpace.reset(new QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix>
		   (*quesoEnv, "param_", total_num_params, NULL));

  QUESO::GslVector paramMins(paramSpace->zeroVector());
  QUESO::GslVector paramMaxs(paramSpace->zeroVector());
  const RealVector& lower_bounds = emulatorModel.continuous_lower_bounds();
  const RealVector& upper_bounds = emulatorModel.continuous_upper_bounds();
  const RealVector& init_point = emulatorModel.continuous_variables();
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "\nInitial Point in original, unscaled space "  << '\n'
	 << init_point << '\n';
    Cout << "Lower bounds of variables in original, unscaled space "  << '\n'
	 << lower_bounds << '\n';
    Cout << "Upper bounds of variables in original, unscaled space "  << '\n'
	 << upper_bounds << '\n';
  }

  if (emulatorType == GP_EMULATOR || emulatorType == KRIGING_EMULATOR ||
      emulatorType == NO_EMULATOR) {
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = lower_bounds[i];
      paramMaxs[i] = upper_bounds[i];
    }
  }
  else { // case PCE_EMULATOR: case SC_EMULATOR:
    Iterator* se_iter = NonDQUESOInstance->stochExpIterator.iterator_rep();
    //Pecos::ProbabilityTransformation& nataf = ((NonD*)se_iter)->variable_transformation(); 
    //RealVector lower_u, upper_u;
    //nataf.trans_X_to_U(lower_bounds,lower_u);
    //nataf.trans_X_to_U(upper_bounds,upper_u);
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = lower_bounds[i];
      paramMaxs[i] = upper_bounds[i];
//      paramMins[i]=lower_u[i];
//      paramMaxs[i]=upper_u[i];
    }
  }
  // the parameter domain will now be expanded by sigma terms if 
  // calibrateSigmaFlag is true
  if (calibrateSigmaFlag) {
    for (int j=0; j<numFunctions; j++){
      Real std_0_j = expData.scalar_sigma(j, 0);
      paramMins[numContinuousVars+j] = 0.01*std_0_j;
      paramMaxs[numContinuousVars+j] = 2.0*std_0_j;
    }
  }
 
  if (quesoStandardizedSpace){
    for (size_t i=0; i<numContinuousVars; i++) {
      paramMins[i] = 0.0;
      paramMaxs[i] = 1.0;
      //init_point[i]=(init_point[i]-lower_bounds[i])/(upper_bounds[i]-lower_bounds[i]);
    }
  }
    
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "calibrateSigmaFlag  " << calibrateSigmaFlag << '\n';
    Cout << "Parameter bounds sent to QUESO (may be scaled): " << '\n'; 
    Cout << "paramMins  " << paramMins << '\n';
    Cout << "paramMaxs  " << paramMaxs << '\n';
  }

  // instantiate QUESO parameters and likelihood
  paramDomain.reset(new QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix>
		    ("param_",*paramSpace,paramMins,paramMaxs));

  paramInitials.reset(new QUESO::GslVector(paramSpace->zeroVector()));
  for (int i=0; i<numContinuousVars; i++) {
    if (init_point[i]) 
      if (!quesoStandardizedSpace)
        (*paramInitials)[i] = init_point[i];
      else
        (*paramInitials)[i] = (init_point[i]-lower_bounds[i])/(upper_bounds[i]-lower_bounds[i]);
      //paramInitials[i] = (paramMaxs[i]+paramMins[i])/2.0;
    else 
      (*paramInitials)[i] = (paramMaxs[i]+paramMins[i])/2.0;
  }
  //for (int i=numContinuousVars;i<total_num_params;i++) {
  //  paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  //}
  Cout << "Initial Parameter values sent to QUESO " 
       << "(may be in scaled space) \n"  << *paramInitials << '\n';
}


/// must be called after paramMins/paramMaxs set above
void NonDQUESOBayesCalibration::default_proposal_covariance()
{
  int total_num_params = paramSpace->dimGlobal();
  QUESO::GslVector covDiag(paramSpace->zeroVector());
  //QUESO::GslMatrix proposalCovMatrix(paramSpace.zeroVector());

  // TODO: remove deprecated proposal covariance scale

  if (!proposalCovScale.empty()) {
    Cout << "Scaling factors for the proposal covariance " 
         << "in the original space. \n" << proposalCovScale << '\n';
  }
  if (!proposalCovScale.empty()) {
    if (!quesoStandardizedSpace) 
      for (int i=0; i<total_num_params; i++) {
        covDiag[i] = proposalCovScale[i];
      }
    else {
      const RealVector& lower_bounds = emulatorModel.continuous_lower_bounds();
      const RealVector& upper_bounds = emulatorModel.continuous_upper_bounds();
      for (int i=0; i<total_num_params; i++) {
        covDiag[i] = 
	  proposalCovScale[i]/
	  ((upper_bounds[i]-lower_bounds[i])*(upper_bounds[i]-lower_bounds[i]));
      }
    }
  }
  else { 
    const QUESO::GslVector& param_min = paramDomain->minValues();
    const QUESO::GslVector& param_max = paramDomain->maxValues();
    for (int i=0; i<total_num_params; i++) {
      covDiag[i] =
	(1.0/12.0)*(param_max[i]-param_min[i])*(param_max[i]-param_min[i]);
    }
  }
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Diagonal elements of the proposal covariance " 
	 << "sent to QUESO (may be in scaled space) \n" << covDiag << '\n';

  proposalCovMatrix.reset(new QUESO::GslMatrix(covDiag));

  //  QUESO::GslMatrix* proposalCovMatrix = postRv.imageSet().vectorSpace().newProposalMatrix(&covDiag,&paramInitials); 
  //QUESO::GslMatrix proposalCovMatrix(covDiag);
  //(*proposalCovMatrix)(0,1)=0.0099;
  //(*proposalCovMatrix)(1,0)=0.0099;
  
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "ProposalCovMatrix " << '\n'; 
    for (size_t i=0; i<total_num_params; i++) {
      for (size_t j=0; j<total_num_params; j++) 
	Cout <<  (*proposalCovMatrix)(i,j) << "  " ; 
      Cout << '\n'; 
    }
  }
}


void NonDQUESOBayesCalibration::
user_proposal_covariance(const String& cov_type, 
			 const RealVector& cov_data, 
			 const String& cov_filename)
{
  // this function will convert user-specified cov_type = "diagonal" |
  // "matrix" data from either cov_data or cov_filename and populate a
  // full QUESO::GslMatrix* in proposalCovMatrix with the covariance

  // should validate that provided data is a valid covariance matrix (SPD)

  // default_proposal_covariance shows constructing from covDiag vector

  // see QUESO::GslMatrix for other possible helpful constructors
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
  calIpOptionsValues->m_dataOutputFileName   = "outputData/tgaCalOutput";
  calIpOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpOptionsValues->m_dataOutputAllowedSet.insert(1);
}


void NonDQUESOBayesCalibration::set_invpb_mh_options() 
{

  calIpMhOptionsValues.reset(new QUESO::MhOptionsValues());

  calIpMhOptionsValues->m_dataOutputFileName   = "outputData/tgaCalOutput";
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
  calIpMhOptionsValues->m_rawChainDataOutputFileName    = "outputData/file_cal_ip_raw";
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

  calIpMhOptionsValues->m_filteredChainGenerate              = true;
  calIpMhOptionsValues->m_filteredChainDiscardedPortion      = 0.;
  calIpMhOptionsValues->m_filteredChainLag                   = 20;
  calIpMhOptionsValues->m_filteredChainDataOutputFileName    = "outputData/file_cal_ip_filt";
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
  
  //Cout << "numExpStdDeviationsRead " << NonDQUESOInstance->numExpStdDeviationsRead << '\n';

  for (i=0; i<num_cont; i++) 
    x(i)=paramValues[i];
  
  if (NonDQUESOInstance->quesoStandardizedSpace){
    const RealVector& xLow = NonDQUESOInstance->emulatorModel.continuous_lower_bounds();
    const RealVector& xHigh = NonDQUESOInstance->emulatorModel.continuous_upper_bounds();
    if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
      Cout << "Values of theta parameter QUESO is seeing" << x << '\n';
    for (i=0; i<num_cont; i++) 
      x(i)=xLow(i)+x(i)*(xHigh(i)-xLow(i));
    if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
      Cout << "Values of theta parameters DAKOTA uses" << x << '\n';
  }

  // FOR NOW:  THE GP and the NO EMULATOR case use an unstandardized 
  // space (original) and the PCE or SC cases use a more general standardized space.  
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).
  // 
  if (NonDQUESOInstance->emulatorType == GP_EMULATOR ||
      NonDQUESOInstance->emulatorType == KRIGING_EMULATOR ||
      NonDQUESOInstance->emulatorType == NO_EMULATOR) {
    //const RealVector& xLow = NonDQUESOInstance->emulatorModel.continuous_lower_bounds();
    //const RealVector& xHigh = NonDQUESOInstance->emulatorModel.continuous_upper_bounds();
    //Cout << "Queso X" << x << '\n';
    //for (i=0; i<num_cont; i++) 
    //  x(i)=xLow(i)+x(i)*(xHigh(i)-xLow(i));
    NonDQUESOInstance->emulatorModel.continuous_variables(x); 
    //Cout << "DAKOTA X" << x << '\n';
  }
  else { //case PCE_EMULATOR: case SC_EMULATOR: 
    NonDQUESOInstance->emulatorModel.continuous_variables(x); 
      RealVector u(num_cont);
      for (i=0; i<num_cont; i++) 
        u(i)=paramValues[i];
      Iterator* se_iter = NonDQUESOInstance->stochExpIterator.iterator_rep();
      Pecos::ProbabilityTransformation& nataf = ((NonD*)se_iter)->variable_transformation(); 
      nataf.trans_U_to_X(u,x);
    //NonDQUESOInstance->emulatorModel.continuous_variables(x); 
  } 

  // Compute simulation response to use in likelihood 
  NonDQUESOInstance->emulatorModel.compute_response();
  RealVector fn_vals = NonDQUESOInstance->emulatorModel.current_response().function_values();
  //Cout << "input is " << x << '\n';
  //Cout << "output is " << fn_vals << '\n';
 
  // Calculate the likelihood depending on what information is available 
  // for the standard deviations
  // NOTE:  If the calibration of the sigma terms is included, we assume 
  // ONE sigma term per function is calibrated. 
  // Otherwise, we assume that yStdData has already had the correct values 
  // placed depending if there is zero, one, num_funcs, or a full num_exp*num_func 
  // matrix of standard deviations.  Thus, we just have to iterate over this to 
  // calculate the likelihood. 
  if (NonDQUESOInstance->calibrateSigmaFlag) {
    for (i=0; i<num_exp; i++) {
      const RealVector& exp_data = NonDQUESOInstance->expData.all_data(i);
      for (j=0; j<num_funcs; j++)
        result += pow((fn_vals(j)-exp_data[j])/paramValues[num_cont+j],2.0);
    }
  }
  else {	
    for (i=0; i<num_exp; i++) {
      // TODO: experiments may vary in length, but functions should be same
      const RealVector& exp_data = NonDQUESOInstance->expData.all_data(i);
      //Cout << " exp_data " << exp_data << '\n';
      RealVector residuals;
      residuals.resize(exp_data.length());
      for (j=0; j<residuals.length(); j++)
        residuals[j] = fn_vals[j] - exp_data[j];
      //Cout << " residuals " << residuals << '\n';
      result += NonDQUESOInstance->expData.apply_covariance(residuals, i);
    }
  }

  result = (result/(NonDQUESOInstance->likelihoodScale));
  result = -0.5*result;
  Cout << "Log likelihood is " << result << '\n';
  if (NonDQUESOInstance->outputLevel > VERBOSE_OUTPUT)
    Cout << "Likelihood is " << exp(result) << '\n';
  
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
