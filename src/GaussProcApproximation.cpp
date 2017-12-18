/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 GaussProcApproximation
//- Description: Class implementation for GaussianProcess Approximation
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "GaussProcApproximation.hpp"
#include "dakota_data_types.hpp"
#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"
//#include "NPSOLOptimizer.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#include "ProblemDescDB.hpp"

#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
using OPTPP::NLPFunction;
using OPTPP::NLPGradient;
#endif

#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseSolver.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

//#define DEBUG
//#define DEBUG_FULL


namespace Dakota {

using Teuchos::rcp;

typedef Teuchos::SerialDenseSolver<int, Real>    RealSolver;
typedef Teuchos::SerialSpdDenseSolver<int, Real> RealSpdSolver;

//initialization of statics
GaussProcApproximation* GaussProcApproximation::GPinstance(NULL);

// --------------- Distance routines for point selection -----------
// These functions are external to GaussProcApproximation, and are used by the
// point selection algorithm to determine whether distance criteria are met.

/** Gets the Euclidean distance between x1 and x2 */
Real getdist(const RealVector& x1, const RealVector& x2)
{
  int j,d;
  d = x1.length();
  if (d!=x2.length()) 
    Cerr << "Size mismatch in getdist in GaussProcApproximation\n"; 
  if (d==0) 
    Cerr << "Zero dimension in getdist in GaussProcApproximation\n"; 
  Real result;
  result = 0.0;
  for (j=0; j<d; j++) {
    result += (x1[j]-x2[j])*(x1[j]-x2[j]);
  }
  return std::sqrt(result);
}


/** Returns the minimum distance between the point x and the points in
    the set xset (compares against all points in xset except point
    "except"): if except is not needed, pass 0. */
Real mindist(const RealVector& x, const RealMatrix& xset, int except) 
{
  int i,j,n,d;
  d = x.length();
  if (d!= xset.numCols()) Cout << "Dimension mismatch in mindist";
  n = xset.numRows();
  Real dist, result;
  RealVector xp(d);
  for (i=0; i<n; i++) {
    for (j=0; j<d; j++) xp[j]=xset(i,j);
    dist = getdist(x,xp);
    if ((dist < result && i != except) || i==0) result=dist;
  }
  return result;
}


/** Gets the min distance between x and points in the set xset defined
    by the nindx values in indx. */
Real mindistindx(const RealVector& x, const RealMatrix& xset,
                 const IntArray& indx)
{
  int i,j,d,n,nindx;
  d = x.length();
  n = xset.numRows();
  nindx = indx.size();
  if (nindx > n) 
    Cerr << "Size error in mindistinx in GaussProcApproximation\n";
  Real dist,result;
  RealVector xp(d);
  for (i=0; i<nindx; i++) {
    for (j=0; j<d; j++) xp[j]=xset(indx[i],j);
    dist = getdist(x,xp);
    if (dist < result || i==0) result = dist;
  }
  return result;
}


/** Gets the maximum of the min distance between each point and the 
    rest of the set. */
Real getRmax(const RealMatrix& xset)
{
  int i,j,d,n;
  d = xset.numCols();
  n = xset.numRows();
  if (n==0 || d==0) 
    Cerr << "Zero size in getRmax in GaussProcApproximation.  n:" 
	 << n << "  d:" << d << std::endl;
  Real mini,result;
  RealVector xp(d);
  for (i=0; i<n; i++) {
    for (j=0; j<d; j++) xp[j] = xset(i,j);
    mini = mindist(xp,xset,i);
    if (mini>result || i==0) result=mini;
  }
  return result;
}
// ----------------------------------------------------------------


GaussProcApproximation::
GaussProcApproximation(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data,
                       const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label),
  usePointSelection(problem_db.get_bool("model.surrogate.point_selection"))
{
  const String& trend_string
    = problem_db.get_string("model.surrogate.trend_order");
  if (trend_string == "constant")               trendOrder = 0;
  else if (trend_string == "linear")            trendOrder = 1;
  else if (trend_string == "reduced_quadratic") trendOrder = 2;
  else {
    Cerr << "\nError (global_gaussian): unsupported trend " << trend_string 
	 << "; valid options are constant, linear, or reduced_quadratic"
	 << std::endl;
    abort_handler(-1);
  }

#ifdef HAVE_NCSU
  Cout << "Using NCSU DIRECT to optimize correlation coefficients."<<std::endl;
#else
  Cerr << "Error: NCSU DIRECT Optimizer is not available to calculate " 
       << "the correlation coefficients governing the Gaussian process." 
       << "Aborting process. " << std::endl;
  abort_handler(-1);
#endif
}


int GaussProcApproximation::min_coefficients() const
{
  // min number of samples required to build the network is equal to
  // the number of design variables + 1

  // Note: Often this is too few samples.  It is better to have about
  // O(n^2) samples, where 'n' is the number of variables.

  return sharedDataRep->numVars + 1;
}


int GaussProcApproximation::num_constraints() const
{ return (approxData.anchor()) ? 1 : 0; }


void GaussProcApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  size_t i, j, offset = 0, num_v = sharedDataRep->numVars;
  numObs = approxData.points();
  // GaussProcApproximation does not directly handle anchorPoint
  // -> treat it as another currentPoint
  if (approxData.anchor()) {
    offset  = 1;
    numObs += 1;
  }

