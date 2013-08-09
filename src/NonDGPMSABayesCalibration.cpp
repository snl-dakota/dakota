/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPMSABayesCalibration
//- Description: Derived class for Bayesian inference using GPMSA
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGPMSABayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "ExperimentData.hpp"
#include "uqStatisticalInverseProblem.h"
#include "uqStatisticalInverseProblemOptions.h"
#include "uqGslVector.h"
#include "uqGslMatrix.h"
#include "uqEnvironment.h"
#include "uqEnvironmentOptions.h"
#include "uqDefines.h"
#include "uqValidationCycle.h"
#include "ProbabilityTransformation.hpp"
#include "uqVectorSpace.h" 
#include "uqJointPdf.h" 
#include "uqGpmsaComputerModel.h"
#include "uqSimulationModelOptions.h"
#include "NonDLHSSampling.hpp"
#include "dakota_data_io.hpp"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

//initialization of statics
NonDGPMSABayesCalibration* NonDGPMSABayesCalibration::NonDGPMSAInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGPMSABayesCalibration::NonDGPMSABayesCalibration(Model& model):
  NonDBayesCalibration(model),
  numSamples(probDescDB.get_int("method.samples")),
  rejectionType(probDescDB.get_string("method.rejection")),
  metropolisType(probDescDB.get_string("method.metropolis")),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  proposalCovScale(probDescDB.get_rv("method.nond.proposal_covariance_scale")),
  likelihoodScale(probDescDB.get_real("method.likelihood_scale")),
  emulatorSamples(probDescDB.get_int("method.nond.emulator_samples"))
{   
  const String& rng = probDescDB.get_string("method.random_number_generator");
  String sample_type("lhs");
  lhsIter.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
        emulatorSamples, randomSeed, rng, true, ACTIVE_UNIFORM), false);
  iteratedModel.init_communicators(lhsIter.maximum_concurrency());
  }


