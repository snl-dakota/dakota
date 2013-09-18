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
  approxImportFile(probDescDB.get_string("method.import_points_file")),
  approxImportAnnotated(probDescDB.get_bool("method.import_points_file_annotated")),
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
 
  Cout << "import points file "<< approxImportFile;
  Cout << "import points annotated " << approxImportAnnotated;
  if (approxImportFile.empty())
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
  envOptionsValues->m_displayVerbosity     = 3;
  envOptionsValues->m_seed = -1;
  envOptionsValues->m_identifyingString="CASLexample";
  //if (randomSeed) 
  //  envOptionsValues->m_seed                 = randomSeed;
  //else
  //  envOptionsValues->m_seed                 = 1 + (int)clock(); 
      
  //uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"/home/lpswile/dakota/src/gpmsa.inp","",envOptionsValues);
  //uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"mlsampling.inp","",NULL);
  uqFullEnvironmentClass* env = new uqFullEnvironmentClass(MPI_COMM_SELF,"","",envOptionsValues);
 
  // Read in all of the experimental data:  any x configuration 
  // variables, y observations, and y_std if available 
  bool calc_sigma_from_data = true; // calculate sigma if not provided
  expData.load_scalar(expDataFileName, "QUESO/GPMSA Bayes Calibration",
		      numExperiments, 
		      numExpConfigVars, numFunctions, numExpStdDeviationsRead,
		      expDataFileAnnotated, calc_sigma_from_data,
		      outputLevel);

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
    paramMins[i]=0.;//lower_bounds[i];
    paramMaxs[i]=1.00000001;//upper_bounds[i]; // prudenci 2013-08-25
  }
 
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);
  Cout << "got to line 219" ;

#if 0
  uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>* paramPriorRvPtr = NULL;
  paramPriorRvPtr = new uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>("prior_",paramDomain);
#else
  uqGaussianVectorRVClass<uqGslVectorClass,uqGslMatrixClass>* paramPriorRvPtr = NULL; // prudenci_new_2013_09_06
  uqGslVectorClass meanVec(paramSpace.zeroVector()); // prudenci_new_2013_09_06
  meanVec[0] = 0.5; // prudenci_new_2013_09_06
  meanVec[1] = 0.5; // prudenci_new_2013_09_06
  meanVec[2] = 0.5; // prudenci_new_2013_09_06
  uqGslMatrixClass covMat(paramSpace.zeroVector()); // prudenci_new_2013_09_06
  covMat(0,0) = 10.*10.; // prudenci_new_2013_09_06
  covMat(1,1) = 10.*10.; // prudenci_new_2013_09_06
  covMat(2,2) = 10.*10.; // prudenci_new_2013_09_06
  paramPriorRvPtr = new uqGaussianVectorRVClass<uqGslVectorClass,uqGslMatrixClass>("prior_",paramDomain,meanVec,covMat); // prudenci_new_2013_09_06