  // Transfer the training data to the Teuchos arrays used by the GP
  trainPoints.shapeUninitialized(numObs, num_v);
  trainValues.shapeUninitialized(numObs, 1);
  // process anchorPoint, if present
  if (approxData.anchor()) {
    const RealVector& c_vars = approxData.anchor_continuous_variables();
    for (j=0; j<num_v; ++j)
      trainPoints(0,j) = c_vars[j];
    trainValues(0,0) = approxData.anchor_function();
  }
  // process currentPoints
  for (i=offset; i<numObs; ++i) {
    const RealVector& c_vars = approxData.continuous_variables(i);
    for (j=0; j<num_v; j++)
      trainPoints(i,j) = c_vars[j];
    trainValues(i,0) = approxData.response_function(i);
  }

  // Build a GP covariance model using the sampled data
  GPmodel_build();
}


Real GaussProcApproximation::value(const Variables& vars)
{ GPmodel_apply(vars.continuous_variables(),false,false); return approxValue; }


const RealVector& GaussProcApproximation::gradient(const Variables& vars)
{ GPmodel_apply(vars.continuous_variables(),false,true); return approxGradient;}


Real GaussProcApproximation::prediction_variance(const Variables& vars)
{ GPmodel_apply(vars.continuous_variables(),true,false); return approxVariance;}


void GaussProcApproximation::GPmodel_build()
{
  // Point selection is off by default, but will be forced if a large
  //   number of points are present
  // Note that even if the point selector uses all the points, it is not 
  //   equivalent to no point selection, b/c the point selector will 
  //   probably use only a subset of the points for MLE, so the theta 
  //   parameters may be different.
  numObs = trainValues.numRows();
  // bool use_point_selection = false;
  //if (someLogicToTurnOnPointSelection) 
  //  use_point_selection = true;

  // Normalize the inputs and output
  normalize_training_data();

  // Initialize the GP parameters
  //trendOrder = 2; // 0-constant, 1-linear, 2-quadratic

  size_t num_v = sharedDataRep->numVars;
  switch (trendOrder) {
  case 0: 
    betaCoeffs.shape(1,1);
    break;
  case 1:  
    betaCoeffs.shape(num_v+1,1);
    break;
  case 2:  
    betaCoeffs.shape(2*num_v+1,1);
    break;
  }

  thetaParams.sizeUninitialized(num_v);
  
  // Calculate the trend function
  get_trend();

  if (usePointSelection) {
    // Make copies of training data
    numObsAll          = numObs;
    normTrainPointsAll = normTrainPoints;
    trainValuesAll     = trainValues;
    trendFunctionAll   = trendFunction; 
    run_point_selection();
  }
  else {
    Cout << "\nBuilding GP using all " << numObs <<" training points...\n"; 
    //optimize_theta_multipoint();
    optimize_theta_global();
    get_cov_matrix();
    get_cholesky_factor();
    get_beta_coefficients();
    get_process_variance();
  }

#ifdef DEBUG
  Cout << "Theta:" << std::endl;
  for (size_t i=0; i<num_v; i++)
    Cout << thetaParams[i] << std::endl;
#endif //DEBUG
}


void GaussProcApproximation::
GPmodel_apply(const RealVector& approx_pt, bool variance_flag,
	      bool gradients_flag)
{
  size_t i, num_v = sharedDataRep->numVars;
  if (approx_pt.length() != num_v) {
    Cerr << "Error: Dimension mismatch in GPmodel_apply" << std::endl;
    abort_handler(-1);
  }

  approxPoint.shapeUninitialized(1,num_v);
  for (i=0; i<num_v; i++)
    approxPoint(0,i) = (approx_pt(i)-trainMeans(i))/trainStdvs(i);

  get_cov_vector();
  predict(variance_flag,gradients_flag);
}


void GaussProcApproximation::normalize_training_data()
{
  size_t i, j, num_v = sharedDataRep->numVars;
  trainMeans.sizeUninitialized(num_v);
  trainStdvs.sizeUninitialized(num_v);
  normTrainPoints = trainPoints;

 //get means of each column for X input
  for (i=0; i<num_v; i++) {
    Real sum = 0.;
    for (j=0; j<numObs; j++)
      sum += normTrainPoints(j,i);
    trainMeans(i) = sum/(double(numObs));
  }

  //subtract means from each value in each column
  for (i=0; i<num_v; i++) {
    trainStdvs(i)=0.;
    for (j=0; j<numObs; j++) {
      normTrainPoints(j,i) -= trainMeans(i);
      trainStdvs(i) += std::pow(normTrainPoints(j,i),2);
    }
    trainStdvs(i) = std::sqrt(trainStdvs(i)/double(numObs-1));
  }

  //divide by standard deviation
  for (i=0; i<num_v; i++)
    for (j=0; j<numObs; j++)
      normTrainPoints(j,i) /= trainStdvs(i);
 
#ifdef DEBUG_FULL
  for (j=0; j<numObs; j++) {
    for (i=0; i<num_v; i++)
      Cout <<" input  " << normTrainPoints(j,i) << '\n';
    Cout <<" output " <<     trainValues(j,0) << '\n';
  }
#endif // DEBUG_FULL
}


void GaussProcApproximation::get_trend()
{
  size_t num_v = sharedDataRep->numVars;
  switch (trendOrder) {
  case 0: 
    trendFunction.shapeUninitialized(numObs,1);
    break;
  case 1: 
    trendFunction.shapeUninitialized(numObs,num_v+1);
    break;
  case 2: 
    trendFunction.shapeUninitialized(numObs,2*num_v+1);
    break;
  }

  // all orders require ones in first column
  for (size_t i=0; i<numObs; i++)
    trendFunction(i,0) = 1.;
 
  if (trendOrder > 0) {
    for (size_t j=0; j<num_v; j++) {
      for (size_t i=0; i<numObs; i++) {
        trendFunction(i,j+1) = normTrainPoints(i,j);
        if (trendOrder == 2) 
          trendFunction(i,num_v+j+1)
	    = normTrainPoints(i,j)*normTrainPoints(i,j);
      }
    }
  }
}


