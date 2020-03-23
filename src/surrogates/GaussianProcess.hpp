/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP
#define DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP

#include "Surrogate.hpp"
#include "DataScaler.hpp"
#include "Eigen/StdVector"
#include <memory>

namespace dakota {
namespace surrogates {

/**
 *  \brief The GaussianProcess constructs a Gaussian Process
 *  regressor surrogate given a matrix of data.
 *
 *  Currently the Gaussian Process (GP) uses an anisotropic
 *  squared exponential kernel with a constant multiplicative
 *  scaling factor. This yields a total of num_features + 1
 *  hyperparameters. These parameters are internally
 *  transformed to a log-space vector (theta) for optimization
 *  and evaluation of the GP.
 *
 *  The hyperparameters are determined through maximum
 *  likelihood estimation (MLE) via minimization of the negative
 *  marginal log-likelihood function. ROL's implementation of
 *  BFGS is used to solve the optimization problem, and the
 *  algorithm may be run from multiple random initial guesses
 *  to improve the chance of finding the global minimum.
 *
 *  Once the GP is constructed its mean, standard deviation,
 *  and covariance matrix can be computed for a set of prediction
 *  points.
 */
class GaussianProcess: public Surrogate {

public:

  // ------------------------------------------------------------
  // Constructors and destructors

  GaussianProcess();

  ~GaussianProcess();

  GaussianProcess(const MatrixXd &samples, const MatrixXd &response,
                  const Teuchos::ParameterList& param_list);

  // BMA: Leaving both ctors during refactoring; consider making
  // following constructor protected or removing

  /**
   * \brief Main constructor for the GaussianProcess.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of targets for surrogate construction - (num_samples by num_qoi = 1; only 1 response is supported currently).
   * \param[in] sigma_bounds Bounds for the scaling hyperparameter
   * \param[in] length_scale_bounds Bounds for each length scale hyperparameter.
   * \param[in] scaler_name String for which type of scaling will be applied to the surrogate data.
   * \param[in] num_restarts Number of restarts for gradient-based optimization.
   * \param[in] nugget_val Value of the Gaussian process's nugget - a small positive constant
   * \param[in] seed Seed for the random number generator. This affects the initial guesses.
   */
  GaussianProcess(const MatrixXd &samples,
                  const MatrixXd &response,
                  const VectorXd &sigma_bounds,
                  const MatrixXd &length_scale_bounds,
                  const std::string scaler_name = "mean_normalization",
                  const int num_restarts = 5,
                  const double nugget_val = 1.0e-10,
                  const int seed = 129);

  // ------------------------------------------------------------
  // Public utility functions

  /**
   *  \brief Evaluate the negative marginal loglikelihood and its 
   *  gradient.
   *  \param[out] obj_value Value of the objection function.
   *  \param[out] obj_gradient Gradient of the objective function.
   */
  void negative_marginal_log_likelihood(double &obj_value, VectorXd &obj_gradient);

  /**
   *  \brief Evaluate the Gaussian Process at a set of prediction points.
   *  \param[in] samples Matrix of prediction points - (num_samples by num_features).
   *  \param[out] approx_values Mean of the Gaussian process at the prediction
   *  points - (num_samples by num_qoi = 1) 
   */
  void value(const MatrixXd &samples, MatrixXd &approx_values) override;

  /**
   *  \brief Evaluate the gradient of the Gaussian process at a single point.
   *  \param[in] samples Coordinates of the prediction points - (num_samples by num_features).
   *  \param[out] gradient Matrix of gradient vectors at the prediction points - 
   *  (num_samples by num_features).
   */
  void gradient(const MatrixXd &samples, MatrixXd &gradient, int qoi = 0) override;

