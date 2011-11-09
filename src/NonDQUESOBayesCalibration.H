/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_QUESO_BAYES_CALIBRATION_H
#define NOND_QUESO_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.H"
//#include "uqGenericScalarFunctionClass.H"
#include "uqGslVector.h"
#include "uqGslMatrix.h"


namespace Dakota {


/// Bayesian inference using the QUESO library from UT Austin

/** This class provides a wrapper to the QUESO library developed 
 * as part of the Predictive Science Academic Alliance Program (PSAAP), 
 * specifically the PECOS (Predictive Engineering and 
 * Computational Sciences) Center at UT Austin. The name QUESO stands for 
 * Quantification of Uncertainty for Estimation, Simulation, and 
 * Optimization.    */

class NonDQUESOBayesCalibration: public NonDBayesCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDQUESOBayesCalibration(Model& model); ///< standard constructor
  ~NonDQUESOBayesCalibration();            ///< destructor

  /// Rejection type (standard or delayed, in the DRAM framework) 
  String rejectionType;
  /// Metropolis type (hastings or adaptive, in the DRAM framework) 
  String metropolisType;
  /// number of samples in the chain (e.g. number of MCMC samples)
  int numSamples;
  /// scale factor for proposal covariance
  Real proposalCovScale;
  /// scale factor for likelihood
  Real likelihoodScale;
         
protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// redefined from DakotaNonD
  void quantify_uncertainty();
  // redefined from DakotaNonD
  //void print_results(std::ostream& s);
  
  //The likelihood routine is in the format that QUESO requires, 
  //with a particular argument list that QUESO expects. 
  //We are not using all of these arguments but may in the future.
  /// Likelihood function for call-back from QUESO to DAKOTA for evaluation
  static double dakotaLikelihoodRoutine(
  const uqGslVectorClass& paramValues,
  const uqGslVectorClass* paramDirection,
  const void*  functionDataPtr,
  uqGslVectorClass*       gradVector,
  uqGslMatrixClass*       hessianMatrix,
  uqGslVectorClass*       hessianEffect);

  //
  //- Heading: Data
  //
  /// Matrix that holds the experimental realizations of state variables x
  RealMatrix xObsData;
  /// Matrix that holds the experimental realizations of responses y 
  RealMatrix yObsData;
  /// Matrix that holds the experimental realizations of std deviations 
  /// of responses y 
  RealMatrix yStdData;
  /// random seed to pass to QUESO
  int randomSeed;

private:
  //
  // - Heading: Data
  // 
  //     
  /// the emulator type: NO_EMULATOR, GAUSSIAN_PROCESS, POLYNOMIAL_CHAOS, or
  /// STOCHASTIC_COLLOCATION
  short emulatorType;
  
  /// Pointer to current class instance for use in static callback functions
  static NonDQUESOBayesCalibration* NonDQUESOInstance;
  
};

} // namespace Dakota

#endif