void GaussProcApproximation::optimize_theta_global()
{
  GPinstance = this;
  Iterator nll_optimizer; // empty envelope

  // bounds for non-log transformation - ie, no exp(theta)
  //RealVector theta_lbnds(num_v,1.e-5), theta_ubnds(num_v,150.);
  // bounds for log transformation of correlation parameters
  size_t num_v = sharedDataRep->numVars;
  RealVector theta_lbnds(num_v, false), theta_ubnds(num_v, false);
  theta_lbnds = -9.; theta_ubnds = 5.;
#ifdef HAVE_NCSU  
  // NCSU DIRECT optimize of Negative Log Likelihood
  // Uses default convergence tolerance settings in NCSUOptimizer wrapper!
  int max_iterations = 1000, max_fn_evals = 10000;
  nll_optimizer.assign_rep(
    new NCSUOptimizer(theta_lbnds,theta_ubnds,max_iterations,max_fn_evals,
		      negloglikNCSU),false);
  nll_optimizer.run(); // no pl_iter needed for this optimization
  const Variables& vars_star = nll_optimizer.variables_results();
  const Response&  resp_star = nll_optimizer.response_results();
  copy_data(vars_star.continuous_variables(), thetaParams);

#ifdef DEBUG
  Cout << "Optimal NLL = " << resp_star.function_value(0) << '\n';
#endif //DEBUG
#endif //HAVE_NCSU
}


void GaussProcApproximation::get_cov_matrix()
{
  // Note, this gets only the lower triangle of covmatrix, which is all
  // that is needed by the Teuchos SPD solvers.  However, if this matrix
  // is to be used in full matrix multiplication, or with a non-SPD
  // solver, then the lower part should be copied to the upper part.

  size_t i, j, k, num_v = sharedDataRep->numVars;
  covMatrix.shape(numObs);
  RealVector expThetaParms(num_v);
  
  for (i=0; i<num_v; i++){
    expThetaParms[i]=std::exp(thetaParams[i]);
  }
  Real sume, delta;
  for (j=0; j<numObs; j++){
    for (k=j; k<numObs; k++){
      sume = 0.;
      for (i=0; i<num_v; i++){
	Real pt_diff = normTrainPoints(j,i) - normTrainPoints(k,i);
	sume += expThetaParms[i]*pt_diff*pt_diff;
	//sume += std::exp(thetaParams[i])*pt_diff*pt_diff;
      }
      covMatrix(k,j) = std::exp(-1.*sume);
    }
  }

#ifdef DEBUG_FULL
  Cout << "covariance matrix" << '\n';
  for (j=0; j<numObs; j++){
    for (k=0; k<numObs; k++)
      Cout << covMatrix(j,k) << " ";
    Cout << '\n';
  }
  Cout << '\n';
#endif //DEBUG_FULL
}


int GaussProcApproximation::get_cholesky_factor()
// Gets the Cholesky factorization of the covariance matrix.  If this
// is called by prediction routines, then they are basically "stuck"
// with the current covariance matrix, so if it turns out to be
// singular to wp, then we iteratively condition it with very small
// values until it is no longer singular.  However, if this routine is
// called by the likelihood estimation procedures, we want them to
// "know" about singular covariance as opposed to "working around it"
// by conditioning.  Thus, this routine returns an integer of 0 if the
// covariance matrix is positive definite, and returns 1 if it is
// singular to wp.
{
  int ok,i;
  Real nugget = 1e-15;
  covSlvr.setMatrix( rcp(&covMatrix, false) );
  covSlvr.factorWithEquilibration(true);
  ok = covSlvr.factor();
  if (ok > 0) {
    do {
      get_cov_matrix();
      for (i=0; i<numObs; i++) covMatrix(i,i) += nugget;
      covSlvr.setMatrix( rcp(&covMatrix, false) );
      covSlvr.factorWithEquilibration(true);
      ok = covSlvr.factor();
      nugget *= 3.0;
    } while (ok > 0);
//#ifdef DEBUG_FULL
    Cout << "COV matrix corrected with nugget: " << nugget/3. << std::endl;
//#endif
    cholFlag = 1;
    return 1;
  }
  else {
    cholFlag = 0;
    return 0;
  }
}
  

void GaussProcApproximation::get_beta_coefficients()
{
  // Default is generalized least squares, but ordinary least squares is
  //    available if desired
  bool ordinary = false;

  size_t num_v = sharedDataRep->numVars, trend_dim = 1 + trendOrder * num_v;
  // Solver for the identity matrix - only used if OLS requested
  RealSpdSolver eye_slvr;
  RealSymMatrix identity_matrix;
  if (ordinary) {
    // Construct the identity matrix:
    identity_matrix.shape(numObs); // inits to 0
    for (size_t i=0; i<numObs; i++)
      identity_matrix(i,i) = 1.;
    eye_slvr.setMatrix( rcp(&identity_matrix, false) );
  }

  RealMatrix Rinv_Y(numObs, 1, false);
  if (ordinary) {
    eye_slvr.setVectors( rcp(&Rinv_Y, false), rcp(&trainValues, false) );
    eye_slvr.solve();
  }
  else {
    covSlvr.setVectors( rcp(&Rinv_Y, false), rcp(&trainValues, false) );
    covSlvr.solve();
  }

  RealMatrix FT_Rinv_Y(trend_dim, 1, false);
  FT_Rinv_Y.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1.0, trendFunction,
		     Rinv_Y, 0.0);
  
  RealMatrix Rinv_F(numObs, trend_dim, false);
  if (ordinary) {
    eye_slvr.setVectors( rcp(&Rinv_F, false), rcp(&trendFunction, false) );
    eye_slvr.solve();
  }
  else {
    covSlvr.setVectors( rcp(&Rinv_F, false), rcp(&trendFunction, false) );
    covSlvr.solve();
  }

  RealMatrix FT_Rinv_F(trend_dim, trend_dim, false);
  FT_Rinv_F.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., trendFunction,
		     Rinv_F, 0.);
  // LPS TO DO:
  //RealSymMatrix FT_Rinv_F(trend_dim, false);
  //Teuchos::symMatTripleProduct(Teuchos::TRANS, 1., trendFunction,
  //                             Rinv, FT_Rinv_F);
  //RealSpdSolver Temp_slvr;
 
  RealMatrix temphold5(trend_dim, 1, false);
  RealSolver Temp_slvr;
  Temp_slvr.setMatrix( rcp(&FT_Rinv_F, false) );
  Temp_slvr.setVectors( rcp(&temphold5, false), rcp(&FT_Rinv_Y, false) );
  Temp_slvr.factorWithEquilibration(true);
  Temp_slvr.factor();
  Temp_slvr.solve();

  for (size_t i=0; i<trend_dim; i++)
    betaCoeffs(i,0) = temphold5(i,0);

  if (betaCoeffs(0,0) != betaCoeffs(0,0)) 
    Cerr << "Nan for beta at exit of get_beta in GaussProcApproximation\n";
}


