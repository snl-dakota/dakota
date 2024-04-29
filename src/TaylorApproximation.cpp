/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  // No computations needed.  Only sanity checking on active approxData.

  // Check number of data points
  if (!approxData.anchor() || approxData.points() != 1) {
    Cerr << "Error: wrong number of data points in TaylorApproximation::"
	 << "build()." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  // Check gradient
  short  bdo   = sharedDataRep->buildDataOrder;
  size_t num_v = sharedDataRep->numVars;
  const Pecos::SurrogateDataResp& anchor_sdr = approxData.anchor_response();
  if ( (bdo & 2) && anchor_sdr.response_gradient().length() != num_v) {
    Cerr << "Error: gradient vector required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  // Check Hessian
  if ( (bdo & 4) && anchor_sdr.response_hessian().numRows() != num_v) {
    Cerr << "Error: Hessian matrix required in TaylorApproximation::build()."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


Real TaylorApproximation::value(const Variables& vars)
{
  short bdo = sharedDataRep->buildDataOrder;
  if (bdo == 1)
    return approxData.anchor_response().response_function();
  else { // build up approx value from constant and derivative terms
    const Pecos::SurrogateDataResp& anchor_sdr = approxData.anchor_response();
    Real approx_val = (bdo & 1) ? anchor_sdr.response_function() : 0.;
    if (bdo & 6) {
      const RealVector& x = vars.continuous_variables();
      const RealVector& x0
	= approxData.anchor_variables().continuous_variables();
      const RealVector&    grad = anchor_sdr.response_gradient();
      const RealSymMatrix& hess = anchor_sdr.response_hessian();
      size_t i, num_v = sharedDataRep->numVars;
      for (i=0; i<num_v; i++) {
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
    return approxData.anchor_response().response_gradient();
  else { // build up approxGradient from derivative terms
    const Pecos::SurrogateDataResp& anchor_sdr = approxData.anchor_response();
    if (bdo & 2) // include gradient terms
      copy_data(anchor_sdr.response_gradient(), approxGradient); // can be view
    else {       // initialize approxGradient to zero
      size_t num_v = sharedDataRep->numVars;
      if (approxGradient.length() != num_v)
	approxGradient.size(num_v);
      else
	approxGradient = 0.;
    }
    if (bdo & 4) { // include Hessian terms
      const RealVector& x = vars.continuous_variables();
      const RealVector& x0
	= approxData.anchor_variables().continuous_variables();
      const RealSymMatrix& hess = anchor_sdr.response_hessian();
      size_t i, j, num_v = sharedDataRep->numVars;
      for (i=0; i<num_v; i++)
	for (j=0; j<num_v; j++)
	  approxGradient[i] += hess(i,j) * (x[j] - x0[j]);
    }
    return approxGradient;
  }
}


const RealSymMatrix& TaylorApproximation::hessian(const Variables& vars)
{
  short bdo = sharedDataRep->buildDataOrder;
  if (bdo & 4)
    return approxData.anchor_response().response_hessian();
  else { // initialize approxHessian to zero
    size_t num_v = sharedDataRep->numVars;
    if (approxHessian.numRows() != num_v) approxHessian.shape(num_v);
    else                                  approxHessian = 0.;
    return approxHessian;
  }
}

} // namespace Dakota
