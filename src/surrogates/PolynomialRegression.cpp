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
// Default constructor that initializes defaultConfigOptions

PolynomialRegression::PolynomialRegression() {
  default_options();
}

// Constructor that only sets parameters
PolynomialRegression::PolynomialRegression(const ParameterList &param_list) {
  default_options();
  configOptions = param_list;
}

// Constructor that mirrors GP
PolynomialRegression::PolynomialRegression(const MatrixXd &samples,
                                           const MatrixXd &response,
                                           const ParameterList &param_list) {
  default_options();
  configOptions = param_list;
  build(samples, response);
}

// Constructor using "options"
//
/*
PolynomialRegression::PolynomialRegression(std::shared_ptr<ParameterList> options)
{
  numVars        = options->get<int>("Num Vars");
  int max_degree = options->get<int>("Max Degree");
  double p_norm  = options->get     ("P-Norm", 1.0);

  basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVars, max_degree, p_norm, *basisIndices);
  numTerms       = basisIndices->cols();

  scalerType = util::SCALER_TYPE::NONE;
}
*/

/*
PolynomialRegression::
PolynomialRegression(const MatrixXd& samples_in, const MatrixXd& response_in,
		     ParameterList& options)
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
*/


// ------------------------------------------------------------

// Constructor
// BMA: This ctor has the advantage of building basis without having the data
PolynomialRegression::PolynomialRegression(int total_order, int nvars) :
  numVars(nvars)
{
  //basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVars, total_order, 1.0, basisIndices);
  numTerms       = basisIndices.cols();

  scalerType = util::SCALER_TYPE::NONE;
}

// ------------------------------------------------------------
// Destructor

PolynomialRegression::~PolynomialRegression() {}

// ------------------------------------------------------------
// Getters

const MatrixXd & PolynomialRegression::get_samples() const { return samples_; }

const MatrixXd & PolynomialRegression::get_response() const { return response_; }

int PolynomialRegression::get_polynomial_order() const { return polynomialOrder; }

util::SCALER_TYPE PolynomialRegression::get_scaler_type() const { return scalerType; }

const MatrixXd & PolynomialRegression::get_polynomial_coeffs() const { return polynomial_coeffs; }

double PolynomialRegression::get_polynomial_intercept() const { return polynomial_intercept; }

const util::LinearSolverBase & PolynomialRegression::get_solver() const { return *solver; }

int PolynomialRegression::get_num_terms() const { return numTerms; }

// Setters

void PolynomialRegression::set_samples(const MatrixXd & samples) { samples_ = samples; }

void PolynomialRegression::set_response(const MatrixXd & response) { response_ = response; }

void PolynomialRegression::set_polynomial_order(int polynomial_order) { polynomialOrder = polynomial_order; }

void PolynomialRegression::set_scaler_type(const util::SCALER_TYPE scaler_type) { scalerType = scaler_type; }

void PolynomialRegression::set_solver(util::SOLVER_TYPE solver_type_) { solver = solver_factory(solver_type_); }

