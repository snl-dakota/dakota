/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TANA3Approximation
//- Description:  Implementation of TANA-3 two-point exponential approximation
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories
 
#include "TANA3Approximation.H"
#include "ProblemDescDB.H"


namespace Dakota {

TANA3Approximation::
TANA3Approximation(ProblemDescDB& problem_db, size_t num_vars):
  Approximation(BaseConstructor(), problem_db, num_vars)
{
  dataOrder = 3;
  pExp.sizeUninitialized(numVars);
  minX.sizeUninitialized(numVars);

  // retrieve actual_model_pointer specification and set the DB
  const String& actual_model_ptr
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(actual_model_ptr);
  // sanity checks on actual model data
  if (problem_db.get_string("responses.gradient_type") == "none") {
    Cerr << "Error: response gradients required in TANA3Approximation."
	 << std::endl;
    abort_handler(-1);
  }
  if (problem_db.get_string("responses.hessian_type") != "none")
    Cerr << "Warning: response Hessians not used within TANA3Approximation."
	 << std::endl;
  // restore the specification
  problem_db.set_db_model_nodes(model_index);
}


int TANA3Approximation::min_coefficients() const
{
  // TANA-3 requires 2 expansion points, each with value and gradient data.
  // However, this class requires a minimum of 1 expansion point, for which a
  // first-order Taylor series is employed as an interim approximation.
  return numVars + 1;
}


int TANA3Approximation::num_constraints() const
{
  // For the minimal first-order Taylor series interim approximation, return
  // the number of constraints within the anchorPoint.
  return (anchorPoint.is_null()) ? 0 : numVars + 1;
}


void TANA3Approximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  // Perform sanity checking to verify two points with gradients
  // (Hessians ignored)

  // *****************************************************************
  // Note: could implement history mechanism within this class (which
  // automatically pushed the previous anchor to the prev_pt).  This
  // would still work for an "instantaneous" approximation, so long as
  // you pushed two anchors at the class.
  // *****************************************************************

  // Check number of data points: require anchor; currentPoints are optional
  if (anchorPoint.is_null()) {
    Cerr << "Error: wrong number of data points in TANA3Approximation::build."
	 << std::endl;
    abort_handler(-1);
  }
  if (!(dataOrder & 1)) {
    Cerr << "Error: response values required in TANA3Approximation::build."
	 << std::endl;
    abort_handler(-1);
  }
  if (!(dataOrder & 2) || anchorPoint.response_gradient().length() != numVars) {
    Cerr << "Error: response gradients required in TANA3Approximation::build."
	 << std::endl;
    abort_handler(-1);
  }

  if (currentPoints.empty()) {
    // Insufficient data for two-point approximation:
    // use 1st-order Taylor series as interim approach.
    // In this case, no additional computations are needed.
  }
  else { // two-point approximation
    // Check gradients
    const Pecos::SurrogateDataPoint& prev_pt = *currentPoints.begin();
    if (prev_pt.response_gradient().length() != numVars) {
      Cerr << "Error: gradients required in TANA3Approximation::build."
	   << std::endl;
      abort_handler(-1);
    }

    // alternate constructor sets numVars after construction
    if (pExp.empty())
      pExp.sizeUninitialized(numVars);
    if (minX.empty())
      minX.sizeUninitialized(numVars);

    // Calculate TANA3 terms
    const RealVector& x1 = prev_pt.continuous_variables();
    const RealVector& x2 = anchorPoint.continuous_variables();
    for (size_t i=0; i<numVars; i++)
      minX[i] = std::min(x1[i], x2[i]);
    find_scaled_coefficients();
  }
}


