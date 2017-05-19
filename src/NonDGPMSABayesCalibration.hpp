/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPMSABayesCalibration
//- Description: Wrapper class for the Bayesian calibration capability 
//-              from the LANL GPM/SA library
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_GPMSA_BAYES_CALIBRATION_H
#define NOND_GPMSA_BAYES_CALIBRATION_H

#include "NonDQUESOBayesCalibration.hpp"

namespace QUESO {
//   template<class T> class SPType;
//   //  template<typename T> struct SharedPtr;
//   template<typename T>
//   struct SharedPtr { typedef SPType<T> Type; }
  class GPMSAOptions;
  template<class V, class M> class GPMSAFactory;
}

namespace Dakota {

/// Generates posterior distribution on model parameters given experiment data 


/** This class provides a wrapper for the functionality provided in 
    the Los Alamos National Laboratory code called GPM/SA (Gaussian 
    Process Models for Simulation Analysis).  Although this is a 
    code that provides input/output mapping, it DOES NOT provide the 
    mapping that we usually think of in the NonDeterministic class 
    hierarchy in DAKOTA, where uncertainty in parameter inputs are mapped to 
    uncertainty in simulation responses.  Instead, this class takes 
    a pre-existing set of simulation data as well as experimental data, and 
    maps priors on input parameters to posterior distributions on those 
    input parameters, according to a likelihood function.  The goal of the 
    MCMC sampling is to produce posterior values of parameter estimates 
    which will produce simulation response values that "match well" to the 
    experimental data.  The MCMC is an integral part of the calibration. 
    The data structures in GPM/SA are fairly detailed and nested.  
    Part of this prototyping exercise is to determine what data 
    structures need to be specified and initialized in DAKOTA and sent 
    to GPM/SA, and what data structures will be returned.  */

class NonDGPMSABayesCalibration: public NonDQUESOBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDGPMSABayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGPMSABayesCalibration();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// performs a forward uncertainty propagation by using GPM/SA to 
  /// generate a posterior distribution on parameters given a set of 
  /// simulation parameter/response data, a set of experimental data, and 
  /// additional variables to be specified here. 
  void calibrate();

  /// specialization to initialize the inverse problem and posterior
  void init_queso_solver();

  /// Populate simulation data (run design of experiments or load build data)
  void acquire_simulation_data();

  /// Determine each scenario min/max and set GPMSA scaling options
  void normalize_configs();

  /// fill the full proposal covariance, inlcuding hyperparameter
  /// entries with user-specified or default theta covariance
  /// information
  void overlay_proposal_covariance(QUESO::GslMatrix& full_prop_cov) const;

  /// populate the simulation data, calculating and applying scaling if needed
  void fill_simulation_data(bool scale_data);

  /// populate the experiment data, applying scaling if needed
  void fill_experiment_data(bool scale_data);

  /// overlay the Dakota user's initial parameters on the full GPMSA
  /// vector of parameters
  void overlay_initial_params(QUESO::GslVector& full_param_initials);

  /// retrieve the chain from QUESO and populate acceptanceChain /
  /// acceptedFnVals
  void cache_acceptance_chain(); 

  // print the final statistics
  void print_results(std::ostream& s);

  //
  //- Heading: Data
  //

  //
  //- Heading: Data
  //

  /// number of samples of the simulation to construct the GP
  int buildSamples;

  /// name of file from which to import build points to build GP
  String approxImportFile;

  /// build data import tabular format
  unsigned short approxImportFormat;

  /// import active variables only
  bool approxImportActiveOnly;

  /// number of user-specified configuration (scenario) vars
  unsigned int userConfigVars;

  /// number of config vars presented to GPMSA (minimum 1)
  unsigned int gpmsaConfigVars;

  /// whether to apply GPMSA-internal variable and data normalization
  bool gpmsaNormalize;

  /// vector space defining the scenario (configuration) variables
  boost::shared_ptr<QUESO::VectorSpace<QUESO::GslVector, QUESO::GslMatrix> > 
  configSpace;

  /// vector space defining the output (response) space for the simulations
  boost::shared_ptr<QUESO::VectorSpace<QUESO::GslVector, QUESO::GslMatrix> >
  nEtaSpace;

  /// vector space defining the output (response) space for the experiments
  boost::shared_ptr<QUESO::VectorSpace<QUESO::GslVector, QUESO::GslMatrix> >
  experimentSpace;

  /// simulation data, one row per simulation build sample, columns
  /// for calibration variables, configuration variables, function
  /// values (duplicates storage, but unifies import vs. DOE cases)
  RealMatrix simulationData;

  /// simulation mean values for scaling, one per function
  RealVector simulationMean;

  /// simulation std deviations for scaling, one per function
  RealVector simulationStdDev;

  /// Configuration options for the GPMSA solver
  boost::shared_ptr<QUESO::GPMSAOptions> gpmsaOptions;

  /// advanced options file name
  String optionsFile;

  /// core factory that manages a GP-based likelihood
  boost::shared_ptr<QUESO::GPMSAFactory<QUESO::GslVector, QUESO::GslMatrix> >
  gpmsaFactory; 


private:

  //
  //- Heading: Data
  //

  //short emulatorType;
 
  /// Pointer to current class instance for use in static callback functions
  static NonDGPMSABayesCalibration* nonDGPMSAInstance;
  /// LHS iterator for generating samples for GP 
  Iterator lhsIter;
};

} // namespace Dakota

#endif
