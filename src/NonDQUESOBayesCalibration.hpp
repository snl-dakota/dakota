/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler, Brian Adams
//- Checked by:
//- Version:

#ifndef NOND_QUESO_BAYES_CALIBRATION_H
#define NOND_QUESO_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"

// forward declare isolate QUESO includes to Dakota .cpp files
namespace QUESO {
  class GslVector;
  class GslMatrix;
  class EnvOptionsValues;
  class FullEnvironment;
  template<class V, class M> class VectorSpace;
  template<class V, class M> class BoxSubset;
  class SipOptionsValues;
  class MhOptionsValues;
}

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

  /// standard constructor
  NonDQUESOBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDQUESOBayesCalibration();
         
protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// redefined from DakotaNonD
  void quantify_uncertainty();
  // redefined from DakotaNonD
  //void print_results(std::ostream& s);

  /// initialize the QUESO FullEnvironment on the Dakota MPIComm
  void init_queso_environment();

  /// intialize the QUESO parameter space, min, max, initial, and domain
  void init_parameter_domain();

  /// use historical default behavior for setting proposal covariance
  void default_proposal_covariance();

  /// set proposal covariance from user-provided diagonal or matrix
  void user_proposal_covariance(const String& cov_type, 
				const RealVector& cov_data, 
				const String& cov_filename);

  /// set proposal covariance, e.g., from inverse of surrogate Hessian
  void proposal_covariance(const RealSymMatrix& hessian);

  /// set inverse problem options calIpOptionsValues common to all solvers
  void set_inverse_problem_options(); 

  /// set MH-specific inverse problem options calIpMhOptionsValues
  void set_invpb_mh_options();

  //The likelihood routine is in the format that QUESO requires, 
  //with a particular argument list that QUESO expects. 
  //We are not using all of these arguments but may in the future.
  /// Likelihood function for call-back from QUESO to DAKOTA for evaluation
  static double dakotaLikelihoodRoutine(
  const QUESO::GslVector& paramValues,
  const QUESO::GslVector* paramDirection,
  const void*  functionDataPtr,
  QUESO::GslVector*       gradVector,
  QUESO::GslMatrix*       hessianMatrix,
  QUESO::GslVector*       hessianEffect);

  //
  //- Heading: Data
  //

  /// whether QUESO is working in a standardized space
  bool quesoStandardizedSpace;

  /// MCMC type ("dram" or "multilevel", both within QUESO) 
  String mcmcType;
  /// Rejection type ("standard" or "delayed", in the DRAM framework) 
  String rejectionType;
  /// Metropolis type ("hastings" or "adaptive", in the DRAM framework) 
  String metropolisType;
  /// scale factor for likelihood; deprecated
  Real likelihoodScale;
  /// flag to indicated if the sigma terms should be calibrated (default true)
  bool calibrateSigmaFlag;

private:
  //
  // - Heading: Data
  // 
  //     
  
  /// Pointer to current class instance for use in static callback functions
  static NonDQUESOBayesCalibration* NonDQUESOInstance;
  
  // the following QUESO objects listed in order of construction;
  // scoped_ptr more appropriate, but don't want to include QUESO
  // headers here (would be needed for checked delete on scoped_ptr)

  // TODO: see if this can be a local withing init function
  /// options for setting up the QUESO Environment
  boost::shared_ptr<QUESO::EnvOptionsValues> envOptionsValues;

  /// top-level QUESO Environment
  boost::shared_ptr<QUESO::FullEnvironment> quesoEnv;
  
  /// QUESO parameter space based on number of calibrated parameters
  boost::shared_ptr<QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix> > 
  paramSpace;

  /// QUESO parameter domain: hypercube based on min/max values
  boost::shared_ptr<QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix> >
  paramDomain;

  /// initial parameter values at which to start chain
  boost::shared_ptr<QUESO::GslVector> paramInitials;

  /// proposal covariance for DRAM
  boost::shared_ptr<QUESO::GslMatrix> proposalCovMatrix;

  /// general inverse problem options
  boost::shared_ptr<QUESO::SipOptionsValues> calIpOptionsValues;

  /// MH-specific inverse problem options
  boost::shared_ptr<QUESO::MhOptionsValues> calIpMhOptionsValues;
};

} // namespace Dakota

#endif
