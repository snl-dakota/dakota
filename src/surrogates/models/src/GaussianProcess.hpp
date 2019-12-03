/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
    _______________________________________________________________________ */

#ifndef SURROGATES_GAUSSIAN_PROCESS_HPP
#define SURROGATES_GAUSSIAN_PROCESS_HPP

#include <memory>

#include "teuchos_data_types.hpp"
#include "DataScaler.hpp"

#include "Eigen/Dense"
#include "Eigen/StdVector"

using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace Surrogates {

class GaussianProcess {

public:


  GaussianProcess();

  ~GaussianProcess();

  /// Main constructor -- missing some options right now
  GaussianProcess(const RealMatrix &samples, const RealMatrix &response,
                  const VectorXd &sigma_bounds,
                  const MatrixXd &length_scale_bounds,
                  const std::string scaler_type = "mean_normalization",
                  const int num_restarts = 5,
                  const double nugget_val = 1.0e-10, const int seed = 129);

  RealVector get_posterior_std_dev() {return posteriorStdDev;}

  RealMatrix get_posterior_covariance() {return posteriorCov;}

  VectorXd get_theta_values() {return thetaValues;}

  void set_theta(const std::vector<double> theta_new);

  // evaluate the objective function and the gradient
  void negative_marginal_log_likelihood(double &obj_value, VectorXd &obj_gradient);

  /** \copydoc Function::value() */
  //virtual void value(const RealMatrix &samples, RealMatrix &approx_values);
  void value(const RealMatrix &samples, RealMatrix &approx_values);

  /* from John's Approximation class */
  /** \copydoc Function::gradient() */
  //virtual void gradient(const RealMatrix &samples, int qoi, RealMatrix &result_0);

  /** \copydoc Function::jacobian() */
  //virtual void jacobian(const RealVector &sample, RealMatrix &result_0);

  /** \copydoc Function::hessian() */
  //virtual void hessian(const RealMatrix &samples, int qoi, RealMatrixList &hessians);

  void vector_teuchos_to_eigen(const RealVector &x, VectorXd &y);
  void matrix_teuchos_to_eigen(const RealMatrix &A, MatrixXd &B);

  void vector_eigen_to_teuchos(const VectorXd &x, RealVector &y);
  void matrix_eigen_to_teuchos(const MatrixXd &A, RealMatrix &B);

private:

  int numVariables;
  int numSamples;

  double nuggetValue;

  MatrixXd targetValues; // save this because it is used by the objective function

  VectorXd thetaValues; // vector of hyperparameters - make matrix later?

  VectorXd optimalThetaValues; // vector of best hyperparameters

  //RealVector fun_history; // history of objective function values
  VectorXd objectiveFunctionHistory; // history of objective function values

  /* Gram matrix \phi_i \phi_j */
  MatrixXd GramMatrix;

  /* Gram matrix derivatives \phi_i \phi_j */
  std::vector<MatrixXd> GramMatrixDerivs;

  /* Distances -> # samples by # samples by # num dim  */
  std::vector<MatrixXd> componentwiseDistances;

  /* Pivoted Cholesky Factorization */
  Eigen::LDLT<MatrixXd> CholFact;

  /* Posterior covariance */
  RealMatrix posteriorCov;
  MatrixXd posteriorCovEigen;

  /* Posterior standard deviation */
  RealVector posteriorStdDev;
  VectorXd posteriorStdDevEigen;

  /// scaler for the input data
  std::shared_ptr<DataScaler> dataScaler;

  const double PI = 3.14159265358979323846;

  double finalObjFunValue;

  void compute_Gram(bool compute_derivs);

  double sq_exp_cov(const int i, const int j);

  void compute_Gram_pred(const MatrixXd &samples, MatrixXd &Gram_pred);

  double sq_exp_cov_pred(const VectorXd &x, const VectorXd &y);

  void compute_prediction_matrix(const MatrixXd &pred_pts, MatrixXd &pred_mat);

  void generate_initial_guesses(MatrixXd &initial_guesses, int num_restarts,
                                const VectorXd &sigma_bounds,
                                const MatrixXd &length_scale_bounds);



}; // class GaussianProcess

}  // namespace Surrogates

#endif  // include guard
