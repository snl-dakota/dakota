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

#include "NonDBayesCalibration.hpp"


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

class NonDGPMSABayesCalibration: public NonDBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDGPMSABayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGPMSABayesCalibration();

  //
  //- Heading: Data
  //

  /// number of samples of the simulation to construct the GP 
  int buildSamples;
  /// flag to indicated if the sigma terms should be calibrated (default true)
  bool calibrateSigmaFlag;
  /// name of file from which to import build points to build GP
  String approxImportFile;
  /// build data import tabular format
  unsigned short approxImportFormat;
  /// import active variables only
  bool approxImportActiveOnly;

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

  // print the final statistics
  //void print_results(std::ostream& s);

  //
  //- Heading: Data
  //

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