void GaussProcApproximation::get_process_variance()
{
  RealMatrix YFb(numObs, 1, false),
    temphold3(1, 1, false);

  Rinv_YFb.shape(numObs, 1); 
  YFb.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., trendFunction,
	       betaCoeffs, 0.);
  YFb.scale(-1);
  YFb += trainValues;

  covSlvr.setVectors( rcp(&Rinv_YFb, false), rcp(&YFb, false) );
  covSlvr.solve();

  temphold3.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., YFb, Rinv_YFb, 0.);
 
  procVar = temphold3(0,0)/double(numObs);
 
}


void GaussProcApproximation::get_cov_vector()
{
  covVector.shapeUninitialized(numObs,1);
  size_t i, j, num_v = sharedDataRep->numVars;

  RealVector expThetaParms(num_v);
  
  for (i=0; i<num_v; i++){
    expThetaParms[i]=std::exp(thetaParams[i]);
  }
  for (j=0; j<numObs; j++){
    Real sume = 0.;    
    for (i=0; i<num_v; i++){
      //sume += thetaParams[i]
      //     *  std::pow((normTrainPoints(j,i)-approxPoint(0,i)),2);
      
      sume += expThetaParms[i]*(normTrainPoints(j,i)-approxPoint(0,i))*
	(normTrainPoints(j,i)-approxPoint(0,i));
    }
    covVector(j,0) = std::exp(-1.*sume);
  }

#ifdef DEBUG_FULL
  Cout << "covariance vector" << '\n';
  for (size_t j=0; j<numObs; j++)
    Cout << covVector(j,0) << " ";
  Cout << '\n';
#endif //DEBUG_FULL
}

void GaussProcApproximation::predict(bool variance_flag, bool gradients_flag)
{
  size_t i, j, k, num_v = sharedDataRep->numVars;

  RealMatrix f_xstar;
  switch (trendOrder) {
  case 0:
    f_xstar.shapeUninitialized(1,1);
    f_xstar(0,0) = 1.;
    break;
  case 1:
    f_xstar.shapeUninitialized(1,num_v+1);
    f_xstar(0,0) = 1.;
    for (j=0; j<num_v; j++)
      f_xstar(0,j+1) = approxPoint(0,j);
    break;
  case 2:
    f_xstar.shapeUninitialized(1,2*num_v+1);
    f_xstar(0,0) = 1.;
    for (j=0; j<num_v; j++) {
      f_xstar(0,j+1) = approxPoint(0,j);
      f_xstar(0,num_v+j+1) = approxPoint(0,j)*approxPoint(0,j);
    }
    break;
  }

  RealMatrix approx_val(1, 1, false);
  approx_val.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., covVector,
		      Rinv_YFb, 0.);

  RealMatrix f_beta(1, 1, false);
  f_beta.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., f_xstar, 
		  betaCoeffs, 0.);

  approxValue = approx_val(0,0) + f_beta(0,0);
#ifdef DEBUG_FULL
  Cout << "prediction " << approxValue << '\n';
#endif //DEBUG_FULL

  // Gradient of prediction with respect to input
  if (gradients_flag) {
#ifdef DEBUG_FULL
    Cout << "\n<<<<< Entering gradient prediction in GP code\n";
#endif //DEBUG_FULL
    // Construct the global matrix of derivatives, gradCovVector
    get_grad_cov_vector();

    RealMatrix gradPred(num_v, 1, false), dotProd(1, 1, false),
       gradCovVector_i(numObs, 1, false);
    approxGradient.sizeUninitialized(num_v);
    for (i=0; i<num_v; i++) {
      // First set up gradCovVector_i to be the ith column of the matrix
      // gradCovVector -- Teuchos allow for a more elegant way of doing this...
      for (j=0; j<numObs; j++) 
    	gradCovVector_i(j,0) = gradCovVector(j,i);
        // Grad_i = dot_prod(gradCovVector_i, inv(R)*YFb)
      dotProd.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., Rinv_YFb,
		       gradCovVector_i, 0.);
      approxGradient[i] = gradPred(i,0) = dotProd(0,0);
      // add trend derivative, accounting for scaling approxPoint(0,i) =
      // (approx_pt(i)-trainMeans(i))/trainStdvs(i) in GPmodel_apply()
      switch (trendOrder) {
      //case 0: // constant trend: no contribution to derivative
      case 1: // derivative of linear trend
	approxGradient[i] +=   betaCoeffs(i+1,0) / trainStdvs(i);  break;
      case 2: // derivative of trend with linear and diagonal quadratic terms
	approxGradient[i] += ( betaCoeffs(i+1,0) + 2.*betaCoeffs(num_v+i+1,0) *
			       approxPoint(0,i) ) / trainStdvs(i); break;
      }
    }
  }

  if (variance_flag) {
    RealMatrix Rinv_covvec(numObs, 1, false), rT_Rinv_r(1, 1, false);

    covSlvr.setVectors( rcp(&Rinv_covvec, false), rcp(&covVector, false) );
    covSlvr.solve();

    rT_Rinv_r.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., covVector,
		       Rinv_covvec, 0.);

    approxVariance = procVar*(1.- rT_Rinv_r(0,0));
