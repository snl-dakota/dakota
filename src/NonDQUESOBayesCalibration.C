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

#include "NonDQUESOBayesCalibration.H"
#include "ProblemDescDB.H"
#include "DakotaModel.H"
#include "tabular_io.h"
#include "uqStatisticalInverseProblem.h"
#include "uqStatisticalInverseProblemOptions.h"
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
  proposalCovScale(probDescDB.get_real("method.proposal_covariance_scale")),
  likelihoodScale(probDescDB.get_real("method.likelihood_scale"))
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
  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the QUESO environment 
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
  envOptionsValues->m_seed                 = randomSeed;
  uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"","",envOptionsValues);

  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    paramSpace(*env, "param_", numContinuousVars, NULL);

  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  uqGslVectorClass paramMins(paramSpace.zeroVector());
  uqGslVectorClass paramMaxs(paramSpace.zeroVector());
  const RealVector& lower_bounds = emulatorModel.continuous_lower_bounds();
  const RealVector& upper_bounds = emulatorModel.continuous_upper_bounds();
  if (emulatorType == GAUSSIAN_PROCESS || emulatorType == NO_EMULATOR) {
    for (size_t i=0;i<numContinuousVars;i++) {
      paramMins[i]=0.0;
      paramMaxs[i]=1.0;
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
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);

  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  
  // a matrix with numExperiments rows and cols
  // numExpConfigVars X, numFunctions Y, [numFunctions Sigma]
  RealMatrix experimental_data;

  size_t num_sigma_read = numExpStdDeviations;
  size_t num_cols = numExpConfigVars + numFunctions + num_sigma_read;

  TabularIO::read_data_tabular(expDataFileName, "QUESO Bayes Calibration", 
			       experimental_data, numExperiments,  num_cols, 
			       expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience
  size_t start_row, start_col;
  if (numExpConfigVars > 0) {
    start_row = 0;
    start_col = 0;
    RealMatrix x_obs_data(Teuchos::View, experimental_data,
			numExperiments, numExpConfigVars,
			start_row, start_col);
    xObsData.reshape(x_obs_data.numRows(),x_obs_data.numCols());
    for (int i=0; i<x_obs_data.numRows(); i++)
      for (int j=0; j<x_obs_data.numCols(); j++)
        xObsData(i,j)=x_obs_data(i,j);
  }
  Cout << "xobs_data" << xObsData << '\n';
  
  start_row = 0;
  start_col = numExpConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
  yObsData.reshape(y_obs_data.numRows(),y_obs_data.numCols());
  for (int i=0; i<y_obs_data.numRows(); i++)
    for (int j=0; j<y_obs_data.numCols(); j++)
      yObsData(i,j)=y_obs_data(i,j);

  Cout << "yobs_data" << yObsData << '\n';

  yStdData.reshape(numExperiments,numFunctions);
  if (num_sigma_read > 0) {
    start_row = 0;
    start_col = numExpConfigVars + numFunctions;
    RealMatrix y_std_data(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
    for (int i=0; i<y_std_data.numRows(); i++)
      for (int j=0; j<y_std_data.numCols(); j++)
        yStdData(i,j)=y_std_data(i,j);
  }
  else {
    if (expStdDeviations.length()==1) {
      for (int i=0; i<numExperiments; i++)
        for (int j=0; j<numFunctions; j++)
          yStdData(i,j)=expStdDeviations(0);
    }
    else if (expStdDeviations.length()==numFunctions) {
      for (int i=0; i<numExperiments; i++)
        for (int j=0; j<numFunctions; j++)
          yStdData(i,j)=expStdDeviations(j);
    }
    else {
      Cerr << "\nError (NonDGPMSABayesCalibration): must specify "
           << "experimental standard deviations either from experimental_data_file "
           << "or in the experimental_std_deviations specification."
           << std::endl;
      abort_handler(-1);
    }
  }
  Cout << "ystd_data" << yStdData << '\n';
    
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
  uqSsOptionsValuesClass ssOptionsValues1;
  uqSsOptionsValuesClass ssOptionsValues2;

  //ssOptionsValues1.m_initialDiscardedPortions.resize(3);
  //ssOptionsValues1.m_initialDiscardedPortions[0] = 0.;
  //ssOptionsValues1.m_initialDiscardedPortions[1] = 0.05;
  //ssOptionsValues1.m_initialDiscardedPortions[2] = 0.10;
  //ssOptionsValues1.m_initialDiscardedPortions[3] = 0.15;
  //ssOptionsValues1.m_initialDiscardedPortions[1] = 0.20;
  //ssOptionsValues1.m_initialDiscardedPortions[5] = 0.25;
  //ssOptionsValues1.m_initialDiscardedPortions[6] = 0.30;
  //ssOptionsValues1.m_initialDiscardedPortions[7] = 0.35;
  //ssOptionsValues1.m_initialDiscardedPortions[2] = 0.40;
  //ssOptionsValues1.m_bmmRun                      = false;
  //ssOptionsValues1.m_fftCompute                  = false;
  //ssOptionsValues1.m_psdCompute                  = false;
  //ssOptionsValues1.m_psdAtZeroCompute            = false;
  //ssOptionsValues1.m_gewekeCompute               = true;
  //ssOptionsValues1.m_gewekeNaRatio               = .1;
  //ssOptionsValues1.m_gewekeNbRatio               = .5;
  //ssOptionsValues1.m_gewekeDisplay               = true;
  //ssOptionsValues1.m_gewekeWrite                 = true;
  //ssOptionsValues1.m_autoCorrComputeViaDef       = false;
  //ssOptionsValues1.m_autoCorrComputeViaFft       = true;
  //ssOptionsValues1.m_autoCorrSecondLag           = 2;
  //ssOptionsValues1.m_autoCorrLagSpacing          = 2;
  //ssOptionsValues1.m_autoCorrNumLags             = 15;
  //ssOptionsValues1.m_autoCorrDisplay             = true;
  //ssOptionsValues1.m_autoCorrWrite               = true;
  //ssOptionsValues1.m_meanStaccCompute            = false;
  //ssOptionsValues1.m_histCompute                 = false;
  //ssOptionsValues1.m_cdfStaccCompute             = false;
  ssOptionsValues1.m_kdeCompute                  = false;
  ssOptionsValues1.m_covMatrixCompute            = true;
  ssOptionsValues1.m_corrMatrixCompute           = true;

  //ssOptionsValues2.m_initialDiscardedPortions.resize(1);
  //ssOptionsValues2.m_initialDiscardedPortions[0] = 0.;
  //ssOptionsValues2.m_bmmRun                      = false;
  //ssOptionsValues2.m_fftCompute                  = false;
  //ssOptionsValues2.m_psdCompute                  = false;
  //ssOptionsValues2.m_psdAtZeroCompute            = false;
  //ssOptionsValues2.m_gewekeCompute               = false;
  //ssOptionsValues2.m_autoCorrComputeViaDef       = false;
  //ssOptionsValues2.m_autoCorrComputeViaFft       = true;
  //ssOptionsValues2.m_autoCorrSecondLag           = 2;
  //ssOptionsValues2.m_autoCorrLagSpacing          = 2;
  //ssOptionsValues2.m_autoCorrNumLags             = 15;
  //ssOptionsValues2.m_autoCorrDisplay             = true;
  //ssOptionsValues2.m_autoCorrWrite               = true;
  //ssOptionsValues2.m_meanStaccCompute            = false;
  //ssOptionsValues2.m_histCompute                 = true;
  //ssOptionsValues2.m_histNumInternalBins         = 250;
  //ssOptionsValues2.m_cdfStaccCompute             = false;
  ssOptionsValues2.m_kdeCompute                  = true;
  ssOptionsValues2.m_kdeNumEvalPositions         = 250;
  ssOptionsValues2.m_covMatrixCompute            = true;
  ssOptionsValues2.m_corrMatrixCompute           = true;

  calIpMhOptionsValues = new uqMhOptionsValuesClass(&ssOptionsValues1,&ssOptionsValues2);
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
  calIpMhOptionsValues->m_rawChainComputeStats          = true;

  //calIpMhOptionsValues->m_displayCandidates         = false;
  calIpMhOptionsValues->m_putOutOfBoundsInChain       = true;
  //calIpMhOptionsValues->m_tkUseLocalHessian         = false;
  //calIpMhOptionsValues->m_tkUseNewtonComponent      = true;
  if (rejectionType.ends("standard"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 0;
  else if (rejectionType.ends("delayed"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 1;
  calIpMhOptionsValues->m_drScalesForExtraStages.resize(1);
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 6;
  if (metropolisType.ends("hastings"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 0;
  else if (metropolisType.ends("adaptive"))
    calIpMhOptionsValues->m_amInitialNonAdaptInterval = 1;
  calIpMhOptionsValues->m_amAdaptInterval           = 100;
  //calIpMhOptionsValues->m_amEta                     = 1.92;
  //calIpMhOptionsValues->m_amEpsilon                 = 1.e-5;

  calIpMhOptionsValues->m_filteredChainGenerate              = true;
  calIpMhOptionsValues->m_filteredChainDiscardedPortion      = 0.;
  calIpMhOptionsValues->m_filteredChainLag                   = 20;
  calIpMhOptionsValues->m_filteredChainDataOutputFileName    = ".";
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(1);
  calIpMhOptionsValues->m_filteredChainComputeStats          = true;

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  uqGslVectorClass paramInitials(paramSpace.zeroVector());
  uqGslMatrixClass proposalCovMatrix(paramSpace.zeroVector());
  for (size_t i=0;i<numContinuousVars;i++) {
    paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
    //paramInitials[i]=0.75;
    //paramInitials[i]=0.25;
    for (size_t j=0;j<numContinuousVars;j++) 
      proposalCovMatrix(i,j)=0.;
    proposalCovMatrix(i,i)=(paramMaxs[i]-paramMins[i])*proposalCovScale;
  }
  ip.solveWithBayesMetropolisHastings(calIpMhOptionsValues,
                                    paramInitials, &proposalCovMatrix);

  // Return
  delete calIpMhOptionsValues;
  delete env;
  delete envOptionsValues;
  MPI_Finalize();

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
  size_t i,j;
  int num_exp = NonDQUESOInstance->numExperiments;
  int num_funcs = NonDQUESOInstance->numFunctions;
  int num_cont = NonDQUESOInstance->numContinuousVars; 
  RealVector x(num_cont);
  
  //Cout << "numExperiments" << num_exp << '\n';
  //Cout << "numFunctions" << num_funcs << '\n';
  //Cout << "numExpStdDeviationsRead " << NonDQUESOInstance->numExpStdDeviationsRead << '\n';

  for (i=0; i<num_cont; i++) 
    x(i)=paramValues[i];
  
  // FOR NOW:  THE GP and the NO EMULATOR case use a standard uniform [0,1] 
  // space and the PCE or SC cases use a more general standardized space.  
  // The standardization for the GP was not fully working, so the 
  // standardization for the GP and the NO EMULATOR case are done manually. 
  // This eventually needs to be fixed. 
  // For now, QUESO is searching in a standardized case in all cases. 
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).
  // 
  if (NonDQUESOInstance->emulatorType == GAUSSIAN_PROCESS || NonDQUESOInstance->emulatorType == NO_EMULATOR) {
    const RealVector& xLow = NonDQUESOInstance->emulatorModel.continuous_lower_bounds();
    const RealVector& xHigh = NonDQUESOInstance->emulatorModel.continuous_upper_bounds();
    //Cout << "Queso X" << x << '\n';
    for (i=0; i<num_cont; i++) 
      x(i)=xLow(i)+x(i)*(xHigh(i)-xLow(i));
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
  Cout << "input is " << x << '\n';
  Cout << "output is " << fn_vals << '\n';
 
  // Calculate the likelihood depending on what information is available 
  // for the standard deviations
  // BMA: note that we now allow reading 1 or N sigmas from the file...
  if (NonDQUESOInstance->numExpStdDeviationsRead > 0) {
    for (i=0; i<num_exp; i++) 
      for (j=0; j<num_funcs; j++)
        result = result+pow((fn_vals(j)-NonDQUESOInstance->yObsData(i,j))/NonDQUESOInstance->yStdData(i,j),2.0);
  }
  else if (NonDQUESOInstance->expStdDeviations.length() == 0) {
 	//assume y_sigma = 1, only take squared diffs for likelihood
    for (i=0; i<num_exp; i++) 
      for (j=0; j<num_funcs; j++)
        result = result+pow((fn_vals(j)-NonDQUESOInstance->yObsData(i,j)),2.0);
  }
  else if (NonDQUESOInstance->expStdDeviations.length() == 1) {
    for (i=0; i<num_exp; i++) 
      for (j=0; j<num_funcs; j++)
        result = result+pow((fn_vals(j)-NonDQUESOInstance->yObsData(i,j))/NonDQUESOInstance->yStdData(0,0),2.0);
  }
  else if (NonDQUESOInstance->expStdDeviations.length() == num_funcs) {
    for (i=0; i<num_exp; i++) 
      for (j=0; j<num_funcs; j++)
      result = result+pow((fn_vals(j)-NonDQUESOInstance->yObsData(i,j))/NonDQUESOInstance->yStdData(i,j),2.0);
  }
  result = (result*(NonDQUESOInstance->likelihoodScale));
  result = -1.0*result;
  Cout << "result final " << result << '\n';
  Cout << "likelihood is " << exp(result) << '\n';
  return result;
}

} // namespace Dakota
