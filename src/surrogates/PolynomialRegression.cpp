/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PolynomialRegression.hpp"
#include "SurrogatesTools.hpp"

namespace dakota {
namespace surrogates {

// ------------------------------------------------------------

// Constructor using "options"

PolynomialRegression::PolynomialRegression(std::shared_ptr<Teuchos::ParameterList> options)
{
  numVars        = options->get<int>("Num Vars");
  int max_degree = options->get<int>("Max Degree");
  double p_norm  = options->get     ("P-Norm", 1.0);

  basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVars, max_degree, p_norm, *basisIndices);
  numTerms       = basisIndices->cols();

  scalerType = util::SCALER_TYPE::NONE;
}

// ------------------------------------------------------------

// Constructor

PolynomialRegression::PolynomialRegression(int total_order, int nvars) :
  numVars(nvars)
{
  basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVars, total_order, 1.0, *basisIndices);
  numTerms       = basisIndices->cols();

  scalerType = util::SCALER_TYPE::NONE;
}

// ------------------------------------------------------------
// Destructor

PolynomialRegression::~PolynomialRegression() {}

// ------------------------------------------------------------
// Getters

const MatrixXd & PolynomialRegression::get_samples() const { return *samples_; }

const MatrixXd & PolynomialRegression::get_response() const { return *response; }

int PolynomialRegression::get_polynomial_order() const { return polynomialOrder; }

util::SCALER_TYPE PolynomialRegression::get_scaler_type() const { return scalerType; }

const MatrixXd & PolynomialRegression::get_polynomial_coeffs() const { return *polynomial_coeffs; }

double PolynomialRegression::get_polynomial_intercept() const { return polynomial_intercept; }

const util::LinearSolverBase & PolynomialRegression::get_solver() const { return *solver; }

// Setters

void PolynomialRegression::set_samples(const MatrixXd & samples) { samples_ = std::make_shared<MatrixXd>(samples); }

void PolynomialRegression::set_response(const MatrixXd & response_) { response = std::make_shared<MatrixXd>(response_); }

void PolynomialRegression::set_polynomial_order(int polynomial_order) { polynomialOrder = polynomial_order; }

void PolynomialRegression::set_scaler_type(const util::SCALER_TYPE scaler_type) { scalerType = scaler_type; }

void PolynomialRegression::set_solver(util::SOLVER_TYPE solver_type_) { solver = solver_factory(solver_type_); }

// ------------------------------------------------------------
// Surrogate

void
PolynomialRegression::compute_basis_matrix(const MatrixXd & samples, MatrixXd & basis_matrix) const
{
  if( samples.size() == 0 )
    return;

  const int num_samples = samples.rows();

  // Generate the basis matrix
  basis_matrix = MatrixXd::Zero(num_samples, numTerms);

  for(int j=0; j<numTerms; ++j) {
    for(int i=0; i<num_samples; ++i) {
      double val = 1.0;
      for( int d=0; d<numVars; ++d )
	val *= std::pow(samples(i,d), (*basisIndices)(d,j));
      basis_matrix(i,j) = val;
    }
  }
}

// ------------------------------------------------------------

void
PolynomialRegression::build_surrogate()
{
  const int num_samples = get_samples().size();
  const int num_responses = get_response().size();

  if(num_samples == 0 || num_responses == 0)
    return;

  // Generate the basis matrix
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(get_samples(), unscaled_basis_matrix);

  // Scale the basis matrix.
  scaler = util::scaler_factory(scalerType, unscaled_basis_matrix);
  MatrixXd scaled_basis_matrix = scaler->get_scaled_features();

  // Solve the basis matrix.
  polynomial_coeffs = std::make_shared<MatrixXd>(*response);
  solver->solve(scaled_basis_matrix, *response, *polynomial_coeffs);

  // Compute the intercept
  polynomial_intercept = get_response().mean() - (scaled_basis_matrix*(get_polynomial_coeffs())).mean();
}

// ------------------------------------------------------------

void
PolynomialRegression::surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values)
{
  // Generate the basis matrix for the eval points
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(eval_points, unscaled_basis_matrix);

  // Scale sample points.
  MatrixXd scaled_basis_matrix = *(scaler->scale_samples(unscaled_basis_matrix));

  // Find the polynomial regression values.
  approx_values = scaled_basis_matrix * (get_polynomial_coeffs());
  approx_values = (approx_values.array() + polynomial_intercept).matrix();
}

// ------------------------------------------------------------

void
PolynomialRegression::gradient(const MatrixXd &samples, MatrixXd &gradient) {
  MatrixXd polynomial_coeffs = get_polynomial_coeffs();
  const int num_variables = samples.cols();
  const int p_norm = 1.0;

  Eigen::MatrixXi basis_indices;
  compute_hyperbolic_indices( num_variables, polynomial_order, p_norm, basis_indices);

  for ( int i = 0; i < num_variables; i++ )
  {
    MatrixXi derivative_matrix;
    VectorXi scaling_factors;

    for ( int j = 0; j < basis_indices.size(); j++ )
    {
      VectorXi next_row;
      bool fail = false;
      for ( int k = 0; k < num_variables; k++ )
      {
        if ( i == k )
        {
          int derivative = basis_indices(i, j) - 1;
          if ( derivative > -1 )
          {
            next_row << derivative;
          }
          else
          {
            fail = true;
            break;
          }
        }
        else
        {
          next_row << basis_indices(i, j);
        }
      }

      if ( !fail )
      {
        derivative_matrix << next_row;
      }
    }

    for ( int l = 0; l < beta_indices.size(); l++ )
    {
      int beta_index = find_matching_row(basis_indices, derivative_matrix(l));
      gradient(i, l) = polynomial_coeffs(beta_indices(l), 0) * scaling_factors(beta_index);
    }
  }
}

int find_matching_row(MatrixXd &hyperbolic_indices, MatrixXd &decremented_indices, VectorXd &target_row) {
  MatrixXd c = hyperbolic_indices - decremented_indices;
  VectorXd d = c.rowwise().sum();

  MatrixXd::Index minRow, minCol;
  double min = d.cwiseAbs().minCoeff(&minRow, &minCol);
  return minRow.index();
}

void
PolynomialRegression::hessian(const MatrixXd &sample, MatrixXd &hessian) {

}

} // namespace surrogates
} // namespace dakota
