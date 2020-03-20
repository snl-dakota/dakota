/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PolynomialRegression.hpp"
#include "SurrogatesTools.hpp"
#include "../util/CommonUtils.hpp"

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

PolynomialRegression::
PolynomialRegression(const MatrixXd& samples_in, const MatrixXd& response_in,
		     Teuchos::ParameterList& options)
{
  // BMA: delegation only required due to shared_ptr
  set_samples(samples_in);
  set_response(response_in);

  // what to do about "Num Vars" option when it's ignored?
  numVars = samples_->cols();

  int max_degree = options.get<int>("Max Degree");
  double p_norm  = options.get     ("P-Norm", 1.0);

  basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVars, max_degree, p_norm, *basisIndices);
  numTerms       = basisIndices->cols();

  scalerType = util::DataScaler::scaler_type(options.get<std::string>("scaler_name", "none"));
}


// ------------------------------------------------------------

// Constructor
// BMA: This ctor has the advantage of building basis without having the data
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

int find_matching_row ( const MatrixXi &hyperbolic_indices, const VectorXi &decremented_indices )
{
  for ( int i = 0; i < hyperbolic_indices.cols(); i++ )
  {
    VectorXi this_col = hyperbolic_indices.col(i);
    if ( matrix_equals ( this_col, decremented_indices ))
    {
      return i;
    }
  }
  return -1;
}

// ------------------------------------------------------------

void
PolynomialRegression::gradient ( const MatrixXd &samples, MatrixXd &gradient ) {
  MatrixXd polynomial_coeffs = get_polynomial_coeffs();
  const int num_variables = samples.cols();
  const int p_norm = 1.0;

  Eigen::MatrixXi basis_indices;
  compute_hyperbolic_indices( num_variables, polynomialOrder, p_norm, basis_indices );

  std::cout << "polynomial basis:" << std::endl;
  std::cout << basis_indices << std::endl;

  gradient.resize(num_variables, basis_indices.cols());

  for ( int i = 0; i < num_variables; i++ )
  {
    int num_derivatives = 0;
    MatrixXi derivative_matrix = basis_indices.replicate(1,1);
    VectorXi scaling_factors(basis_indices.cols());

    for ( int j = 0; j < basis_indices.cols(); j++ )
    {
      int derivative = basis_indices(i, j) - 1;
      if ( derivative > -1 )
      {
        derivative_matrix(i, j) = derivative;
        num_derivatives++;
      }
      else
      {
        for ( int k = 0; k < num_variables; k++ )
        {
          derivative_matrix(k,j) = -1;
        }
      }
      scaling_factors(j) = basis_indices(i,j);
    }

    std::cout << "derivative_matrix:" << std::endl;
    std::cout << "for variable " << i << std::endl;
    std::cout << derivative_matrix << std::endl;

    int gradient_index = 0;
    for ( int j = 0; j < basis_indices.cols(); j++ )
    {
      int beta_index = find_matching_row(basis_indices, derivative_matrix.col(j));
      if ( beta_index != -1 )
      {
        double gradient_value = polynomial_coeffs(beta_index, 0) * scaling_factors(beta_index);
        if ( std::abs(gradient_value) < 1.0e-12 )
        {
          gradient(i, gradient_index) = 0.0;
        }
        else
        {
          gradient(i, gradient_index) = gradient_value;
        }
        gradient_index ++;
      }
    }
    for ( int j = gradient_index; j < basis_indices.cols(); j++)
    {
      gradient(i, j) = 0.0;
    }
  }
}
/*
void
PolynomialRegression::hessian(const MatrixXd &sample, MatrixXd &hessian) {

}
*/

} // namespace surrogates
} // namespace dakota
