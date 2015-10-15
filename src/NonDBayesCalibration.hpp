/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_BAYES_CALIBRATION_H
#define NOND_BAYES_CALIBRATION_H

#include "NonDCalibration.hpp"
#include <boost/math/distributions/inverse_gamma.hpp>

namespace Dakota {


/// Base class for Bayesian inference: generates posterior
/// distribution on model parameters given experimental data

/** This class will eventually provide a general-purpose framework for
    Bayesian inference.  In the short term, it only collects shared
    code between QUESO and GPMSA implementations. */

class NonDBayesCalibration: public NonDCalibration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDBayesCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDBayesCalibration();

  //
  //- Heading: Member functions
  //

  /// compute the prior PDF for a particular MCMC sample
  template <typename VectorType>
  Real prior_density(const VectorType& vec);
  /// compute the log prior PDF for a particular MCMC sample
  template <typename VectorType>
  Real log_prior_density(const VectorType& vec);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  //void print_results(std::ostream& s);

  const Model& algorithm_space_model() const;

  //
  //- Heading: Member functions
  //

  /// initialize emulator model and probability space transformations
  void initialize_model();

  /// initialize a residual response for use in data transformations
  void init_residual_response(short request_value_needed);
  
  /// update member residualResponse from passed simulation or emulator Response
  void update_residual_response(const Response& resp);

  /// compute the (approximate) gradient of the negative log posterior by
  /// augmenting the (approximate) gradient of the negative log likelihood
  /// with the gradient of the negative log prior
  template <typename VectorType1, typename VectorType2> 
  void augment_gradient_with_log_prior(VectorType1& log_grad,
				       const VectorType2& vec);
  /// compute the (approximate) Hessian of the negative log posterior by
  /// augmenting the (approximate) Hessian of the negative log likelihood
  /// with the Hessian of the negative log prior
  template <typename MatrixType, typename VectorType> 
  void augment_hessian_with_log_prior(MatrixType& log_hess,
				      const VectorType& vec);

  /// calculate log-likelihood from the passed residual response
  Real log_likelihood(const Response& residual_resp, 
		      const RealVector& hyper_params);

  /// calculate the misfit function from the vector of residuals
  Real misfit(const Response& residual_resp, const RealVector& hyper_params);

  /// static function passed by pointer to negLogPostModel recast model
  static void neg_log_post_resp_mapping(const Variables& model_vars,
                                        const Variables& nlpost_vars,
                                        const Response& model_resp,
                                        Response& nlpost_resp);

  //
  //- Heading: Data
  //

  // technically doesn't apply to GPMSA, but leaving here for now
  /// the emulator type: NO_EMULATOR, GP_EMULATOR, PCE_EMULATOR, or SC_EMULATOR
  short emulatorType;
  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model mcmcModel;

  /// SQP or NIP optimizer for pre-solving for the MAP point prior to MCMC.
  /// This is restricted to emulator cases for now, but as for derivative
  /// preconditioning, could be activated for no-emulator cases with a
  /// specification option (not active by default).
  Iterator mapOptimizer;
  /// RecastModel for solving for MAP using negative log posterior
  Model negLogPostModel;

  /// NonDSampling method for computing sample statistics on final MCMC chain
  Iterator chainStatsSampler;

  /// NonDPolynomialChaos or NonDStochCollocation instance for defining a
  /// PCE/SC-based mcmcModel
  Iterator stochExpIterator;

  /// number of samples in the chain (e.g. number of MCMC samples)
  int numSamples;
  /// number of update cycles for MCMC chain (implemented by restarting
  /// of short chains)
  int chainCycles;
  /// random seed for MCMC process
  int randomSeed;

  /// mode for number of observation error multipliers to calibrate
  /// (default none)
  unsigned short obsErrorMultiplierMode;
  /// calculated number of hyperparameters augmenting the calibration
  /// parameter set, e.g., due to calibrate observation error multipliers
  int numHyperparams;
  /// alphas for inverse gamma distribution on hyper-params
  RealVector invGammaAlphas;
  /// alphas for inverse gamma distribution on hyper-params
  RealVector invGammaBetas;
  /// distributions for hyper-params: BMA TODO: migrate to Pecos RV
  std::vector<boost::math::inverse_gamma_distribution<> > invGammaDists;

