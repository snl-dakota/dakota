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

#include "uqStatisticalInverseProblem.h"
#include "uqStatisticalInverseProblemOptions.h"
#include "uqGslVector.h"
#include "uqGslMatrix.h"
#include "uqEnvironment.h"
#include "uqEnvironmentOptions.h"
#include "uqDefines.h"
#include "uqValidationCycle.h"

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
  yObsDataFile(probDescDB.get_string("method.y_obs_data_file")),
  yStdDataFile(probDescDB.get_string("method.y_std_data_file"))
{ }


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::quantify_uncertainty()
{
  // instantiate QUESO objects and execute
  NonDQUESOInstance=this;
  Cout << "Rejection type  "<< rejectionType << '\n';
  Cout << "Metropolis type " << metropolisType << '\n';
  Cout << "Num Samples " << numSamples;
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
  envOptionsValues->m_seed                 = 0;
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

  for (size_t i=0;i<numContinuousVars;i++) {
    paramMins[i]=lower_bounds[i];
    paramMaxs[i]=upper_bounds[i];
  }
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);

  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  int i=0, num_obs_data = 0;

  std::ifstream data_file1(yObsDataFile);
  if (!data_file1) {
    Cerr << "Could not open user data source for y observations "
         << yObsDataFile << " in "
         << "QUESO Bayesian calibration method." << std::endl;
    abort_handler(-1);
  }
  else {
    std::string line;
    while (getline(data_file1,line))
      num_obs_data++;
    yObsData.resize(num_obs_data);
    yStdData.resize(num_obs_data);

    data_file1.clear();
    data_file1.seekg(0, std::ios::beg);

    i=0;
    while ((i<num_obs_data) && (data_file1 >> yObsData[i]))
      i++;
    data_file1.close();
  }

  std::ifstream data_file3(yStdDataFile);
  if (!data_file3) {
    Cerr << "Could not open user data source for y error on observations "
         << yStdDataFile << " in "
         << "QUESO Bayesian calibration method." << std::endl;
  abort_handler(-1);
  }
  else {
    i = 0;
    while ((i<num_obs_data) &&  (data_file3 >> yStdData[i]))
      i++;
    data_file3.close();
  }
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

  //ssOptionsValues1.m_initialDiscardedPortions.resize(9);
  //ssOptionsValues1.m_initialDiscardedPortions[0] = 0.;
  //ssOptionsValues1.m_initialDiscardedPortions[1] = 0.05;
  //ssOptionsValues1.m_initialDiscardedPortions[2] = 0.10;
  //ssOptionsValues1.m_initialDiscardedPortions[3] = 0.15;
  //ssOptionsValues1.m_initialDiscardedPortions[4] = 0.20;
  //ssOptionsValues1.m_initialDiscardedPortions[5] = 0.25;
  //ssOptionsValues1.m_initialDiscardedPortions[6] = 0.30;
  //ssOptionsValues1.m_initialDiscardedPortions[7] = 0.35;
  //ssOptionsValues1.m_initialDiscardedPortions[8] = 0.40;
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
  //calIpMhOptionsValues->m_putOutOfBoundsInChain     = true;
  //calIpMhOptionsValues->m_tkUseLocalHessian         = false;
  //calIpMhOptionsValues->m_tkUseNewtonComponent      = true;
  if (rejectionType.ends("standard"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 0;
  else if (rejectionType.ends("delayed"))
    calIpMhOptionsValues->m_drMaxNumExtraStages = 1;
  calIpMhOptionsValues->m_drScalesForExtraStages.resize(1);
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 5.;
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
    paramInitials[i]=(lower_bounds[i]+upper_bounds[i])/2.0;
    for (size_t j=0;j<numContinuousVars;j++) 
      proposalCovMatrix(i,j)=0.;
    proposalCovMatrix(i,i)=(upper_bounds[i]-lower_bounds[i])/2.0;
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
  size_t i, num_cont;
  size_t num_data_pts = NonDQUESOInstance->yObsData.length();

  num_cont = NonDQUESOInstance->numContinuousVars; 
  RealVector x(num_cont);
  
  for (i=0; i<num_cont; i++) 
    x(i)=paramValues[i];
  
  NonDQUESOInstance->emulatorModel.continuous_variables(x); 
  // TO DO: continuous to start
  NonDQUESOInstance->emulatorModel.compute_response();
  RealVector fn_vals = NonDQUESOInstance->emulatorModel.current_response().function_values();
  
  //for now return sum squares of residuals, need to add sigma
  // for now assume iid, constant sigma
  
  for (i=0; i<num_data_pts; i++){ 
  result = result+pow((fn_vals(0)-NonDQUESOInstance->yObsData[i]),2.0);
  }
  result = -1.0*result;
 
  return result;
}

} // namespace Dakota
