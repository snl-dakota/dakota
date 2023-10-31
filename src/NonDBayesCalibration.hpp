/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_BAYES_CALIBRATION_H
#define NOND_BAYES_CALIBRATION_H

#include "NonDCalibration.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "InvGammaRandomVariable.hpp"
#include "GaussianKDE.hpp"
#include "ANN/ANN.h" 

//#define DEBUG

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

  /// return the mean of the prior distribution
  template <typename VectorType>
  void prior_mean(VectorType& mean_vec) const;

  /// return the covariance of the prior distribution
  template <typename MatrixType>
  void prior_variance(MatrixType& var_mat) const;

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
  static Real knn_mutual_info(RealMatrix& Xmatrix, int dimX, int dimY,
			      unsigned short alg);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  virtual void print_results(std::ostream& s, 
      short results_state = FINAL_RESULTS);
  /// convenience function to print calibration parameters, e.g., for
  /// MAP / best parameters
  void print_variables(std::ostream& s, const RealVector& c_vars);


  const Model& algorithm_space_model() const;

  //
  //- Heading: New virtual functions
  //

  /// Methods for instantiating a Bayesian inverse problem.
  /// No-ops in the base class that can be specialized by child
  /// classes.
  virtual void specify_prior(){}
  virtual void specify_likelihood(){}
  virtual void init_bayesian_solver(){}
  virtual void specify_posterior(){}

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

  /// initialize the MAP optimizer selection
  void init_map_optimizer();
  /// construct the negative log posterior RecastModel (negLogPostModel)
  void construct_map_model();
  /// construct the MAP optimizer for minimization of negLogPostModel
  void construct_map_optimizer();

  virtual void map_pre_solve();

  /// initialize emulator model and probability space transformations
  void initialize_model();

  /// Run calibration, looping to refine emulator around posterior mode.
  void calibrate_with_adaptive_emulator(); 
  /// Filter mcmc chain for PCE adaptive emulator.
  /// extract batchSize points from the MCMC chain and store final
  /// aggregated set within allSamples; unique points with best
  /// conditioning are selected, as determined by pivoted LU
  virtual void filter_chain_by_conditioning(){ 
    Cerr << "Error: filter_chain_by_conditioning() has not been"
      << "implemented in the child class." << std::endl;
      abort_handler(METHOD_ERROR);
  }
  /// copy bestSamples to allSamples to use in surrogate update
  void best_to_all();
  /// evaluates allSamples on iteratedModel and update the mcmcModel emulator
  /// with all{Samples,Responses}
  void update_model();
  /// compute the L2 norm of the change in emulator coefficients
  Real assess_emulator_convergence();

  /// calibrate the model to a high-fidelity data source, using mutual
  /// information-guided design of experiments (adaptive experimental
  /// design)
  void calibrate_to_hifi();
  /// evaluate stopping criteria for calibrate_to_hifi
  void eval_hi2lo_stop(bool& stop_metric, double& prev_MI, 
                 const RealVector& MI_vec, 
                 int num_hifi, int max_hifi, 
		             int num_candidates);
  /// print calibrate_to_hifi progress to file
  void print_hi2lo_file(std::ostream& out_file, int num_it, 
                        const VariablesArray& optimal_config_matrix, 
                        const RealVector& MI_vec, RealMatrix& resp_matrix );
      //double max_MI);
  /// print calibrate_to_hifi progress
  void print_hi2lo_begin(int num_it);
  void print_hi2lo_status(int num_it, int i, const Variables& xi_i, double MI);
  void print_hi2lo_batch_status(int num_it, int batch_n, int batchEvals, 
      			const Variables& optimal_config, double max_MI);
  void print_hi2lo_selected(int num_it, // int batchEvals, 
                const VariablesArray& optimal_config_matrix, 
                const RealVector& MI_vec);
  void print_hi2lo_chain_moments();
  /// supplement high-fidelity data with LHS samples
  void add_lhs_hifi_data();
  /// calculate the optimal points to add for a given batch
  void choose_batch_from_mutual_info( int random_seed,
                          int num_it, 
                          int max_hifi, int num_hifi, 
                          RealMatrix& mi_chain,
                          VariablesArray& design_matrix, 
                          VariablesArray& optimal_config_matrix, RealVector& MI_vec);
  /// apply simulation error vector
  void apply_hifi_sim_error(int& random_seed, int num_exp, int exp_offset=0 );
  void apply_error_vec(const RealVector& error_vec, int &seed, int experiment);
  /// build matrix of errors 
  void build_error_matrix(const RealVector& sim_error_vec, 
      			  RealMatrix& sim_error_matrix, int &seed);
  /// build matrix of candidate points
  void build_designs(VariablesArray& design_matrix);
  /// build matrix to calculate mutual information for calibrate_to_hifi
  void build_hi2lo_xmatrix(RealMatrix& Xmatrix, int i, const RealMatrix& 
                           mi_chain, RealMatrix& sim_error_matrix);
  /// run high-fidelity model at several configs and add to experiment data 
  void run_hifi(const VariablesArray& optimal_config_matrix,
		RealMatrix& resp_matrix);
  
  /// calculate model discrepancy with respect to experimental data
  void build_model_discrepancy();
  void build_scalar_discrepancy();
  void build_field_discrepancy();
  void build_GP_field(const RealMatrix& t, RealMatrix& t_pred,
                           const RealVector& concat_disc, RealVector& disc_pred,
                           RealVector& disc_var);


  /// calculate a Kernel Density Estimate (KDE) for the posterior samples
  void calculate_kde();
  /// calculate the model evidence
  void calculate_evidence();

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
  Real log_likelihood(const RealVector& residuals,
		      const RealVector& hyper_params);

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

  /// Wrap iteratedModel in a RecastModel that performs response scaling
  void scale_model();

  /// Wrap iteratedModel in a RecastModel that weights the residuals
  void weight_model();

  //
  //- Heading: Data
  //
  
  String scalarDataFilename;

  // technically doesn't apply to GPMSA, but leaving here for now
  /// the emulator type: NO_EMULATOR, GP_EMULATOR, PCE_EMULATOR,
  /// SC_EMULATOR, ML_PCE_EMULATOR, MF_PCE_EMULATOR, or MF_SC_EMULATOR
  short emulatorType;
  /// cache previous expansion coefficients for assessing convergence of
  /// emulator refinement process
  RealVectorArray prevCoeffs;

  /// Model instance employed in the likelihood function; provides response
  /// function values from Gaussian processes, stochastic expansions (PCE/SC),
  /// or direct access to simulations (no surrogate option)
  Model mcmcModel;

  /// whether the MCMC Model is a surrogate, or a thin transformation
  /// around a surrogate, so can be cheaply re-evaluated in chain recovery
  bool mcmcModelHasSurrogate;

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
  /// random seed for MCMC process
  int randomSeed;

  /// number of points to add to surrogate at each iteration of
  /// calibrate_with_adaptive_emulator
  unsigned int batchSize;

  /// order of derivatives used in MCMC process (bitwise like ASV)
  short mcmcDerivOrder;

  /// solution for most recent MAP pre-solve; also serves as initial guess
  /// for initializing the first solve and warm-starting the next solve
  /// (posterior emulator refinement)
  RealVector mapSoln;

  // settings specific to adaptive DOE

  /// whether to perform iterative design of experiments with
  /// high-fidelity model
  bool adaptExpDesign;
  /// number of candidate designs for adaptive Bayesian experimental design
  size_t numCandidates;
  /// whether to import candidate design points for adaptive Bayesian 
  /// experimental design
  String importCandPtsFile;
  /// tabular format for the candidate design points import file
  unsigned short importCandFormat;
  /// maximum number of high-fidelity model runs to be used for adaptive
  /// Bayesian experimental design
  int maxHifiEvals;
  /// number of optimal designs selected per iteration of experimental design
  /// algorithm
  int batchEvals;
  /// algorithm to employ in calculating mutual information
  unsigned short mutualInfoAlg;

  // settings specific to model discrepancy

  /// need field coordinates for model discrepancy
  bool readFieldCoords;
  /// flag whether to calculate model discrepancy
  bool calModelDiscrepancy;
  /// set discrepancy type
  String discrepancyType; 
  /// filename for corrected model (model+discrepancy) calculations output
  String exportCorrModelFile;
  /// filename for discrepancy calculations output
  String exportDiscrepFile;
  /// filename for corrected model variance calculations 
  String exportCorrVarFile;
  /// format options for corrected model output
  unsigned short exportCorrModelFormat;
  /// format options for discrepancy output
  unsigned short exportDiscrepFormat;
  /// format options for corrected model variance output
  unsigned short exportCorrVarFormat;
  /// specify polynomial or trend order for discrepancy correction
  short discrepPolyOrder;
  /// number of prediction configurations at which to calculate model 
  /// discrepancy
  size_t numPredConfigs;
  /// lower bounds on configuration domain
  RealVector configLowerBnds;
  /// upper bounds on configuration domain
  RealVector configUpperBnds;
  /// array containing predicted of model+discrepancy
  ResponseArray discrepancyResponses;
  /// array containing predicted of model+discrepancy
  ResponseArray correctedResponses;
  /// matrix containing variances of model+discrepancy
  RealMatrix correctedVariances;
  /// list of prediction configurations at which to calculate model discrepancy
  RealVector predictionConfigList;
  /// whether to import prediction configurations at which to calculate model
  /// discrepancy
  String importPredConfigs;
  /// tabular format for prediction configurations import file
  unsigned short importPredConfigFormat;
  /// print tabular files containing model+discrepancy responses and variances
  void export_discrepancy(RealMatrix& pred_config_mat); 
  /// print tabular files containing model+discrepancy responses and variances
  /// for field responses
  void export_field_discrepancy(RealMatrix& pred_vars_mat);
  /// array containing predicted of model+discrepancy
  RealVector discrepancyFieldResponses;
  /// array containing predicted of model+discrepancy
  RealVector correctedFieldResponses;
  /// matrix containing variances of model+discrepancy
  RealVector correctedFieldVariances;

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
  /// flag indicating the calculation of KL divergence between prior and
  /// posterior
  bool posteriorStatsKL;
  /// flag indicating the calculation of mutual information between prior and
  /// posterior
  bool posteriorStatsMutual;
  /// flag indicating the calculation of the kernel density estimate of the
  /// posteriors
  bool posteriorStatsKDE;
  /// flag indicating calculation of chain diagnostics
  bool chainDiagnostics;
  /// flag indicating calculation of confidence intervals as a chain
  /// diagnositc
  bool chainDiagnosticsCI;
  /// flag indicating calculation of the evidence of the model
  bool calModelEvidence;
  /// flag indicating use of Monte Carlo approximation to calculate evidence
  bool calModelEvidMC;
  /// flag indicating use of Laplace approximation to calculate evidence
  bool calModelEvidLaplace;
  /// number of samples to be used in model evidence calculation
  int evidenceSamples;
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

  /// container for managing best MCMC samples (points and associated
  /// log posterior) collected across multiple (restarted) chains
  std::/*multi*/map<Real, RealVector> bestSamples;

  /// number of MCMC samples to discard from acceptance chain
  int burnInSamples;
  /// period or skip in post-processing the acceptance chain
  int subSamplingPeriod;

  /// Pointer to current class instance for use in static callback functions
  static NonDBayesCalibration* nonDBayesInstance;

  /// Compute final stats for MCMC chains
  virtual void compute_statistics();
  RealMatrix chainStats;
  RealMatrix fnStats;

  /// export the acceptance chain in user space
  void export_chain();

  /// Print filtered posterior and function values (later: credibility
  /// and prediction intervals)
  void export_chain(RealMatrix& filtered_chain, RealMatrix& filtered_fn_vals);

  /// Perform chain filtering based on target chain length
  void filter_chain(const RealMatrix& acceptance_chain, RealMatrix& filtered_chain, 
      		    int target_length);
  /// Perform chain filtering with burn-in and sub-sampling
  void filter_chain(const RealMatrix& acceptance_chain, RealMatrix& filtered_chain);
  void filter_fnvals(const RealMatrix& accepted_fn_vals, RealMatrix& filtered_fn_vals);

  /// return a newly allocated filtered matrix including start_index and
  /// every stride-th index after; for burn-in cases, start_index is the
  /// number of burn-in discards
  void filter_matrix_cols(const RealMatrix& orig_matrix, int start_index,
			  int stride, RealMatrix& filtered_matrix);

  /// Compute credibility and prediction intervals of final chain
  RealMatrix predVals;
  /// cached filtered function values for printing (may be a view of acceptedFnVals)
  RealMatrix filteredFnVals;
  void compute_intervals();
  void compute_prediction_vals(RealMatrix& filtered_fn_vals,
      			       RealMatrix& PredVals, int num_filtered,
			       size_t num_exp, size_t num_concatenated);
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
  static void ann_dist(const ANNpointArray matrix1, 
                const ANNpointArray matrix2, RealVector& distances, 
		Int2DArray& indices, int NX, int NY, int dim2, 
		IntVector& k, double eps);
  Real kl_est;	
  void print_kl(std::ostream& stream);		
  void print_chain_diagnostics(std::ostream& s);
  void print_batch_means_intervals(std::ostream& s); 

  /// whether response scaling is active
  bool scaleFlag;
  // /// Shallow copy of the scaling transformation model, when present
  // /// (cached in case further wrapped by other transformations)
  // Model scalingModel;

  /// whether weight scaling is active
  bool weightFlag;

