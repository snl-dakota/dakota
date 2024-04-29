/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_QUESO_BAYES_CALIBRATION_H
#define NOND_QUESO_BAYES_CALIBRATION_H

#include "NonDBayesCalibration.hpp"

// forward declare here to isolate QUESO includes to Dakota .cpp files
namespace QUESO {
  class GslVector;
  class GslMatrix;
  class EnvOptionsValues;
  class FullEnvironment;
  template<class V, class M> class VectorSpace;
  template<class V, class M> class BoxSubset;
  template<class V, class M> class GenericScalarFunction;
  template<class V, class M> class BaseVectorRV;
  template<class V, class M> class GenericVectorRV;
  template<class V, class M> class StatisticalInverseProblem;
  class SipOptionsValues;
  class MhOptionsValues;
}

namespace Dakota {

// forward declarations of Dakota specializations
template <class V, class M> class DerivInformedPropCovTK;
template <class V, class M> class DerivInformedPropCovLogitTK;


/// Bayesian inference using the QUESO library from UT Austin

/** This class wraps the Quantification of Uncertainty for Estimation,
    Simulation, and Optimization (QUESO) library, developed as part of
    the Predictive Science Academic Alliance Program (PSAAP)-funded
    Predictive Engineering and Computational Sciences (PECOS) Center
    at UT Austin. */
class NonDQUESOBayesCalibration: public NonDBayesCalibration
{
  /// Random walk transition kernel needs callback access to QUESO details
  friend class DerivInformedPropCovTK<QUESO::GslVector, QUESO::GslMatrix>;
  /// Logit random walk transition kernel needs callback access to QUESO details
  friend class DerivInformedPropCovLogitTK<QUESO::GslVector, QUESO::GslMatrix>;

public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDQUESOBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDQUESOBayesCalibration();

  //
  //- Heading: public member functions
  //

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void calibrate();
  void print_results(std::ostream& s, short 
      results_state = FINAL_RESULTS) override;

  /// initialize the QUESO FullEnvironment on the Dakota MPIComm
  void init_queso_environment();

  /// initialize the ASV value for preconditioned cases
  void init_precond_request_value();
  
  /// intialize the QUESO parameter space, min, max, initial, domain, and prior
  /// define solver options, likelihood callback, posterior RV, and
  /// inverse problem
  void specify_prior() override; 
  void specify_likelihood() override;
  void init_bayesian_solver() override;
  void specify_posterior() override;

  /// use derivative information from the emulator to define the proposal
  /// covariance (inverse of misfit Hessian)
  void precondition_proposal(unsigned int chain_index);

  /// perform the MCMC process
  void run_queso_solver();

  void map_pre_solve() override;

  /// short term option to restart the MCMC chain with updated proposal
  /// density computed from the emulator at a new starting point
  void run_chain();

  /// cache the chain to acceptanceChain and acceptedFnVals
  void cache_chain();

  /// log at most batchSize best chain points into bestSamples
  void log_best();

  /// extract batchSize points from the MCMC chain and store final
  /// aggregated set within allSamples; unique points with best
  /// conditioning are selected, as determined by pivoted LU
  void filter_chain_by_conditioning() override;

  void init_proposal_covariance();

  /// use covariance of prior distribution for setting proposal covariance
  void prior_proposal_covariance();

  /// Set proposal covariance from user-provided diagonal or matrix
  void user_proposal_covariance(const String& input_fmt, 
				const RealVector& cov_data, 
				const String& cov_filename);

  // perform sanity checks on proposalCovMatrix
  void validate_proposal();

  /// Set inverse problem options calIpOptionsValues common to all solvers
  void set_ip_options(); 
  /// Set MH-specific inverse problem options calIpMhOptionsValues
  void set_mh_options();
  /// update MH-specific inverse problem options calIpMhOptionsValues
  void update_chain_size(unsigned int size);

