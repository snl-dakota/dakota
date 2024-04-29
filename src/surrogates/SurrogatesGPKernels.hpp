/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GP_KERNELS_HPP
#define DAKOTA_SURROGATES_GP_KERNELS_HPP

#include "util_data_types.hpp"

#include <memory>

namespace dakota {
namespace surrogates {

/// Kernel functions for the Gaussian Process surrogate.
class Kernel {
 public:
  Kernel();

  virtual ~Kernel();

  /**
   *  \brief Compute a Gram matrix given a vector of squared distances and
   *  kernel hyperparameters.
   *  \param[in] dists2 Vector of squared distance matrices.
   *  \param[in] theta_values Vector of hyperparameters.
   *  \param[inout] gram Gram matrix.
   *  \returns Gram matrix.
   */
  virtual void compute_gram(const std::vector<MatrixXd>& dists2,
                            const VectorXd& theta_values, MatrixXd& gram) = 0;

  /**
   *  \brief Compute the derivatives of the Gram matrix with respect to the
   *  kernel hyperparameters.
   *  \param[in] gram Gram Matrix
   *  \param[in] dists2 Vector of squared distance matrices.
   *  \param[in] theta_values Vector of hyperparameters.
   *  \param[inout] gram_derivs Vector of Gram matrix derivatives.
   *  \returns Derivatives of the Gram matrix w.r.t. the hyperparameters.
   */
  virtual void compute_gram_derivs(const MatrixXd& gram,
                                   const std::vector<MatrixXd>& dists2,
                                   const VectorXd& theta_values,
                                   std::vector<MatrixXd>& gram_derivs) = 0;

  /**
   *  \brief Compute the first derivatve of the prediction matrix for a given
   *  component.
   *  \param[in] pred_gram Prediction Gram matrix - Rectangular matrix
   *  of kernel evaluations between the surrogate and prediction points.
   *  \param[in] mixed_dists Component-wise signed distances between the
   *  prediction and build points.
   *  \param[in] theta_values Vector of hyperparameters.
   *  \param[in] index Specifies the component of the derivative.
   *  \returns first_deriv_pred_gram First derivative of the prediction
   *  Gram matrix for a given component.
   */
  virtual MatrixXd compute_first_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index) = 0;

  /**
   *  \brief Compute the second derivatve of the prediction matrix for a pair of
   *  components.
   *  \param[in] pred_gram Prediction Gram matrix - Rectangular
   *  matrix of kernel evaluations between the surrogate and prediction points.
   *  \param[in] mixed_dists Component-wise signed distances between the
   *  prediction and build points.
   *  \param[in] theta_values Vector of hyperparameters.
   *  \param[in] index_i Specifies the first component of the second derivative.
   *  \param[in] index_j Specifies the second component of the second
   *  derivative.
   *  \returns second_deriv_pred_gram Second derivative of the
   *  prediction matrix for a pair of components.
   */
  virtual MatrixXd compute_second_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index_i, const int index_j) = 0;

 protected:
  /**
   *  \brief Compute the ``Dbar'' matrices of scaled distances
   *  \param[in] cw_dists2 Vector of component-wise squared distance matrices.
   *  \param[in] theta_values Vector of hyperparameters.
   *  \param[in] take_sqrt Flag for computing the square root of Dbar2.
   *  \returns Matrix of hyperparameter-scaled distances.
   */
  void compute_Dbar(const std::vector<MatrixXd>& cw_dists2,
                    const VectorXd& theta_values, bool take_sqrt = true);

  MatrixXd Dbar, Dbar2;
};

/// Stationary kernel with C^\infty smooth realizations.
class SquaredExponentialKernel : public Kernel {
 public:
  SquaredExponentialKernel();

  ~SquaredExponentialKernel();

  void compute_gram(const std::vector<MatrixXd>& dists2,
                    const VectorXd& theta_values, MatrixXd& gram) override;

  void compute_gram_derivs(const MatrixXd& gram,
                           const std::vector<MatrixXd>& dists2,
                           const VectorXd& theta_values,
                           std::vector<MatrixXd>& gram_derivs) override;

  MatrixXd compute_first_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index) override;

  MatrixXd compute_second_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index_i,
      const int index_j) override;
};

/// Stationary kernel with C^1 smooth realizations.
class Matern32Kernel : public Kernel {
 public:
  Matern32Kernel();

  ~Matern32Kernel();

  void compute_gram(const std::vector<MatrixXd>& dists2,
                    const VectorXd& theta_values, MatrixXd& gram) override;

  void compute_gram_derivs(const MatrixXd& gram,
                           const std::vector<MatrixXd>& dists2,
                           const VectorXd& theta_values,
                           std::vector<MatrixXd>& gram_derivs) override;

  MatrixXd compute_first_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index) override;

  MatrixXd compute_second_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index_i,
      const int index_j) override;

 private:
  const double sqrt3 = sqrt(3.);
};

/// Stationary kernel with C^2 smooth realizations.
class Matern52Kernel : public Kernel {
 public:
  Matern52Kernel();

  ~Matern52Kernel();

  void compute_gram(const std::vector<MatrixXd>& dists2,
                    const VectorXd& theta_values, MatrixXd& gram) override;

  void compute_gram_derivs(const MatrixXd& gram,
                           const std::vector<MatrixXd>& dists2,
                           const VectorXd& theta_values,
                           std::vector<MatrixXd>& gram_derivs) override;

  MatrixXd compute_first_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index) override;

  MatrixXd compute_second_deriv_pred_gram(
      const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
      const VectorXd& theta_values, const int index_i,
      const int index_j) override;

 private:
  const double sqrt5 = sqrt(5.);
};

/**
 *  \brief Compute a vector of component-wise squared distances from a vector of
 *  component-wise signed distances.
 *  \param[in] dists2 Vector of signed component-wise distances.
 *  \returns Vector of squared distances.
 */
std::vector<MatrixXd> compute_cw_dists_squared(
    const std::vector<MatrixXd>& cw_dists);

/**
 *  \brief Creates a derived Kernel class.
 *  \param[in] kernel_type Name of the kernel.
 *  \returns Pointer to specialized kernel class.
 */
std::shared_ptr<Kernel> kernel_factory(const std::string& kernel_type);

}  // namespace surrogates
}  // namespace dakota

#endif  // include guard
