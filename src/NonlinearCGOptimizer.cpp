/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonlinearCGOptimizer
//- Description: Implementation code for the NonlinearCGOptimizer class
//- Owner:       Brian Adams
//- Checked by:

#include "NonlinearCGOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include <boost/math/tools/minima.hpp>

// uncomment to use the Boost Brent's algorithm
//#define BOOST_BRENT 1

using namespace std;

namespace Dakota {

// wrapper around linesearch evaluator needed for use in Brent's algorithm
class boost_ls_eval {

public:

  // constructor
  boost_ls_eval(NonlinearCGOptimizer& pimpl_): pImpl(pimpl_)
  { /* empty constructor */ }

  // evaluator
  Real operator()(const Real& trial_step)
  { return ( pImpl.linesearch_eval(trial_step, 1) ); }

private:

  // pointer to class instance so we have a context when performing evaluations
  NonlinearCGOptimizer& pImpl;

};


NonlinearCGOptimizer::
NonlinearCGOptimizer(ProblemDescDB& problem_db, Model& model): 
  Optimizer(problem_db, model), initialStep(0.01), linesearchTolerance(1.0e-2),
  linesearchType(CG_LS_SIMPLE), maxLinesearchIters(10), relFunctionTol(0.0),
  relGradientTol(0.0), resetStep(true), restartIter(1000000),
  updateType(CG_FLETCHER_REEVES)
{
  if (numFunctions > 1 || numConstraints > 0 || boundConstraintFlag) {
    Cerr << "ERROR: NonlinearCG only supports unconstrainted single objective "
	 << "problems!" << endl;
    abort_handler(-1);
  }
  // some of the defaults may be overridden by user-supplied options
  parse_options();

  stepLength = initialStep;
}


NonlinearCGOptimizer::~NonlinearCGOptimizer()
{ }


void NonlinearCGOptimizer::core_run()
{

  // TODO: Once DAKOTA moves to Teuchos for its numerical data type,
  // remove the various copies below.  (Also, could use std::copy for some.)
  copy_data(iteratedModel.continuous_variables(), designVars); // view->copy
  searchDirection.sizeUninitialized(numContinuousVars);

  if (linesearchType > CG_FIXED_STEP)
    trialVars.sizeUninitialized(numContinuousVars);

  for (iterCurr = 0; iterCurr < maxIterations; iterCurr++) {

    // get function and gradient -- should discern whether linesearch
    // is in use and only request gradient if appropriate
    activeSet.request_values(3);
    iteratedModel.evaluate(activeSet);
    const Response&   response  = iteratedModel.current_response();
    const RealVector& functions = response.function_values();

    functionCurr = functions[0];
    gradCurr = response.function_gradient_view(0);

    // always store ||g||^2
    gradDotGrad_curr = gradCurr.dot(gradCurr);
    if (iterCurr == 0)
      gradDotGrad_init = gradDotGrad_curr;

    // TODO: accumulate stats
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "INFO (NonlinearCG): iteration statistics\n"; 
      Cout << "iter J                norm(g)          steplen\n";
      Cout << "---- ---------------- ---------------- ----------------\n";
      Cout << setw(4) << iterCurr << " " << functionCurr << " " 
	   << sqrt(gradDotGrad_curr) << " " << stepLength << "\n" << endl;
    }
    // check convergence: two-norm of gradient
    if (sqrt(gradDotGrad_curr) < convergenceTol) {
      Cout << "INFO (NonlinearCG): hard convergence reached (gradient norm "
	   << "within tolerance)." << endl;
      break;
    }

    if ( iterCurr > 0)
      // could do as CTK and check this in conjunction with test above...
      if (sqrt(gradDotGrad_curr) < relGradientTol*gradDotGrad_init) {
	Cout << "INFO (NonlinearCG): convergence reached (gradient reduction "
	     << "within tolerance)." << endl;
	break;
      }
    if ( std::fabs(functionCurr - functionPrev) / 
	 std::max(1.0,std::fabs(functionPrev)) <
	   relFunctionTol ) {
	Cout << "INFO (NonlinearCG): convergence reached (function change "
	     << "within tolerance)." << endl;
	break;
      }

    // will set searchDirection to steepest descent, or CG direction
    // TODO: recourse to steepest descent
    compute_direction();
    if (sqrt(searchDirection.dot(searchDirection)) < 1e-16) {
      Cout << "INFO (NonlinearCG): degenerate search direction. Exiting."
	   << endl;
      break;
    }

    // will set stepLength if successful, return false if failed
    if (!compute_step()) {
      Cout << "INFO (NonlinearCG): failure computing step length. Exiting."
	   << endl;
      break;
    }

    // update design vars here, gradient evaluated on loop continuation
    // compute designVars <- stepLength * searchDirection + designVars
    designVars.AXPY(designVars.length(), stepLength, searchDirection.values(),
		    1, designVars.values(), 1);

    // TODO: this is a duplicate copy in the linesearch case -- rework to avoid
    iteratedModel.continuous_variables(designVars);
    
    // archive gradient and derived metric for next iteration
    functionPrev = functionCurr;
    gradPrev = response.function_gradient_copy(0);
    gradDotGrad_prev = gradDotGrad_curr;

  }

