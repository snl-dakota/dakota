/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
TaylorApproximation(ProblemDescDB& problem_db,
		    const SharedApproxData& shared_data,
                    const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
{ }


int TaylorApproximation::min_coefficients() const
{
  short  bdo   = sharedDataRep->buildDataOrder;
  size_t num_v = sharedDataRep->numVars;
  int num_coeffs = 1; // zeroth-order Taylor series
  if (bdo & 2)        //  first-order Taylor series
    num_coeffs += num_v;
  if (bdo & 4)        // second-order Taylor series
    num_coeffs += num_v*(num_v + 1)/2;
  return num_coeffs;
}


void TaylorApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // No computations needed.  Just do sanity checking on approxData.

  // Check number of data points
  if (!approxData.anchor() || approxData.points()) {
    Cerr << "Error: wrong number of data points in TaylorApproximation::"
	 << "build()." << std::endl;
    abort_handler(-1);
  }

  // Check gradient
  short  bdo   = sharedDataRep->buildDataOrder;
  size_t num_v = sharedDataRep->numVars;
  if ( (bdo & 2) && approxData.anchor_gradient().length() != num_v) {
    Cerr << "Error: gradient vector required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(-1);
  }

  // Check Hessian
  if ( (bdo & 4) && approxData.anchor_hessian().numRows() != num_v) {
    Cerr << "Error: Hessian matrix required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(-1);
  }
}


Real TaylorApproximation::value(const Variables& vars)
{
  short bdo = sharedDataRep->buildDataOrder;
  if (bdo == 1)
    return approxData.anchor_function();
  else { // build up approx value from constant and derivative terms
    Real approx_val = (bdo & 1) ? approxData.anchor_function() : 0.;
    if (bdo & 6) {
      const RealVector&       x = vars.continuous_variables();
      const RealVector&      x0 = approxData.anchor_continuous_variables();
      const RealVector&    grad = approxData.anchor_gradient();
      const RealSymMatrix& hess = approxData.anchor_hessian();
      size_t num_v = sharedDataRep->numVars;
      for (size_t i=0; i<num_v; i++) {
	Real dist_i = x[i] - x0[i];
	if (bdo & 2) // include gradient terms
	  approx_val += grad[i] * dist_i;
	if (bdo & 4) // include Hessian terms
	  for (size_t j=0; j<num_v; j++)
	    approx_val += dist_i * hess(i,j) * (x[j] - x0[j])/2.;
      }
    }
    return approx_val;
  }
}


const RealVector& TaylorApproximation::gradient(const Variables& vars)
{
  short bdo = sharedDataRep->buildDataOrder;
  if (bdo == 2)
    return approxData.anchor_gradient();
  else { // build up approxGradient from derivative terms
    if (bdo & 2) // include gradient terms
      approxGradient = approxData.anchor_gradient();
    else {       // initialize approxGradient to zero
      size_t num_v = sharedDataRep->numVars;
      if (approxGradient.length() != num_v)
	approxGradient.size(num_v);
      else
	approxGradient = 0.;
    }
    if (bdo & 4) { // include Hessian terms
      const RealVector&       x = vars.continuous_variables();
      const RealVector&      x0 = approxData.anchor_continuous_variables();
      const RealSymMatrix& hess = approxData.anchor_hessian();
      size_t num_v = sharedDataRep->numVars;
      for (size_t i=0; i<num_v; i++)
	for (size_t j=0; j<num_v; j++)
	  approxGradient[i] += hess(i,j) * (x[j] - x0[j]);
    }
    return approxGradient;
  }
}


const RealSymMatrix& TaylorApproximation::hessian(const Variables& vars)
{
  short bdo = sharedDataRep->buildDataOrder;
  if (bdo & 4)
    return approxData.anchor_hessian();
  else { // initialize approxHessian to zero
    size_t num_v = sharedDataRep->numVars;
    if (approxHessian.numRows() != num_v) approxHessian.shape(num_v);
    else                                  approxHessian = 0.;
    return approxHessian;
  }
}

} // namespace Dakota
