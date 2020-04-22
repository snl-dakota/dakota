/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PolynomialRegression.hpp"
#include "surrogates_tools.hpp"

namespace dakota {
namespace surrogates {

using SCALER_TYPE = util::DataScaler::SCALER_TYPE;
using SOLVER_TYPE = util::LinearSolverBase::SOLVER_TYPE;

PolynomialRegression::PolynomialRegression() {
  default_options();
}

PolynomialRegression::PolynomialRegression(const ParameterList &param_list) {
  default_options();
  configOptions = param_list;
}

PolynomialRegression::PolynomialRegression(const MatrixXd &samples,
                                           const MatrixXd &response,
                                           const ParameterList &param_list) {
  default_options();
  configOptions = param_list;
  build(samples, response);
}

PolynomialRegression::~PolynomialRegression() {}

void PolynomialRegression::compute_basis_matrix(const MatrixXd &samples,
                                                MatrixXd &basis_matrix) const {

  const int num_samples = samples.rows();
  basis_matrix = MatrixXd::Zero(num_samples, numTerms);
  double val;

  for (int j = 0; j < numTerms; ++j) {
    for (int i = 0; i < num_samples; ++i) {
      val = 1.0;
      for (int d = 0; d < numVariables; ++d) {
	      val *= std::pow(samples(i,d), basisIndices(d,j));
      }
      basis_matrix(i,j) = val;
    }
  }
}

void PolynomialRegression::build(const MatrixXd &samples, const MatrixXd &response) {

  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  std::cout << "\nBuilding Polynomial with configuration options\n"
	  << configOptions << std::endl;


  numQOI = response.cols();
  numSamples = samples.rows();
  numVariables = samples.cols();

  int max_degree = configOptions.get<int>("max degree");
  double p_norm  = configOptions.get<double>("p-norm");
  compute_hyperbolic_indices(numVariables, max_degree, p_norm, basisIndices);
  numTerms = basisIndices.cols();

  /* Construct the basis matrix */
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(samples, unscaled_basis_matrix);

  /* Scale the basis matrix */
  SCALER_TYPE scalerType = util::DataScaler::scaler_type(
      configOptions.get<std::string>("scaler type"));
  dataScaler = util::scaler_factory(scalerType, unscaled_basis_matrix);
  MatrixXd scaled_basis_matrix = dataScaler->get_scaled_features();

  /* Solve the for the polynomial coefficients */
  SOLVER_TYPE solverType = util::LinearSolverBase::solver_type(
      configOptions.get<std::string>("regression solver type"));
  linearSolver = util::solver_factory(solverType);
  linearSolver->solve(scaled_basis_matrix, response, polynomialCoeffs);

  /* Compute the intercept */
  polynomialIntercept = response.mean()
                      - (scaled_basis_matrix*polynomialCoeffs).mean();
}

void PolynomialRegression::value(const MatrixXd &eval_points,
                                 MatrixXd &approx_values) {

  /* Construct the basis matrix for the eval points */
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(eval_points, unscaled_basis_matrix);

  /* Scale the sample points */
  MatrixXd scaled_basis_matrix = *(dataScaler->scale_samples(unscaled_basis_matrix));

  /* Compute the prediction values*/
  approx_values = scaled_basis_matrix*(polynomialCoeffs);
  approx_values = (approx_values.array() + polynomialIntercept).matrix();
}

void PolynomialRegression::default_options() {
  defaultConfigOptions.set("max degree", 1, "Maximum polynomial order");
  defaultConfigOptions.set("p-norm", 1.0, "P-Norm in hyperbolic cross");
  defaultConfigOptions.set("scaler type", "none", "Type of data scaling");
  defaultConfigOptions.set("regression solver type", "SVD", "Type of regression solver");
}

void PolynomialRegression::gradient(const MatrixXd &samples, MatrixXd &gradient,
                                    const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  MatrixXd basis_indices = basisIndices.cast<double>();
  basis_indices.transposeInPlace();
  MatrixXd deriv_coeffs = MatrixXd::Zero(numTerms, numVariables);
  MatrixXd diff;
  MatrixXd::Index index;

  for (int i = 0; i < numVariables; i++) {
    MatrixXd dec_basis_indices = basis_indices;
    dec_basis_indices.col(i).array() -= 1.0;
    for (int k = 0; k < numTerms; k++) {
      /* check for a -1 entries in the relevant row
       * of the decremented basis indices array */
      if (dec_basis_indices(k,i) > -0.5) {
        diff = basis_indices;
        (diff.rowwise() - dec_basis_indices.row(k)).rowwise().squaredNorm().minCoeff(&index);
        deriv_coeffs(index,i) = basis_indices(k,i)*polynomialCoeffs(k);
      }
    }
  }

  /* Generate the basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(samples, unscaled_eval_pts_basis_matrix);

  /* Scale the basis matrix */
  dataScaler->scale_samples(unscaled_eval_pts_basis_matrix,
                        scaled_eval_pts_basis_matrix);

  /* Compute the gradient */
  gradient = scaled_eval_pts_basis_matrix*deriv_coeffs;
}

void PolynomialRegression::hessian(const MatrixXd &sample, MatrixXd &hessian,
                                   const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  hessian.resize(numVariables, numVariables);
  MatrixXd basis_indices = basisIndices.cast<double>();
  basis_indices.transposeInPlace();
  MatrixXd deriv_coeffs = VectorXd::Zero(numTerms);
  MatrixXd dec_basis_indices, diff;
  MatrixXd::Index index;

  /* Generate the (row) basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(sample, unscaled_eval_pts_basis_matrix);

  /* Scale the (row) basis matrix */
  dataScaler->scale_samples(unscaled_eval_pts_basis_matrix,
                            scaled_eval_pts_basis_matrix);

  for (int i = 0; i < numVariables; i++) {
    for (int j = i; j < numVariables; j++) {
      deriv_coeffs.setZero();
      dec_basis_indices = basis_indices;
      dec_basis_indices.col(i).array() -= 1.0;
      dec_basis_indices.col(j).array() -= 1.0;
      for (int k = 0; k < numTerms; k++) {
        /* check for any -1 entries in the relevant row
         * of the decremented basis indices array */
        if (!(dec_basis_indices.row(k).any() < -0.5)) {
          diff = basis_indices;
          (diff.rowwise() - dec_basis_indices.row(k)).rowwise().squaredNorm().minCoeff(&index);
          if (i == j) {
            deriv_coeffs(index) = basis_indices(k,i)*(basis_indices(k,i) - 1.0)*
                                  polynomialCoeffs(k);
          }
          else {
            deriv_coeffs(index) = basis_indices(k,i)*basis_indices(k,j)*
                                  polynomialCoeffs(k);
          }
        }
      }
      hessian(i,j) = (scaled_eval_pts_basis_matrix*deriv_coeffs)(0,0);
      if (i != j) {
        hessian(j,i) = hessian(i,j);
      }
    }
  }
}

const MatrixXd & PolynomialRegression::get_polynomial_coeffs() const { return polynomialCoeffs; }
double PolynomialRegression::get_polynomial_intercept() const { return polynomialIntercept; }
int PolynomialRegression::get_num_terms() const { return numTerms; }

void PolynomialRegression::set_polynomial_coeffs(const MatrixXd &coeffs) { polynomialCoeffs = coeffs; }

} // namespace surrogates
} // namespace dakota