  /// flag indicating use of a variable transformation to standardized
  /// probability space for the model or emulator
  bool standardizedSpace;
  /// flag indicating usage of adaptive posterior refinement; currently makes
  /// sense for unstructured grids in GP and PCE least squares/CS
  bool adaptPosteriorRefine;

  /// approach for defining proposal covariance
  String proposalCovarType;
  /// data from user input of proposal covariance
  RealVector proposalCovarData;
  /// filename for user-specified proposal covariance
  String proposalCovarFilename;
  /// approach for defining proposal covariance
  String proposalCovarInputType;

  /// local Response in which to store computed residuals (accounting
  /// for data); must be kept updated after any compute_response
  Response residualResponse;

  /// Pointer to current class instance for use in static callback functions
  static NonDBayesCalibration* nonDBayesInstance;

private:

  //
  //- Heading: Data
  //

};


inline const Model& NonDBayesCalibration::algorithm_space_model() const
{ return mcmcModel; }


template <typename VectorType> 
Real NonDBayesCalibration::prior_density(const VectorType& vec)
{
  // TO DO: consider QUESO-based approach for this using priorRv.pdf(),
  // which may in turn call back to our GenericVectorRV prior plug-in

  if (natafTransform.x_correlation()) {
    Cerr << "Error: prior_density() uses a product of marginal densities\n"
	 << "       and can only be used for independent random variables."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  Real pdf = 1.;
  if (standardizedSpace)
    for (size_t i=0; i<numContinuousVars; ++i)
      pdf *= natafTransform.u_pdf(vec[i], i);
  else
    for (size_t i=0; i<numContinuousVars; ++i)
      pdf *= natafTransform.x_pdf(vec[i], i);

  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    pdf *= boost::math::pdf(invGammaDists[i], vec[numContinuousVars + i]);

  return pdf;
}


template <typename VectorType> 
Real NonDBayesCalibration::log_prior_density(const VectorType& vec)
{
  if (natafTransform.x_correlation()) {
    Cerr << "Error: log_prior_density() uses a sum of log marginal densities\n"
	 << "       and can only be used for independent random variables."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  Real log_pdf = 0.;
  if (standardizedSpace)
    for (size_t i=0; i<numContinuousVars; ++i)
      log_pdf += natafTransform.u_log_pdf(vec[i], i);
  else
    for (size_t i=0; i<numContinuousVars; ++i)
      log_pdf += natafTransform.x_log_pdf(vec[i], i);


  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    log_pdf += std::log(boost::math::pdf(invGammaDists[i], 
					 vec[numContinuousVars + i]));

  return log_pdf;
}


template <typename VectorType1, typename VectorType2> 
void NonDBayesCalibration::
augment_gradient_with_log_prior(VectorType1& log_grad, const VectorType2& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> gradient of neg log posterior = misfit gradient - log prior gradient
  if (standardizedSpace)
    for (size_t i=0; i<numContinuousVars; ++i)
      log_grad[i] -= natafTransform.u_log_pdf_gradient(vec[i], i);
  else
    for (size_t i=0; i<numContinuousVars; ++i)
      log_grad[i] -= natafTransform.x_log_pdf_gradient(vec[i], i);
}


template <typename MatrixType, typename VectorType> 
void NonDBayesCalibration::
augment_hessian_with_log_prior(MatrixType& log_hess, const VectorType& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> Hessian of neg log posterior = misfit Hessian - log prior Hessian
  if (standardizedSpace)
    for (size_t i=0; i<numContinuousVars; ++i)
      log_hess(i, i) -= natafTransform.u_log_pdf_hessian(vec[i], i);
  else
    for (size_t i=0; i<numContinuousVars; ++i)
      log_hess(i, i) -= natafTransform.x_log_pdf_hessian(vec[i], i);
}

} // namespace Dakota

#endif
