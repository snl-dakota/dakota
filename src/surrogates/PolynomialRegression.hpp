/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP
#define DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP

#include <iostream>
#include <memory>
#include "DataScaler.hpp"
#include "../util/LinearSolvers.hpp"

#include "Teuchos_ParameterList.hpp"

namespace dakota {
namespace surrogates {

class PolynomialRegression {

public:

  // Constructor

  // Options-based constructor
  PolynomialRegression(std::shared_ptr<Teuchos::ParameterList> options);

  // Simple constructor
  PolynomialRegression(int total_order, int nvars);

  // Destructor

  ~PolynomialRegression();

  // Getters

  const MatrixXd & get_samples() const;
  const MatrixXd & get_response() const;
  int get_polynomial_order() const;
  util::SCALER_TYPE get_scaler_type() const;
  const MatrixXd & get_polynomial_coeffs() const;
  double get_polynomial_intercept() const;
  const util::LinearSolverBase & get_solver() const;

  // Setters

  void set_samples(const MatrixXd & );
  void set_response(const MatrixXd & );
  void set_polynomial_order(int);
  void set_scaler_type(const util::SCALER_TYPE);
  void set_solver(const util::SOLVER_TYPE);

  // Surrogate

  void compute_basis_matrix(const MatrixXd & samples, MatrixXd & basis_matrix) const;
  void build_surrogate();
  void surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values);

  // Derivatives

  void gradient(const MatrixXd &samples, MatrixXd &gradient);
  // void hessian(const MatrixXd &sample, MatrixXd &hessian); 

private:

  // Input fields
  int numTerms;
  int numVars;
  std::shared_ptr<MatrixXd> samples_;
  std::shared_ptr<MatrixXd> response;
  std::shared_ptr<MatrixXi> basisIndices;
  int polynomialOrder;
  util::SCALER_TYPE scalerType;
  std::shared_ptr<util::DataScaler> scaler;
  std::shared_ptr<util::LinearSolverBase> solver;

  // Polynomial surrogate fields
  std::shared_ptr<MatrixXd> polynomial_coeffs;
  double polynomial_intercept;
};
} // namespace surrogates
} // namespace dakota

#endif