#ifdef DEBUG_FULL
    Cout << "variance " << approxVariance << "\n\n";
#endif //DEBUG_FULL

    // set this flag to true to use longer equation
    bool full_var_flag = true;
    if (full_var_flag) {
      size_t trend_dim = 1 + trendOrder * num_v;
      
      //RealMatrix full_variance(1,1);
      RealMatrix f_FT_Rinv_r(trend_dim, 1, false),
	f_xstar_T(trend_dim, 1, false),	Rinv_F(numObs, trend_dim, false),
	temp9(trend_dim, 1, false), temp10(1,1, false);
      f_FT_Rinv_r.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., trendFunction,
			   Rinv_covvec, 0.); 
      f_FT_Rinv_r.scale(-1);

      for (size_t i=0; i<trend_dim; i++) 
	f_xstar_T(i,0) = f_xstar(0,i);
      f_FT_Rinv_r += f_xstar_T;
      
      covSlvr.setVectors( rcp(&Rinv_F, false), rcp(&trendFunction, false) );
      covSlvr.solve();

      RealMatrix FT_Rinv_F(trend_dim, trend_dim, false);
      FT_Rinv_F.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., trendFunction,
			 Rinv_F, 0.);
      // LPS TO DO:
      //RealSymMatrix FT_Rinv_F(trend_dim, false);
      //Teuchos::symMatTripleProduct(Teuchos::TRANS, 1., trendFunction,
      //                             Rinv, FT_Rinv_F);
      //RealSpdSolver Temp_slvr;

      RealSolver Temp_slvr;
      Temp_slvr.setMatrix( rcp(&FT_Rinv_F, false) );
      Temp_slvr.setVectors( rcp(&temp9, false), rcp(&f_FT_Rinv_r, false) );
      Temp_slvr.factorWithEquilibration(true);
      Temp_slvr.factor();
      Temp_slvr.solve();

      temp10.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., temp9, 
		      f_FT_Rinv_r, 0.);

      //full_variance(0,0)=procVar*(1.0-rT_Rinv_r(0,0)+temp10(0,0));
      //approxVariance = full_variance(0,0);
      approxVariance = procVar*(1.- rT_Rinv_r(0,0) + temp10(0,0));

#ifdef DEBUG_FULL
      Cout << "full variance " << approxVariance << "\n\n";
#endif //DEBUG_FULL
    }
 
    // check for small (possibly negative) variance
    if (approxVariance < 1.e-9)
      approxVariance = 1.e-9;
  }
}


void GaussProcApproximation::get_grad_cov_vector()
// Gets the derivatives of the covVector with respect to each
// component in X (note that the derivatives are taken with respect to
// the pre-transformed values of X).  Each column of gradCovVector is
// set to one of the derivatives.  covVector will be accessed, so it
// must be set prior to calling get_grad_cov_vector, and approxPoint
// will be accessed as well.  Also note that this is currently only
// valid for the squared-exponential correlation function.
{
  size_t i, j, num_v = sharedDataRep->numVars;
  gradCovVector.shapeUninitialized(numObs,num_v);
  for (i=0; i<numObs; i++)
    for (j=0; j<num_v; j++)
      gradCovVector(i,j) = covVector(i,0) * -2. * std::exp(thetaParams[j]) *
	(approxPoint(0,j) - normTrainPoints(i,j)) / trainStdvs(j);
      // division by trainStdvs(j) accounts for standardization transformation:
}


Real GaussProcApproximation::calc_nll()
// This routine is called only after the covariance matrix has been
// computed and factored, so that we work with the global solver
// Cov_slvr
{
  Real det = 1., nll;
  // determinant based on Cholesky factor is simply the square of
  // the product of the diagonals

  for (size_t i=0; i<numObs; i++)
    det = det*(*covSlvr.getFactoredMatrix())(i, i);
  det = det*det;
  if (det <= 0.) cholFlag = -1; // indicates singular matrix

  if (cholFlag == 0) { //not singular
    get_beta_coefficients();
    get_process_variance();

    nll = numObs*std::log(procVar) + std::log(det);
  }
  else { 
#ifdef DEBUG_FULL
    Cout << "Singular covariance encountered" << std::endl;
#endif
    nll = 5.e100; 
  }
  return (nll);
}

// function maps a single point x to function value f
double GaussProcApproximation::negloglikNCSU(const RealVector &x)
{
  GPinstance->thetaParams = x;
  GPinstance->get_cov_matrix();
  GPinstance->get_cholesky_factor(); //get the factored matrix only once
  return(GPinstance->calc_nll());
}

