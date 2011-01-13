/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TaylorApproximation
//- Description:  Implementation of a first- or second-order Taylor series 
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories
 
#include "TaylorApproximation.H"
#include "ProblemDescDB.H"


using namespace std;

namespace Dakota {

TaylorApproximation::
TaylorApproximation(ProblemDescDB& problem_db, size_t num_vars):
  Approximation(BaseConstructor(), problem_db, num_vars)
{
  // retrieve actual_model_pointer specification and set the DB
  const String& actual_model_ptr
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(actual_model_ptr);
  // set the data order based on the Hessian specification for the actual model
  if (problem_db.get_string("responses.gradient_type") == "none") {
    Cerr << "Error: response gradients required in TaylorApproximation."
	 << std::endl;
    abort_handler(-1);
  }
  dataOrder
    = (problem_db.get_string("responses.hessian_type") == "none") ? 3 : 7;
  // restore the specification
  problem_db.set_db_model_nodes(model_index);
}


int TaylorApproximation::min_coefficients() const
{
  int num_coeffs = numVars + 1; // first-order Taylor series
  if (dataOrder & 4)
    num_coeffs += numVars*(numVars + 1)/2;
  return num_coeffs;
}


void TaylorApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // No computations needed.  Just do sanity checking on currentPoints.

  // Check number of data points
  if (anchorPoint.is_null() || !currentPoints.empty()) {
    Cerr << "Error: wrong number of data points in TaylorApproximation::build."
	 << endl;
    abort_handler(-1);
  }

  // Check gradient
  if (anchorPoint.response_gradient().length() != numVars) {
    Cerr << "Error: gradient vector required in TaylorApproximation::build."
	 << endl;
    abort_handler(-1);
  }

  // Check Hessian
  if (dataOrder & 4) {
    const RealSymMatrix& hess = anchorPoint.response_hessian();
    if (hess.numRows() != numVars) {
      Cerr << "Error: Hessian matrix required in TaylorApproximation::build."
	   << endl;
      abort_handler(-1);
    }
  }
}


const Real& TaylorApproximation::get_value(const RealVector& x)
{ 
  approxValue               = anchorPoint.response_function();
  const RealVector&  c_vars = anchorPoint.continuous_variables();
  const RealVector&    grad = anchorPoint.response_gradient();
  const RealSymMatrix& hess = anchorPoint.response_hessian();
  for (size_t i=0; i<numVars; i++) {
    Real dist_i = x[i] - c_vars[i];
    approxValue += grad[i] * dist_i;
    if (dataOrder & 4) // include Hessian terms
      for (size_t j=0; j<numVars; j++)
        approxValue += dist_i * hess(i,j) * (x[j] - c_vars[j])/2.;
  }
  return approxValue;
}


const RealVector& TaylorApproximation::
get_gradient(const RealVector& x)
{
  approxGradient = anchorPoint.response_gradient();
  if (dataOrder & 4) { // include Hessian terms
    const RealVector&  c_vars = anchorPoint.continuous_variables();
    const RealSymMatrix& hess = anchorPoint.response_hessian();
    for (size_t i=0; i<numVars; i++)
      for (size_t j=0; j<numVars; j++)
        approxGradient[i] += hess(i,j) * (x[j] - c_vars[j]);
  }
  return approxGradient;
}


const RealSymMatrix& TaylorApproximation::
get_hessian(const RealVector& x)
{
  if (dataOrder & 4)
    return anchorPoint.response_hessian();
  else {
    if (approxHessian.numRows() != numVars)
      approxHessian.reshape(numVars);
    approxHessian = 0.;
    return approxHessian;
  }
}

} // namespace Dakota