  //The likelihood routine is in the format that QUESO requires, 
  //with a particular argument list that QUESO expects. 
  //We are not using all of these arguments but may in the future.
  /// Log Likelihood function for call-back from QUESO to DAKOTA for evaluation
  static double dakotaLogLikelihood(const QUESO::GslVector& paramValues,
				    const QUESO::GslVector* paramDirection,
				    const void*             functionDataPtr,
				    QUESO::GslVector*       gradVector,
				    QUESO::GslMatrix*       hessianMatrix,
				    QUESO::GslVector*       hessianEffect);

  /// local copy_data utility from GslVector to RealVector
  void copy_gsl(const QUESO::GslVector& qv, RealVector& rv);
  /// local copy_data utility from RealVector to GslVector
  void copy_gsl(const RealVector& rv, QUESO::GslVector& qv);

  /// local copy_data utility from portion of GslVector to RealVector
  void copy_gsl_partial(const QUESO::GslVector& qv, size_t start, 
			RealVector& rv);
  /// local copy_data utility from RealVector to portion of GslVector
  void copy_gsl_partial(const RealVector& rv, QUESO::GslVector& qv,
			size_t start);

  /// local copy_data utility from GslVector to column in RealMatrix
  void copy_gsl(const QUESO::GslVector& qv, RealMatrix& rm, int i);

  /// equality tester for two GslVectors
  bool equal_gsl(const QUESO::GslVector& qv1, const QUESO::GslVector& qv2);

  //
  //- Heading: Data
  //

  /// MCMC type ("dram" or "delayed_rejection" or "adaptive_metropolis" 
  /// or "metropolis_hastings" or "multilevel",  within QUESO) 
  String mcmcType;
  /// period (number of accepted chain samples) for proposal covariance update
  int propCovUpdatePeriod;
  /// the active set request value to use in proposal preconditioning
  short precondRequestValue;
  /// flag indicating user activation of logit transform option
  /** this option is useful for preventing rejection or resampling for
      out-of-bounds samples by transforming bounded domains to [-inf,inf]. */
  bool logitTransform;

  // the following QUESO objects listed in order of construction;
  // scoped_ptr more appropriate, but don't want to include QUESO
  // headers here (would be needed for checked delete on scoped_ptr)

  // TODO: see if this can be a local withing init function
  /// options for setting up the QUESO Environment
  std::shared_ptr<QUESO::EnvOptionsValues> envOptionsValues;

  /// top-level QUESO Environment
  std::shared_ptr<QUESO::FullEnvironment> quesoEnv;
  
  /// QUESO parameter space based on number of calibrated parameters
  std::shared_ptr<QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix> > 
  paramSpace;

  /// QUESO parameter domain: hypercube based on min/max values
  std::shared_ptr<QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix> >
  paramDomain;

  /// initial parameter values at which to start chain
  std::shared_ptr<QUESO::GslVector> paramInitials;

  /// random variable for the prior
  std::shared_ptr<QUESO::BaseVectorRV<QUESO::GslVector,QUESO::GslMatrix> >
    priorRv;

  /// proposal covariance for DRAM
  std::shared_ptr<QUESO::GslMatrix> proposalCovMatrix;

  /// optional multiplier to scale prior-based proposal covariance 
  double priorPropCovMult;

  /// general inverse problem options
  std::shared_ptr<QUESO::SipOptionsValues> calIpOptionsValues;

  /// MH-specific inverse problem options
  std::shared_ptr<QUESO::MhOptionsValues> calIpMhOptionsValues;

  std::shared_ptr<QUESO::GenericScalarFunction<QUESO::GslVector,
    QUESO::GslMatrix> > likelihoodFunctionObj;

  /// random variable for the posterior
  std::shared_ptr<QUESO::GenericVectorRV<QUESO::GslVector,QUESO::GslMatrix> >
    postRv;

  /// QUESO inverse problem solver
  std::shared_ptr<QUESO::StatisticalInverseProblem<QUESO::GslVector,
    QUESO::GslMatrix> > inverseProb;

  /// advanced options file name (GPMSA only); settings from this file
  /// override any C++ / Dakota input file settings
  String advancedOptionsFile;

  /// Pointer to current class instance for use in static callback functions
  static NonDQUESOBayesCalibration* nonDQUESOInstance;

private:

  //
  // - Heading: Data
  // 
  
};

} // namespace Dakota

#endif
