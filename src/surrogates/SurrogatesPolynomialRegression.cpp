/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesPolynomialRegression.hpp"

#include "surrogates_tools.hpp"

namespace dakota {
namespace surrogates {

/// alias for util SCALER_TYPE enum
using SCALER_TYPE = util::DataScaler::SCALER_TYPE;
/// alias for util SOLVER_TYPE enum
using SOLVER_TYPE = util::LinearSolverBase::SOLVER_TYPE;

PolynomialRegression::PolynomialRegression() { default_options(); }

PolynomialRegression::PolynomialRegression(const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
}

PolynomialRegression::PolynomialRegression(
    const std::string& param_list_yaml_filename) {
  default_options();
  auto param_list =
      Teuchos::getParametersFromYamlFile(param_list_yaml_filename);
  configOptions = *param_list;
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
}

PolynomialRegression::PolynomialRegression(const MatrixXd& samples,
                                           const MatrixXd& response,
                                           const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
  build(samples, response);
}

PolynomialRegression::PolynomialRegression(
    const MatrixXd& samples, const MatrixXd& response,
    const std::string& param_list_yaml_filename) {
  default_options();
  auto param_list =
      Teuchos::getParametersFromYamlFile(param_list_yaml_filename);
  configOptions = *param_list;
  build(samples, response);
}

PolynomialRegression::~PolynomialRegression() {}

void PolynomialRegression::compute_basis_matrix(const MatrixXd& samples,
                                                MatrixXd& basis_matrix) const {
  const int num_samples = samples.rows();
  basis_matrix = MatrixXd::Zero(num_samples, numTerms);
  double val;

  for (int j = 0; j < numTerms; ++j) {
    for (int i = 0; i < num_samples; ++i) {
      val = 1.0;
      for (int d = 0; d < numVariables; ++d) {
        val *= std::pow(samples(i, d), basisIndices(d, j));
      }
      basis_matrix(i, j) = val;
    }
  }
}

void PolynomialRegression::build(const MatrixXd& samples,
                                 const MatrixXd& response) {
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  verbosity = configOptions.get<int>("verbosity");

  if (verbosity > 0) {
    if (verbosity == 1) {
      std::cout << "\nBuilding Polynomial\n\n";
    } else if (verbosity == 2) {
      std::cout << "\nBuilding Polynomial with configuration options\n"
                << configOptions << "\n";
    } else
      throw(
          std::runtime_error("Invalid verbosity int for Polynomial surrogate"));
  }

  numQOI = response.cols();
  numSamples = samples.rows();
  numVariables = samples.cols();

  int max_degree = configOptions.get<int>("max degree");
  double p_norm = configOptions.get<double>("p-norm");
  bool use_reduced_basis = configOptions.get<bool>("reduced basis");
  bool standardize_response = configOptions.get<bool>("standardize response");
  if (use_reduced_basis)
    compute_reduced_indices(numVariables, max_degree, basisIndices);
  else
    compute_hyperbolic_indices(numVariables, max_degree, p_norm, basisIndices);
  numTerms = basisIndices.cols();

  /* Standardize the response */
  MatrixXd scaled_response;
  if (standardize_response) {
    SCALER_TYPE responseScalerType =
        util::DataScaler::scaler_type("standardization");
    auto responseScaler = util::scaler_factory(responseScalerType, response);
    scaled_response = responseScaler->scale_samples(response);
    responseOffset = responseScaler->get_scaler_features_offsets()(0);
    responseScaleFactor =
        responseScaler->get_scaler_features_scale_factors()(0);
  } else
    scaled_response = response;

  /* Construct the basis matrix */
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(samples, unscaled_basis_matrix);

  /* Scale the basis matrix */
  SCALER_TYPE scalerType = util::DataScaler::scaler_type(
      configOptions.get<std::string>("scaler type"));
  dataScaler = *(util::scaler_factory(scalerType, unscaled_basis_matrix));
  MatrixXd scaled_basis_matrix;
  dataScaler.scale_samples(unscaled_basis_matrix, scaled_basis_matrix);

  /* Solve the for the polynomial coefficients */
  SOLVER_TYPE solverType = util::LinearSolverBase::solver_type(
      configOptions.get<std::string>("regression solver type"));
  linearSolver = util::solver_factory(solverType);
  linearSolver->solve(scaled_basis_matrix, scaled_response, polynomialCoeffs);

  /* Compute the intercept */
  polynomialIntercept =
      scaled_response.mean() - (scaled_basis_matrix * polynomialCoeffs).mean();
}

VectorXd PolynomialRegression::value(const MatrixXd& eval_points,
                                     const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  VectorXd approx_values;

  /* Construct the basis matrix for the eval points */
  MatrixXd unscaled_eval_pts_basis_matrix;
  compute_basis_matrix(eval_points, unscaled_eval_pts_basis_matrix);

  /* Scale the sample points */
  MatrixXd scaled_eval_pts_basis_matrix;
  dataScaler.scale_samples(unscaled_eval_pts_basis_matrix,
                           scaled_eval_pts_basis_matrix);

  /* Compute the prediction values*/
  approx_values = scaled_eval_pts_basis_matrix * polynomialCoeffs;
  approx_values =
      (approx_values.array() + polynomialIntercept) * responseScaleFactor +
      responseOffset;
  return approx_values;
}

void PolynomialRegression::default_options() {
  defaultConfigOptions.set("reduced basis", false, "Use reduced basis");
  defaultConfigOptions.set("max degree", 1, "Maximum polynomial order");
  defaultConfigOptions.set("p-norm", 1.0, "P-Norm in hyperbolic cross");
  defaultConfigOptions.set("scaler type", "none", "Type of data scaling");
  defaultConfigOptions.set("regression solver type", "SVD",
                           "Type of regression solver");
  defaultConfigOptions.set("standardize response", false,
                           "Make the response zero mean and unit variance");
  /* Verbosity levels
     2 - maximum level: print out config options and building notification
     1 - minimum level: print out building notification
     0 - no output */
  defaultConfigOptions.set("verbosity", 1, "console output verbosity");
}

MatrixXd PolynomialRegression::gradient(const MatrixXd& eval_points,
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
      if (dec_basis_indices(k, i) > -0.5) {
        diff = basis_indices;
        (diff.rowwise() - dec_basis_indices.row(k))
            .rowwise()
            .squaredNorm()
            .minCoeff(&index);
        deriv_coeffs(index, i) = basis_indices(k, i) * polynomialCoeffs(k);
      }
    }
  }