NonDGPMSABayesCalibration::~NonDGPMSABayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDGPMSABayesCalibration::quantify_uncertainty()
{
  // construct emulatorModel, if needed
  //NonDBayesCalibration::quantify_uncertainty();
 
  lhsIter.run_iterator(Cout);
  // instantiate QUESO objects and execute
  NonDGPMSAInstance=this;
  Cout << "Rejection type  "<< rejectionType << '\n';
  Cout << "Metropolis type " << metropolisType << '\n';
  Cout << "Num Samples " << numSamples << '\n';
  // For now, set calcSigmaFlag to true: this should be read from input
  calibrateSigmaFlag = true;

  ////////////////////////////////////////////////////////
  // Step 0: Instantiate the QUESO environment 
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
  envOptionsValues->m_seed = -1;
  envOptionsValues->m_identifyingString="towerExampleToMatchGPMSA";
  //if (randomSeed) 
  //  envOptionsValues->m_seed                 = randomSeed;
  //else
  //  envOptionsValues->m_seed                 = 1 + (int)clock(); 
      
  //uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"/home/lpswile/dakota/src/gpmsa.inp","",envOptionsValues);
  uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"","",envOptionsValues);
 
  // Read in all of the experimental data:  any x configuration 
  // variables, y observations, and y_std if available 
  bool calc_sigma_from_data = true; // calculate sigma if not provided
  read_historical_data(expDataFileName, "QUESO/GPMSA Bayes Calibration",
		       numExperiments, 
		       numExpConfigVars, numFunctions, numExpStdDeviationsRead,
		       expDataFileAnnotated, expStdDeviations,
		       calc_sigma_from_data,
		       xObsData, yObsData, yStdData);

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "xobs_data" << xObsData << '\n';
    Cout << "yobs_data" << yObsData << '\n';
    Cout << "ystd_data" << yStdData << '\n';
  }

  //***********************************************************************
  //  Step 01 of 09: Instantiate parameter space, parameter domain, and prior Rv
  // *********************************************************************
  int total_num_params = numUncertainVars;
  
  Cout << "total_num_params " << total_num_params;
  
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    paramSpace(*env, "param_", total_num_params, NULL);

  Cout << "Got to line 196 \n" ;
  uqGslVectorClass paramMins(paramSpace.zeroVector());
  uqGslVectorClass paramMaxs(paramSpace.zeroVector());
  const RealVector& lower_bounds = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bounds = iteratedModel.continuous_upper_bounds();
  const RealVector& init_point = iteratedModel.continuous_variables();
  Cout << "Initial Points " << init_point << '\n';
  Cout << "lower_bounds " << lower_bounds << '\n';
  Cout << "emulatorType " << emulatorType << '\n';

  for (size_t i=0;i<numUncertainVars;i++) {
    paramMins[i]=lower_bounds[i];
    paramMaxs[i]=upper_bounds[i];
  }
 
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);
  Cout << "got to line 219" ;
  uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>* paramPriorRvPtr = NULL;
  
  paramPriorRvPtr = new uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>("prior_",paramDomain);

  Cout << "got to line 218\n";
  //***********************************************************************
  // Step 02 of 09: Instantiate the 'scenario' and 'output' spaces
  //***********************************************************************
  int num_config_vars = numContStateVars;
  Cout << "num_config_vars " << num_config_vars << '\n';
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    config_x_space(*env, "scenario_", num_config_vars, NULL);
               
  unsigned int num_eta = numFunctions; // simulation output dimension; 'n_eta' in paper; 16 in tower example
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    n_eta_space(*env, "output_", num_eta, NULL);
  Cout << "num_eta " << num_eta << '\n';
  
 //***********************************************************************
 //  Step 03 of 09: Instantiate the simulation storage
 //  Regarding simulation scenario input values, QUESO will standardize them so that
 //  they exist inside a hypercube: this will be done in the 'uqSimulationModelClass'
 //  constructor, step 04 of 09.
 //  Regarding simulation output data, QUESO will transform it so that the mean is
 //       zero and the variance is one: this will be done in the 'uqSimulationModelClass'
 //        constructor, step 04 of 09.
 // ***********************************************************************
 
  int num_simulations = emulatorSamples;
  Cout << "num_simulations " << num_simulations << '\n';

  uqSimulationStorageClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass> 
    simulationStorage(config_x_space,paramSpace,n_eta_space,num_simulations);
 
  // Add simulations: what if none?
  uqGslVectorClass extraSimulationGridVec(n_eta_space.zeroVector());
  std::vector<uqGslVectorClass* > simulationScenarios(num_simulations,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > paramVecs(num_simulations,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > outputVecs(num_simulations,(uqGslVectorClass*) NULL);
            
  for (int i = 0; i < num_simulations; ++i) {
    simulationScenarios[i] = new uqGslVectorClass(config_x_space.zeroVector());   // 'x_{i+1}^*' in paper
    paramVecs          [i] = new uqGslVectorClass(paramSpace.zeroVector());   // 't_{i+1}^*' in paper
    outputVecs         [i] = new uqGslVectorClass(n_eta_space.zeroVector()); // 'eta_{i+1}' in paper
  }
                           
  // Populate all objects just instantiated
  for (int i = 0; i<numFunctions; i++) 
    extraSimulationGridVec[i]=1.5+1.5*i;
  Cout << "Extra Simulation Grid Vec " << extraSimulationGridVec << '\n';

  const RealMatrix&  all_samples = lhsIter.all_samples();
  const IntResponseMap& all_resp = lhsIter.all_responses();
  RealVector temp_cvars;
  int i,j,k;
  uqGslVectorClass gsl_cvars(paramSpace.zeroVector());
  uqGslVectorClass gsl_configvars(config_x_space.zeroVector());
  for (i = 0; i < num_simulations; ++i) {
    temp_cvars = Teuchos::getCol(Teuchos::View, const_cast<RealMatrix&>(all_samples), i);
    for (j=0; j<numUncertainVars;++j){
      gsl_cvars[j]=temp_cvars[j];
      Cout << "temp_cvars j" << temp_cvars[j] << '\n';
    }
    for (j=0; j<numUncertainVars;++j)
      (*(paramVecs[i]))[j] = gsl_cvars[j];
    Cout << *(paramVecs[i]) << '\n';
    for (j=0; j<numStateVars;++j) {
      gsl_configvars[j]=temp_cvars[j+numUncertainVars];
      Cout << "config_vars j" << temp_cvars[j+numUncertainVars] << '\n';
    }
    for (j=0; j<numStateVars;++j)
      (*(simulationScenarios[i]))[j] = gsl_configvars[j];
    Cout << *(simulationScenarios[i]) << '\n';
    //(*(simulationScenarios[i]))[0]=1.0;
  }
  (*(paramVecs[0]))[0]=0.2105;
  (*(paramVecs[1]))[0]=0.1795;
  (*(paramVecs[2]))[0]=0.1167;
  (*(paramVecs[3]))[0]=0.1457;
  (*(paramVecs[4]))[0]=0.0610;
  (*(paramVecs[5]))[0]=0.2376;
  (*(paramVecs[6]))[0]=0.0982;
  (*(paramVecs[7]))[0]=0.1072;
  (*(paramVecs[8]))[0]=0.0742;
  (*(paramVecs[9]))[0]=0.0979;
  (*(paramVecs[10]))[0]=0.1639;
  (*(paramVecs[11]))[0]=0.2275;
  (*(paramVecs[12]))[0]=0.1977;
  (*(paramVecs[13]))[0]=0.2237;
  (*(paramVecs[14]))[0]=0.1914;
  (*(paramVecs[15]))[0]=0.2466;
  (*(paramVecs[16]))[0]=0.0702;
  (*(paramVecs[17]))[0]=0.1345;
  (*(paramVecs[18]))[0]=0.1564;
  (*(paramVecs[19]))[0]=0.1762;
  (*(paramVecs[20]))[0]=0.1498;
  (*(paramVecs[21]))[0]=0.1236;
  (*(paramVecs[22]))[0]=0.2087;
  (*(paramVecs[23]))[0]=0.0544;
  (*(paramVecs[24]))[0]=0.0885;
 
  (*(simulationScenarios[0]))[0] = 0.0996;
  (*(simulationScenarios[1]))[0] = 0.3995;
  (*(simulationScenarios[2]))[0] = 0.2956;
  (*(simulationScenarios[3]))[0] = 0.4033;
  (*(simulationScenarios[4]))[0] = 0.4478;
  (*(simulationScenarios[5]))[0] = 0.0971;
  (*(simulationScenarios[6]))[0] = 0.1222;
  (*(simulationScenarios[7]))[0] = 0.3155;
  (*(simulationScenarios[8]))[0] = 0.1676;
  (*(simulationScenarios[9]))[0] = 0.1480;
  (*(simulationScenarios[10]))[0] = 0.2331;
  (*(simulationScenarios[11]))[0] = 0.2251;
  (*(simulationScenarios[12]))[0] = 0.0795;
  (*(simulationScenarios[13]))[0] = 0.3272;
  (*(simulationScenarios[14]))[0] = 0.2460;
  (*(simulationScenarios[15]))[0] = 0.2881;
  (*(simulationScenarios[16]))[0] = 0.3502;
  (*(simulationScenarios[17]))[0] = 0.3613;
  (*(simulationScenarios[18]))[0] = 0.3833;
  (*(simulationScenarios[19]))[0] = 0.2095;
  (*(simulationScenarios[20]))[0] = 0.1308;
  (*(simulationScenarios[21]))[0] = 0.1898;
  (*(simulationScenarios[22]))[0] = 0.4264;
  (*(simulationScenarios[23]))[0] = 0.0579;
  (*(simulationScenarios[24]))[0] = 0.2676;


  IntRespMCIter resp_it = all_resp.begin();
  for (j=0, resp_it=all_resp.begin(); j<num_simulations; ++j, ++resp_it) {
    RealVector temp_resp(numFunctions);
    for (k=0; k<numFunctions; k++)
      temp_resp(k) = resp_it->second.function_value(k);
    for (k=0; k<numFunctions; k++)
      (*(outputVecs[j]))[k]=temp_resp[k];
    //Cout << "temp_resp " << temp_resp << '\n';
    Cout << *(outputVecs[j]) << '\n';
  }
    //(*(simulationScenarios[i]))[0] = inputSimulScenarioVector [i];
    //(*(paramVecs          [i]))[0] = inputSimulParameterVector[i];
    //outputVecs[i] = ;
 
  //Finally, add information to the simulation storage
  for (int i = 0; i < num_simulations; ++i) {
    simulationStorage.addSimulation(*(simulationScenarios[i]),*(paramVecs[i]),*(outputVecs[i]));
  }
  Cout << "got to line 294 " << '\n';
  //***********************************************************************
  // Step 04 of 09: Instantiate the simulation model
  //***********************************************************************
  uqSmOptionsValuesClass *smVarOptions = NULL;
  smVarOptions = new uqSmOptionsValuesClass();
  smVarOptions->m_dataOutputFileName = ".";
  smVarOptions->m_dataOutputAllowAll = 0;
  smVarOptions->m_dataOutputAllowedSet.insert(0);
  smVarOptions->m_p_eta = 2;
  smVarOptions->m_zeroRelativeSingularValue = 0.;
  smVarOptions->m_cdfThresholdForPEta = 0.;
  smVarOptions->m_a_w = 5.;
  smVarOptions->m_b_w = 5.;
  smVarOptions->m_a_rho_w = 1.;
  smVarOptions->m_b_rho_w = 0.1;
  smVarOptions->m_a_eta = 5.;
  smVarOptions->m_b_eta = 0.005;
  smVarOptions->m_a_s = 3.;
  smVarOptions->m_b_s = 0.003;

  //simOptionsValues->m_ov.zeroRelativeSingularValue = 0.0;
  //simOptionsValues->m_ov.cdfThresholdForPEta = 0.95;
  uqSimulationModelClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>
    simulationModel("",   // prefix
                    smVarOptions, // options values
                    simulationStorage);
 

  unsigned int num_bases_eta = simulationModel.numBasis(); // number of simulation basis; 'p_eta' in paper; 2 in tower example
  Cout << "num_bases_eta " << num_bases_eta << '\n';
  Cout << "got to line 306 " << '\n';


  //***********************************************************************
  // Step 05 of 09: Instantiate the experiment storage
  // Regarding experimental scenario input values, QUESO will standardize them so that
  //    they exist inside a hypercube: this will be done in the 'uqExperimentModelClass'
  //    constructor, step 06 of 09.
  // Regarding experimental data, the user has to provide it already in transformed
  //    format, that is, with mean zero and standard deviation one.
  //***********************************************************************
#if 1 // Replace by "if 0" if there is no experimental data available
  // number of experiments; 'n' in paper; 3 in tower example, numExperiments

  uqExperimentStorageClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>* experimentStoragePtr = NULL;
  experimentStoragePtr = new uqExperimentStorageClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>(config_x_space, numExperiments);
 
  Cout << "Got to line 323 " << '\n';
  // Add experiments
  std::vector<uqGslVectorClass* > experimentScenarios_original(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentScenarios_standard(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<unsigned int> experimentDims(numExperiments,0);
  std::vector<uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>* > experimentSpaces          (numExperiments,(uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>*) NULL);
  std::vector<uqGslVectorClass* > extraExperimentGridVecs(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_original   (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_auxMean    (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_transformed(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslMatrixClass* > experimentMats_original   (numExperiments,(uqGslMatrixClass*) NULL);
  std::vector<uqGslMatrixClass* > experimentMats_transformed(numExperiments,(uqGslMatrixClass*) NULL);

  //for (i = 0; i < numExperiments; ++i) {
    //experimentDims[i] = numFunctions; // constant for now
  //}
  experimentDims[0]=4; 
  experimentDims[1]=4; 
  experimentDims[2]=3;
  Cout << "Got to line 338 " << '\n';
 
  for (i = 0; i < numExperiments; ++i) {
    experimentScenarios_original[i] = new uqGslVectorClass(config_x_space.zeroVector());               // 'x_{i+1}' in paper
    experimentScenarios_standard[i] = new uqGslVectorClass(config_x_space.zeroVector());     
    experimentSpaces[i] = new uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>(*env, "expSpace", experimentDims[i], NULL);
    extraExperimentGridVecs[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    experimentVecs_original[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    experimentVecs_auxMean[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    experimentVecs_transformed[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          // 'y_{i+1}' in paper
    experimentMats_original   [i] = new uqGslMatrixClass(experimentSpaces[i]->zeroVector());          //
    experimentMats_transformed[i] = new uqGslMatrixClass(experimentSpaces[i]->zeroVector());          // 'W_{i+1}' in paper
  }

  //***********************************************************************
  // Populate information regarding experiment '0'
  //***********************************************************************
  for (unsigned int i = 0; i < numExperiments; ++i) {
    for (unsigned int j = 0; j < num_config_vars; ++j) 
      (*(experimentScenarios_original[i]))[j] = xObsData(i,j);
    Cout << *(experimentScenarios_original[i]) << '\n';
  }
  
  for (unsigned int i = 0; i < numExperiments; ++i) {
    *(experimentScenarios_standard[i])  = *(experimentScenarios_original[i]);
    *(experimentScenarios_standard[i]) -= simulationModel.xSeq_original_mins();
    for (unsigned int j = 0; j < num_config_vars; ++j) {
      (*(experimentScenarios_standard[i]))[j] /= simulationModel.xSeq_original_ranges()[j];
    }
  }
  //(*(experimentScenarios[0]))[0] = 0.1; // 'x_1' in paper; Radius in tower example

    (*(extraExperimentGridVecs[0]))[0] = 5.;
    (*(extraExperimentGridVecs[0]))[1] = 10.;
    (*(extraExperimentGridVecs[0]))[2] = 15.;
    (*(extraExperimentGridVecs[0]))[3] = 20.;
    (*(extraExperimentGridVecs[1]))[0] = 5.;
    (*(extraExperimentGridVecs[1]))[1] = 10.;
    (*(extraExperimentGridVecs[1]))[2] = 15.;
    (*(extraExperimentGridVecs[1]))[3] = 20.;
    (*(extraExperimentGridVecs[2]))[0] = 5.;
    (*(extraExperimentGridVecs[2]))[1] = 10.;
    (*(extraExperimentGridVecs[2]))[2] = 15.;

  for (unsigned int i = 0; i < numExperiments; ++i) {
    for (unsigned int j = 0; j < experimentDims[i]; ++j) { 
      (*(experimentVecs_original[i]))[j] = yObsData(i,j);
    }
    Cout << *(experimentVecs_original[i]) << '\n';
    Cout << *(extraExperimentGridVecs[i]) << '\n';
  }
  Cout << "Got to line 367 " << '\n';
  for (unsigned int i = 0; i < numExperiments; ++i) {
    experimentVecs_auxMean[i]->matlabLinearInterpExtrap(extraSimulationGridVec,simulationModel.etaSeq_original_mean(),*(extraExperimentGridVecs[i]));
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
         *env->subDisplayFile() << "In compute(), step 05, experiment " << i 
                          << "\n  extraSimulationGridVec = "              << extraSimulationGridVec
                          << "\n  simulationModel.etaSeq_original_mean() = " << simulationModel.etaSeq_original_mean()
                          << "\n  *(extraExperimentGridVecs[i]) = "       << *(extraExperimentGridVecs[i])
                          << "\n  *(experimentVecs_auxMean[i]) = "           << *(experimentVecs_auxMean[i])
                          << std::endl;
    }
    *(experimentVecs_transformed[i]) = (1./simulationModel.etaSeq_allStd()) * ( *(experimentVecs_original[i]) - *(experimentVecs_auxMean[i]) ); // 'y_1' in paper
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
      *env->subDisplayFile() << "In compute(), step 05, experiment " << i
                          << "\n  *(experimentVecs_original[i]) = "    << *(experimentVecs_original[i])
                          << "\n  *(experimentVecs_auxMean[i]) = "     << *(experimentVecs_auxMean[i])
                          << "\n  simulationModel.etaSeq_allStd() = "  << simulationModel.etaSeq_allStd()
                          << "\n  *(experimentVecs_transformed[i]) = " << *(experimentVecs_transformed[i])
                          << std::endl;
    }
  }

  for (unsigned int i = 0; i < numExperiments; ++i) {
    for (unsigned int j = 0; j < experimentDims[i] ; ++j) {
      (*(experimentMats_original[i]))(j,j) = 1.;
    }
  }

  for (unsigned int i = 0; i < numExperiments; ++i) {
    *(experimentMats_transformed[i]) = *(experimentMats_original[i]);
  }

  //***********************************************************************
  // Finally, add information to the experiment storage
  //***********************************************************************
  for (unsigned int i = 0; i < numExperiments; ++i) {
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
      *env->subDisplayFile() << "In compute(), step 05"
                            << ": calling experimentStorage.addExperiment() for experiment of id '" << i << "'..."
                            << std::endl;
    }
    experimentStoragePtr->addExperiment(*(experimentScenarios_standard[i]),*(experimentVecs_transformed[i]),*(experimentMats_transformed[i]));
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
      *env->subDisplayFile() << "In compute(), step 05"
                            << ": returned from experimentStorage.addExperiment() for experiment of id '" << i << "'"
                            << std::endl;
    }
  }
  Cout << "Got to line 435 \n";
  //***********************************************************************
  // Step 06 of 09: Instantiate the experiment model
  // User has to provide 'D' matrices
  // User has to interpolate 'K_eta' matrix in order to form 'K_i' matrices
  // 'K_eta' is 'Ksim' in the GPMSA tower example document (page 9)
  //  'K_i' is 'Kobs' in the GPMSA tower example document (page 9)
  //***********************************************************************
  unsigned int num_delta_bases = 13; // number of experiment basis; 'p_delta' in paper; 13 in tower example

    //***********************************************************************
    // Form and compute 'DsimMat'
    // Not mentioned in the paper
    // 'Dsim' in the GPMSA tower example document (page 11)
    //***********************************************************************
  double kernelSigma = 2.;
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> delta_space(*env, "delta_space", num_delta_bases, NULL);
  uqGslVectorClass kernelCenters(delta_space.zeroVector());
  for (unsigned int i = 1; i < num_delta_bases; ++i) { // Yes, '1'
    kernelCenters[i] = kernelSigma*((double) i);
  }
  uqGslMatrixClass DsimMat(*env,n_eta_space.map(),num_delta_bases); // Important matrix (not mentioned on paper)
  uqGslVectorClass DsimCol(n_eta_space.zeroVector());
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> oneDSpace(*env, "oneDSpace", 1, NULL);
  uqGslVectorClass oneDVec(oneDSpace.zeroVector());
  uqGslMatrixClass oneDMat(oneDSpace.zeroVector());
  oneDMat(0,0) = kernelSigma*kernelSigma;
  for (unsigned int colId = 0; colId < DsimMat.numCols(); ++colId) {
    oneDVec[0] = kernelCenters[colId];
    uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
    for (unsigned int rowId = 0; rowId < num_eta; ++rowId) {
      oneDVec[0] = extraSimulationGridVec[rowId];
      DsimCol[rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
    }
    DsimMat.setColumn(colId,DsimCol);
  }

  //***********************************************************************
  // Populate information regarding experiment 'i'
  //   'D_{i+1}' in the paper
  //   'Dobs' in the GPMSA tower example document (page 11)
  //***********************************************************************

  std::vector<uqGslMatrixClass* >   DobsMats(numExperiments, (uqGslMatrixClass*) NULL);
  DobsMats.resize(numExperiments, (uqGslMatrixClass*) NULL); // Important matrices (D_i's on paper)
  for (unsigned int i = 0; i < numExperiments; ++i) {
    DobsMats[i] = new uqGslMatrixClass(*env,experimentSpaces[i]->map(),num_delta_bases); // 'D_{i+1}' in paper
    uqGslVectorClass DobsCol(experimentSpaces[i]->zeroVector());
    for (unsigned int colId = 0; colId < DobsMats[i]->numCols(); ++colId) {
      oneDVec[0] = kernelCenters[colId];
      uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
      for (unsigned int rowId = 0; rowId < DobsCol.sizeLocal(); ++rowId) {
        oneDVec[0] = (*(extraExperimentGridVecs[1]))[rowId];
        DobsCol[rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
      }
      DobsMats[i]->setColumn(colId,DobsCol);
    }
  }

    //***********************************************************************
    //   Normalize 'DsimMat' and all 'DobsMats'
    //***********************************************************************
  uqGslMatrixClass DsimMatTranspose(*env,delta_space.map(),num_eta);
  DsimMatTranspose.fillWithTranspose(0,0,DsimMat,true,true);
  double Dmax = (DsimMat * DsimMatTranspose).max();
  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                          << ": Dmax = " << Dmax
                          << std::endl;
  }
  DsimMat /= std::sqrt(Dmax);

  if (env->subDisplayFile()) {
    DsimMat.setPrintHorizontally(false);
    *env->subDisplayFile() << "In compute()"
                          << ": 'DsimMat'"
                          << ", nRows = "      << DsimMat.numRowsLocal()
                          << ", nCols = "      << DsimMat.numCols()
                          << ", contents =\n " << DsimMat
                          << std::endl;
  }
  for (unsigned int i = 0; i < numExperiments; ++i) {
    *(DobsMats[i]) /= std::sqrt(Dmax);
    DobsMats[i]->setPrintHorizontally(false);
    if (env->subDisplayFile()) {
      *env->subDisplayFile() << "In compute()"
                            << ": 'DobsMats', i = " << i
                            << ", nRows = "         << DobsMats[i]->numRowsLocal()
                            << ", nCols = "         << DobsMats[i]->numCols()
                            << ", contents =\n"     << *(DobsMats[i])
                            << std::endl;
    }
  }


  //***********************************************************************
  // Compute 'K_i' matrices
  //***********************************************************************
  std::vector<uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>* > Kmats_interp_spaces(numExperiments, (uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>*) NULL);
  std::vector<uqGslMatrixClass*                                      > Kmats_interp       (numExperiments, (uqGslMatrixClass*) NULL); // Interpolations of 'Kmat_eta' = 'K_i's' in paper

  for (unsigned int i = 0; i < numExperiments; ++i) {
    Kmats_interp_spaces[i] = new uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>(*env,"Kmats_interp_spaces_",experimentStoragePtr->n_ys_transformed()[i],NULL);
    Kmats_interp       [i] = new uqGslMatrixClass(*env,Kmats_interp_spaces[i]->map(),num_bases_eta);
    Kmats_interp       [i]->matlabLinearInterpExtrap(extraSimulationGridVec,simulationModel.Kmat_eta(),*(extraExperimentGridVecs[i]));
    Kmats_interp[i]->setPrintHorizontally(false);
    if (env->subDisplayFile()) {
      *env->subDisplayFile() << "In compute()"
                            << ": 'Kmats_interp', i = " << i
                            << ", nRows = "             << Kmats_interp[i]->numRowsLocal()
                            << ", nCols = "             << Kmats_interp[i]->numCols()
                            << ", contents =\n"         << *(Kmats_interp[i])
                            << std::endl;
    }
  }

  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                          << ": finished computing 'K_i' matrices"
                          << std::endl;
  }

  uqExperimentModelClass  <uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>* experimentModelPtr = NULL;

  uqEmOptionsValuesClass *emVarOptions = NULL;
  emVarOptions = new uqEmOptionsValuesClass();
  emVarOptions->m_Gvalues.resize(1,0.);
  emVarOptions->m_Gvalues[0] = 13;
  emVarOptions->m_a_v = 1.;
  emVarOptions->m_b_v = 0.001;
  emVarOptions->m_a_rho_v = 1.;
  emVarOptions->m_b_rho_v = 0.1;
  emVarOptions->m_a_y = 1.;
  emVarOptions->m_b_y = 0.001;


  experimentModelPtr = new uqExperimentModelClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>("",   // prefix
                    emVarOptions, // options values
                    *experimentStoragePtr,
                    DobsMats,
                    Kmats_interp);
#endif // "if 0" if there is no experimental data available





  //***********************************************************************
  // Step 07 of 09: Instantiate the GPMSA computer model
  //***********************************************************************
 
  uqGpmsaComputerModelClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>* gcm;
  uqGcmOptionsValuesClass *gcmVarOptions = NULL;
  gcmVarOptions = new uqGcmOptionsValuesClass();
  gcmVarOptions->m_checkAgainstPreviousSample = 0;
  gcmVarOptions->m_dataOutputFileName = ".";
  gcmVarOptions->m_dataOutputAllowAll = 0;
  gcmVarOptions->m_dataOutputAllowedSet.insert(0);
  gcmVarOptions->m_dataOutputAllowedSet.insert(1);
  gcmVarOptions->m_priorSeqNumSamples = 24000;
  gcmVarOptions->m_priorSeqDataOutputFileName = "outputData/priorSeq";
  gcmVarOptions->m_priorSeqDataOutputFileType = "m";
  gcmVarOptions->m_priorSeqDataOutputAllowAll = 0;
  gcmVarOptions->m_priorSeqDataOutputAllowedSet.insert(0);
  gcmVarOptions->m_nuggetValueForBtWyB = 0.0001;       // IMPORTANT
  gcmVarOptions->m_nuggetValueForBtWyBInv = 1e-06;     // IMPORTANT
  gcmVarOptions->m_formCMatrix = 0;
  gcmVarOptions->m_useTildeLogicForRankDefficientC = 0;
  gcmVarOptions->m_predVUsBySamplingRVs = 0;
  gcmVarOptions->m_predVUsBySummingRVs = 0;
  gcmVarOptions->m_predVUsAtKeyPoints = 1;
  gcmVarOptions->m_predWsBySamplingRVs = 0;
  gcmVarOptions->m_predWsBySummingRVs = 1;
  gcmVarOptions->m_predWsAtKeyPoints = 0;
  gcmVarOptions->m_predLag = 15;

  gcm = new uqGpmsaComputerModelClass<uqGslVectorClass,uqGslMatrixClass,
                                      uqGslVectorClass,uqGslMatrixClass,
                                      uqGslVectorClass,uqGslMatrixClass,
                                      uqGslVectorClass,uqGslMatrixClass>
          ("",
           gcmVarOptions,
           simulationStorage,
           simulationModel,
           experimentStoragePtr, // pass "NULL" if there is no experimental data available
           experimentModelPtr,   // pass "NULL" if there is no experimental data available
           paramPriorRvPtr);
           //&dakotaLikelihoodRoutine);
 
  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                          << ": finished instantiating 'gcm'"
                          << std::endl;
  }

   //***********************************************************************
   // Step 08 of 09: Calibrate the computer model
   //***********************************************************************
  uqGslVectorClass totalInitialVec(gcm->totalSpace().zeroVector());
  gcm->totalPriorRv().realizer().realization(totalInitialVec); // todo_r
  totalInitialVec[ 0] = 2.9701e+04;          // lambda_eta = lamWOs
  totalInitialVec[ 1] = 1.;                  // lambda_w_1 = lamUz
  totalInitialVec[ 2] = 1.;                  // lambda_w_2 =
  totalInitialVec[ 3] = std::exp(-0.1*0.25); // rho_w_1_1  = exp(-model.betaU.*(0.5^2));
  totalInitialVec[ 4] = std::exp(-0.1*0.25); // rho_w_1_2  =
  totalInitialVec[ 5] = std::exp(-0.1*0.25); // rho_w_2_1  =
  totalInitialVec[ 6] = std::exp(-0.1*0.25); // rho_w_2_2  =
  totalInitialVec[ 7] = 1000.;               // lambda_s_1 = lamWs
  totalInitialVec[ 8] = 1000.;               // lambda_s_2 =
  totalInitialVec[ 9] = 999.999;             // lambda_y   = lamOs
  totalInitialVec[10] = 20.;                 // lambda_v_1 = lamVz
  totalInitialVec[11] = std::exp(-0.1*0.25); // rho_v_1_1  = betaV
  totalInitialVec[12] = 0.5;                 // theta_1    = theta
 

  uqGslVectorClass diagVec(gcm->totalSpace().zeroVector());
  diagVec.cwSet(0.25);
  diagVec[ 0] = 2500.; // lambda_eta = lamWOs
  diagVec[ 1] = 0.09;  // lambda_w_1 = lamUz
  diagVec[ 2] = 0.09;  // lambda_w_2 =
  diagVec[ 3] = 0.01;  // rho_w_1_1  = betaU
  diagVec[ 4] = 0.01;  // rho_w_1_2  =
  diagVec[ 5] = 0.01;  // rho_w_2_1  =
  diagVec[ 6] = 0.01;  // rho_w_2_2  =
  diagVec[ 7] = 2500.; // lambda_s_1 = lamWs
  diagVec[ 8] = 2500;  // lambda_s_2 = 
  diagVec[ 9] = 2500;  // lambda_y   = lamOs
  diagVec[10] = 2500;  // lambda_v_1 = lamVz
  diagVec[11] = 0.01;  // rho_v_1_1  = betaV
  diagVec[12] = 100.;  // theta_1    = theta

  uqGslMatrixClass totalInitialProposalCovMatrix(diagVec); // todo_r
  Cout << "Got to line 597 \n"; 
  
  uqMhOptionsValuesClass *mhVarOptions = NULL;
  mhVarOptions = new uqMhOptionsValuesClass();
  mhVarOptions->m_dataOutputFileName = ".";
  mhVarOptions->m_dataOutputAllowAll = 0;
  mhVarOptions->m_dataOutputAllowedSet.insert(0);
  mhVarOptions->m_dataOutputAllowedSet.insert(1);
  mhVarOptions->m_totallyMute = 0;
  mhVarOptions->m_initialPositionDataInputFileName = ".";
  mhVarOptions->m_initialPositionDataInputFileType = "m";
  mhVarOptions->m_initialProposalCovMatrixDataInputFileName = ".";
  mhVarOptions->m_initialProposalCovMatrixDataInputFileType = "m";
  mhVarOptions->m_rawChainDataInputFileName = ".";
  mhVarOptions->m_rawChainDataInputFileType = "m";
  mhVarOptions->m_rawChainSize = 132000;                                  
   // IMPORTANT
  mhVarOptions->m_rawChainGenerateExtra = 0;
  mhVarOptions->m_rawChainDisplayPeriod = 10;
  mhVarOptions->m_rawChainMeasureRunTimes = 1;
  mhVarOptions->m_rawChainDataOutputPeriod = 10;
  mhVarOptions->m_rawChainDataOutputFileName = "outputData/rawChain_mh";
   // IMPORTANT
  mhVarOptions->m_rawChainDataOutputFileType = "m";                     
   // IMPORTANT
  mhVarOptions->m_rawChainDataOutputAllowAll = 0;                       
   // IMPORTANT
  mhVarOptions->m_rawChainDataOutputAllowedSet.insert(0);               
   // IMPORTANT
  mhVarOptions->m_filteredChainGenerate = 1;                            
   // IMPORTANT
  mhVarOptions->m_filteredChainDiscardedPortion = 0.1;                 
   // IMPORTANT
  mhVarOptions->m_filteredChainLag = 12;                                 \
   // IMPORTANT
   mhVarOptions->m_filteredChainDataOutputFileName = "outputData/filtChain_mh";
   mhVarOptions->m_filteredChainDataOutputFileType = "m";                
  mhVarOptions->m_filteredChainDataOutputAllowAll = 0;                  
  mhVarOptions->m_filteredChainDataOutputAllowedSet.insert(0);          
  mhVarOptions->m_displayCandidates = 0;
  mhVarOptions->m_putOutOfBoundsInChain = 0;
  mhVarOptions->m_tkUseLocalHessian = 0;
  mhVarOptions->m_tkUseNewtonComponent = 1;
  mhVarOptions->m_drMaxNumExtraStages = 1;          // IMPORTANT
  mhVarOptions->m_drScalesForExtraStages.resize(1);
  mhVarOptions->m_drScalesForExtraStages[0] = 5; // IMPORTANT
  mhVarOptions->m_drDuringAmNonAdaptiveInt = 1;     // IMPORTANT
  mhVarOptions->m_amKeepInitialMatrix = 0;          // IMPORTANT
  mhVarOptions->m_amInitialNonAdaptInterval = 100;  // IMPORTANT
  mhVarOptions->m_amAdaptInterval = 100;            // IMPORTANT
  mhVarOptions->m_amAdaptedMatricesDataOutputPeriod = 0;
  mhVarOptions->m_amAdaptedMatricesDataOutputFileName = ".";
  mhVarOptions->m_amAdaptedMatricesDataOutputFileType = "m";
  mhVarOptions->m_amAdaptedMatricesDataOutputAllowAll = 0;
  //mhVarOptions->m_am_adaptedMatrices_dataOutputAllowedSet.insert(0);
  mhVarOptions->m_amEta = 0.45;                     // IMPORTANT
  mhVarOptions->m_amEpsilon = 1e-05;                // IMPORTANT
  mhVarOptions->m_enableBrooksGelmanConvMonitor = 0;
  mhVarOptions->m_BrooksGelmanLag = 100;
                                          
  gcm->calibrateWithBayesMetropolisHastings(mhVarOptions,totalInitialVec,&totalInitialProposalCovMatrix);
 
  Cout << "Got to line 600 \n"; 
  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                          << ": finished calibrating 'gcm'"
                          << std::endl;
  }
 
  //***********************************************************************
  // Step 09 of 09: Make predictions with the calibrated computer model
  //***********************************************************************
  //uqGslVectorClass newExperimentScenarioVec(config_x_space.zeroVector());              // todo_rr
  //uqGslMatrixClass newKmat_interp          (*env,n_eta_space.map(),num_bases_eta);   // todo_rr
  //uqGslMatrixClass newDmat                 (*env,n_eta_space.map(),num_delta_bases); // todo_rr
  //uqGslVectorClass simulationOutputVec     (n_eta_space.zeroVector()); // Yes, size of simulation, since it is a prediction using the emulator
  //uqGslVectorClass discrepancyVec          (n_eta_space.zeroVector());
  //gcm->predictExperimentResults(newExperimentScenarioVec,newKmat_interp,newDmat,simulationOutputVec,discrepancyVec);
 
  //uqGslVectorClass newSimulationScenarioVec (config_x_space.zeroVector()); // todo_rr
  //uqGslVectorClass newSimulationParameterVec(paramSpace.zeroVector()); // todo_rr
  //uqGslVectorClass newSimulationOutputVec   (n_eta_space.zeroVector());
  //gcm->predictSimulationOutputs(newSimulationScenarioVec,newSimulationParameterVec,newSimulationOutputVec);

  // Return
  //delete env;
  delete envOptionsValues;
  delete paramPriorRvPtr;
  delete experimentModelPtr;
  delete experimentStoragePtr;

  //MPI_Finalize();

  return;

}

//void NonDQUESOBayesCalibration::print_results(std::ostream& s)
//{
//  NonDBayesCalibration::print_results(s);
//
//  additional QUESO output
//}

} // namespace Dakota