  /**
   *  \brief Evaluate the Hessian of the Gaussian process at a single point.
   *  \param[in] sample Coordinates of the prediction point - (num_samples by num_features).
   *  \param[out] hessian Hessian matrix at the prediction point - 
   *  (num_features by num_features).
   */
  void hessian(const MatrixXd &sample, MatrixXd &hessian, int qoi = 0) override;

  // ------------------------------------------------------------
  // Getters

  /**
   *  \brief Get the standard deviations for a set of prediction points
   *  passed to value.
   *  \returns Vector of standard deviations for the prediction points.
   */
  const VectorXd & get_posterior_std_dev() const;

  /**
   *  \brief Get the covariance matrix for a set of prediction points
   *  passed to value.
   *  \returns Covariance matrix for the prediction points.
   */
  const MatrixXd & get_posterior_covariance() const;

  /**
   *  \brief Get the vector of log-space hyperparameters (theta).
   *  \returns Vector of log-space hyperparameters (theta).
   */
  const VectorXd & get_theta_values() const;

  /**
   *  \brief Get the dimension of the feature space.
   *  \param[out] numVariables The dimension of the feature space.
   */
  int get_num_variables() const;


  // ------------------------------------------------------------
  // Setters

  /**
   *  \brief Update the vector of log-space hyperparamters (theta).
   *  \param[in] theta_new Vector of new theta values.
   */
  void set_theta(const std::vector<double> theta_new);


private:

  /// Key/value options to configure the surrogate (encapsulates
  /// defaults, but can be overridden by user)
  Teuchos::ParameterList defaultConfigOptions;

  /// runtime build once data are finalized
  void build(const MatrixXd &samples, const MatrixXd &response);

  /*
  void build(const MatrixXd &samples,
	     const MatrixXd &response,
	     const VectorXd &sigma_bounds,
	     const MatrixXd &length_scale_bounds,
	     const std::string scaler_name,
	     const int num_restarts,
	     const double nugget_val,
	     const int seed);
  */

  /// Dimension of the feature space.
  //int numVariables = 0;
  /// Number of samples in the surrogate dataset.
  //int numSamples = 0;

  /// Small constant added to the diagonal of the Gram matrix to avoid
  /// ill-conditioning.
  double nuggetValue;

  /// Corresponding target values for the surrogate dataset.
  std::shared_ptr<MatrixXd> targetValues;

  /// Vector of log-space hyperparameters (theta).
  std::shared_ptr<VectorXd> thetaValues;

  /// Vector of best hyperparameters from MLE with restarts.
  std::shared_ptr<VectorXd> bestThetaValues;

  /// Final objective function values for each optimization run.
  std::shared_ptr<VectorXd> objectiveFunctionHistory;

  /// Gram matrix for the GaussianProcess kernel.
  std::shared_ptr<MatrixXd> GramMatrix;

  /// Derivatives of the Gram matrix w.r.t. the hyperparameters.
  std::vector<MatrixXd> GramMatrixDerivs;

  /// Component-wise distances between points in the surrogate dataset.
  std::vector<MatrixXd> componentwiseDistances;

  /// Pivoted Cholesky factorization.
  Eigen::LDLT<MatrixXd> CholFact;

  /// Posterior covariance matrix for prediction points.
  std::shared_ptr<MatrixXd> posteriorCov;

  /// Vector of posterior standard deviation at prediction points.
  std::shared_ptr<VectorXd> posteriorStdDev;

  /// DataScaler for the surrogate data.
  std::shared_ptr<util::DataScaler> dataScaler;

  /// Numerical constant -- needed for negative marginal log-likelihood.
  const double PI = 3.14159265358979323846;

  /// Final objective function value.
  double bestObjFunValue = std::numeric_limits<double>::max();

  // ------------------------------------------------------------
  // Private utility functions

  /// setup default options: one possible way to encode the defaults
  /// when using a ParameterList
  void default_options();


  /**
   *  \brief Compute the GramMatrix for the surrogate dataset and
   *  optionally compute its derivatives.
   *  \param[in] compute_derivs Bool for whether or not to compute the 
   *  derivatives of the Gram matrix.
   */
  void compute_gram(bool compute_derivs);

