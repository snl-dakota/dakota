/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQUESOBayesCalibration
//- Description: Derived class for Bayesian inference using QUESO
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDQUESOBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "uqStatisticalInverseProblem.h"
#include "uqStatisticalInverseProblemOptions.h"
#include "uqMetropolisHastingsSGOptions.h"
#include "uqSequenceStatisticalOptions.h"
#include "uqGslVector.h"
#include "uqGslMatrix.h"
#include "uqEnvironment.h"
#include "uqEnvironmentOptions.h"
#include "uqDefines.h"
#include "uqValidationCycle.h"
#include "ProbabilityTransformation.hpp"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

//initialization of statics
NonDQUESOBayesCalibration* NonDQUESOBayesCalibration::NonDQUESOInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDQUESOBayesCalibration::NonDQUESOBayesCalibration(Model& model):
  NonDBayesCalibration(model),
  numSamples(probDescDB.get_int("method.samples")),
  rejectionType(probDescDB.get_string("method.rejection")),
  metropolisType(probDescDB.get_string("method.metropolis")),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  proposalCovScale(probDescDB.get_rv("method.nond.proposal_covariance_scale")),
  likelihoodScale(probDescDB.get_real("method.likelihood_scale")),
  calibrateSigmaFlag(probDescDB.get_bool("method.nond.calibrate_sigma"))
{ }


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::quantify_uncertainty()
{
  // construct emulatorModel, if needed
  NonDBayesCalibration::quantify_uncertainty();
 
  Cout << "Standardized space " << standardizedSpace << '\n';
  // instantiate QUESO objects and execute
  NonDQUESOInstance=this;
  Cout << "Rejection type  "<< rejectionType << '\n';
  Cout << "Metropolis type " << metropolisType << '\n';
  Cout << "Num Samples " << numSamples << '\n';
  Cout << "Calibrate Sigma Flag " << calibrateSigmaFlag  << '\n';
  // For now, set calcSigmaFlag to true: this should be read from input
  //calibrateSigmaFlag = true;
 
  ////////////////////////////////////////////////////////
  // Step 1 of 5: Instantiate the QUESO environment 
  ////////////////////////////////////////////////////////
  // NOTE:  for now we are assuming that DAKOTA will be run with 
  // mpiexec to call MPI_Init.  Eventually we need to generalize this 
  // and send QUESO the proper MPI subenvironments.

  uqEnvOptionsValuesClass* envOptionsValues = NULL;
  envOptionsValues = new uqEnvOptionsValuesClass();
  envOptionsValues->m_subDisplayFileName   = "outputData/display";
  envOptionsValues->m_subDisplayAllowedSet.insert(0);
  envOptionsValues->m_subDisplayAllowedSet.insert(1);
  envOptionsValues->m_displayVerbosity     = 2;
  if (randomSeed) 
    envOptionsValues->m_seed                 = randomSeed;
  else
    envOptionsValues->m_seed                 = 1 + (int)clock(); 
      
  uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"","",envOptionsValues);
 
  // Read in all of the experimental data:  any x configuration 
  // variables, y observations, and y_std if available 
  bool calc_sigma_from_data = true; // calculate sigma if not provided
  Cout << "numExperiment " << numExperiments << '\n';
  Cout << "numReplicates " << numReplicates << '\n';
  Cout << " expDataFileName " << expDataFileName << '\n';
  expData.load_scalar(expDataFileName, "QUESO Bayes Calibration",
		      numExperiments, numReplicates, 
		      numExpConfigVars, numFunctions, numExpStdDeviationsRead,
		      expDataFileAnnotated, calc_sigma_from_data,
		      outputLevel);
  
  // for now, assume that if you are reading in experimental 
  // standard deviations, you do NOT want to calibrate sigma terms
  if ((numExpStdDeviationsRead > 0) && !(calibrateSigmaFlag))
    calibrateSigmaFlag = false;
  
  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  int total_num_params;
  if (calibrateSigmaFlag) 
    total_num_params = numContinuousVars + numFunctions;
  else 
    total_num_params = numContinuousVars; 
  
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    paramSpace(*env, "param_", total_num_params, NULL);

  uqGslVectorClass paramMins(paramSpace.zeroVector());
  uqGslVectorClass paramMaxs(paramSpace.zeroVector());
  const RealVector& lower_bounds = emulatorModel.continuous_lower_bounds();
  const RealVector& upper_bounds = emulatorModel.continuous_upper_bounds();
  const RealVector& init_point = emulatorModel.continuous_variables();
  Cout << "Initial Points " << init_point << '\n';

  if (emulatorType == GAUSSIAN_PROCESS || emulatorType == KRIGING || emulatorType == NO_EMULATOR) {
    for (size_t i=0;i<numContinuousVars;i++) {
      paramMins[i]=lower_bounds[i];
      paramMaxs[i]=upper_bounds[i];
    }
  }
  else { // case POLYNOMIAL_CHAOS: case STOCHASTIC_COLLOCATION:
    Iterator* se_iter = NonDQUESOInstance->stochExpIterator.iterator_rep();
    Pecos::ProbabilityTransformation& nataf = ((NonD*)se_iter)->variable_transformation(); 
    RealVector lower_u, upper_u;
    nataf.trans_X_to_U(lower_bounds,lower_u);
    nataf.trans_X_to_U(upper_bounds,upper_u);
    for (size_t i=0;i<numContinuousVars;i++) {
//      paramMins[i]=lower_bounds[i];
//      paramMaxs[i]=upper_bounds[i];
      paramMins[i]=lower_u[i];
      paramMaxs[i]=upper_u[i];
    }
  }
  // the parameter domain will now be expanded by sigma terms if 
  // calibrateSigmaFlag is true
  if (calibrateSigmaFlag) {
    for (int j=0; j<numFunctions; j++){
      int replicate = 0;
      Real std_0_j = expData.scalar_sigma(j, 0, replicate);
      paramMins[numContinuousVars+j]=0.01*std_0_j;
      paramMaxs[numContinuousVars+j]=2.0*std_0_j;
    }
  }
 
  Cout << "calibrateSigmaFlag  " << calibrateSigmaFlag << '\n';
  Cout << "paramMins  " << paramMins << '\n';
  Cout << "paramMaxs  " << paramMaxs << '\n';
  // instantiate QUESO parameters and likelihood
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);
  
  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  uqGenericScalarFunctionClass<uqGslVectorClass,uqGslMatrixClass>
    likelihoodFunctionObj("like_",
                          paramDomain,
                          &dakotaLikelihoodRoutine,
                          (void *) NULL,
                          true); // routine computes [ln(function)]

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>
    priorRv("prior_", paramDomain);
  uqGenericVectorRVClass<uqGslVectorClass,uqGslMatrixClass>
    postRv("post_", paramSpace);
  // Inverse problem: instantiate it (posterior rv is instantiated internally)
  uqSipOptionsValuesClass* calIpOptionsValues = NULL;
  calIpOptionsValues = new uqSipOptionsValuesClass();
  //definitely want to retain computeSolution
  calIpOptionsValues->m_computeSolution      = true;
  calIpOptionsValues->m_dataOutputFileName   = "outputData/tgaCalOutput";
  calIpOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpOptionsValues->m_dataOutputAllowedSet.insert(1);

  uqStatisticalInverseProblemClass<uqGslVectorClass,uqGslMatrixClass>
    ip("", calIpOptionsValues, priorRv, likelihoodFunctionObj, postRv);

  uqMhOptionsValuesClass* calIpMhOptionsValues = NULL;

  calIpMhOptionsValues = new uqMhOptionsValuesClass();
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
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 6;
  if (strends(metropolisType, "hastings"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 0;
  else if (strends(metropolisType, "adaptive"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 1;
  calIpMhOptionsValues->m_amAdaptInterval           = 100;
  //calIpMhOptionsValues->m_amEta                     = 1.92;
  //calIpMhOptionsValues->m_amEpsilon                 = 1.e-5;

  calIpMhOptionsValues->m_filteredChainGenerate              = true;
  calIpMhOptionsValues->m_filteredChainDiscardedPortion      = 0.;
  calIpMhOptionsValues->m_filteredChainLag                   = 20;
  calIpMhOptionsValues->m_filteredChainDataOutputFileName    = "outputData/file_cal_ip_filt";
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(1);
  //calIpMhOptionsValues->m_filteredChainComputeStats          = true;

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  uqGslVectorClass paramInitials(paramSpace.zeroVector());
  uqGslVectorClass covDiag(paramSpace.zeroVector());
  //uqGslMatrixClass proposalCovMatrix(paramSpace.zeroVector());
  for (int i=0;i<numContinuousVars;i++) {
    if (init_point[i])
      paramInitials[i]=init_point[i];
    else 
      paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  }
  for (int i=numContinuousVars;i<total_num_params;i++) {
    paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  }
  Cout << "proposalCovScale " << proposalCovScale << '\n';
  if (!proposalCovScale.empty()) {
    for (int i=0;i<total_num_params;i++) {
      covDiag[i] =(1.0/12.0)*(paramMaxs[i]-paramMins[i])*(paramMaxs[i]-paramMins[i])*proposalCovScale[i];
    }
  }
  else { 
    for (int i=0;i<total_num_params;i++) {
      covDiag[i] =(1.0/12.0)*(paramMaxs[i]-paramMins[i])*(paramMaxs[i]-paramMins[i]);
    }
  }

  Cout << "covDiag " << covDiag << '\n';
  Cout << "initParams " << paramInitials << '\n';

  uqGslMatrixClass* proposalCovMatrix = postRv.imageSet().vectorSpace().newProposalMatrix(&covDiag,&paramInitials); 
  //uqGslMatrixClass proposalCovMatrix(covDiag);
  Cout << "ProposalCovMatrix " << '\n'; 
  for (size_t i=0;i<total_num_params;i++) {
    for (size_t j=0;j<total_num_params;j++) 
       Cout <<  (*proposalCovMatrix)(i,j) << "  " ; 
  }
  ip.solveWithBayesMetropolisHastings(calIpMhOptionsValues,
                                    paramInitials, proposalCovMatrix);

  //ip.solveWithBayesMLSampling();
  // Return
  delete proposalCovMatrix;
  delete calIpMhOptionsValues;
  //delete env;
  delete envOptionsValues;
  //MPI_Finalize();

  return;

}


//void NonDQUESOBayesCalibration::print_results(std::ostream& s)
//{
//  NonDBayesCalibration::print_results(s);
//
//  additional QUESO output
//}


double NonDQUESOBayesCalibration::dakotaLikelihoodRoutine(
  const uqGslVectorClass& paramValues,
  const uqGslVectorClass* paramDirection,
  const void*  functionDataPtr,
  uqGslVectorClass*       gradVector,
  uqGslMatrixClass*       hessianMatrix,
  uqGslVectorClass*       hessianEffect)
{
  
  double result = 0.;
  size_t i,j,k;
  int num_exp = NonDQUESOInstance->numExperiments;
  IntVector num_replicates = NonDQUESOInstance->numReplicates;
  int num_funcs = NonDQUESOInstance->numFunctions;
  int num_cont = NonDQUESOInstance->numContinuousVars; 
  RealVector x(num_cont);
  
  //Cout << "numExpStdDeviationsRead " << NonDQUESOInstance->numExpStdDeviationsRead << '\n';

  for (i=0; i<num_cont; i++) 
    x(i)=paramValues[i];
  
  // FOR NOW:  THE GP and the NO EMULATOR case use an unstandardized 
  // space (original) and the PCE or SC cases use a more general standardized space.  
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).
  // 
  if (NonDQUESOInstance->emulatorType == GAUSSIAN_PROCESS || NonDQUESOInstance->emulatorType == KRIGING
      || NonDQUESOInstance->emulatorType == NO_EMULATOR) {
    //const RealVector& xLow = NonDQUESOInstance->emulatorModel.continuous_lower_bounds();
    //const RealVector& xHigh = NonDQUESOInstance->emulatorModel.continuous_upper_bounds();
    //Cout << "Queso X" << x << '\n';
    //for (i=0; i<num_cont; i++) 
    //  x(i)=xLow(i)+x(i)*(xHigh(i)-xLow(i));
    NonDQUESOInstance->emulatorModel.continuous_variables(x); 
    //Cout << "DAKOTA X" << x << '\n';
  }
  else { //case POLYNOMIAL_CHAOS: case STOCHASTIC_COLLOCATION: 
  //    RealVector u(num_cont);
  //    Iterator* se_iter = NonDQUESOInstance->stochExpIterator.iterator_rep();
  //    Pecos::ProbabilityTransformation& nataf = ((NonD*)se_iter)->variable_transformation(); 
  //    nataf.trans_X_to_U(x,u);
    NonDQUESOInstance->emulatorModel.continuous_variables(x); 
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
    for (j=0; j<num_funcs; j++)
      for (i=0; i<num_exp; i++) 
        for (k=0; k<num_replicates(i); k++){
	  Real data_i_j = NonDQUESOInstance->expData.scalar_data(j, i, k);
	  result = result+pow((fn_vals(j)-data_i_j)/paramValues[num_cont+j],2.0);
        }
  }
  else {	
    for (j=0; j<num_funcs; j++)
      for (i=0; i<num_exp; i++) 
        for (k=0; k<num_replicates(i); k++){
	  Real data_i_j = NonDQUESOInstance->expData.scalar_data(j, i, k);
	  Real std_i_j = NonDQUESOInstance->expData.scalar_sigma(j, i, k);
          result = result+pow((fn_vals(j)-data_i_j)/std_i_j,2.0);
          //result = result+pow((fn_vals(j)-data_i_j),2.0);
        }
  }

  result = (result/(NonDQUESOInstance->likelihoodScale));
  result = -1.0*result;
  Cout << "result final " << result << '\n';
  Cout << "likelihood is " << exp(result) << '\n';
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