  if (iterCurr == maxIterations)
    Cout << "INFO (NonlinearCG): maxIterations = " << maxIterations 
	 << " reached. Exiting." << endl; 


  // return the optimal point in best*; should do DB lookup?
  bestVariablesArray.front().continuous_variables(designVars);
  if (!localObjectiveRecast) // else local_objective_recast_retrieve() used
    bestResponseArray.front().function_value(functionCurr, 0);
}


void NonlinearCGOptimizer::compute_direction()
{

  if (iterCurr == 0 || (iterCurr % restartIter == 0) || 
      updateType == CG_STEEPEST) {

    // choose steepest descent on first iteration or on reset
    if (outputLevel >= VERBOSE_OUTPUT && iterCurr > 0)
      Cout << "INFO (NonlinearCG): Iteration = " << iterCurr 
	   << ", resetting to steepest descent." << endl;
    searchDirection.putScalar(0.0);
    searchDirection -= gradCurr;

  }
  else {

    double beta = 0.0;
    if (updateType == CG_FLETCHER_REEVES)
      beta = gradDotGrad_curr/gradDotGrad_prev;	
    else {

      // Polak-Ribiere or Hestenes-Stiefel
      gradDiff = iteratedModel.current_response().function_gradient_copy(0);
      gradDiff -= gradPrev;
      Real gradCurr_dot_gradDiff = 
	gradCurr.dot(gradDiff);
      
      if (updateType == CG_POLAK_RIBIERE)
	beta = gradCurr_dot_gradDiff / gradDotGrad_prev;
      else if (updateType == CG_POLAK_RIBIERE_PLUS)
	// use max(B_PR, 0.0))
	beta = std::max(gradCurr_dot_gradDiff / gradDotGrad_prev, 0.0);
      else if (updateType == CG_HESTENES_STIEFEL)
	// use (gradCurr - gradPrev)'*searchDirection in denominator
	beta = gradCurr_dot_gradDiff / 
	  gradDiff.dot(searchDirection);
    }

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "DEBUG (NonlinearCG): beta = " << beta << endl;
    // searchDirection <-- beta*searchDirection - gradCurr
    searchDirection.scale(beta);
    searchDirection.AXPY(searchDirection.length(), -1.0,
			 gradCurr.values(), 1,
			 searchDirection.values(), 1);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "DEBUG (NonlinearCG): new search direction is:\n"
	 << searchDirection << endl;

}
  

// returns whether a valid step was found
bool NonlinearCGOptimizer::compute_step()
{

  // TODO: stepLength should be chosen based on descent direction and 
  // previous successful step
  if (resetStep)
    stepLength = initialStep;

  switch (linesearchType) {

  case CG_FIXED_STEP:
    // do nothing (leave initial stepLength )
    return(true);
    break;

  case CG_LS_SIMPLE: {

    // value-based line search with simple decrease
    bool decrease = false;
    unsigned bt_iter = 0;
    while (!decrease && bt_iter < maxLinesearchIters) {

      if (linesearch_eval(stepLength) < functionCurr) {
	if (outputLevel > NORMAL_OUTPUT)
	  Cout << "INFO (NonlinearCG_LS): Simple decrease achieved; step = " 
	       << stepLength << "." << endl;
	decrease = true;
      }
      else {
	if (outputLevel > NORMAL_OUTPUT) {
	  if (bt_iter == 0)
	    Cout << "INFO (NonlinearCG_LS): Initiating simple linesearch.\n";
	  Cout << "INFO (NonlinearCG_LS): Backtracking." << endl;
	}
	stepLength /= 2;
	bt_iter++;
      }

    }
    if (bt_iter == maxLinesearchIters && outputLevel > NORMAL_OUTPUT) {
      Cout << "INFO (NonlinearCG_LS): Could not find step yielding simple "
	   << "decrease." << endl;
    }
    return(decrease);

    break;
  }

  case CG_LS_BRENT: {

    if (outputLevel > NORMAL_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Initiating Brent linesearch." << endl;

    // first bracket the minimum, then use Brent's algorithm from Boost to 
    // hone in on the minimum
    Real xa, xb, xc, fa, fb, fc;
    xa = 0.0;
    fa = functionCurr;

    if (outputLevel > NORMAL_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Evaluating at initial step = " 
	   << stepLength << "." << endl;
    xb = stepLength;
    fb = linesearch_eval(xb);

    if (outputLevel > NORMAL_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Initiating bracketing procedure." << endl;
    bracket_min(xa, xb, xc, fa, fb, fc);
    if (outputLevel > NORMAL_OUTPUT) {
      Cout << "INFO (NonlinearCG_LS): Bracketing complete:\n";
      Cout << "Bracket:   [" << xa << ", " << xb << ", " << xc << "]" << endl;
      Cout << "Functions: [" << fa << ", " << fb << ", " << fc << "]" << endl;
      Cout << "INFO (NonlinearCG_LS): Initiating 1-D minimization." << endl;
    }

#ifdef BOOST_BRENT
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Using Boost Brent." << endl;
    int exponent;
    frexp(linesearchTolerance, &exponent);
    int bits = 1 - exponent;
    boost::uintmax_t max_it = maxLinesearchIters;

    pair<Real,Real> opt_len = 
      boost::math::tools::brent_find_minima<boost_ls_eval,Real>
      (boost_ls_eval(*this), xa, xc, bits, max_it);
    stepLength = opt_len.first;
#else
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Using native Brent." << endl;
    stepLength = brent_minimize(xa, xc, linesearchTolerance);
#endif

    if (outputLevel > NORMAL_OUTPUT)
      Cout << "INFO (NonlinearCG_LS): Linesearch complete; step = " 
	   << stepLength << "." << endl;
    
    return(true);
    
    break;

  }

  default:
    Cerr << "ERROR (NonlinearCG_LS): Requested linesearch type not available." 
	 << endl;
    abort_handler(-1);
    break;

  }

  return(false);

}


void NonlinearCGOptimizer::bracket_min(Real& xa, Real& xb, Real& xc,
				       Real& fa, Real& fb, Real& fc) 
{
  // Repeatedly evaluate the function along the search direction until
  // we know we've bracketed a minimum.

  
  const Real GOLDEN_RATIO = 1.618033988749895, SMALL_DIV = 1e-16,
    MAX_EXTRAP_FACTOR = 100.0;
  Real tmp, q, r, xm, xlim, fm = 0.0;

  // TODO: implement option to do simple backtracking to get a lower
  // function value in the current search direction; for now, just
  // swap the values of a and b if necessary
  // also, need to bound the search >= 0 when changing sign...
  if (fb > fa) {
    Cout << "swapping points, fa = " << fa << "fb = " << fb << "diff " 
	 << fb-fa << endl;
    tmp = xa;
    xa = xb;
    xb = tmp;
    tmp = fa;
    fa = fb;
    fb = tmp;
  }

  xc = xb + GOLDEN_RATIO*(xb-xa);
  fc = linesearch_eval(xc);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Bracket:   [" << xa << ", " << xb << ", " << xc << "]" << endl;
    Cout << "Functions: [" << fa << ", " << fb << ", " << fc << "]" << endl;
  }

  while (fb >= fc) {
    
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Bracket:   [" << xa << ", " << xb << ", " << xc << "]" << endl;
      Cout << "Functions: [" << fa << ", " << fb << ", " << fc << "]" << endl;
    }

    // find the extremum xm of a quadratic model interpolating a, b, c
    q = (fb-fa)*(xb-xc);
    r = (fb-fc)*(xb-xa);
    // avoid division by small (q-r) by bounding with signed minimum
    tmp = std::fabs(q-r);
    tmp = (tmp < SMALL_DIV) ? SMALL_DIV : tmp;
    tmp = (q-r < 0 ) ? -tmp : tmp;
    xm = xb - (q*(xb-xc) -r*(xb-xa))/2.0/tmp;
    // maximum point for which we trust the interpolation
    xlim = xb + MAX_EXTRAP_FACTOR * (xc-xb);

    // now detect which interval xm is in and act accordingly
    // [xb, xc]
    if ( (xb-xm)*(xm-xc) > 0.0 ) {
      
      fm = linesearch_eval(xm);
      if (fm < fc) { 
	// use points [b, xm, c]
	xa = xb;
	fa = fb;
        xb = xm;
	fb = fm;
	return;
      }
      else if (fm > fb) {
	// use points [a, b, xm]
	xc = xm;
	fc = fm;
	return;
      }
      xm = xc + GOLDEN_RATIO*(xc-xb);
      fm = linesearch_eval(xm);

    }
    // [xc, xlim]
    else if ((xc-xm)*(xm-xlim) > 0.0) {
      if (fm < fc) {
	xb = xc;
	fb = fc;
	xc = xm;
	fc = fm;  
	xm = xc+GOLDEN_RATIO*(xc-xb);
	fm = linesearch_eval(xm);
      }
    }
    // [xlim, inf]
    else if ((xm-xlim)*(xlim-xc) >= 0.0 ) {
      xm = xlim;
      fm = linesearch_eval(xm);
    }
    // [0,xb]
    else {
      xm = xc + GOLDEN_RATIO*(xc-xb);
      fm = linesearch_eval(xm);
    }

    // shift to newest 3 points before loop
    xa = xb;
    fa = fb;
    xb = xc;
    fb = fc;
    xc = xm;
    fc = fm;

  }

}

/** Perform 1-D minimization for the stepLength using Brent's method.
    This is a C translation of fmin.f from Netlib. 
*/
Real NonlinearCGOptimizer::brent_minimize(Real a, Real b, Real tol)
{

  // c is the squared inverse of the golden ratio
  Real c = 0.5*(3.0-sqrt(5.0));

  // eps is approximately the square root of the relative machine
  // precision.

  // initialization 
  Real eps = std::numeric_limits<double>::epsilon();
  Real tol1 = eps+1.0;
  eps = sqrt(eps);

  Real v = a+c*(b-a);
  Real w = v;
  Real x = v;
  Real e = 0.0;
  Real fx = linesearch_eval(x);
  Real fv = fx;
  Real fw = fx;
  Real tol3 = tol/3.0;

  Real xm = 0.5*(a+b);
  tol1 = eps*std::fabs(x)+tol3;
  Real t2 = 2.0*tol1;

  Real d, p, q, r, u, fu;  // temp variables

  unsigned iter = 1;
  while ( iter < maxLinesearchIters && std::fabs(x-xm) > (t2-0.5*(b-a)) ) {

    d = 0.0;
    p = 0.0;
    q = 0.0;
    r = 0.0;
    if ( std::fabs(e) > tol1 ) { 

      // fit parabola
      r = (x-w)*(fx-fv);
      q = (x-v)*(fx-fw);
      p = (x-v)*q-(x-w)*r;
      q = 2.0*(q-r);
  
      if (q <= 0.0)
	q = -q;
      else
	p = -p;
      r = e;
      e = d;
  
    }

    if ( (std::fabs(p) < std::fabs(0.5*q*r)) && (p > q*(a-x)) && (p < q*(b-x)) ) {

      // a parabolic-interpolation step
      d = p/q;
      u = x+d;
      // f must not be evaluated too close to ax or bx
      if ( ((u-a) < t2) || ((b-u) < t2) ) {
	d = tol1;
	if (x >= xm) 
	  d = -d;
      }
    }
    else {

      // a golden-section step
      if (x >= xm)
	e = a-x;
      else
	e = b-x;
      d = c*e;
      
    }

    // f must not be evaluated too close to x
    if ( std::fabs(d) < tol1 )
      if ( d <= 0.0 )
	u = x-tol1;
      else
	u = x+tol1;
    else
      u = x+d;
    
    fu = linesearch_eval(u);

    // update  a, b, v, w, and x
    if (fx <= fu) 
      if (u >= x)
	b = u;
      else
	a = u;

    if (fu <= fx) {

      if (u >= x)
	a = x;
      else
	b = x;
      v = w;
      fv = fw;
      w = x;
      fw = fx;
      x = u;
      fx = fu;

    }
    else if ( (fu > fw) && (w != x) ) {

      if ((fu<=fv) || (v == x) || (v == w)) {
	v = u;
	fv = fu;
      }

    }
    else {

      v = w;
      fv = fw;
      w = u;
      fw = fu;

    }

    xm = 0.5*(a+b);
    tol1 = eps*std::fabs(x)+tol3;
    t2 = 2.0*tol1;
    iter++;

  } // end while

  if (iter > maxLinesearchIters && outputLevel >= NORMAL_OUTPUT) {
    Cout << "WARN (NonlinearCG_LS): Step length not found within "
	 << "maxLinesearchIters; using best known." << endl;
  }

  return(x);

} 


// Function evaluator to use in linesearches
// Uses the current designVars and searchDirection 
// TODO: Support gradient-based evals, likely returning a const Response&
Real NonlinearCGOptimizer::linesearch_eval(const Real& trial_step,
					   short req_val) 
{
  // evaluate function only 
  for (size_t i=0; i<numContinuousVars; i++)
    trialVars[i] = designVars[i] + trial_step * searchDirection[i];
  iteratedModel.continuous_variables(trialVars);
  activeSet.request_values(req_val);
  iteratedModel.evaluate(activeSet);
  const Response& response = iteratedModel.current_response();
  const RealVector& functions = response.function_values();
  
  return(functions[0]);
}


void NonlinearCGOptimizer::parse_options()
{
  // Allowed update options
  map<string, int> update_type;
  update_type["steepest"]              = 0;
  update_type["fletcher_reeves"]       = 1;
  update_type["polak_ribiere"]         = 2;
  update_type["polak_ribiere_plus"]    = 3;
  update_type["hestenes_stiefel"]      = 4;

  // Allowed linesearch options
  map<string, int> search_type;
  search_type["fixed_step"] = 0;
  search_type["ls_simple"]  = 1;
  search_type["ls_brent"]   = 2;
  search_type["ls_wolfe"]   = 3;

  map<string,string> opts;
  const StringArray& db_opts = probDescDB.get_sa("method.coliny.misc_options");
  StringArray::const_iterator db_it = db_opts.begin();
  StringArray::const_iterator db_end = db_opts.end();
  String::const_iterator delim;

  for ( ; db_it != db_end; ++db_it)
    if ( (delim = find(db_it->begin(), db_it->end(), '=')) != db_it->end()) {

      String opt(*db_it, 0, distance(db_it->begin(), delim));
      String val(*db_it, distance(db_it->begin(), delim+1),
		 distance(delim, db_it->end()));
      
      if (opt == "initial_step")
	initialStep = atof(val.c_str());
      else if (opt == "linesearch_tolerance")
	linesearchTolerance = atof(val.c_str());
      else if (opt == "linesearch_type") {
	map<string, int>::const_iterator cit;
	if ( (cit = search_type.find(val)) != search_type.end())
	  linesearchType = cit->second;
	else {
	  Cerr << "ERROR (NonlinearCG): Invalid linesearch_type." << endl;
	  abort_handler(-1);
	}
      }
      else if (opt == "max_linesearch_iters")
	maxLinesearchIters = atoi(val.c_str());
      else if (opt == "rel_function_tol")
	relFunctionTol = atof(val.c_str());
      else if (opt == "rel_gradient_tol")
	relGradientTol = atof(val.c_str());
      else if (opt == "restart_iter")
	restartIter = atoi(val.c_str());
      else if (opt == "reset_step")
	resetStep = (val == "true") ? true : false;
      else if (opt == "update_type") {
	map<string, int>::const_iterator cit;
	if ( (cit = update_type.find(val)) != update_type.end())
	  updateType = cit->second;
	else {
	  Cerr << "ERROR (NonlinearCG): Invalid update_type." << endl;
	  abort_handler(-1);
	}
      }
      else {
	Cerr << "ERROR (NonlinearCG): Unknown misc_option." << endl;
	abort_handler(-1);
      }

      if (outputLevel > NORMAL_OUTPUT)
	Cout << "INFO (NonlinearCG): User parameter '" << opt << "': " << val 
	     << endl;

    }
    else {
      Cerr << "ERROR (NonlinearCG): Invalid misc_options format." << endl;
      abort_handler(-1);
    }

}


} // namespace Dakota