private:

  //
  //- Heading: Data
  //

};


inline const Model& NonDBayesCalibration::algorithm_space_model() const
{ return residualModel; }


template <typename VectorType> 
Real NonDBayesCalibration::prior_density(const VectorType& vec)
{
  // template supports QUESO::GslVector which has to use long form

  // TO DO: consider QUESO-based approach for this using priorRv.pdf(),
  // which may in turn call back to our GenericVectorRV prior plug-in

  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();
  if (mv_dist.correlation()) {
    Cerr << "Error: prior_density() uses a product of marginal densities\n"
	 << "       and can only be used for independent random variables."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Don't need to map indices so long as activeVars corresponds to vec
  //const SharedVariablesData& svd
  //  = iteratedModel.current_variables().shared_data();

  const BitArray& active_vars = mv_dist.active_variables();
  bool no_mask = active_vars.empty();
  size_t v, num_rv = mv_dist.random_variables().size(),
         active_rv = (no_mask) ? num_rv : active_vars.count();
  if (active_rv != numContinuousVars) { // avoid call to .length()/.size()
    PCerr << "Error: active variable size mismatch in NonDBayesCalibration::"
	   << "prior_density(): " << active_rv << " expected, "
	  << numContinuousVars << " provided." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  Real pdf = 1.;
  if (no_mask)
    for (v=0; v<num_rv; ++v) {
#ifdef DEBUG
      Cout << "Variable " << v+1 << " = " << vec[v] << " prior density = "
	   << mv_dist.pdf(vec[v], v) << std::endl;
#endif // DEBUG
      pdf *= mv_dist.pdf(vec[v], v);
    }
  else {
    size_t av_cntr = 0;
    for (v=0; v<num_rv; ++v)
      if (active_vars[v]) {
#ifdef DEBUG
	Cout << "Variable " << v+1 << " = " << vec[av_cntr]<<" prior density = "
	     << mv_dist.pdf(vec[av_cntr], v) << std::endl;
#endif // DEBUG
	pdf *= mv_dist.pdf(vec[av_cntr++], v);
      }
  }

  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    pdf *= invGammaDists[i].pdf(vec[numContinuousVars + i]);

  return pdf;
}


template <> 
inline Real NonDBayesCalibration::prior_density(const RealVector& vec)
{
  // template specialization supports RealVector (e.g., for MAP pre-solve)

  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution() // u_dist
    : iteratedModel.multivariate_distribution();                      // x_dist

  Real pdf;
  if (numHyperparams) {
    // evaluate PDF for random variables
    RealVector rv_vec(Teuchos::View, vec.values(), numContinuousVars);
    pdf = mv_dist.pdf(rv_vec);
    // augment w/ hyperparams: estimated param is mult^2 ~ invgamma(alpha,beta)
    for (size_t i=0; i<numHyperparams; ++i)
      pdf *= invGammaDists[i].pdf(vec[numContinuousVars + i]);
  }
  else
    pdf = mv_dist.pdf(vec);
    
  return pdf;
}


template <typename VectorType> 
Real NonDBayesCalibration::log_prior_density(const VectorType& vec)
{
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();
  if (mv_dist.correlation()) {
    Cerr << "Error: log_prior_density() uses a sum of log marginal densities\n"
	 << "       and can only be used for independent random variables."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Don't need to map indices so long as activeVars corresponds to vec
  //const SharedVariablesData& svd
  //  = iteratedModel.current_variables().shared_data();

  const BitArray& active_vars = mv_dist.active_variables();
  bool no_mask = active_vars.empty();
  size_t v, num_rv = mv_dist.random_variables().size(),
         active_rv = (no_mask) ? num_rv : active_vars.count();
  if (active_rv != numContinuousVars) { // avoid call to .length()/.size()
    PCerr << "Error: active variable size mismatch in NonDBayesCalibration::"
	   << "log_prior_density(): " << active_rv << " expected, "
	  << numContinuousVars << " provided." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  Real log_pdf = 0.;
  if (no_mask)
    for (v=0; v<num_rv; ++v) {
#ifdef DEBUG
      Cout << "Variable " << v+1 << " = " << vec[v] << " prior density = "
	   << mv_dist.pdf(vec[v], v) << " log prior density = "
	   << mv_dist.log_pdf(vec[v], v) << std::endl;
#endif // DEBUG
      log_pdf += mv_dist.log_pdf(vec[v], v);
    }
  else {
    size_t av_cntr = 0;
    for (v=0; v<num_rv; ++v)
      if (active_vars[v]) {
#ifdef DEBUG
	Cout << "Variable " << v+1 << " = " << vec[av_cntr]
	     << " prior density = "     << mv_dist.pdf(vec[av_cntr], v) 
	     << " log prior density = " << mv_dist.log_pdf(vec[av_cntr], v)
	     << std::endl;
#endif // DEBUG
	log_pdf += mv_dist.log_pdf(vec[av_cntr++], v);
      }
  }

  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    log_pdf += invGammaDists[i].log_pdf(vec[numContinuousVars + i]);

  return log_pdf;
}


template <> 
inline Real NonDBayesCalibration::log_prior_density(const RealVector& vec)
{
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution() // u_dist
    : iteratedModel.multivariate_distribution();                      // x_dist

  Real log_pdf;
  if (numHyperparams) {
    // evaluate PDF for random variables
    RealVector rv_vec(Teuchos::View, vec.values(), numContinuousVars);
    log_pdf = mv_dist.log_pdf(rv_vec);
    // augment w/ hyperparams: estimated param is mult^2 ~ invgamma(alpha,beta)
    for (size_t i=0; i<numHyperparams; ++i)
      log_pdf += invGammaDists[i].log_pdf(vec[numContinuousVars + i]);
  }
  else
    log_pdf = mv_dist.log_pdf(vec);

  return log_pdf;
}


template <typename Engine> 
void NonDBayesCalibration::prior_sample(Engine& rng, RealVector& prior_samples)
{
  if (prior_samples.empty())
    prior_samples.sizeUninitialized(numContinuousVars + numHyperparams);

  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();
  const std::shared_ptr<Pecos::MarginalsCorrDistribution> mv_dist_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (mv_dist.multivar_dist_rep());
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  if (mv_dist_rep->correlation()) {
    Cerr << "Error: prior_sample() does not support correlated prior samples."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (size_t i=0; i<numContinuousVars; ++i)
    prior_samples[i]
      = mv_dist_rep->draw_sample(svd.cv_index_to_all_index(i), rng);

  // the estimated param is mult^2 ~ invgamma(alpha,beta)
  for (size_t i=0; i<numHyperparams; ++i)
    prior_samples[numContinuousVars + i] = invGammaDists[i].draw_sample(rng);
}


/** Assume the target mean_vec is sized by client */
template <typename VectorType>
void NonDBayesCalibration::prior_mean(VectorType& mean_vec) const
{
  // SVD index conversion is more general, but not required for current uses
  //const SharedVariablesData& svd
  //  = iteratedModel.current_variables().shared_data();

  // returns set of means that correspond to activeVars
  RealVector mvd_means
    = (standardizedSpace) ? residualModel.multivariate_distribution().means()
    : iteratedModel.multivariate_distribution().means();

  for (size_t i=0; i<numContinuousVars; ++i)
    mean_vec[i] = mvd_means[i];//[svd.cv_index_to_active_index(i)];

  for (size_t i=0; i<numHyperparams; ++i)
    mean_vec[numContinuousVars + i] = invGammaDists[i].mean();
}


/** Assumes the target var_mat is sized by client */
template <typename MatrixType>
void NonDBayesCalibration::prior_variance(MatrixType& var_mat) const
{
  // SVD index conversion is more general, but not required for current uses
  //const SharedVariablesData& svd
  //  = iteratedModel.current_variables().shared_data();

  if (standardizedSpace) {
    // returns set of RV variances that correspond to activeVars
    RealVector u_var = residualModel.multivariate_distribution().variances();
    for (size_t i=0; i<numContinuousVars; ++i)
      var_mat(i,i) = u_var[i];//[svd.cv_index_to_active_index(i)];
  }
  else {
    const Pecos::MultivariateDistribution& x_dist
      = iteratedModel.multivariate_distribution();
    if (x_dist.correlation()) {
      // returns set of RV stdevs that correspond to activeVars
      RealVector x_std = x_dist.std_deviations();
      const RealSymMatrix& x_correl = x_dist.correlation_matrix();
      size_t i, j;//, rv_i, rv_j;
      for (i=0; i<numContinuousVars; ++i) {
	//rv_i = svd.cv_index_to_active_index(i);
	var_mat(i,i) = x_std[i] * x_std[i];//x_std[rv_i] * x_std[rv_i];
	for (j=0; j<i; ++j) {
	  //rv_j = svd.cv_index_to_active_index(j);
	  var_mat(i,j) = var_mat(j,i) = x_correl(i,j) * x_std[i] * x_std[j];
	  //var_mat(i,j) = var_mat(j,i)
	  //  = x_correl(rv_i,rv_j) * x_std[rv_i] * x_std[rv_j];
	}
      }
    }
    else {
      // returns set of RV variances that correspond to activeVars
      RealVector x_var = x_dist.variances();
      for (size_t i=0; i<numContinuousVars; ++i)
	var_mat(i,i) = x_var[i];//[svd.cv_index_to_active_index(i)];
    }
  }

  for (size_t i=0; i<numHyperparams; ++i)
    var_mat(numContinuousVars + i, numContinuousVars + i) = 
      invGammaDists[i].variance();
}


template <typename VectorType1, typename VectorType2> 
void NonDBayesCalibration::
augment_gradient_with_log_prior(VectorType1& log_grad, const VectorType2& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> gradient of neg log posterior = misfit gradient - log prior gradient
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();

  for (size_t i=0; i<numContinuousVars; ++i)
    log_grad[i] -=
      mv_dist.log_pdf_gradient(vec[i], svd.cv_index_to_all_index(i));
}


template <typename MatrixType, typename VectorType> 
void NonDBayesCalibration::
augment_hessian_with_log_prior(MatrixType& log_hess, const VectorType& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> Hessian of neg log posterior = misfit Hessian - log prior Hessian
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();

  for (size_t i=0; i<numContinuousVars; ++i)
    log_hess(i, i) -=
      mv_dist.log_pdf_hessian(vec[i], svd.cv_index_to_all_index(i));
}


/*
template <> 
inline void NonDBayesCalibration::
augment_gradient_with_log_prior(RealVector& log_grad, const RealVector& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> Hessian of neg log posterior = misfit Hessian - log prior Hessian
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();

  log_grad -= mv_dist.log_pdf_gradient(vec);
}


template <> 
inline void NonDBayesCalibration::
augment_hessian_with_log_prior(RealSymMatrix& log_hess, const RealVector& vec)
{
  // neg log posterior = neg log likelihood + neg log prior = misfit - log prior
  // --> Hessian of neg log posterior = misfit Hessian - log prior Hessian
  const Pecos::MultivariateDistribution& mv_dist
    = (standardizedSpace) ? residualModel.multivariate_distribution()
    : iteratedModel.multivariate_distribution();

  log_hess -= mv_dist.log_pdf_hessian(vec); // only need diagonal correction...
}
*/


inline void NonDBayesCalibration::
get_positive_definite_covariance_from_hessian( const RealSymMatrix &hessian,
					       RealSymMatrix &covariance )
{
  get_positive_definite_covariance_from_hessian(hessian, priorCovCholFactor,
						covariance, outputLevel);
}

} // namespace Dakota

#endif