void GaussProcApproximation::optimize_theta_multipoint()
{
  GPinstance = this;
  Iterator nll_optimizer; // empty envelope

  size_t num_v = sharedDataRep->numVars;
  // bounds for non-log transformation - ie, no exp(theta)
  //RealVector theta_lbnds(num_v,1.e-5), theta_ubnds(num_v,150.);
  // bounds for log transformation of correlation parameters
  RealVector theta_lbnds(num_v,-9.), theta_ubnds(num_v,5.);
  RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
  RealVector lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
    nln_ineq_lower_bnds, nln_ineq_upper_bnds, nln_eq_targets; 
  
  size_t i,j;
  RealVector THETA_INIT_STARTS(3);
  THETA_INIT_STARTS[0] = std::log(0.1);
  THETA_INIT_STARTS[1] = std::log(1.0);
  THETA_INIT_STARTS[2] = std::log(4.0);
  Real nll, nll_best = DBL_MAX;
  RealVector theta_best(num_v);

#ifdef HAVE_OPTPP
  for (i=0; i<3; i++) {
    for (j=0; j<num_v; j++)
      thetaParams[j] = THETA_INIT_STARTS[i];
    nll_optimizer.assign_rep(
      new SNLLOptimizer(
        thetaParams, theta_lbnds, theta_ubnds, lin_ineq_coeffs,
	lin_ineq_lower_bnds, lin_ineq_lower_bnds, lin_eq_coeffs,
	lin_eq_targets, nln_ineq_lower_bnds, nln_ineq_upper_bnds,
	nln_eq_targets, negloglik, constraint_eval), false);
    nll_optimizer.run(); // no pl_iter needed for this optimization
    const Variables& vars_star = nll_optimizer.variables_results();
    const Response&  resp_star = nll_optimizer.response_results();
    copy_data(vars_star.continuous_variables(), thetaParams);
    nll = resp_star.function_value(0);
    if (nll < nll_best) {
      nll_best = nll;
      for (j=0; j<num_v; j++) 
	theta_best[j] = thetaParams[j];
    }
  }
  // Now put the best run back into thetaParams and nll
  for (j=0; j<num_v; j++)
    thetaParams[j] = theta_best[j];
  nll = nll_best;
#endif //OPTPP
  
#ifdef DEBUG
  Cout << "Optimal NLL = " << nll << '\n';
#endif //DEBUG
}


void GaussProcApproximation::calc_grad_nll()
{
  size_t i, j, k, num_v = sharedDataRep->numVars;

  double trace_i;

  // First check if the covariance is singular.
  Real det = 1.;
  for (i=0; i<numObs; i++)
    det = det*(*covSlvr.getFactoredMatrix())(i, i);
  det = det*det;
  if (det <= 0.) cholFlag = -1;  //Singular matrix

  gradNegLogLikTheta.shapeUninitialized(num_v,1);

  if (cholFlag == 0) { // ok to go ahead with gradient calcs

    RealMatrix YFb(numObs, 1, false), Rinv_Y(numObs, 1, false),
      Rk(numObs, numObs, false), trace(numObs, numObs, false),
      temphold5(numObs, 1, false), temphold6(1, 1, false);
    YFb.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., trendFunction,
		 betaCoeffs, 0.);
    YFb.scale(-1);
    YFb += trainValues;

    covSlvr.setVectors( rcp(&Rinv_Y, false), rcp(&YFb, false) );
    covSlvr.solve();
    // Note that trans(Y)*inv(R) = transpose(inv(R)*Y)

    // Note: Rk is symmetric, but trace=inv(R)*Rk is not.  This is the
    // only time we need to do a matrix fill of a symmetric matrix
 
    for (i=0; i<num_v; i++){
      for (k=0; k<numObs; k++){
	for (j=k; j<numObs; j++){
	  Real pt_diff = normTrainPoints(j,i)-normTrainPoints(k,i);
	  //Rk(j,k) = -1*thetaParams[i]*pt_diff*pt_diff*covMatrix(j,k);
	  Rk(j,k) = -std::exp(thetaParams[i])*pt_diff*pt_diff*covMatrix(j,k);
	  Rk(k,j) = Rk(j,k);  //fill upper part without redoing the calcs
	}
      }
  
      covSlvr.setVectors( rcp(&trace, false), rcp(&Rk, false) );
      covSlvr.solve();

      trace_i = 0.;
      for (j=0; j<numObs; j++)
	trace_i += trace(j,j);
    
      temphold5.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., Rk,
			 Rinv_Y, 0.);
      temphold6.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., Rinv_Y, 
			 temphold5, 0.);

      gradNegLogLikTheta(i,0) = trace_i - ((1./procVar)*temphold6(0,0));
    }
  }
  else {
#ifdef DEBUG_FULL
    Cout << "Singular covariance encountered in gradient routine" << std::endl;
#endif
    //encourage thetas to increase, but not too much
    for (i=0; i<num_v; i++)
      gradNegLogLikTheta(i,0) = -1.e3;
  }
}


#ifdef HAVE_OPTPP
void GaussProcApproximation::
negloglik(int mode, int n, const Teuchos::SerialDenseVector<int, double>& x, Real& fx, 
	  Teuchos::SerialDenseVector<int, double>& grad_f, int& result_mode)
{
  for (size_t i=0; i<n; i++)
    GPinstance->thetaParams[i] = x(i+1);
  GPinstance->get_cov_matrix();
  GPinstance->get_cholesky_factor(); //get the factored matrix only once
  fx = GPinstance->calc_nll();
  GPinstance->calc_grad_nll();
  if (mode & NLPFunction){
    result_mode = NLPFunction;
  }
  if (mode & NLPGradient){
    for (size_t i=0; i<n; i++)
      grad_f(i+1)=GPinstance->gradNegLogLikTheta(i,0);
    result_mode = NLPGradient;
  }
}


void GaussProcApproximation::
constraint_eval(int mode, int n, const Teuchos::SerialDenseVector<int, double>& x, 
		Teuchos::SerialDenseVector<int, double>& g, Teuchos::SerialDenseMatrix<int, double>& grad_c, 
		int& res_mode)
{ }
#endif //HAVE_OPTPP