#endif
 

  Cout << "got to line 218\n";
  //***********************************************************************
  // Step 02 of 09: Instantiate the 'scenario' and 'output' spaces
  //***********************************************************************
  //int num_config_vars = numContStateVars;
  int num_config_vars = 1;
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

  std::vector<unsigned int> simulationChunkSizes(2,0.); // prudenci_new_2013_09_06
  simulationChunkSizes[0] = 1001; // prudenci_new_2013_09_06
  simulationChunkSizes[1] = 1001; // prudenci_new_2013_09_06

  uqSimulationStorageClass<uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass,uqGslVectorClass,uqGslMatrixClass>
    simulationStorage(config_x_space,
                      paramSpace,
                      n_eta_space,
#ifdef UQ_GPMSA_CODE_TREATS_SIMULATION_VECTORS_IN_CHUNKS // prudenci_new_2013_09_06
                      simulationChunkSizes, // prudenci_new_2013_09_06
#endif // prudenci_new_2013_09_06
                      num_simulations);

 
  // Add simulations: what if none?
  // We have to dimension these properly. 
  double num_each_grid = num_eta/2;
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    n_grid(*env, "output_", num_each_grid, NULL);
  uqGslVectorClass extraSimulationGridVec(n_eta_space.zeroVector());
  uqGslVectorClass extraSimulationGridVec_forUvel(n_grid.zeroVector());
  uqGslVectorClass extraSimulationGridVec_forVvel(n_grid.zeroVector());
  std::vector<uqGslVectorClass* > simulationScenarios(num_simulations,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > paramVecs(num_simulations,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > outputVecs(num_simulations,(uqGslVectorClass*) NULL);
            
  for (int i = 0; i < num_simulations; ++i) {
    simulationScenarios[i] = new uqGslVectorClass(config_x_space.zeroVector());   // 'x_{i+1}^*' in paper
    paramVecs          [i] = new uqGslVectorClass(paramSpace.zeroVector());   // 't_{i+1}^*' in paper
    outputVecs         [i] = new uqGslVectorClass(n_eta_space.zeroVector()); // 'eta_{i+1}' in paper
  }
                           
  // Populate all objects just instantiated
  //for (int i = 0; i<numFunctions; i++) 
  //  extraSimulationGridVec[i]=1.5+1.5*i;
  extraSimulationGridVec_forUvel[0]=0.0;
  extraSimulationGridVec_forVvel[0]=0.0;
  for (int i = 1; i<1001; i++){ 
    extraSimulationGridVec_forUvel[i]=extraSimulationGridVec_forUvel[i-1]+0.001;
    extraSimulationGridVec_forVvel[i]=extraSimulationGridVec_forVvel[i-1]+0.001;
  }
  //extraSimulationGridVec[1001]=0.0;
  //for (int i = 52; i<102; i++) 
  //  extraSimulationGridVec[i]=extraSimulationGridVec[i-1]+0.02;
  Cout << "Extra Simulation Grid Vec1 " << extraSimulationGridVec_forUvel << '\n';
  Cout << "Extra Simulation Grid Vec2 " << extraSimulationGridVec_forVvel << '\n';

  RealVector temp_cvars;
  uqGslVectorClass gsl_cvars(paramSpace.zeroVector());
  uqGslVectorClass gsl_configvars(config_x_space.zeroVector());
  int i,j,k;
  if (approxImportFile.empty())
  { 
    const RealMatrix&  all_samples = lhsIter.all_samples();
    const IntResponseMap& all_resp = lhsIter.all_responses();
    for (i = 0; i < num_simulations; ++i) {
      temp_cvars = Teuchos::getCol(Teuchos::View, const_cast<RealMatrix&>(all_samples), i);
      for (j=0; j<numUncertainVars;++j){
        gsl_cvars[j]=temp_cvars[j];
        Cout << "temp_cvars j" << temp_cvars[j] << '\n';
      }
      for (j=0; j<numUncertainVars;++j)
        (*(paramVecs[i]))[j] = gsl_cvars[j];
      Cout << *(paramVecs[i]) << '\n';
    //for (j=0; j<numStateVars;++j) {
    //  gsl_configvars[j]=temp_cvars[j+numUncertainVars];
    //  Cout << "config_vars j" << temp_cvars[j+numUncertainVars] << '\n';
    //}
     // for (j=0; j<numStateVars;++j)
    //  (*(simulationScenarios[i]))[j] = gsl_configvars[j];
    // Cout << *(simulationScenarios[i]) << '\n';
        (*(simulationScenarios[i]))[0]=0.5;
      Cout << *(simulationScenarios[i]) << '\n';
    }

  //(*(paramVecs[0]))[0]=0.2105;
  //(*(paramVecs[1]))[0]=0.1795;
  //(*(paramVecs[2]))[0]=0.1167;
  //(*(paramVecs[3]))[0]=0.1457;
  //(*(paramVecs[4]))[0]=0.0610;
  //(*(paramVecs[5]))[0]=0.2376;
  //(*(paramVecs[6]))[0]=0.0982;
  //(*(paramVecs[7]))[0]=0.1072;
  //(*(paramVecs[8]))[0]=0.0742;
  //(*(paramVecs[9]))[0]=0.0979;
  //(*(paramVecs[10]))[0]=0.1639;
  //(*(paramVecs[11]))[0]=0.2275;
  //(*(paramVecs[12]))[0]=0.1977;
  //(*(paramVecs[13]))[0]=0.2237;
  //(*(paramVecs[14]))[0]=0.1914;
  //(*(paramVecs[15]))[0]=0.2466;
  //(*(paramVecs[16]))[0]=0.0702;
  //(*(paramVecs[17]))[0]=0.1345;
  //(*(paramVecs[18]))[0]=0.1564;
  //(*(paramVecs[19]))[0]=0.1762;
  //(*(paramVecs[20]))[0]=0.1498;
  //(*(paramVecs[21]))[0]=0.1236;
  //(*(paramVecs[22]))[0]=0.2087;
  //(*(paramVecs[23]))[0]=0.0544;
  //(*(paramVecs[24]))[0]=0.0885;
 
  //(*(simulationScenarios[0]))[0] = 0.5;
 // (*(simulationScenarios[1]))[0] = 0.5;
 // (*(simulationScenarios[2]))[0] = 0.5;


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
  }
  else {
    bool verbose=(outputLevel>NORMAL_OUTPUT);
    RealMatrix the_data;
    TabularIO::read_data_tabular(approxImportFile,"GPMSA input points",the_data,num_simulations,(numUncertainVars+numFunctions),approxImportAnnotated,verbose); 
    RealVector temp_resp(numFunctions);
    for (i = 0; i < num_simulations; ++i) {
      for (j=0; j<(numUncertainVars);++j){
        //if (j<numUncertainVars) {
         gsl_cvars[j]=the_data[j][i];
         Cout << "gsl_cvars j" << gsl_cvars[j] << '\n';
        //} 
        //else
        //temp_resp[j-3] = the_data[j][i];
      }
      for (j=0; j<numUncertainVars;++j)
        (*(paramVecs[i]))[j] = gsl_cvars[j];
      Cout << *(paramVecs[i]) << '\n';
      for (k=0; k<numFunctions; k++)
        (*(outputVecs[i]))[k]=the_data[numUncertainVars+k][i];
      Cout << *(outputVecs[i]) << '\n';
      (*(simulationScenarios[i]))[0]=0.5;
      Cout << *(simulationScenarios[i]) << '\n';
    }

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
  smVarOptions->m_p_eta = 3;
  smVarOptions->m_zeroRelativeSingularValue = 0.;
  smVarOptions->m_cdfThresholdForPEta = 0.;
  smVarOptions->m_a_w = 5.;
  smVarOptions->m_b_w = 5.;
  smVarOptions->m_a_rho_w = 1.;
  smVarOptions->m_b_rho_w = 0.1;
  smVarOptions->m_a_eta = 5.;
  smVarOptions->m_b_eta = 0.005;
  smVarOptions->m_a_s = 1.;
  smVarOptions->m_b_s = 0.0001;

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
  std::vector<unsigned int> experimentDimsEach(numExperiments,0);
  std::vector<uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>* > experimentSpaces          (numExperiments,(uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>*) NULL);
  std::vector<uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>* > eachResponseSpace          (numExperiments,(uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>*) NULL);
  std::vector<uqGslVectorClass* > extraExperimentGridVecs(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > extraExperimentGridVecs_forUvel(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > extraExperimentGridVecs_forVvel(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_original   (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_auxMean    (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_auxMean1    (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_auxMean2    (numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslVectorClass* > experimentVecs_transformed(numExperiments,(uqGslVectorClass*) NULL);
  std::vector<uqGslMatrixClass* > experimentMats_original   (numExperiments,(uqGslMatrixClass*) NULL);
  std::vector<uqGslMatrixClass* > experimentMats_transformed(numExperiments,(uqGslMatrixClass*) NULL);

  //for (i = 0; i < numExperiments; ++i) {
    //experimentDims[i] = numFunctions; // constant for now
  //}
  experimentDims[0]=54; 
  //experimentDims[1]=4; 
  //experimentDims[2]=3;
  experimentDimsEach[0]=27;
  Cout << "Got to line 338 " << '\n';
 
  for (i = 0; i < numExperiments; ++i) {
    experimentScenarios_original[i] = new uqGslVectorClass(config_x_space.zeroVector());               // 'x_{i+1}' in paper
    experimentScenarios_standard[i] = new uqGslVectorClass(config_x_space.zeroVector());     
    experimentSpaces[i] = new uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>(*env, "expSpace", experimentDims[i], NULL);
    eachResponseSpace[i] = new uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>(*env, "expSpace", experimentDimsEach[i], NULL);
    extraExperimentGridVecs[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    extraExperimentGridVecs_forUvel[i] = new uqGslVectorClass(eachResponseSpace[i]->zeroVector());          //
    extraExperimentGridVecs_forVvel[i] = new uqGslVectorClass(eachResponseSpace[i]->zeroVector());          //
    experimentVecs_original[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    experimentVecs_auxMean[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          //
    experimentVecs_auxMean1[i] = new uqGslVectorClass(eachResponseSpace[i]->zeroVector());          //
    experimentVecs_auxMean2[i] = new uqGslVectorClass(eachResponseSpace[i]->zeroVector());          //
    experimentVecs_transformed[i] = new uqGslVectorClass(experimentSpaces[i]->zeroVector());          // 'y_{i+1}' in paper
    experimentMats_original   [i] = new uqGslMatrixClass(experimentSpaces[i]->zeroVector());          //
    experimentMats_transformed[i] = new uqGslMatrixClass(experimentSpaces[i]->zeroVector());          // 'W_{i+1}' in paper
  }

  //***********************************************************************
  // Populate information regarding experiment '0'
  //***********************************************************************
#if 0
  for (unsigned int i = 0; i < numExperiments; ++i) {
    // config vars are same for all functions for now
    size_t fn_ind = 0;
    const RealVector& xobs_i = expData.config_vars(fn_ind, i);
    for (unsigned int j = 0; j < num_config_vars; ++j) 
      (*(experimentScenarios_original[i]))[j] = xobs_i[j];
    Cout << *(experimentScenarios_original[i]) << '\n';
  }
#else
// prudenci 2013-08-25
   for (unsigned int i = 0; i < numExperiments; ++i) {
     for (unsigned int j = 0; j < num_config_vars; ++j) {
       (*(experimentScenarios_original[i]))[j] = 0.5;
     }
     std::cout << *(experimentScenarios_original[i]) << '\n';
   }
#endif
  for (unsigned int i = 0; i < numExperiments; ++i) {
    *(experimentScenarios_standard[i])  = *(experimentScenarios_original[i]);
   // *(experimentScenarios_standard[i]) -= simulationModel.xSeq_original_mins();
   // for (unsigned int j = 0; j < num_config_vars; ++j) {
   //   (*(experimentScenarios_standard[i]))[j] /= simulationModel.xSeq_original_ranges()[j];
   // }
  }
  //(*(experimentScenarios[0]))[0] = 0.1; // 'x_1' in paper; Radius in tower example

	 (*(extraExperimentGridVecs_forUvel[0]))[0] = 0.00000E+0;   
	 (*(extraExperimentGridVecs_forUvel[0]))[1] = 0.008219;   
	 (*(extraExperimentGridVecs_forUvel[0]))[2] = 0.010959;   
         (*(extraExperimentGridVecs_forUvel[0]))[3] = 0.021918;
	 (*(extraExperimentGridVecs_forUvel[0]))[4] = 0.035616;
 	 (*(extraExperimentGridVecs_forUvel[0]))[5] = 0.049315;
	 (*(extraExperimentGridVecs_forUvel[0]))[6] = 0.065753;
	 (*(extraExperimentGridVecs_forUvel[0]))[7] = 0.10137;
	 (*(extraExperimentGridVecs_forUvel[0]))[8] = 0.131507;
 	 (*(extraExperimentGridVecs_forUvel[0]))[9] = 0.161644;
	 (*(extraExperimentGridVecs_forUvel[0]))[10] = 0.20274;
	 (*(extraExperimentGridVecs_forUvel[0]))[11] = 0.29863;
	 (*(extraExperimentGridVecs_forUvel[0]))[12] = 0.39726;
	 (*(extraExperimentGridVecs_forUvel[0]))[13] = 0.50137;
	 (*(extraExperimentGridVecs_forUvel[0]))[14] = 0.6;
	 (*(extraExperimentGridVecs_forUvel[0]))[15] = 0.70137;
	 (*(extraExperimentGridVecs_forUvel[0]))[16] = 0.80274;
	 (*(extraExperimentGridVecs_forUvel[0]))[17] = 0.843836;
	 (*(extraExperimentGridVecs_forUvel[0]))[18] = 0.863014;
	 (*(extraExperimentGridVecs_forUvel[0]))[19] = 0.9013699;
	 (*(extraExperimentGridVecs_forUvel[0]))[20] = 0.9369863;
	 (*(extraExperimentGridVecs_forUvel[0]))[21] = 0.9534247;
	 (*(extraExperimentGridVecs_forUvel[0]))[22] = 0.969863;
	 (*(extraExperimentGridVecs_forUvel[0]))[23] =  0.9835616;
	 (*(extraExperimentGridVecs_forUvel[0]))[24] = 0.99178082;
	 (*(extraExperimentGridVecs_forUvel[0]))[25] = 0.99452055;
	 (*(extraExperimentGridVecs_forUvel[0]))[26] = 1.00000E+0;  
 	 (*(extraExperimentGridVecs_forVvel[0]))[0] = 5.55112E-17;  
	 (*(extraExperimentGridVecs_forVvel[0]))[1] = 8.06452E-3;   
	 (*(extraExperimentGridVecs_forVvel[0]))[2] = 5.37634E-3;   
	 (*(extraExperimentGridVecs_forVvel[0]))[3] = 1.61290E-2;   
	 (*(extraExperimentGridVecs_forVvel[0]))[4] = 3.22581E-2;   
	 (*(extraExperimentGridVecs_forVvel[0]))[5] = 4.83871E-2;   
	 (*(extraExperimentGridVecs_forVvel[0]))[6] = 6.18280E-2;   
	 (*(extraExperimentGridVecs_forVvel[0]))[7] = 9.67742E-2;   
	 (*(extraExperimentGridVecs_forVvel[0]))[8] = 1.29032E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[9] = 1.58602E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[10] = 1.98925E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[11] = 2.98387E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[12] = 3.97849E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[13] = 4.97312E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[14] = 5.99462E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[15] = 6.98925E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[16] = 7.98387E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[17] = 8.38710E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[18] = 8.70968E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[19] = 9.00538E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[20] = 9.30108E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[21] = 9.51613E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[22] = 9.62366E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[23] = 9.81183E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[24] = 9.91935E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[25] = 9.91935E-1;   
	 (*(extraExperimentGridVecs_forVvel[0]))[26] = 9.97312E-1; 
   // (*(extraExperimentGridVecs[0]))[0] = ;
   // (*(extraExperimentGridVecs[0]))[1] = 10.;
   // (*(extraExperimentGridVecs[0]))[2] = 15.;
   // (*(extraExperimentGridVecs[0]))[3] = 20.;
   // (*(extraExperimentGridVecs[1]))[0] = 5.;
   // (*(extraExperimentGridVecs[1]))[1] = 10.;
   // (*(extraExperimentGridVecs[1]))[2] = 15.;
   // (*(extraExperimentGridVecs[1]))[3] = 20.;
   // (*(extraExperimentGridVecs[2]))[0] = 5.;
   // (*(extraExperimentGridVecs[2]))[1] = 10.;
   // (*(extraExperimentGridVecs[2]))[2] = 15.;

  for (unsigned int i = 0; i < numExperiments; ++i) {
    for (unsigned int j = 0; j < experimentDims[i]; ++j) { 
      size_t replicate = 0;
      Real yobs = expData.scalar_data(j, i, replicate);
      (*(experimentVecs_original[i]))[j] = yobs;
    }
    Cout << *(experimentVecs_original[i]) << '\n';
    //Cout << *(extraExperimentGridVecs[i]) << '\n';
    Cout << *(extraExperimentGridVecs_forUvel[i]) << '\n';
    Cout << *(extraExperimentGridVecs_forVvel[i]) << '\n';
  }
  Cout << "Got to line 367 " << '\n';
  for (unsigned int i = 0; i < numExperiments; ++i) {
    uqGslVectorClass simulationModelEtaSeq1(n_grid.zeroVector());
    uqGslVectorClass simulationModelEtaSeq2(n_grid.zeroVector());
    for (int j = 0; j < 1001; ++j) {
      simulationModelEtaSeq1[j]=simulationModel.etaSeq_original_mean()[j];
      simulationModelEtaSeq2[j]=simulationModel.etaSeq_original_mean()[1001+j];
    }
    Cout << "simulationModelEtaSeq1 " << simulationModelEtaSeq1 << '\n';
    Cout << "simulationModelEtaSeq2 " << simulationModelEtaSeq2 << '\n';
 
    experimentVecs_auxMean1[i]->matlabLinearInterpExtrap(extraSimulationGridVec_forUvel, simulationModelEtaSeq1,*(extraExperimentGridVecs_forUvel[i]));
    experimentVecs_auxMean2[i]->matlabLinearInterpExtrap(extraSimulationGridVec_forVvel, simulationModelEtaSeq2,*(extraExperimentGridVecs_forVvel[i]));
    Cout << "matlab interpolation done" << std::endl;
    for (int j = 0; j < 27; ++j) {
      (*experimentVecs_auxMean[i])[j]=(*experimentVecs_auxMean1[i])[j];
    }
    for (int j = 0; j < 27; ++j) {
      (*experimentVecs_auxMean[i])[j+27]=(*experimentVecs_auxMean2[i])[j];
    }
    Cout << *(experimentVecs_auxMean[i]) << '\n';
    
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
         *env->subDisplayFile() << "In compute(), step 05, experiment " << i 
                          << "\n  extraSimulationGridVec = "              << extraSimulationGridVec
                          << "\n  simulationModel.etaSeq_original_mean() = " << simulationModel.etaSeq_original_mean()
                          << "\n  *(extraExperimentGridVecs[i]) = "       << *(extraExperimentGridVecs[i])
                          << "\n  *(experimentVecs_auxMean[i]) = "           << *(experimentVecs_auxMean[i])
                          << std::endl;
    }
#ifdef UQ_GPMSA_CODE_TREATS_SIMULATION_VECTORS_IN_CHUNKS // prudenci_new_2013_09_06
    for (unsigned int j = 0; j < 27; ++j) { // prudenci_new_2013_09_06
      (*(experimentVecs_transformed[i]))[j   ] = (1./simulationModel.etaSeq_chunkStd(0)) * ( (*(experimentVecs_original[i]))[j   ] - (*(experimentVecs_auxMean[i]))[j   ] ); // 'y_1' in paper // prudenci_new_2013_09_06
    } // prudenci_new_2013_09_06
    for (unsigned int j = 0; j < 27; ++j) { // prudenci_new_2013_09_06
      (*(experimentVecs_transformed[i]))[j+27] = (1./simulationModel.etaSeq_chunkStd(1)) * ( (*(experimentVecs_original[i]))[j+27] - (*(experimentVecs_auxMean[i]))[j+27] ); // 'y_1' in paper // prudenci_new_2013_09_06
    } // prudenci_new_2013_09_06
#else // prudenci_new_2013_09_06
    *(experimentVecs_transformed[i]) = (1./simulationModel.etaSeq_allStd()) * ( *(experimentVecs_original[i]) - *(experimentVecs_auxMean[i]) ); // 'y_1' in paper
#endif // prudenci_new_2013_09_06
    if ((env->subDisplayFile()) && (env->displayVerbosity() >= 2)) {
     *env->subDisplayFile() << "In compute(), step 05, experiment " << i
                          << "\n  *(experimentVecs_original[i]) = "    << *(experimentVecs_original[i])
                          << "\n  *(experimentVecs_auxMean[i]) = "     << *(experimentVecs_auxMean[i])
#ifdef UQ_GPMSA_CODE_TREATS_SIMULATION_VECTORS_IN_CHUNKS // prudenci_new_2013_09_06
#else // prudenci_new_2013_09_06
                          << "\n  simulationModel.etaSeq_allStd() = "  << simulationModel.etaSeq_allStd()
#endif // prudenci_new_2013_09_06
                          << "\n  *(experimentVecs_transformed[i]) = " << *(experimentVecs_transformed[i])
                          << std::endl;
    }
  }

  Cout << "Got to line 367A " << '\n';
  for (unsigned int i = 0; i < numExperiments; ++i) {
    for (unsigned int j = 0; j < experimentDims[i] ; ++j) {
      (*(experimentMats_original[i]))(j,j) = 0.0001;
    }
  }

  for (unsigned int i = 0; i < numExperiments; ++i) {
#ifdef UQ_GPMSA_CODE_TREATS_SIMULATION_VECTORS_IN_CHUNKS // prudenci_new_2013_09_06
    for (unsigned int j = 0; j < 27; ++j) { // prudenci_new_2013_09_06
      (*(experimentMats_transformed[i]))(j   ,j   ) = (*(experimentMats_original[i]))(j   ,j   )/(simulationModel.etaSeq_chunkStd(0)*simulationModel.etaSeq_chunkStd(0)); // prudenci_new_2013_09_06
    } // prudenci_new_2013_09_06
    for (unsigned int j = 0; j < 27; ++j) { // prudenci_new_2013_09_06
      (*(experimentMats_transformed[i]))(j+27,j+27) = (*(experimentMats_original[i]))(j+27,j+27)/(simulationModel.etaSeq_chunkStd(1)*simulationModel.etaSeq_chunkStd(1)); // prudenci_new_2013_09_06
    } // prudenci_new_2013_09_06
#else // prudenci_new_2013_09_06
    for (unsigned int j = 0; j < experimentDims[i] ; ++j) {
      (*(experimentMats_transformed[i]))(j,j) = (*(experimentMats_original[i]))(j,j)/simulationModel.etaSeq_allStd(); // different
    }
#endif // prudenci_new_2013_09_06
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
  unsigned int num_delta_bases_forUvel = 7;
  unsigned int num_delta_bases_forVvel = 7;
  unsigned int num_delta_bases =  num_delta_bases_forUvel + num_delta_bases_forVvel; // number of experiment basis; 'p_delta' in paper; 13 in tower example

  double kernelSigma = 0.1;
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> oneDSpace(*env, "oneDSpace", 1, NULL);
  uqGslVectorClass oneDVec(oneDSpace.zeroVector());
  uqGslMatrixClass oneDMat(oneDSpace.zeroVector());
  oneDMat(0,0) = kernelSigma*kernelSigma;

  //***********************************************************************
  // Form and compute 'DsimMat'
  // Not mentioned in the paper
  // 'Dsim' in the GPMSA tower example document (page 11)
  //***********************************************************************
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> delta_space(*env, "delta_space", num_delta_bases, NULL);

  uqGslVectorClass kernelSigmas (delta_space.zeroVector()); // prudenci_new_2013_09_06
  kernelSigmas[ 0] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[ 1] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[ 2] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[ 3] = 0.05; // prudenci_new_2013_09_06
  kernelSigmas[ 4] = 0.15; // prudenci_new_2013_09_06
  kernelSigmas[ 5] = 0.25; // prudenci_new_2013_09_06
  kernelSigmas[ 6] = 0.25; // prudenci_new_2013_09_06
  kernelSigmas[ 7] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[ 8] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[ 9] = 0.025; // prudenci_new_2013_09_06
  kernelSigmas[10] = 0.05; // prudenci_new_2013_09_06
  kernelSigmas[11] = 0.15; // prudenci_new_2013_09_06
  kernelSigmas[12] = 0.25; // prudenci_new_2013_09_06
  kernelSigmas[13] = 0.25; // prudenci_new_2013_09_06

  uqGslVectorClass kernelCenters(delta_space.zeroVector());
  kernelCenters[ 0] = 0.0; // For 'u' velocity
  kernelCenters[ 1] = 0.025;
  kernelCenters[ 2] = 0.05;
  kernelCenters[ 3] = 0.1;
  kernelCenters[ 4] = 0.25;
  kernelCenters[ 5] = 0.5;
  kernelCenters[ 6] = 0.75;
  kernelCenters[ 7] = 0.0; // For 'v' velocity
  kernelCenters[ 8] = 0.025;
  kernelCenters[ 9] = 0.05;
  kernelCenters[10] = 0.1;
  kernelCenters[11] = 0.25;
  kernelCenters[12] = 0.5;
  kernelCenters[13] = 0.75;

  uqGslMatrixClass DsimMat(*env,n_eta_space.map(),num_delta_bases); // Important matrix (not mentioned on paper)
  uqGslVectorClass DsimCol(n_eta_space.zeroVector());
  unsigned int num_eta_forUvel = 1001;
  unsigned int num_eta_forVvel = 1001;
  Cout << "Before we do 1st subgroup of DsimMat" << '\n';
  // Take care of 1st subgroup of columns in 'DsimMat'
  for (unsigned int colId = 0; colId < num_delta_bases_forUvel; ++colId) {
    oneDVec[0] = kernelCenters[colId];
    oneDMat(0,0) = kernelSigmas[colId]*kernelSigmas[colId]; // prudenci_new_2013_09_06
    uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
    for (unsigned int rowId = 0; rowId < num_eta_forUvel; ++rowId) {
      oneDVec[0] = extraSimulationGridVec_forUvel[rowId];
      DsimCol[rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
    }
    DsimMat.setColumn(colId,DsimCol);
  }
  Cout << "Before we do 2nd subgroup of DsimMat" << '\n';

  // Take care of 2nd subgroup of columns in 'DsimMat'
  for (unsigned int colId = num_delta_bases_forUvel/* Yes, forU */; colId < num_delta_bases; ++colId) {
    oneDVec[0] = kernelCenters[colId];
    oneDMat(0,0) = kernelSigmas[colId]*kernelSigmas[colId]; // prudenci_new_2013_09_06
    uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
    DsimCol.cwSet(0.);
    for (unsigned int rowId = 0; rowId < num_eta_forVvel; ++rowId) {
      oneDVec[0] = extraSimulationGridVec_forVvel[rowId];
      DsimCol[num_eta_forUvel/* Yes, forU */ + rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
    }
    DsimMat.setColumn(colId,DsimCol);
  }
  Cout << "Got to line 435 A \n";

  //***********************************************************************
  // Populate information regarding experiment 'i'
  //   'D_{i+1}' in the paper
  //   'Dobs' in the GPMSA tower example document (page 11)
  //***********************************************************************
  std::vector<uqGslMatrixClass* > DobsMats(numExperiments, (uqGslMatrixClass*) NULL);
  DobsMats.resize(numExperiments, (uqGslMatrixClass*) NULL); // Important matrices (D_i's on paper)
  for (unsigned int i = 0; i < numExperiments; ++i) {
    DobsMats[i] = new uqGslMatrixClass(*env,experimentSpaces[i]->map(),num_delta_bases); // 'D_{i+1}' in paper
    uqGslVectorClass DobsCol(experimentSpaces[i]->zeroVector());
    unsigned int num_experimentPoints_forUvel = 27; // check
    unsigned int num_experimentPoints_forVvel = 27;

    // Take care of 1st subgroup of columns in 'DobsMats[i]'
    for (unsigned int colId = 0; colId < num_delta_bases_forUvel; ++colId) {
      oneDVec[0] = kernelCenters[colId];
      oneDMat(0,0) = kernelSigmas[colId]*kernelSigmas[colId]; // prudenci_new_2013_09_06
      uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
      for (unsigned int rowId = 0; rowId < num_experimentPoints_forUvel; ++rowId) {
        oneDVec[0] = (*(extraExperimentGridVecs_forUvel[i]))[rowId];
        DobsCol[rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
      }
      DobsMats[i]->setColumn(colId,DobsCol);
    }

    // Take care of 2nd subgroup of columns in 'DobsMats[i]'
    for (unsigned int colId = num_delta_bases_forUvel/* Yes, forU */; colId < num_delta_bases; ++colId) {
      oneDVec[0] = kernelCenters[colId];
      oneDMat(0,0) = kernelSigmas[colId]*kernelSigmas[colId]; // prudenci_new_2013_09_06
      uqGaussianJointPdfClass<uqGslVectorClass,uqGslMatrixClass> kernelPdf("",oneDSpace,oneDVec,oneDMat);
      DobsCol.cwSet(0.);
      for (unsigned int rowId = 0; rowId < num_experimentPoints_forVvel; ++rowId) {
        oneDVec[0] = (*(extraExperimentGridVecs_forVvel[i]))[rowId];
        DobsCol[num_experimentPoints_forUvel/* Yes, forU */ + rowId] = kernelPdf.actualValue(oneDVec,NULL,NULL,NULL,NULL);
      }
      DobsMats[i]->setColumn(colId,DobsCol);
    }
  }

  Cout << "Got to line 435 B \n";
  //***********************************************************************
  // Normalize 'DsimMat' and all 'DobsMats'
  //***********************************************************************

  // Extract submatrices from 'DsimMat'
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> n_eta_space_forUvel(*env, "m_eta_space_forUvel", num_eta_forUvel, NULL);
  uqGslMatrixClass DsimMat_forUvel(*env,n_eta_space_forUvel.map(),num_delta_bases_forUvel);
  for (unsigned int i = 0; i < num_eta_forUvel; ++i) {
    for (unsigned int j = 0; j < num_delta_bases_forUvel; ++j) {
      DsimMat_forUvel(i,j) = DsimMat(i,j);
    }
  }

  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> n_eta_space_forVvel(*env, "m_eta_space_forVvel", num_eta_forVvel, NULL);
  uqGslMatrixClass DsimMat_forVvel(*env,n_eta_space_forVvel.map(),num_delta_bases_forVvel);
  for (unsigned int i = 0; i < num_eta_forVvel; ++i) {
    for (unsigned int j = 0; j < num_delta_bases_forVvel; ++j) {
      DsimMat_forVvel(i,j) = DsimMat(num_eta_forUvel/* Yes, forU*/ + i,num_delta_bases_forUvel/* Yes, forU*/ + j);
    }
  }

  // Normalize
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> delta_space_forUvel(*env, "delta_space_forUvel", num_delta_bases_forUvel, NULL);
  uqGslMatrixClass DsimMat_forUvelTranspose(*env,delta_space_forUvel.map(),num_eta_forUvel);
  DsimMat_forUvelTranspose.fillWithTranspose(0,0,DsimMat_forUvel,true,true);
  double Dmax_forUvel = (DsimMat_forUvel * DsimMat_forUvelTranspose).max();
  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                           << ": Dmax_forUvel = " << Dmax_forUvel
                           << std::endl;
  }
  DsimMat_forUvel /= std::sqrt(Dmax_forUvel);

  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> delta_space_forVvel(*env, "delta_space_forVvel", num_delta_bases_forVvel, NULL);
  uqGslMatrixClass DsimMat_forVvelTranspose(*env,delta_space_forVvel.map(),num_eta_forVvel);
  DsimMat_forVvelTranspose.fillWithTranspose(0,0,DsimMat_forVvel,true,true);
  double Dmax_forVvel = (DsimMat_forVvel * DsimMat_forVvelTranspose).max();
  if (env->subDisplayFile()) {
    *env->subDisplayFile() << "In compute()"
                           << ": Dmax_forVvel = " << Dmax_forVvel
                           << std::endl;
  }
  DsimMat_forVvel /= std::sqrt(Dmax_forVvel);

  // Write values back to 'DsimMat'
  for (unsigned int i = 0; i < num_eta_forUvel; ++i) {
    for (unsigned int j = 0; j < num_delta_bases_forUvel; ++j) {
      DsimMat(i,j) = DsimMat_forUvel(i,j);
    }
  }

  for (unsigned int i = 0; i < num_eta_forVvel; ++i) {
    for (unsigned int j = 0; j < num_delta_bases_forVvel; ++j) {
      DsimMat(num_eta_forUvel/* Yes, forU*/ + i,num_delta_bases_forUvel/* Yes, forU*/ + j) = DsimMat_forVvel(i,j);
    }
  }

  // Print out 'DsimMat'
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
    // Extract submatrices from 'DobsMats[i]'
    unsigned int num_experimentPoints_forUvel = 27; // check
    unsigned int num_experimentPoints_forVvel = 27;

    uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> experiment_space_forUvel(*env, "experiment_space_forUvel", num_experimentPoints_forUvel, NULL);
    uqGslMatrixClass DobsMat_forUvel(*env,experiment_space_forUvel.map(),num_delta_bases_forUvel);
    for (unsigned int j = 0; j < num_experimentPoints_forUvel; ++j) {
      for (unsigned int k = 0; k < num_delta_bases_forUvel; ++k) {
        DobsMat_forUvel(j,k) = (*(DobsMats[i]))(j,k);
      }
    }

    uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> experiment_space_forVvel(*env, "experiment_space_forVvel", num_experimentPoints_forVvel, NULL);
    uqGslMatrixClass DobsMat_forVvel(*env,experiment_space_forVvel.map(),num_delta_bases_forVvel);
    for (unsigned int j = 0; j < num_experimentPoints_forVvel; ++j) {
      for (unsigned int k = 0; k < num_delta_bases_forVvel; ++k) {
        DobsMat_forVvel(j,k) = (*(DobsMats[i]))(num_experimentPoints_forUvel/* Yes, forU*/ + j,num_delta_bases_forUvel/* Yes, forU*/ + k);
      }
    }

    // Normalize
    DobsMat_forUvel /= std::sqrt(Dmax_forUvel);
    DobsMat_forVvel /= std::sqrt(Dmax_forVvel);

    // Write values back to 'DobsMats[i]'
    for (unsigned int j = 0; j < num_experimentPoints_forUvel; ++j) {
      for (unsigned int k = 0; k < num_delta_bases_forUvel; ++k) {
        (*(DobsMats[i]))(j,k) = DobsMat_forUvel(j,k);
      }
    }

    for (unsigned int j = 0; j < num_experimentPoints_forVvel; ++j) {
      for (unsigned int k = 0; k < num_delta_bases_forVvel; ++k) {
        (*(DobsMats[i]))(num_experimentPoints_forUvel/* Yes, forU*/ + j,num_delta_bases_forUvel/* Yes, forU*/ + k) = DobsMat_forVvel(j,k);
      }
    }

    // Print out 'DobsMats[i]'
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

  Cout << "Got to line 435 C \n";
  
  //***********************************************************************
  // Compute 'K_i' matrices (Kobs in the matlab documentation)
  //***********************************************************************
  std::vector<uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>* > Kmats_interp_spaces(numExperiments, (uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>*) NULL);
  std::vector<uqGslMatrixClass*                                      > Kmats_interp       (numExperiments, (uqGslMatrixClass*) NULL); // Interpolations of 'Kmat_eta' = 'K_i's' in paper

  // Extract submatrices from 'Kmat_eta' (Ksim in the matlab documentation)
  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> Kmat_eta_space_forUvel(*env,"Kmat_interp_space_forUvel",num_eta_forUvel,NULL);
  uqGslMatrixClass                                      Kmat_eta_forUvel      (*env,Kmat_eta_space_forUvel.map(),num_bases_eta);
  for (unsigned int i = 0; i < num_eta_forUvel; ++i) {
    for (unsigned int j = 0; j < num_bases_eta; ++j) {
      Kmat_eta_forUvel(i,j) = simulationModel.Kmat_eta()(i,j);
    }
  }

  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> Kmat_eta_space_forVvel(*env,"Kmat_interp_space_forVvel",num_eta_forVvel,NULL);
  uqGslMatrixClass                                      Kmat_eta_forVvel      (*env,Kmat_eta_space_forVvel.map(),num_bases_eta);
  for (unsigned int i = 0; i < num_eta_forVvel; ++i) {
    for (unsigned int j = 0; j < num_bases_eta; ++j) {
      Kmat_eta_forVvel(i,j) = simulationModel.Kmat_eta()(num_eta_forUvel/* Yes, forU */ + i,j);
    }
  }

  for (unsigned int i = 0; i < numExperiments; ++i) {
    unsigned int num_experimentPoints_forUvel = 27;
    unsigned int num_experimentPoints_forVvel = 27;

    // Interpolate
    uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> Kmat_interp_space_forUvel(*env,"Kmat_interp_space_forUvel",num_experimentPoints_forUvel,NULL);
    uqGslMatrixClass                                      Kmat_interp_forUvel      (*env,Kmat_interp_space_forUvel.map(),num_bases_eta);
    Kmat_interp_forUvel.matlabLinearInterpExtrap(extraSimulationGridVec_forUvel,Kmat_eta_forUvel,*(extraExperimentGridVecs_forUvel[i]));

    uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass> Kmat_interp_space_forVvel(*env,"Kmat_interp_space_forVvel",num_experimentPoints_forVvel,NULL);
    uqGslMatrixClass                                      Kmat_interp_forVvel      (*env,Kmat_interp_space_forVvel.map(),num_bases_eta);
    Kmat_interp_forVvel.matlabLinearInterpExtrap(extraSimulationGridVec_forVvel,Kmat_eta_forVvel,*(extraExperimentGridVecs_forVvel[i]));

    // Form matrix 'Kmats_interp[i]'
    Kmats_interp_spaces[i] = new uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>(*env,"Kmats_interp_spaces_",experimentStoragePtr->n_ys_transformed()[i],NULL);
    Kmats_interp       [i] = new uqGslMatrixClass(*env,Kmats_interp_spaces[i]->map(),num_bases_eta); // check

    for (unsigned int j = 0; j < num_experimentPoints_forUvel; ++j) {
      for (unsigned int k = 0; k < num_bases_eta; ++k) {
        (*(Kmats_interp[i]))(j,k) = Kmat_interp_forUvel(j,k);
      }
    }

    for (unsigned int j = 0; j < num_experimentPoints_forVvel; ++j) {
      for (unsigned int k = 0; k < num_bases_eta; ++k) {
        (*(Kmats_interp[i]))(num_experimentPoints_forUvel/*Yes, forU*/ + j,k) = Kmat_interp_forVvel(j,k);
      }
    }

    // Print out 'Kmats_interp[i]'
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
  emVarOptions->m_Gvalues[0] = 14;
  emVarOptions->m_a_v = 1.;
  emVarOptions->m_b_v = 0.001;
  emVarOptions->m_a_rho_v = 1.;
  emVarOptions->m_b_rho_v = 0.1;
  emVarOptions->m_a_y = 1000.;
  emVarOptions->m_b_y = 1000.;


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
  gcmVarOptions->m_priorSeqNumSamples = 10;
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
  //gcm->totalPriorRv().realizer().realization(totalInitialVec);
  totalInitialVec[ 0] = 126.675;             // lambda_eta = lamWOs
  totalInitialVec[ 1] = 1.;                  // lambda_w_1 = lamUz
  totalInitialVec[ 2] = 1.;                  // lambda_w_2 =
  totalInitialVec[ 3] = 1.;                  // lambda_w_3 =
  totalInitialVec[ 4] = std::exp(-0.1*0.25); // rho_w_1_1  = exp(-model.betaU.*(0.5^2));
  totalInitialVec[ 5] = std::exp(-0.1*0.25); // rho_w_1_2  =
  totalInitialVec[ 6] = std::exp(-0.1*0.25); // rho_w_1_3  =
  totalInitialVec[ 7] = std::exp(-0.1*0.25); // rho_w_1_4  =
  totalInitialVec[ 8] = std::exp(-0.1*0.25); // rho_w_2_1  =
  totalInitialVec[ 9] = std::exp(-0.1*0.25); // rho_w_2_2  =
  totalInitialVec[10] = std::exp(-0.1*0.25); // rho_w_2_3  =
  totalInitialVec[11] = std::exp(-0.1*0.25); // rho_w_2_4  =
  totalInitialVec[12] = std::exp(-0.1*0.25); // rho_w_3_1  =
  totalInitialVec[13] = std::exp(-0.1*0.25); // rho_w_3_2  =
  totalInitialVec[14] = std::exp(-0.1*0.25); // rho_w_3_3  =
  totalInitialVec[15] = std::exp(-0.1*0.25); // rho_w_3_4  =
  totalInitialVec[16] = 1000.;               // lambda_s_1 = lamWs
  totalInitialVec[17] = 1000.;               // lambda_s_2 =
  totalInitialVec[18] = 1000.;               // lambda_s_2 =
  totalInitialVec[19] = 1.0;                 // lambda_y   = lamOs
  totalInitialVec[20] = 20.;                 // lambda_v_1 = lamVz
  totalInitialVec[21] = std::exp(-0.1*0.25); // rho_v_1_1  = betaV prudenci 2013-08-24
  totalInitialVec[22] = 0.5;                // theta_1    = theta
  totalInitialVec[23] = 0.5;                 // theta_2    = theta
  totalInitialVec[24] = 0.5;                 // theta_3    = theta
 
  uqGslVectorClass diagVec(gcm->totalSpace().zeroVector());
  diagVec.cwSet(0.25);
  diagVec[ 0] = 2500;  //2500.;  // lambda_eta = lamWOs (gamma(5,0.005)?)
  diagVec[ 1] = 0.09;  //0.09;   // lambda_w_1 = lamUz (gamma(5,5))
  diagVec[ 2] = 0.09;  //0.09;   // lambda_w_2 =
  diagVec[ 3] = 0.09;  //0.09;   // lambda_w_3 =
  diagVec[ 3] = 0.01;  //0.0001; // rho_w_1_1  = betaU
  diagVec[ 5] = 0.01;  //0.0001; // rho_w_1_2  =
  diagVec[ 6] = 0.01;  //0.0001; // rho_w_1_3  =
  diagVec[ 7] = 0.01;  //0.0001; // rho_w_1_4  =
  diagVec[ 8] = 0.01;  //0.0001; // rho_w_2_1  =
  diagVec[ 9] = 0.01;  //0.0001; // rho_w_2_2  =
  diagVec[10] = 0.01;  //0.0001; // rho_w_2_3  =
  diagVec[11] = 0.01;  //0.0001; // rho_w_2_4  =
  diagVec[12] = 0.01;  //0.0001; // rho_w_3_1  =
  diagVec[13] = 0.01;  //0.0001; // rho_w_3_2  =
  diagVec[14] = 0.01;  //0.0001; // rho_w_3_3  =
  diagVec[15] = 0.01;  //0.0001; // rho_w_3_4  =
  diagVec[16] = 900.;            // lambda_s_1 = lamWs (gamma(3,0.003)?)
  diagVec[17] = 900.;            // lambda_s_2 =
  diagVec[18] = 900.;            // lambda_s_2 =
  diagVec[19] = 1.e-4;           // lambda_y   = lamOs gamma(1000,1000))
  diagVec[20] = 900.;  //25.;    // lambda_v_1 = lamVz (gamma(1,0.0001))
  diagVec[21] = 0.01;  //0.0001; // rho_v_1_1  = betaV (beta(1,0.1)
  diagVec[22] = 0.01;  //0.0001; // theta_1    = theta
  diagVec[23] = 0.01;  //0.0001; // theta_2    = theta
  diagVec[24] = 0.01;  //0.0001; // theta_3    = theta

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
  mhVarOptions->m_rawChainSize = numSamples;                                  
   // IMPORTANT
  mhVarOptions->m_rawChainGenerateExtra = 0;
  mhVarOptions->m_rawChainDisplayPeriod = 1000;
  mhVarOptions->m_rawChainMeasureRunTimes = 1;
  mhVarOptions->m_rawChainDataOutputPeriod = 1000;
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
  mhVarOptions->m_filteredChainLag = 6;                                 \
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
  mhVarOptions->m_drScalesForExtraStages[0] = 10; // IMPORTANT
 // mhVarOptions->m_drScalesForExtraStages[1] = 7; // IMPORTANT
 // mhVarOptions->m_drScalesForExtraStages[2] = 10; // IMPORTANT
 // mhVarOptions->m_drScalesForExtraStages[3] = 20; // IMPORTANT
  mhVarOptions->m_drDuringAmNonAdaptiveInt = 1;     // IMPORTANT
  mhVarOptions->m_amKeepInitialMatrix = 0;          // IMPORTANT
  mhVarOptions->m_amInitialNonAdaptInterval = 100;  // IMPORTANT
  mhVarOptions->m_amAdaptInterval = 100;            // IMPORTANT
  mhVarOptions->m_amAdaptedMatricesDataOutputPeriod = 0;
  mhVarOptions->m_amAdaptedMatricesDataOutputFileName = ".";
  mhVarOptions->m_amAdaptedMatricesDataOutputFileType = "m";
  mhVarOptions->m_amAdaptedMatricesDataOutputAllowAll = 0;
  //mhVarOptions->m_am_adaptedMatrices_dataOutputAllowedSet.insert(0);
  mhVarOptions->m_amEta = 2.4*2.4/((double) totalInitialVec.sizeLocal()); // IMPORTANT // prudenci 2013-08-25
  mhVarOptions->m_amEpsilon = 1e-05;                // IMPORTANT
  mhVarOptions->m_enableBrooksGelmanConvMonitor = 0;
  mhVarOptions->m_BrooksGelmanLag = 100;
                                          
  gcm->calibrateWithBayesMetropolisHastings(mhVarOptions,totalInitialVec,&totalInitialProposalCovMatrix);
  //gcm->calibrateWithBayesMLSampling();
 
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