  /**
   *  \brief Evaluate the squared exponential kernel for the surrogate dataset
   *  using pre-computed distances.
   *  \param[in] i Index for ith sample point.
   *  \param[in] j Index for jth sample point.
   *  \returns The value of the squared exponential kernel.
   */
  double sq_exp_cov(const int i, const int j);

  /**
   *  \brief Compute the Gram matrix for a set of prediction points.
   *  \param[in] samples Matrix of prediction points.
   *  \param[out] Gram_pred Gram matrix for the prediction points.
   */
  void compute_gram_pred(const MatrixXd &samples, MatrixXd &Gram_pred);


  /**
   *  \brief Evaluate the squared exponential kernel for the prediction points
   *  given a pair a points.
   *  \param[in] x Predicition point x.
   *  \param[in] y Predicition point y.
   *  \returns The value of the squared exponential kernel.
   */
  double sq_exp_cov_pred(const VectorXd &x, const VectorXd &y);

  /**
   *  \brief Evaluate the rectangular matrix of kernel evaluations between
   *  the surrogate and prediction datasets.
   *  \param[in] pred_pts Matrix of prediction points.
   *  \param[out] pred_mat Matrix of surrogate data-prediction points kernel evaluations.
   */
  void compute_prediction_matrix(const MatrixXd &scaled_pred_pts, MatrixXd &pred_mat);

  /**
   *  \brief Compute the first derivatve of the prediction matrix for a given component.
   *  \param[in] pred_mat Prediction matrix - Rectangular matrix of kernel evaluations between 
   *  the surrogate and prediction points.
   *  \param[in] scaled_pred_pts Matrix of prediction points.
   *  \param[in] index Specifies the component of the derivative.
   *  \param[out] first_deriv_pred_mat First derivative of the prediction matrix for a given component.
   */
  void compute_first_deriv_pred_mat(const MatrixXd &pred_mat, const MatrixXd &scaled_pred_pts,
                                    const int index, MatrixXd &fist_deriv_pred_mat);

  /**
   *  \brief Compute the second derivatve of the prediction matrix for a pair of components.
   *  \param[in] pred_mat Prediction matrix - Rectangular matrix of kernel evaluations between
   *  the surrogate and prediction points.
   *  \param[in] scaled_pred_pts Matrix of prediction points.
   *  \param[in] index_i Specifies the first component of the second derivative.
   *  \param[in] index_j Specifies the second component of the second derivative.
   *  \param[out] second_deriv_pred_mat Second derivative of the prediction matrix for a pair of components.
   */
  void compute_second_deriv_pred_mat(const MatrixXd &pred_mat, const MatrixXd &scaled_pred_pts,
                                     const int index_i, const int index_j,
                                     MatrixXd &second_deriv_pred_mat);

  /**
   *  \brief Randomly generate initial guesses for the optimization routine.
   *  \param[out] initial_guesses Matrix of initial guesses.
   *  \param[in] num_restarts Number of restarts for the optimizer.
   *  \param[in] sigma_bounds Bounds for the scaling hyperparameter (sigma).
   *  \param[in] length_scale_bounds Bounds for the length scales for each feature (l).
  */
  void generate_initial_guesses(MatrixXd &initial_guesses, int num_restarts,
                                const VectorXd &sigma_bounds,
                                const MatrixXd &length_scale_bounds);

  /**
   *  \brief Set the default optimization parameters for ROL for GP hyperparameter
   *  estimation.
   *  \param[in] rol_params RCP to a Teuchos::ParameterList of ROL's options.
   */
  void set_default_optimization_params(Teuchos::RCP<Teuchos::ParameterList> rol_params);


}; // class GaussianProcess


}  // namespace surrogates
}  // namespace dakota


#endif  // include guard