void GaussProcApproximation::run_point_selection()
// Runs the point selection algorithm, which may modify numObs,
// normTrain, and trainVals accordingly.  The point selector works by starting
// out with an initial subset of the training data of size
// num_v^2+1 and iteratively adding points in an optimal fashion.
// This incurs additional computational costs associated with building
// the GP model, but it has the advantage that it will usually avoid
// situations where clusters of points or very large data sets result
// in a singular covariance matrix (for any reasonable values of
// theta).
{
  const int N_MAX_MLE = 35, N_MAX = 500, LOOP_MAX = 100;
  int counter = 0, Chol_return=0, delta_increase_count = 0, nadded;
  const Real TOLDELTA = 1e-2;
  RealArray delta(numObsAll);
  Real dmaxprev, maxdelta;
  size_t i, num_v = sharedDataRep->numVars;

  Cout << "\nUsing point selection routine..." << std::endl;

  pointsAddedIndex.resize(0);  // initialize
  initialize_point_selection();

  do {
    dmaxprev = maxdelta;
    //Cout << "\n\n\nNew point_sel iteration, n = " << numObs << std::endl;
    if (numObs<N_MAX_MLE) {
      // Note that within optimize_nll(), before each call to the
      // objective function, get_cov_matrix() and get_cholesky_factor()
      // are both called
      optimize_theta_global();
      //optimize_theta_multipoint();
#ifdef DEBUG_FULL
      Cout << "New theta values:  " << thetaParams[0] << "  " 
	   << thetaParams[1] << std::endl;
#endif
    }
    get_cov_matrix();
    get_cholesky_factor();
    pointsel_get_errors(delta);
    nadded = pointsel_add_sel(delta);
    maxdelta = maxval(delta);
    Cout << "Points: " << numObs << "  Maxdelta: " << maxdelta << std::endl;
    if (maxdelta >= dmaxprev && counter!=0) ++delta_increase_count;
    if (maxdelta < dmaxprev) delta_increase_count=0;
    ++counter;

    // Need to allow maxdelta to go up one or two times to avoid stopping too early
  } while(maxdelta > TOLDELTA && numObs < N_MAX && counter < LOOP_MAX 
	  && numObs< numObsAll && delta_increase_count<=5);

  get_process_variance(); //in case variance estimates are needed

  Cout << "Number of points used:  " << numObs << std::endl;
  Cout << "Maximum CV error at next to last iteration:  " << maxdelta;
  if (numObsAll-(numObs-nadded)<=5) 
    Cout << "  (only " << numObsAll-(numObs-nadded) << " CV test point(s))" 
	 << std::endl;
  else Cout << std::endl;

#ifdef DEBUG_FULL
  Cout << "Final theta values:\n";
  for (i=0; i<num_v; ++i) Cout << i+1 << ":  " << thetaParams[i] << std::endl;
#endif //DEBUG_FULL

  if (numObs < numObsAll/2.5 && numObs < 100*num_v) {
    Cerr << "***Possible early termination of point selection in " 
	 << "GaussProcApproximation***" << std::endl;
    Cerr << "***Only " << numObs << " of " << numObsAll 
	 << " points were used" << std::endl;
  }
}


void GaussProcApproximation::initialize_point_selection()
// This function initializes the point selection algorithm by choosing
// a small initial subset of the training points with which to start.
// A possible future improvement might be to make sure this initial
// set isn't a "cluster" -- this could make use of the various
// distance calculation routines
{
  size_t i, ninit, num_v = sharedDataRep->numVars;

  if (num_v==1) 
    ninit = (5 <= numObs) ? 5 : numObs;
  else 
    ninit = (num_v*num_v+1 <= numObs) ? num_v*num_v+1 : numObs;

  size_t q; // dimension of trend function
  q = trendFunction.numCols();

  numObs = ninit;

  normTrainPoints.reshape(numObs,num_v);
  trainValues.reshape(numObs,1);
  trendFunction.reshape(numObs,q);

  for (i=0;i<numObs;i++)
    pointsAddedIndex.push_back(i);
}


void GaussProcApproximation::pointsel_get_errors(RealArray& delta)
// Uses the current GP model to compute predictions at all of the
// training points and find the errors
{
  size_t i, j, num_v = sharedDataRep->numVars;

  RealVector xpred(num_v, false);

  for (i=0; i<numObsAll; i++) {
    for (j=0; j<num_v; j++) 
      xpred[j] = trainPoints(i,j);  
    //Here we want to pass un-normalized values to the predictor
    GPmodel_apply(xpred,false,false);
    delta[i]  = std::fabs(approxValue - trainValuesAll(i,0));
    //Cout << i << "  abs error: " << delta[i] << std::endl;
  }
}


namespace idx_table
// stuff for making a sort index.  Simply call
// idx_table:indexx(x.begin(),x.end(),indx.begin()).  x will not be
// altered, and the integer vector indx will be such that x[indx[i]]
// is in ascending order
{
   template<class T, class U>
   struct ComparePair1stDeref
   {
      bool operator()( const std::pair<T,U>& a,
		       const std::pair<T,U>& b ) const
      { return *a.first < *b.first; }
   };

   template<class IterIn, class IterOut>
   void indexx( IterIn first, IterIn last, IterOut out )
   {
      std::vector< std::pair<IterIn,int> > s( last-first );
      for( int i=0; i < s.size(); ++i )
	 s[i] = std::make_pair( first+i, i );
      std::sort( s.begin(), s.end(),
		 ComparePair1stDeref<IterIn,int>() );
      for( int i=0; i < s.size(); ++i, ++out )
	 *out = s[i].second;
   }
}