  /* Generate the basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(eval_points, unscaled_eval_pts_basis_matrix);

  /* Scale the basis matrix */
  dataScaler.scale_samples(unscaled_eval_pts_basis_matrix,
                           scaled_eval_pts_basis_matrix);

  /* Compute the gradient */
  return scaled_eval_pts_basis_matrix * deriv_coeffs * responseScaleFactor;
}

MatrixXd PolynomialRegression::hessian(const MatrixXd& eval_point,
                                       const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  if (eval_point.rows() != 1) {
    throw(std::runtime_error(
        "Polynomial Hessian evaluation is for a single point."
        "The input contains more than one sample."));
  }

  MatrixXd hessian(numVariables, numVariables);
  MatrixXd basis_indices = basisIndices.cast<double>();
  basis_indices.transposeInPlace();
  MatrixXd deriv_coeffs = VectorXd::Zero(numTerms);
  MatrixXd dec_basis_indices, diff;
  MatrixXd::Index index;

  /* Generate the (row) basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(eval_point, unscaled_eval_pts_basis_matrix);

  /* Scale the (row) basis matrix */
  dataScaler.scale_samples(unscaled_eval_pts_basis_matrix,
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
          (diff.rowwise() - dec_basis_indices.row(k))
              .rowwise()
              .squaredNorm()
              .minCoeff(&index);
          if (i == j) {
            deriv_coeffs(index) = basis_indices(k, i) *
                                  (basis_indices(k, i) - 1.0) *
                                  polynomialCoeffs(k);
          } else {
            deriv_coeffs(index) =
                basis_indices(k, i) * basis_indices(k, j) * polynomialCoeffs(k);
          }
        }
      }
      hessian(i, j) = (scaled_eval_pts_basis_matrix * deriv_coeffs)(0, 0);
      if (i != j) {
        hessian(j, i) = hessian(i, j);
      }
    }
  }
  return hessian * responseScaleFactor;
}

const MatrixXd& PolynomialRegression::get_polynomial_coeffs() const {
  return polynomialCoeffs;
}
double PolynomialRegression::get_polynomial_intercept() const {
  return polynomialIntercept;
}
int PolynomialRegression::get_num_terms() const { return numTerms; }
void PolynomialRegression::set_polynomial_coeffs(const MatrixXd& coeffs) {
  polynomialCoeffs = coeffs;
}

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::PolynomialRegression)