void TANA3Approximation::find_scaled_coefficients()
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

  const Pecos::SurrogateDataPoint& prev_pt = *currentPoints.begin();
  const RealVector& x1    = prev_pt.continuous_variables();
  const Real&       f1    = prev_pt.response_function();
  const RealVector& grad1 = prev_pt.response_gradient();
  const RealVector& x2    = anchorPoint.continuous_variables();
  const Real&       f2    = anchorPoint.response_function();
  const RealVector& grad2 = anchorPoint.response_gradient();

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
  RealVector s_eval(numVars), sgrad1(numVars), sgrad2(numVars);
  for (i=0; i<numVars; i++) {
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
  for (i=0; i<numVars; i++) {
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
  for (i=0; i<numVars; i++) {
    Real s2i = scX2[i], pi = pExp[i];
    H -= grad2[i]*std::pow(s2i,1.-pi)/pi*(std::pow(scX1[i],pi)-std::pow(s2i,pi));
  }
  H *= 2.;

#ifdef DEBUG
  Cout << "\n\nTANA inputs X1: " << x1 << "\nX2: " << x2 << "\nF(X1): " << f1
       << " F(X2): " << f2 << "\n\ndF/dX(X1): " << grad1 << "\ndF/dX(X2): "
       << grad2 << "\nScaled TANA inputs S1: " << scX1 << "\nS2: " << scX2
     //<< "\ndF/dS(S1): " << sgrad1 << "\ndF/dS(S2): " << sgrad2
       << "\nTANA outputs p: " << pExp << "\nH: " << H << '\n';
#endif // DEBUG
}


void TANA3Approximation::offset(const RealVector& x, RealVector& s)
{
  copy_data(x, s);
  for (size_t i=0; i<numVars; i++) {
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


const Real& TANA3Approximation::get_value(const RealVector& x)
{
  if (currentPoints.empty()) { // First-order Taylor series (interim approx)
    approxValue                  = anchorPoint.response_function();
    const RealVector& c_vars = anchorPoint.continuous_variables();
    const RealVector& grad   = anchorPoint.response_gradient();
    for (size_t i=0; i<numVars; i++) {
      Real dist_i = x[i] - c_vars[i];
      approxValue += grad[i] * dist_i;
    }
  }
  else { // TANA-3 approximation

    // Check existing scaling to verify that it is sufficient for x
    size_t i;
    bool rescale_flag = false;
    RealVector s_eval;
    offset(x, s_eval);
    for (i=0; i<numVars; i++)
      if (x[i] < minX[i] && s_eval[i] < 0.) { // *** change from old NonDRel ***
	minX[i] = x[i];
	rescale_flag = true;
      }
    if (rescale_flag) {
      find_scaled_coefficients();
      offset(x, s_eval);
    }

    // Calculate approxValue
    const Real&           f2    = anchorPoint.response_function();
    const RealVector& grad2 = anchorPoint.response_gradient();
    Real sum1 = 0., sum_diff1_sq = 0., sum_diff2_sq = 0.;
    for (i=0; i<numVars; i++) {
      Real pi = pExp[i], sp = std::pow(s_eval[i],pi), s2i = scX2[i],
	diff1 = sp - std::pow(scX1[i],pi), diff2 = sp - std::pow(s2i,pi);
      sum1 += grad2[i]*std::pow(s2i,1.-pi)/pi*diff2;
      sum_diff1_sq += diff1*diff1;
      sum_diff2_sq += diff2*diff2;
    }
    Real epsilon = H/(sum_diff1_sq + sum_diff2_sq);
    approxValue  = f2 + sum1 + sum_diff2_sq*epsilon/2.;
#ifdef DEBUG
    Cout << "epsilon: " << epsilon << " sum1: " << sum1 << " approxValue: "
	 << approxValue << '\n';
#endif // DEBUG
  }

  return approxValue;
}


const RealVector& TANA3Approximation::get_gradient(const RealVector& x)
{
  if (currentPoints.empty()) // First-order Taylor series (interim approx)
    approxGradient = anchorPoint.response_gradient();
  else { // TANA-3 approximation

    // Check existing scaling to verify that it is sufficient for x
    size_t i;
    bool rescale_flag = false;
    RealVector s_eval;
    offset(x, s_eval);
    for (i=0; i<numVars; i++)
      if (x[i] < minX[i] && s_eval[i] < 0.) {
	minX[i] = x[i];
	rescale_flag = true;
      }
    if (rescale_flag) {
      find_scaled_coefficients();
      offset(x, s_eval);
    }

    // Calculate approxGradient
    const RealVector& grad2 = anchorPoint.response_gradient();
    Real sum_diff1_sq = 0., sum_diff2_sq = 0.;
    for (i=0; i<numVars; i++) {
      Real pi = pExp[i], sp = std::pow(s_eval[i],pi),
	diff1 = sp - std::pow(scX1[i],pi), diff2 = sp - std::pow(scX2[i],pi);
      sum_diff1_sq += diff1*diff1;
      sum_diff2_sq += diff2*diff2;
    }
    for (i=0; i<numVars; i++) {
      Real svi = s_eval[i], s2i = scX2[i], pi = pExp[i], sp = std::pow(svi,pi),
	diff1 = sp - std::pow(scX1[i],pi), diff2 = sp - std::pow(s2i,pi);
      Real E= H*pi*std::pow(svi,pi-1.)*(diff2*sum_diff1_sq - diff1*sum_diff2_sq)
	/ std::pow(sum_diff1_sq + sum_diff2_sq,2.);
      // df/dx = df/ds * ds/dx = df/ds for simple offsets
      approxGradient[i] = std::pow(svi/s2i,pi-1.)*grad2[i] + E;
#ifdef DEBUG
      Cout << "E: " << E << " approxGradient[" << i << "]: "
	   << approxGradient[i] << '\n';
#endif // DEBUG
    }
  }

  return approxGradient;
}


//const RealMatrix& TANA3Approximation::get_hessian(const RealVector& x)
//{ return approxHessian; }

} // namespace Dakota
