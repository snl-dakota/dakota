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
  buildDataOrder
    = (problem_db.get_string("responses.hessian_type") == "none") ? 3 : 7;
  // restore the specification
  problem_db.set_db_model_nodes(model_index);
}


int TaylorApproximation::min_coefficients() const
{
  int num_coeffs = numVars + 1; // first-order Taylor series
  if (buildDataOrder & 4)
    num_coeffs += numVars*(numVars + 1)/2;
  return num_coeffs;
}


void TaylorApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // No computations needed.  Just do sanity checking on approxData.

  // Check number of data points
  if (!approxData.anchor() || approxData.size()) {
    Cerr << "Error: wrong number of data points in TaylorApproximation::"
	 << "build()." << std::endl;
    abort_handler(-1);
  }

  // Check gradient
  if (approxData.anchor_gradient().length() != numVars) {
    Cerr << "Error: gradient vector required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(-1);
  }

  // Check Hessian
  if ( (buildDataOrder & 4) &&
       approxData.anchor_hessian().numRows() != numVars) {
    Cerr << "Error: Hessian matrix required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(-1);
  }
}


const Real& TaylorApproximation::get_value(const RealVector& x)
{
  approxValue               = approxData.anchor_function();
  const RealVector&  c_vars = approxData.anchor_continuous_variables();
  const RealVector&    grad = approxData.anchor_gradient();
  const RealSymMatrix& hess = approxData.anchor_hessian();
  for (size_t i=0; i<numVars; i++) {
    Real dist_i = x[i] - c_vars[i];
    approxValue += grad[i] * dist_i;
    if (buildDataOrder & 4) // include Hessian terms
      for (size_t j=0; j<numVars; j++)
        approxValue += dist_i * hess(i,j) * (x[j] - c_vars[j])/2.;
  }
  return approxValue;
}


const RealVector& TaylorApproximation::
get_gradient(const RealVector& x)
{
  approxGradient = approxData.anchor_gradient();
  if (buildDataOrder & 4) { // include Hessian terms
    const RealVector&  c_vars = approxData.anchor_continuous_variables();
    const RealSymMatrix& hess = approxData.anchor_hessian();
    for (size_t i=0; i<numVars; i++)
      for (size_t j=0; j<numVars; j++)
        approxGradient[i] += hess(i,j) * (x[j] - c_vars[j]);
  }
  return approxGradient;
}


const RealSymMatrix& TaylorApproximation::
get_hessian(const RealVector& x)
{
  if (buildDataOrder & 4)
    return approxData.anchor_hessian();
  else {
    if (approxHessian.numRows() != numVars)
      approxHessian.reshape(numVars);
    approxHessian = 0.;
    return approxHessian;
  }
}

} // namespace Dakota
