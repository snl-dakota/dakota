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
 
#include "TaylorApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"


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
  // set the data order based on the derivative spec for the actual model
  buildDataOrder = 1;
  if (problem_db.get_string("responses.gradient_type") != "none")
    buildDataOrder |= 2;
  if (problem_db.get_string("responses.hessian_type")  != "none")
    buildDataOrder |= 4;
  // restore the specification
  problem_db.set_db_model_nodes(model_index);
}


int TaylorApproximation::min_coefficients() const
{
  int num_coeffs = 1;     // zeroth-order Taylor series
  if (buildDataOrder & 2) //  first-order Taylor series
    num_coeffs += numVars;
  if (buildDataOrder & 4) // second-order Taylor series
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
  if ( (buildDataOrder & 2) &&
       approxData.anchor_gradient().length() != numVars) {
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


Real TaylorApproximation::value(const Variables& vars)
{
  if (buildDataOrder == 1)
    return approxData.anchor_function();
  else { // build up approx value from constant and derivative terms
    Real approx_val = (buildDataOrder & 1) ? approxData.anchor_function() : 0.;
    if (buildDataOrder & 6) {
      const RealVector&       x = vars.continuous_variables();
      const RealVector&      x0 = approxData.anchor_continuous_variables();
      const RealVector&    grad = approxData.anchor_gradient();
      const RealSymMatrix& hess = approxData.anchor_hessian();
      for (size_t i=0; i<numVars; i++) {
	Real dist_i = x[i] - x0[i];
	if (buildDataOrder & 2) // include gradient terms
	  approx_val += grad[i] * dist_i;
	if (buildDataOrder & 4) // include Hessian terms
	  for (size_t j=0; j<numVars; j++)
	    approx_val += dist_i * hess(i,j) * (x[j] - x0[j])/2.;
      }
    }
    return approx_val;
  }
}


const RealVector& TaylorApproximation::gradient(const Variables& vars)
{
  if (buildDataOrder == 2)
    return approxData.anchor_gradient();
  else { // build up approxGradient from derivative terms
    if (buildDataOrder & 2)   // include gradient terms
      approxGradient = approxData.anchor_gradient();
    else {                    // initialize approxGradient to zero
      if (approxGradient.length() != numVars) approxGradient.size(numVars);
      else                                    approxGradient = 0.;
    }
    if (buildDataOrder & 4) { // include Hessian terms
      const RealVector&       x = vars.continuous_variables();
      const RealVector&      x0 = approxData.anchor_continuous_variables();
      const RealSymMatrix& hess = approxData.anchor_hessian();
      for (size_t i=0; i<numVars; i++)
	for (size_t j=0; j<numVars; j++)
	  approxGradient[i] += hess(i,j) * (x[j] - x0[j]);
    }
    return approxGradient;
  }
}


const RealSymMatrix& TaylorApproximation::hessian(const Variables& vars)
{
  if (buildDataOrder & 4)
    return approxData.anchor_hessian();
  else { // initialize approxHessian to zero
    if (approxHessian.numRows() != numVars) approxHessian.shape(numVars);
    else                                    approxHessian = 0.;
    return approxHessian;
  }
}

} // namespace Dakota
