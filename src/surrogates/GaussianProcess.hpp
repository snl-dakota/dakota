/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP
#define DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP

#include <memory>
#include "DataScaler.hpp"
#include "Eigen/StdVector"

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
class GaussianProcess {

public:


  GaussianProcess();

  ~GaussianProcess();


/**
 * \brief Main constructor for the GaussianProcess.
 *
 * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
 * \param[in] response Vector of targets for surrogate construction.
 * \param[in] sigma_bounds Bounds for the scaling hyperparameter
 * \param[in] length_scale_bounds Bounds for each length scale hyperparameter.
 * \param[in] scaler_type String for which type of scaling will be applied to the surrogate data.
 * \param[in] num_restarts Number of restarts for gradient-based optimization.
 * \param[in] nugget_val Value of the Gaussian process's nugget - a small positive constant
 * \param[in] seed Seed for the random number generator. This affects the initial guesses.
 */
  GaussianProcess(const MatrixXd &samples, const MatrixXd &response,
                  const VectorXd &sigma_bounds,
                  const MatrixXd &length_scale_bounds,
                  const std::string scaler_type = "mean_normalization",
                  const int num_restarts = 5,
                  const double nugget_val = 1.0e-10, const int seed = 129);

/**
 *  \brief Get the standard deviations for a set of prediction points
 *  passed to value.
 *  \returns Vector of standard deviations for the prediction points.
*/
  VectorXd get_posterior_std_dev() {return posteriorStdDev;}

/**
 *  \brief Get the covariance matrix for a set of prediction points
 *  passed to value.
 *  \returns Covariance matrix for the prediction points.
*/
  MatrixXd get_posterior_covariance() {return posteriorCov;}

/**
 *  \brief Get the vector of log-space hyperparameters (theta).
 *  \returns Vector of log-space hyperparameters (theta).
*/
  VectorXd get_theta_values() {return thetaValues;}

/**
 *  \brief Update the vector of log-space hyperparamters (theta).
 *  \param[in] theta_new Vector of new theta values.
*/
  void set_theta(const std::vector<double> theta_new);

/**
 *  \brief Evaluate the negative marginal loglikelihood and its 
 *  gradient.
 *  \param[out] obj_value Value of the objection function.
 *  \param[out] obj_gradient Gradient of the objective function.
*/
  void negative_marginal_log_likelihood(double &obj_value, VectorXd &obj_gradient);

/**
 *  \brief Evaluate the Gaussian Process at a set of prediction points.
 *  \param[in] samples Matrix of prediction points.
 *  \param[out] approx_values Mean of the Gaussian process at the prediction
 *  points.
*/
  void value(const MatrixXd &samples, MatrixXd &approx_values);

/**
 *  \brief Get the dimension of the feature space
*/
  int get_num_variables() const
    { return numVariables; }


private:

  /// Dimension of the feature space.
  int numVariables;
  /// Number of samples in the surrogate dataset.
  int numSamples;

  /// Small constant added to the diagonal of the Gram matrix to avoid
  /// ill-conditioning.
  double nuggetValue;

  /// Corresponding target values for the surrogate dataset.
  MatrixXd targetValues;

  /// Vector of log-space hyperparameters (theta).
  VectorXd thetaValues;

  /// Vector of best hyperparameters from MLE with restarts.
  VectorXd bestThetaValues;

  /// Final objective function values for each optimization run.
  VectorXd objectiveFunctionHistory;

  /// Gram matrix for the GaussianProcess kernel.
  MatrixXd GramMatrix;

  /// Derivatives of the Gram matrix w.r.t. the hyperparameters.
  std::vector<MatrixXd> GramMatrixDerivs;

  /// Component-wise distances between points in the surrogate dataset.
  std::vector<MatrixXd> componentwiseDistances;

  /// Pivoted Cholesky factorization.
  Eigen::LDLT<MatrixXd> CholFact;

  /// Posterior covariance matrix for prediction points.
  MatrixXd posteriorCov;

  /// Vector of posterior standard deviation at prediction points.
  VectorXd posteriorStdDev;

  /// DataScaler for the surrogate data.
  std::shared_ptr<util::DataScaler> dataScaler;

  /// Numerical constant -- needed for negative marginal log-likelihood.
  const double PI = 3.14159265358979323846;

  /// Final objective function value.
  double bestObjFunValue;

/**
 *  \brief Compute the GramMatrix for the surrogate dataset and
 *  optionally compute its derivatives.
 *  \param[in] compute_derivs Bool for whether or not to compute the 
 *  derivatives of the Gram matrix.
*/
  void compute_Gram(bool compute_derivs);

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
  void compute_Gram_pred(const MatrixXd &samples, MatrixXd &Gram_pred);

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
  void compute_prediction_matrix(const MatrixXd &pred_pts, MatrixXd &pred_mat);

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



}; // class GaussianProcess


}  // namespace surrogates
}  // namespace dakota


#endif  // include guard
