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
#include "InvGammaRandomVariable.hpp"
#include "ANN/ANN.h" 

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

  /// draw a multivariate sample from the prior distribution
  template <typename Engine>
  void prior_sample(Engine& rng, RealVector& prior_samples);

  /**
   * \brief Compute the proposal covariance C based on low-rank approximation
   * to the prior-preconditioned misfit Hessian.
   */
  static void get_positive_definite_covariance_from_hessian(
    const RealSymMatrix &hessian, const RealMatrix& prior_chol_fact,
    RealSymMatrix &covariance, short output_lev);

  // compute information metrics
  static Real knn_kl_div(RealMatrix& distX_samples, RealMatrix& distY_samples,
      		size_t dim); 
  static Real knn_mutual_info(RealMatrix& Xmatrix, int dimX, int dimY);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void core_run();

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void print_results(std::ostream& s);

  const Model& algorithm_space_model() const;

  //
  //- Heading: New virtual functions
  //

  /// Perform Bayesian calibration (all derived classes must implement)
  virtual void calibrate() = 0;


  //
  //- Heading: Member functions
  //


  /// construct mcmcModel (no emulation, GP, PCE, or SC) that wraps
  /// inbound Model
  void construct_mcmc_model();

  /// initialize the hyper-parameter priors
  void init_hyper_parameters();

  /// construct the negative log posterior RecastModel (wraps
  /// residualModel) and corresponding MAP optimizer
  void construct_map_optimizer();

  /// initialize emulator model and probability space transformations
  void initialize_model();

  /// calibrate the model to a high-fidelity data source, using mutual
  /// information-guided design of experiments (adaptive experimental
  /// design)
  void calibrate_to_hifi();

  void extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
					  const RealMatrix &samples_for_posterior_eval, 
					  const RealVector &posterior_density, RealMatrix &posterior_data ) const;

  void export_posterior_samples_to_file(const std::string filename,
					const RealMatrix &posterior_data ) const;

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

  /// calculate log-likelihood from the passed residuals (assuming
  /// they are already sized and scaled by covariance / hyperparams...
  Real
  log_likelihood(const RealVector& residuals, const RealVector& hyper_params);

  /// compute priorCovCholFactor based on prior distributions for random
  /// variables and any hyperparameters
  void prior_cholesky_factorization();

  /// member version forwards member data to static function
  void get_positive_definite_covariance_from_hessian(
    const RealSymMatrix &hessian, RealSymMatrix &covariance);

  /// static function passed by pointer to negLogPostModel recast model
  static void neg_log_post_resp_mapping(const Variables& model_vars,
                                        const Variables& nlpost_vars,
                                        const Response& model_resp,
                                        Response& nlpost_resp);

  //
  //- Heading: Data
  //
  
  String scalarDataFilename;
  String importCandPtsFile;
  unsigned short importCandFormat;
  size_t maxHifiEvals;
  size_t numCandidates;

  // technically doesn't apply to GPMSA, but leaving here for now
  /// the emulator type: NO_EMULATOR, GP_EMULATOR, PCE_EMULATOR, or SC_EMULATOR
  short emulatorType;
  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model mcmcModel;

  /// DataTransformModel wrapping the mcmcModel
  Model residualModel;

  /// SQP or NIP optimizer for pre-solving for the MAP point prior to MCMC.
  /// This is restricted to emulator cases for now, but as for derivative
  /// preconditioning, could be activated for no-emulator cases with a
  /// specification option (not active by default).
  Iterator mapOptimizer;
  /// RecastModel for solving for MAP: reduces residualModel to scalar
  /// negative log posterior
  Model negLogPostModel;
  /// user setting of the MAP optimization algorithm type
  unsigned short mapOptAlgOverride;

  /// NonDPolynomialChaos or NonDStochCollocation instance for defining a
  /// PCE/SC-based mcmcModel
  Iterator stochExpIterator;

  /// number of samples in the chain (e.g. number of MCMC samples);
  /// for iterative update cycles, number of samples per update cycle
  int chainSamples;
  /// number of update cycles for MCMC chain (implemented by restarting
  /// of short chains)
  int chainCycles;
  /// random seed for MCMC process
  int randomSeed;

  /// order of derivatives used in MCMC process (bitwise like ASV)
  short mcmcDerivOrder;

  // settings specific to adaptive DOE

  /// whether to perform iterative design of experiments with
  /// high-fidelity model
  bool adaptExpDesign;
  /// a high-fidelity model data source (given by pointer in input)
  Model hifiModel;
  /// initial high-fidelity model samples
  int initHifiSamples;
  /// LHS iterator to generate hi-fi model data
  Iterator hifiSampler;

  /// the Cholesky factor of the prior covariance
  RealMatrix priorCovCholFactor;

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
  /// distributions for hyper-params
  std::vector<Pecos::RandomVariable> invGammaDists;

  /// flag indicating use of a variable transformation to standardized
  /// probability space for the model or emulator
  bool standardizedSpace;
  /// flag indicating the calculation of KL divergence between prior and posterior
  bool posteriorStatsKL;
  /// flag indicating the calculation of mutual information between prior and posterior
  bool posteriorStatsMutual;
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

  /// Post-processing-related controls

  /// accumulation of acceptance chain across restarts (stored in
  /// user-space) TO DO: retire once restarts are retired; optimize to
  /// convert to user-space only in final results
  RealMatrix acceptanceChain;
  /// cached function values corresponding to acceptanceChain for
  /// final statistics reporting
  RealMatrix acceptedFnVals;

  /// number of MCMC samples to discard from acceptance chain
  int burnInSamples;
  /// period or skip in post-processing the acceptance chain
  int subSamplingPeriod;

  /// Pointer to current class instance for use in static callback functions
  static NonDBayesCalibration* nonDBayesInstance;

  /// Compute final stats for MCMC chains
  void compute_statistics();
  RealMatrix chainStats;
  RealMatrix fnStats;

  /// export the acceptance chain in user space
  void export_chain();

  /// Print filtered posterior and function values (later: credibility
  /// and prediction intervals)
  void export_chain(RealMatrix& filtered_chain, RealMatrix& filtered_fn_vals);

  /// Perform chain filtering with burn-in and sub-sampling
  void filter_chain(RealMatrix& acceptance_chain, RealMatrix& filtered_chain);
  void filter_fnvals(RealMatrix& accepted_fn_vals, RealMatrix& filtered_fn_vals);
  /// Compute credibility and prediction intervals of final chain
  RealMatrix predVals;
  /// cached filtered function values for printing
  RealMatrix filteredFnVals;
  void compute_intervals();
  void compute_prediction_vals(RealMatrix& filtered_fn_vals,
      			       RealMatrix& PredVals, int num_filtered,
			       size_t num_exp, size_t num_concatenated);
  void compute_col_means(RealMatrix& matrix, RealVector& avg_vals);
  void compute_col_stdevs(RealMatrix& matrix, RealVector& avg_vals, 
      			  RealVector& std_devs);
  void print_intervals_file(std::ostream& stream, RealMatrix& functionvalsT,
  			      RealMatrix& predvalsT, int length, 
			      size_t aug_length);
  void print_intervals_screen(std::ostream& stream, RealMatrix& functionvalsT,
  			      RealMatrix& predvalsT, int length);
  /// output filename for the MCMC chain
  String exportMCMCFilename;
  // BMA TODO: user control of filtered file name and format?  Or use
  // a base name + extensions?
  /// output formatting options for MCMC export
  short exportMCMCFormat;
  short filteredMCMCFormat;

  /// Compute information metrics
  void kl_post_prior(RealMatrix& acceptanceChain);
  void prior_sample_matrix(RealMatrix& prior_dist_samples);
  void mutual_info_buildX();
  static void ann_dist(const ANNpointArray matrix1, const ANNpointArray matrix2, 
     		RealVector& distances, int NX, int NY, int dim2, IntVector& k, 
		double eps);
  Real kl_est;	
  void print_kl(std::ostream& stream);		

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
    pdf *= invGammaDists[i].pdf(vec[numContinuousVars + i]);

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
    log_pdf += invGammaDists[i].log_pdf(vec[numContinuousVars + i]);

  return log_pdf;
}


template <typename Engine> 
void NonDBayesCalibration::prior_sample(Engine& rng, RealVector& prior_samples)
{
  if (natafTransform.x_correlation()) {
    Cerr << "Error: prior_sample() does not support correlated prior samples."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  if (prior_samples.empty())
    prior_samples.sizeUninitialized(numContinuousVars + numHyperparams);
  if (standardizedSpace)
    for (size_t i=0; i<numContinuousVars; ++i)
      prior_samples[i] = natafTransform.draw_u_sample(i, rng);
  else
    for (size_t i=0; i<numContinuousVars; ++i)
      prior_samples[i] = natafTransform.draw_x_sample(i, rng);

  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    prior_samples[numContinuousVars + i] = invGammaDists[i].draw_sample(rng);
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


inline void NonDBayesCalibration::
get_positive_definite_covariance_from_hessian( const RealSymMatrix &hessian,
					       RealSymMatrix &covariance )
{
  get_positive_definite_covariance_from_hessian(hessian, priorCovCholFactor,
						covariance, outputLevel);
}

} // namespace Dakota

#endif
