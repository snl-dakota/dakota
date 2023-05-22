/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGPKernels.hpp"

namespace dakota {
namespace surrogates {

Kernel::Kernel() {}
Kernel::~Kernel() {}

SquaredExponentialKernel::SquaredExponentialKernel() {}
SquaredExponentialKernel::~SquaredExponentialKernel() {}

void SquaredExponentialKernel::compute_gram(const std::vector<MatrixXd>& dists2,
                                            const VectorXd& theta_values,
                                            MatrixXd& gram) {
  compute_Dbar(dists2, theta_values, false);
  gram = exp(2.0 * theta_values(0)) * (-0.5 * Dbar2.array()).exp();
}

void SquaredExponentialKernel::compute_gram_derivs(
    const MatrixXd& gram, const std::vector<MatrixXd>& dists2,
    const VectorXd& theta_values, std::vector<MatrixXd>& gram_derivs) {
  const int num_derivs = dists2.size() + 1;
  gram_derivs[0] = 2.0 * gram;
  for (int k = 1; k < num_derivs; k++) {
    gram_derivs[k] =
        gram.cwiseProduct(dists2[k - 1]) * exp(-2.0 * theta_values(k));
  }
}

MatrixXd SquaredExponentialKernel::compute_first_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index) {
  MatrixXd first_deriv_pred_gram = -pred_gram.cwiseProduct(mixed_dists[index]) *
                                   exp(-2.0 * theta_values(index + 1));
  return first_deriv_pred_gram;
}

MatrixXd SquaredExponentialKernel::compute_second_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index_i, const int index_j) {
  double delta_ij = 0.0;
  if (index_i == index_j) delta_ij = 1.0;
  MatrixXd second_deriv_pred_gram =
      pred_gram.array() * exp(-2.0 * theta_values(index_i + 1)) *
      ((mixed_dists[index_i].cwiseProduct(mixed_dists[index_j])).array() *
           exp(-2.0 * theta_values(index_j + 1)) -
       delta_ij);
  return second_deriv_pred_gram;
}

Matern32Kernel::Matern32Kernel() {}
Matern32Kernel::~Matern32Kernel() {}

void Matern32Kernel::compute_gram(const std::vector<MatrixXd>& dists2,
                                  const VectorXd& theta_values,
                                  MatrixXd& gram) {
  compute_Dbar(dists2, theta_values);
  Dbar *= sqrt3;
  gram = exp(2.0 * theta_values(0)) *
         (1.0 + Dbar.array()).cwiseProduct((-Dbar).array().exp());
}

void Matern32Kernel::compute_gram_derivs(const MatrixXd& gram,
                                         const std::vector<MatrixXd>& dists2,
                                         const VectorXd& theta_values,
                                         std::vector<MatrixXd>& gram_derivs) {
  const int num_derivs = dists2.size() + 1;
  const double sig2 = exp(2.0 * theta_values(0));
  compute_Dbar(dists2, theta_values);
  Dbar *= sqrt3;
  gram_derivs[0] = 2.0 * gram;
  for (int k = 1; k < num_derivs; k++) {
    gram_derivs[k] = sig2 * 3.0 * exp(-2.0 * theta_values(k)) *
                     (-Dbar).array().exp() * dists2[k - 1].array();
  }
}

MatrixXd Matern32Kernel::compute_first_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index) {
  silence_unused_args(pred_gram);
  const double sig2 = exp(2.0 * theta_values(0));
  MatrixXd first_deriv_pred_gram;
  std::vector<MatrixXd> mixed_dists2 = compute_cw_dists_squared(mixed_dists);
  compute_Dbar(mixed_dists2, theta_values);
  first_deriv_pred_gram = -3.0 * sig2 *
                          ((-sqrt3 * Dbar).array().exp())
                              .matrix()
                              .cwiseProduct(mixed_dists[index]) *
                          exp(-2.0 * theta_values(index + 1));
  return first_deriv_pred_gram;
}

MatrixXd Matern32Kernel::compute_second_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index_i, const int index_j) {
  silence_unused_args(pred_gram, mixed_dists, theta_values, index_i, index_j);
  MatrixXd second_deriv_pred_gram;
  throw("Error: Matern 3/2 kernel does not have a Hessian.");
  return second_deriv_pred_gram;
}

Matern52Kernel::Matern52Kernel() {}
Matern52Kernel::~Matern52Kernel() {}

void Matern52Kernel::compute_gram(const std::vector<MatrixXd>& dists2,
                                  const VectorXd& theta_values,
                                  MatrixXd& gram) {
  compute_Dbar(dists2, theta_values);
  Dbar *= sqrt5;
  gram = exp(2.0 * theta_values(0)) *
         (1.0 + Dbar.array() + Dbar.array().square() / 3.0)
             .cwiseProduct((-Dbar).array().exp());
}