int GaussProcApproximation::pointsel_add_sel(const RealArray& delta)
// Uses unsorted errors in delta, finds which points to add, and adds them
{
  size_t ntest, i, j, itest, nadded, num_v = sharedDataRep->numVars;

  int chol_return;
  IntArray added_index(0), indx(numObsAll);
  const Real alph = 0.05;
  Real Rmax, dist;
  RealVector xp(num_v);

  RealMatrix xmat    = normTrainPoints;
  RealMatrix xallmat = normTrainPointsAll;

  idx_table::indexx(delta.begin(),delta.end(),indx.begin());

  ntest = int(alph*(numObsAll-numObs));
#ifdef DEBUG_FULL
  Cout << "Max delta:  " << delta[indx[numObsAll-1]] << std::endl;
#endif

  addpoint(indx[numObsAll-1],added_index);
  Rmax = getRmax(xmat);

  //Now test remaining 100*alph % points for closeness criterion
  for (i=3; i<ntest+3; i++) {
    itest = indx[numObsAll-i+1];
    for (j=0; j<num_v; j++) xp[j] = normTrainPointsAll(itest,j);
    dist = mindistindx(xp,xallmat,added_index);
    if (dist > 0.5*Rmax || added_index.size()==0) addpoint(itest,added_index);
  }

  if (added_index.size()>0)
    nadded = added_index.size()-1;
  else 
    nadded = 0;

#ifdef DEBUG_FULL
  Cout << "Added " << nadded << " of " << ntest << " new points" << "\n"; 
  // The - 1 above is to not count the first point added (the maxdelta point)
  Cout << "Total points:  " << numObs << "\n" << "\n" << std::endl;
#endif

  get_cov_matrix();
  chol_return = get_cholesky_factor();
  get_beta_coefficients();

  return nadded+1;
}


Real GaussProcApproximation::maxval(const RealArray& x) const
{
  Real result;
  for (size_t i=0; i<x.size(); i++) {
    if (x[i]>result || i==0) 
      result = x[i];
  }
  return result;
}


int GaussProcApproximation::addpoint(int pnum, IntArray& added_index)
// Adds a point to the effective training set.  Returns 1 on success.
{
  size_t i, q = trendFunction.numCols(), num_v = sharedDataRep->numVars;

  bool already_in = false;
  // JMM: shouldn't need this already_in check b/c delta should be 0
  // (dist criteria only checks against points in the local addindx
  // set).  However, delta!=0 may indicate numerical problems with the
  // covariance

  for (i=0; i<numObs; i++) {
    if (pnum==pointsAddedIndex[i]) 
      already_in = true;
  }

  if (!already_in) {

    numObs++;

    normTrainPoints.reshape(numObs,num_v);
    trendFunction.reshape(numObs,q);
    trainValues.reshape(numObs,1);

    for (i=0; i<num_v; i++) 
      normTrainPoints(numObs-1,i) = normTrainPointsAll(pnum,i);

    for (i=0; i<q; i++) 
      trendFunction(numObs-1,i) = trendFunctionAll(pnum,i);

    trainValues(numObs-1,0) = trainValuesAll(pnum,0);

    added_index.push_back(pnum);
    pointsAddedIndex.push_back(pnum);

    return 1;
  }
  else {
#ifdef DEBUG_FULL
    Cout << "Attempted to add a point already in the set: " << pnum << std::endl;
#endif
    return 0;
  }
}


void GaussProcApproximation::pointsel_write_points()
{
// Writes the training set before and after point selection, so they
// may be easily compared
  writex("daktx.txt");
  run_point_selection();
  writex("daktxsel.txt");
}


void GaussProcApproximation::lhood_2d_grid_eval()
{
  // Can be used to evaluate the likelihood on a grid (for problems
  // with two dimensional input
  Real lb = 1e-9, ub = 1.0, nll, eps = 0.005;
  if (sharedDataRep->numVars != 2) {
    Cerr << "lhood_2d_grid_eval is only for two-dimensional problems\n";
    return;
  }
  thetaParams[0] = lb;
  thetaParams[1] = lb;
  std::ofstream fout("lhood.txt");
  do {
    do {
      get_cov_matrix();
      get_cholesky_factor();
      nll = calc_nll();
      if (nll > 10000) 
	nll = 1000.0;
      fout << thetaParams[0] << " " << thetaParams[1] << " " << nll << std::endl;
      thetaParams[0] += eps;
    } while (thetaParams[0] < ub);
    thetaParams[1] += eps;
    thetaParams[0] = lb;
  } while (thetaParams[1] < ub);
  fout.close();
}


void GaussProcApproximation::writeCovMat(char filename[])
{
  std::ofstream fout(filename);
  size_t i,j;
  for (i=0; i<numObs; i++) {
    for (j=0; j<numObs; j++) 
      fout << covMatrix(i,j) << "\t";
    fout << std::endl;
  }
  fout.close();
}
  

void GaussProcApproximation::writex(const char filename[])
{
  // writes out the current training set in original units.  This can
  // be used to compare the selected subset to the entire set by
  // calling this before and after point selection with different
  // filenames.  Also, we convert back to original units before
  // writing.
  std::ofstream fout(filename);
  size_t i, j, num_v = sharedDataRep->numVars;

  //RealMatrix x_orig_units(normTrainPoints);
  Real xij;
  for (i=0; i<numObs; i++) {
    for (j=0; j<num_v; j++) {
      xij = normTrainPoints(i,j)*trainStdvs(j) + trainMeans(j);
      fout << xij << "\t";
    }
    fout << std::endl;
  }
  fout.close();
}

} // namespace Dakota