void PolynomialRegression::set_polynomial_coeffs(const MatrixXd &coeffs) { polynomial_coeffs = coeffs; }

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

  double val;

  for (int j = 0; j < numTerms; ++j) {
    for (int i = 0; i < num_samples; ++i) {
      val = 1.0;
      for (int d = 0; d < numVars; ++d) {
	      val *= std::pow(samples(i,d), basisIndices(d,j));
      }
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
  compute_basis_matrix(samples_, unscaled_basis_matrix);

  // Scale the basis matrix.
  scaler = util::scaler_factory(scalerType, unscaled_basis_matrix);
  MatrixXd scaled_basis_matrix = scaler->get_scaled_features();

  // Solve the basis matrix.
  //polynomial_coeffs = std::make_shared<MatrixXd>(*response);
  solver->solve(scaled_basis_matrix, response_, polynomial_coeffs);

  // Compute the intercept
  polynomial_intercept = response_.mean() - (scaled_basis_matrix*polynomial_coeffs).mean();
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
  approx_values = scaled_basis_matrix*polynomial_coeffs;
  approx_values = (approx_values.array() + polynomial_intercept).matrix();
}

void PolynomialRegression::build(const MatrixXd &samples, const MatrixXd &response) {

  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  std::cout << "Building Polynomial with configuration options\n"
	  << configOptions << std::endl;


  numQOI = response.cols();
  numSamples = samples.rows();
  numVariables = samples.cols();
  numVars = samples.cols();
  //set_samples(samples);
  //set_response(response);

  int max_degree = configOptions.get<int>("max degree");
  double p_norm  = configOptions.get<double>("p-norm");
  //basisIndices   = std::make_shared<MatrixXi>();
  compute_hyperbolic_indices(numVariables, max_degree, p_norm, basisIndices);
  numTerms       = basisIndices.cols();

  // Generate the basis matrix
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(samples, unscaled_basis_matrix);

  // Scale the basis matrix.
  scalerType = util::DataScaler::scaler_type(configOptions.get<std::string>("scaler type"));
  scaler = util::scaler_factory(scalerType, unscaled_basis_matrix);
  MatrixXd scaled_basis_matrix = scaler->get_scaled_features();

  // Solve the basis matrix.
  solverType = util::LinearSolverBase::solver_type(configOptions.get<std::string>("regression solver type"));
  solver = util::solver_factory(solverType);
  /* DTS: I get why this is here but it seems to be an artifact due to
   * the use of a shared pointer for polynomial_coeffs.*/
  //polynomial_coeffs = std::make_shared<MatrixXd>(response);
  solver->solve(scaled_basis_matrix, response, polynomial_coeffs);

  // Compute the intercept
  //polynomial_intercept = get_response().mean() - (scaled_basis_matrix*(get_polynomial_coeffs())).mean();
  polynomial_intercept = response.mean() - (scaled_basis_matrix*polynomial_coeffs).mean();


}

void PolynomialRegression::value(const MatrixXd &eval_points, 
                                 MatrixXd &approx_values)
{
  // Generate the basis matrix for the eval points
  MatrixXd unscaled_basis_matrix;
  compute_basis_matrix(eval_points, unscaled_basis_matrix);

  // Scale sample points.
  MatrixXd scaled_basis_matrix = *(scaler->scale_samples(unscaled_basis_matrix));

  // Find the polynomial regression values.
  approx_values = scaled_basis_matrix*(get_polynomial_coeffs());
  approx_values = (approx_values.array() + polynomial_intercept).matrix();
}

void PolynomialRegression::default_options() {
  defaultConfigOptions.set("max degree", 1, "Maximum polynomial order");
  defaultConfigOptions.set("p-norm", 1.0, "P-Norm in hyperbolic cross");
  defaultConfigOptions.set("scaler type", "none", "Type of data scaling");
  defaultConfigOptions.set("regression solver type", "SVD", "Type of regression solver");
}

void PolynomialRegression::gradient(const MatrixXd &samples, MatrixXd &gradient,
                                    const int qoi) {

  // Surrogate models don't yet support multiple responses
  assert(qoi == 0);

  MatrixXd basis_indices = basisIndices.cast<double>();
  basis_indices.transposeInPlace();
  MatrixXd deriv_coeffs = MatrixXd::Zero(numTerms, numVars);
  MatrixXd diff;
  MatrixXd::Index index;

  for (int i = 0; i < numVars; i++) {
    MatrixXd dec_basis_indices = basis_indices;
    dec_basis_indices.col(i).array() -= 1.0;
    for (int k = 0; k < numTerms; k++) {
      /* check for a -1 entries in the relevant row
       * of the decremented basis indices array */
      if (dec_basis_indices(k,i) > -0.5) {
        diff = basis_indices;
        (diff.rowwise() - dec_basis_indices.row(k)).rowwise().squaredNorm().minCoeff(&index);
        deriv_coeffs(index,i) = basis_indices(k,i)*polynomial_coeffs(k);
      }
    }
  }

  /* Generate the basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(samples, unscaled_eval_pts_basis_matrix);

  /* Scale the basis matrix */
  scaler->scale_samples(unscaled_eval_pts_basis_matrix,
                        scaled_eval_pts_basis_matrix);

  /* Compute the gradient */
  gradient = scaled_eval_pts_basis_matrix*deriv_coeffs;
}

void PolynomialRegression::hessian(const MatrixXd &sample, MatrixXd &hessian,
                                   const int qoi) {


  // Surrogate models don't yet support multiple responses
  assert(qoi == 0);

  hessian.resize(numVars, numVars);
  MatrixXd basis_indices = basisIndices.cast<double>();
  basis_indices.transposeInPlace();
  MatrixXd deriv_coeffs = VectorXd::Zero(numTerms);
  MatrixXd dec_basis_indices, diff;
  MatrixXd::Index index;

  /* Generate the (row) basis matrix */
  MatrixXd unscaled_eval_pts_basis_matrix, scaled_eval_pts_basis_matrix;
  compute_basis_matrix(sample, unscaled_eval_pts_basis_matrix);

  /* Scale the (row) basis matrix */
  scaler->scale_samples(unscaled_eval_pts_basis_matrix,
                        scaled_eval_pts_basis_matrix);

  for (int i = 0; i < numVars; i++) {
    for (int j = i; j < numVars; j++) {
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
                                  polynomial_coeffs(k);
          }
          else {
            deriv_coeffs(index) = basis_indices(k,i)*basis_indices(k,j)*
                                  polynomial_coeffs(k);
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

// ------------------------------------------------------------

} // namespace surrogates
} // namespace dakota
