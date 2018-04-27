/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        QMEApproximation
//- Description:  Implementation of QMEA Quadratic Multipoint Exponential Approximation
//-               
//- Owner:        Robert A. Canfield, Virginia Tech
 
#include "QMEApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include <math.h>

#define DEBUG

namespace Dakota {

QMEApproximation::
QMEApproximation(ProblemDescDB& problem_db,
		   const SharedApproxData& shared_data,
                   const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
{
  // sanity checks
  if (sharedDataRep->buildDataOrder != 3) {
    Cerr << "Error: response values and gradients required in "
	 << "QMEApproximation." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  pExp.sizeUninitialized(sharedDataRep->numVars);
  minX.sizeUninitialized(sharedDataRep->numVars);
}


int QMEApproximation::min_coefficients() const
{
  // QMEA requires at least two expansion points, each with a value,  
  // and at least two with gradient data.
  // However, this class requires a minimum of one expansion point, for which a
  // first-order Taylor series is employed as an interim approximation.
  return sharedDataRep->numVars + 1;
}


/*
int QMEApproximation::num_constraints() const
{
  // For the minimal first-order Taylor series interim approximation, return
  // the number of constraints within approxData's anchor point.
  return (approxData.anchor()) ? sharedDataRep->numVars + 1 : 0;
}
*/


void QMEApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // New data is appended via push_back(), so leading data (index 0) is older
  // (previous iterate) and trailing data (index 1) is newer (current iterate)

  // Sanity checking:
  size_t num_pts = approxData.points(), num_v = sharedDataRep->numVars;
  if (num_pts < 1) {
    Cerr << "Error: insufficient data (" << num_pts << " points) in "
	 << "QMEApproximation::build." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  // gradients required for expansion point:
  if (!approxData.anchor() ||
      approxData.anchor_response().response_gradient().length() != num_v) {
    Cerr << "Error: gradients required for expansion point in "
	 << "QMEApproximation::build()." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  if (num_pts >= 2) { // QMEA
//  const size_t k=num_pts-1, p=num_pts-2; // indices to current and previous points
    const size_t k=1,         p=0;         // indices to current and previous points

    // alternate constructor sets numVars after construction
    if (pExp.empty()) pExp.sizeUninitialized(num_v);
    if (minX.empty()) minX.sizeUninitialized(num_v);

    // Calculate TANA3 terms
    const Pecos::SDVArray& sdv_array = approxData.variables_data();
    const RealVector& x1 = sdv_array[p].continuous_variables();
    const RealVector& x2 = sdv_array[k].continuous_variables();
    for (size_t i=0; i<num_v; i++)
      minX[i] = std::min(x1[i], x2[i]);
    find_scaled_coefficients();
  }
  else {
    // Insufficient data accumulated for multipoint approximation.
    // Fall back to 1st-order Taylor series as interim approach,
    // for which no additional computations are needed.

    if (approxData.num_gradient_variables() != num_v) {
      Cerr << "Error: response gradients required in QMEApproximation::build."
	   << std::endl;
      abort_handler(APPROX_ERROR);
    }
  }
}


void QMEApproximation::find_scaled_coefficients()
{
  // Note: x notation follows TANA references and is generic
  // (it does not imply x-space in reliability analysis).
  //
  //                                      Original     Scaled
  //                                      --------     ------
  //  Evaluation point                    x_eval       s_eval
  //  Last expansion point                x1           scX1
  //    Function value at x1              f1           N/A
  //    Function gradient at x1           grad1        N/A (for offset)
  //  Current expansion point             x2           scX2
  //    Function value at x2              f2           N/A
  //    Function gradient at x2           grad2        N/A (for offset)
  //  Lower variable bounds for scaling   l_bnds
  //  Upper variable bounds for scaling   u_bnds 

  size_t num_pts = approxData.points(), num_v = sharedDataRep->numVars;
//const size_t k=num_pts-1, p=num_pts-2; // indices to current and previous points
  const size_t k=1, p=0;                 // indices to current and previous points
  Cout << "QMEA num_pts=" << num_pts << ", k=" << k << ", p=" << p << '\n';

  const Pecos::SDVArray& sdv_array = approxData.variables_data();
  const Pecos::SDRArray& sdr_array = approxData.response_data();

  const RealVector& x1    = sdv_array[p].continuous_variables();
  Real              f1    = sdr_array[p].response_function();
  const RealVector& grad1 = sdr_array[p].response_gradient();

  const RealVector& x2    = sdv_array[k].continuous_variables();
  Real              f2    = sdr_array[k].response_function();
  const RealVector& grad2 = sdr_array[k].response_gradient();

  if ( num_pts > 2 ) {
  const RealVector& x0    = sdv_array[2].continuous_variables();
  Real              f0    = sdr_array[2].response_function();
  const RealVector& grad0 = sdr_array[2].response_gradient();
  Cout << "\n\nQMEA inputs X0:\n" << x0 << "\nF(X0): " << f0
       << "\n\ndF/dX(X0):\n" << grad0 << '\n';
  }

#ifdef DEBUG
  Cout << "\n\nQMEA inputs X1:\n" << x1 << "\nX2:\n" << x2 << "\nF(X1): " << f1
       << " F(X2): " << f2 << "\n\ndF/dX(X1):\n" << grad1 << "\ndF/dX(X2):\n"
       << grad2 << '\n';
#endif // DEBUG

  // Numerical safeguarding must be performed since x^p is problematic for
  // x < 0 and nonintegral p.  Three approaches for this have been explored:
  // 1. scale x using a standard bounds scaling: [l,u] -> [1,2]
  //    advantage: standard scaling approach, exact p[i] (for scaled problem)
  //      can be used.
  //    disadvantage: exponents can grow to O(10^2) since scaled x's are small,
  //      safeguarding based on bounds may be applied when not strictly needed
  //      for x_eval[i], x1[i], x2[i].
  // 2. offset x using the current minimum of x_eval[i], x1[i], and x2[i].
  //    advantage: safeguarding only applied when needed, exact p[i]
  //      (for offset problem) can be used.
  //    disadvantage: exponents observed to grow moderately (e.g., 5 to 7).
  // 3. for x < 0, promote p[i] to integral value.
  //    advantage: p[i]'s for original problem tend to be small.
  //    disadvantage: approximated p[i]'s do not reproduce values/gradients,
  //      small p[i]'s -> effect of integer promotion can be severe.

  /*
  // *** Safeguarding approach 1: ***
  // scale x_eval, x1, x2 to lie within interval of [1,2] in order to avoid 
  // numerical difficulties for x <= 0.
  // s = (x-l)/(u-l) + 1  -->  x = l + (s-1)(u-l)  -->  dx/ds = u-l
  // This approach has been observed to result in large p[i]'s.
  RealVector s_eval(num_v), sgrad1(num_v), sgrad2(num_v);
  for (i=0; i<num_v; i++) {
    Real lb = l_bnds[i], bdiff = u_bnds[i] - lb;
    s_eval[i] = (x_eval[i] - lb)/bdiff + 1.;
    scX1[i]   = (x1[i]     - lb)/bdiff + 1.;
    scX2[i]   = (x2[i]     - lb)/bdiff + 1.;
    sgrad1[i] = grad1[i]*bdiff; // df/ds = df/dx * dx/ds
    sgrad2[i] = grad2[i]*bdiff; // df/ds = df/dx * dx/ds
  }
  */

  // *** Safeguarding approach 2: ***
  // offset x values to avoid numerical difficulties for x <= 0.
  // This approach avoids scaling when not needed.
  offset(x1, scX1);
  offset(x2, scX2);

  // Calculate p exponents
  size_t i;
  Real p_max = 10.; // TANA papers use either 5 or application-specific logic
  for (i=0; i<num_v; i++) {
    // A number of numerical problems are possible:
    // if grad2[i]   -> 0 or s2[i] -> 0, grad_ratio or pt_ratio -> Infinity
    // if grad_ratio <= 0, log(grad_ratio) -> -Inf (if 0), FPE (if negative)
    // if pt_ratio   <= 0, log(pt_ratio)   -> -Inf (if 0), FPE (if negative)
    // if pt_ratio   -> 1 (convergence), log(pt_ratio) -> 0 & p_val -> Infinity
    Real g2i = grad2[i], s2i = scX2[i];
    Real grad_ratio = (std::fabs(g2i) > DBL_MIN) ? grad1[i] / g2i : -1.;
    Real pt_ratio   = (std::fabs(s2i) > DBL_MIN) ?  scX1[i] / s2i : -1.;
    if ( grad_ratio < DBL_MIN || pt_ratio < DBL_MIN ||
	 std::fabs(std::log(pt_ratio)) < DBL_MIN ) {
      // Both linear (p_val = 1) and reciprocal (p_val = -1) will exactly
      // reproduce f(s1) = f1, but neither reliably reproduce grad(s1) = grad1.
      // Select the one with lower gradient error at s1.
      Real err_minus1 = std::fabs(std::pow(s2i/scX1[i],2)*g2i - grad1[i]),
           err_plus1  = std::fabs(g2i - grad1[i]);
      pExp[i] = (err_minus1 < err_plus1) ? -1. : 1.;
    }
    else {
      Real p_val = 1. + std::log(grad_ratio)/std::log(pt_ratio);
      /*
      // *** Safeguarding approach 3: ***
      minX[i] = std::min( std::min(x1[i], x2i), x_eval[i]);
      if (minX[i] < 0.) { // promote p to an integral value for negative x's
	Cout << "Promoting p_val from " << p_val;
        // round to nearest integer except 0
	if (p_val >= 0.)
	  p_val = (p_val >  1.) ? floor(p_val+.5) :  1.;
	else
	  p_val = (p_val < -1.) ?  ceil(p_val-.5) : -1.;
	Cout << " to " << p_val << '\n';
      }
      */

      if (p_val > p_max)
        pExp[i] =  p_max;
      else if (p_val < -p_max)
        pExp[i] = -p_max;
      else
	pExp[i] =  p_val;
    }
  }

  // Calculate H
  H = f1 - f2;
  for (i=0; i<num_v; i++) {
    Real s2i = scX2[i], p_i = pExp[i];
    H -= grad2[i] * std::pow(s2i,1.-p_i) / p_i *
      (std::pow(scX1[i],p_i) - std::pow(s2i,p_i));
  }
  H *= 2.;

#ifdef DEBUG
  Cout << "\nScaled QMEA inputs S1:\n" << scX1 << "\nS2:\n" << scX2
     //<< "\ndF/dS(S1): " << sgrad1 << "\ndF/dS(S2): " << sgrad2
       << "\nQMEA outputs p:\n" << pExp << "\nH: " << H << '\n';
#endif // DEBUG

// Find previous points to build QMEAM surrogate (RAC)
  size_t n;
  RealVector y(num_v);
  for (i=0; i<num_pts; ++i) {
    const Pecos::SurrogateDataVars& sdv_i = sdv_array[i];
    const RealVector& c_vars = sdv_i.continuous_variables();
    Cout << "QMEA point = " << i << '\n';
    for (n=0; n<num_v; ++n ) {
      y[n] = pow( c_vars[n], pExp[n] );
      Cout << "     dv(" << n+1 << ")=" << c_vars[n] << '\n'; 
      Cout << "      y(" << n+1 << ")=" << y[n]      << '\n';
    }

    const Pecos::SurrogateDataResp& sdr_i = sdr_array[i];
    Real fn_val = sdr_i.response_function();
	 Cout << "QMEA fn_val=" << fn_val << '\n';
  }
}


void QMEApproximation::offset(const RealVector& x, RealVector& s)
{
  copy_data(x, s);
  size_t i, num_v = sharedDataRep->numVars;
  for (i=0; i<num_v; i++) {
    // Offset based on lower bound:
    //Real lb = l_bnds[i];
    //Real offset = (lb <= 0.) ? 1. - lb : 0.;

    // Offset based on current min for each variable:
    //minX[i] = std::min( std::min(x1[i], x2[i]), x_eval[i]);
    // offset of -2*minX is intended to preserve the magnitude of x (results
    // in a simple sign flip for the smallest x value).  A minX of 0 causes
    // problems with negative exponents and a minX less than 0 causes problems
    // with nonintegral exponents.
    if (std::fabs(minX[i]) < 1.e-10) // use different logic near zero
      s[i] += 0.1;
    else if (minX[i] < 0.)      // normal logic to preserve magnitude of x
      s[i] -= 2.*minX[i];
  }
}


Real QMEApproximation::value(const Variables& vars)
{
  Real approx_val;
  const RealVector& x = vars.continuous_variables();
  size_t i, num_v = sharedDataRep->numVars, num_pts = approxData.points();

  if (num_pts == 1) { // First-order Taylor series (interim approx)
    const Pecos::SurrogateDataVars& center_sdv = approxData.variables_data()[0];
    const Pecos::SurrogateDataResp& center_sdr = approxData.response_data()[0];
    const RealVector&    center_x = center_sdv.continuous_variables();
    approx_val                    = center_sdr.response_function();
    const RealVector& center_grad = center_sdr.response_gradient();
    for (i=0; i<num_v; i++) {
      Real dist_i = x[i] - center_x[i];
      approx_val += center_grad[i] * dist_i;
    }
  }
  else {// QMEA 

    // Check existing scaling to verify that it is sufficient for x
    RealVector s_eval;
    offset(x, s_eval);
    bool rescale_flag = false;
    for (i=0; i<num_v; i++)
      if (x[i] < minX[i] && s_eval[i] < 0.) { // *** change from old NonDRel
	minX[i] = x[i];
	rescale_flag = true;
      }
    if (rescale_flag) {
      find_scaled_coefficients();
      offset(x, s_eval);
    }

    // Calculate approx_val
    const Pecos::SurrogateDataResp& curr_sdr = approxData.response_data()[1];
    Real f2 = curr_sdr.response_function(), sum1 = 0.,
      sum_diff1_sq = 0., sum_diff2_sq = 0.;
    const RealVector& grad2 = curr_sdr.response_gradient();
    for (i=0; i<num_v; i++) {
      Real p_i = pExp[i], sp = std::pow(s_eval[i],p_i), s2i = scX2[i],
	diff1 = sp - std::pow(scX1[i],p_i), diff2 = sp - std::pow(s2i,p_i);
      sum1 += grad2[i]*std::pow(s2i,1.-p_i)/p_i*diff2;
      sum_diff1_sq += diff1 * diff1;
      sum_diff2_sq += diff2 * diff2;
    }
    Real epsilon = H/(sum_diff1_sq + sum_diff2_sq);
    approx_val  = f2 + sum1 + sum_diff2_sq*epsilon/2.;
#ifdef DEBUG
    Cout << "epsilon: " << epsilon << " sum1: " << sum1 << " approx value: "
	 << approx_val << '\n';
#endif // DEBUG
  }

  return approx_val;
}


const RealVector& QMEApproximation::gradient(const Variables& vars)
{
  size_t num_pts = approxData.points();

  if (num_pts == 1) { // First-order Taylor series (interim approx)
    const Pecos::SurrogateDataResp& center_sdr = approxData.response_data()[0];
    return center_sdr.response_gradient(); // can be view of DB response

    //copy_data(center_sdr.response_gradient(), approxGradient);// can be view
    //return approxGradient;
  }
  else { // QMEA 

    // Check existing scaling to verify that it is sufficient for x
    const RealVector& x = vars.continuous_variables();
    RealVector s_eval;
    offset(x, s_eval);
    size_t i, num_v = sharedDataRep->numVars; bool rescale_flag = false;
    for (i=0; i<num_v; i++)
      if (x[i] < minX[i] && s_eval[i] < 0.) {
	minX[i] = x[i];
	rescale_flag = true;
      }
    if (rescale_flag) {
      find_scaled_coefficients();
      offset(x, s_eval);
    }

    // Calculate approxGradient
    const Pecos::SurrogateDataResp& curr_sdr = approxData.response_data()[1];
    const RealVector& grad2 = curr_sdr.response_gradient();
    Real sum_diff1_sq = 0., sum_diff2_sq = 0.;
    for (i=0; i<num_v; i++) {
      Real p_i = pExp[i], sp = std::pow(s_eval[i],p_i),
	diff1 = sp - std::pow(scX1[i],p_i), diff2 = sp - std::pow(scX2[i],p_i);
      sum_diff1_sq += diff1*diff1;
      sum_diff2_sq += diff2*diff2;
    }
    if (approxGradient.length() != num_v)
      approxGradient.sizeUninitialized(num_v);
    for (i=0; i<num_v; i++) {
      Real svi = s_eval[i], s2i = scX2[i], p_i = pExp[i],
	sp = std::pow(svi,p_i),	diff1 = sp - std::pow(scX1[i],p_i),
	diff2 = sp - std::pow(s2i,p_i), E = H*p_i*std::pow(svi,p_i-1.)
	  * (diff2*sum_diff1_sq - diff1*sum_diff2_sq)
	  / std::pow(sum_diff1_sq + sum_diff2_sq,2.);
      // df/dx = df/ds * ds/dx = df/ds for simple offsets
      approxGradient[i] = std::pow(svi/s2i,p_i-1.)*grad2[i] + E;
#ifdef DEBUG
      Cout << "E: " << E << " approxGradient[" << i << "]: "
	   << approxGradient[i] << '\n';
#endif // DEBUG
    }
    return approxGradient;
  }
}


//const RealMatrix& QMEApproximation::hessian(const Variables& vars)
//{ return approxHessian; }

} // namespace Dakota