void Matern52Kernel::compute_gram_derivs(const MatrixXd& gram,
                                         const std::vector<MatrixXd>& dists2,
                                         const VectorXd& theta_values,
                                         std::vector<MatrixXd>& gram_derivs) {
  const int num_derivs = dists2.size() + 1;
  const double sig2 = exp(2.0 * theta_values(0));
  compute_Dbar(dists2, theta_values);
  Dbar *= sqrt5;
  gram_derivs[0] = 2.0 * gram;
  for (int k = 1; k < num_derivs; k++) {
    gram_derivs[k] = sig2 * 5.0 / 3.0 * exp(-2.0 * theta_values(k)) *
                     (1.0 + Dbar.array()) * ((-Dbar).array()).exp() *
                     dists2[k - 1].array();
  }
}

MatrixXd Matern52Kernel::compute_first_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index) {
  silence_unused_args(pred_gram);

  const double sig2 = exp(2.0 * theta_values(0));
  MatrixXd first_deriv_pred_gram;
  std::vector<MatrixXd> mixed_dists2 = compute_cw_dists_squared(mixed_dists);
  compute_Dbar(mixed_dists2, theta_values);
  first_deriv_pred_gram = -5.0 / 3.0 * sig2 *
                          exp(-2.0 * theta_values(index + 1)) *
                          ((-sqrt5 * Dbar).array().exp())
                              .matrix()
                              .cwiseProduct(sqrt5 * Dbar2 + Dbar)
                              .cwiseProduct(mixed_dists[index])
                              .cwiseQuotient(Dbar);
  return first_deriv_pred_gram;
}

MatrixXd Matern52Kernel::compute_second_deriv_pred_gram(
    const MatrixXd& pred_gram, const std::vector<MatrixXd>& mixed_dists,
    const VectorXd& theta_values, const int index_i, const int index_j) {
  silence_unused_args(pred_gram);

  const double sig2 = exp(2.0 * theta_values(0));
  MatrixXd second_deriv_pred_gram;
  std::vector<MatrixXd> mixed_dists2 = compute_cw_dists_squared(mixed_dists);
  compute_Dbar(mixed_dists2, theta_values);
  if (index_i == index_j) {
    const double exp_ls = exp(-2.0 * theta_values(index_i + 1));
    second_deriv_pred_gram =
        -5.0 / 3.0 * sig2 * exp_ls *
        ((-sqrt5 * Dbar).array().exp())
            .matrix()
            .cwiseProduct(Dbar + sqrt5 * Dbar2 -
                          5.0 *
                              Dbar.cwiseProduct(exp_ls * mixed_dists2[index_i]))
            .cwiseQuotient(Dbar);
  } else {
    second_deriv_pred_gram =
        25.0 / 3.0 * sig2 *
        exp(-2.0 * (theta_values(index_i + 1) + theta_values(index_j + 1))) *
        ((-sqrt5 * Dbar).array().exp())
            .matrix()
            .cwiseProduct(mixed_dists[index_i])
            .cwiseProduct(mixed_dists[index_j]);
  }
  return second_deriv_pred_gram;
}

std::vector<MatrixXd> compute_cw_dists_squared(
    const std::vector<MatrixXd>& cw_dists) {
  const int num_variables = cw_dists.size();
  std::vector<MatrixXd> D2(num_variables);
  for (int k = 0; k < num_variables; k++) {
    D2[k] = cw_dists[k].array().square();
  }
  return D2;
}

void Kernel::compute_Dbar(const std::vector<MatrixXd>& dists2,
                          const VectorXd& theta_values, bool take_sqrt) {
  const int num_rows = dists2[0].rows();
  const int num_cols = dists2[0].cols();
  const int num_variables = dists2.size();
  Dbar2.resize(num_rows, num_cols);
  Dbar.resize(num_rows, num_cols);

  for (int k = 0; k < num_variables; k++) {
    if (k == 0)
      Dbar2 = dists2[k] * exp(-2.0 * theta_values(k + 1));
    else
      Dbar2 += dists2[k] * exp(-2.0 * theta_values(k + 1));
  }
  if (take_sqrt) Dbar = Dbar2.cwiseSqrt();
}

std::shared_ptr<Kernel> kernel_factory(const std::string& kernel_type) {
  if (kernel_type == "squared exponential") {
    return std::make_shared<SquaredExponentialKernel>();
  } else if (kernel_type == "Matern 3/2") {
    return std::make_shared<Matern32Kernel>();
  } else if (kernel_type == "Matern 5/2") {
    return std::make_shared<Matern52Kernel>();
  } else {
    throw("Error: invalid kernel type");
  }
}

}  // namespace surrogates
}  // namespace dakota
