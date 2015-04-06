/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       LeastSq
//- Description: Implementation code for the LeastSq class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaLeastSq.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "RecastModel.hpp"
#include "Teuchos_LAPACK.hpp"
#include "pecos_stat_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaLeastSq.cpp 7031 2010-10-22 16:23:52Z mseldre $";


using namespace std;

namespace Dakota {
  extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
LeastSq* LeastSq::leastSqInstance(NULL);

/** This constructor extracts the inherited data for the least squares
    branch and performs sanity checking on gradient and constraint
    settings. */
LeastSq::LeastSq(ProblemDescDB& problem_db, Model& model):
  Minimizer(problem_db, model),
  // initial value from Minimizer as accounts for fields and transformations
  numLeastSqTerms(numUserPrimaryFns),
  weightFlag(!iteratedModel.primary_response_fn_weights().empty())
				// TODO: wrong because of recasting layers
{
  optimizationFlag  = false;

  bool err_flag = false;
  // Check for proper function definition
  if (model.primary_fn_type() != CALIB_TERMS) {
    Cerr << "\nError: model must have calibration terms to apply least squares "
	 << "methods." << std::endl;
    err_flag = true;
  }
  // Check for correct bit associated within methodName
  if ( !(methodName & LEASTSQ_BIT) ) {
    Cerr << "\nError: least squares bit not activated for method instantiation "
	 << "within LeastSq branch." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  // Initialize a best variables instance; bestVariablesArray should
  // be in calling context; so initialized before any recasts
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // Wrap the iteratedModel in 0 -- 3 RecastModels, potentially resulting
  // in weight(scale(data(model)))
  if (calibrationDataFlag) {
    data_transform_model();
    // update local data sizes
    numLeastSqTerms = numTotalCalibTerms;
  }
  if (scaleFlag)
    scale_model();
  if (weightFlag)
    weight_model();
}


LeastSq::LeastSq(unsigned short method_name, Model& model):
  Minimizer(method_name, model),
  numLeastSqTerms(numFunctions - numNonlinearConstraints),
  weightFlag(false) //(!model.primary_response_fn_weights().empty()), // TO DO
{
  bool err_flag = false;
  // Check for proper function definition
  if (numLeastSqTerms <= 0) {
    Cerr << "\nError: number of least squares terms must be greater than zero "
         << "for least squares methods." << std::endl;
    err_flag = true;
  }

  if (!model.primary_response_fn_weights().empty()) { // TO DO: support this
    Cerr << "Error: on-the-fly LeastSq instantiations do not currently support "
	 << "residual weightings." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  optimizationFlag  = false;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


/** Setup Recast for weighting model the weighting transformation
    doesn't resize, so use numUserPrimaryFns.  No vars, active set or
    secondary mapping.  All indices are one-to-one mapped (no change
    in counts). */
void LeastSq::weight_model()
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing weighting transformation" << std::endl;

  // the size of the recast is the total numLeastSqTerms being iterated
  size_t i;
  Sizet2DArray var_map_indices(numContinuousVars), 
    primary_resp_map_indices(numLeastSqTerms), 
    secondary_resp_map_indices(numNonlinearConstraints);
  bool nonlinear_vars_map = false;
  BoolDequeArray nonlinear_resp_map(numLeastSqTerms+numNonlinearConstraints);

  for (i=0; i<numContinuousVars; i++) {
    var_map_indices[i].resize(1);
    var_map_indices[i][0] = i;
  }
  for (i=0; i<numLeastSqTerms; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
    nonlinear_resp_map[i].resize(1);
    nonlinear_resp_map[i][0] = false;
  }
  for (i=0; i<numNonlinearConstraints; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = numLeastSqTerms + i;
    nonlinear_resp_map[numLeastSqTerms+i].resize(1);
    nonlinear_resp_map[numLeastSqTerms+i][0] = false;
  }

  void (*vars_recast) (const Variables&, Variables&) = NULL;
  void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
  void (*pri_resp_recast) (const Variables&, const Variables&,
			   const Response&, Response&)
    = primary_resp_weighter;
  void (*sec_resp_recast) (const Variables&, const Variables&,
			   const Response&, Response&)
    = secondary_resp_copier;

  size_t recast_secondary_offset = numNonlinearIneqConstraints;
  SizetArray recast_vars_comps_total;  // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation

  iteratedModel.assign_rep(new
    RecastModel(iteratedModel, var_map_indices, recast_vars_comps_total, 
		all_relax_di, all_relax_dr, nonlinear_vars_map, vars_recast,
		set_recast, primary_resp_map_indices,
		secondary_resp_map_indices, recast_secondary_offset,
		nonlinear_resp_map, pri_resp_recast, sec_resp_recast), false);
  ++minimizerRecasts;

  // This transformation consumes weights, so the resulting wrapped
  // model doesn't need them any longer, however don't want to recurse
  // and wipe out in sub-models.  Be explicit in case later
  // update_from_sub_model is used instead.
  bool recurse_flag = false;
  iteratedModel.primary_response_fn_weights(RealVector(), recurse_flag);

  // Preserve sense through the weighting transformation.
  const BoolDeque& submodel_sense = 
    iteratedModel.subordinate_model().primary_response_fn_sense();
  iteratedModel.primary_response_fn_sense(submodel_sense);
}


/** Redefines default iterator results printing to include nonlinear
    least squares results (residual terms and constraints). */
void LeastSq::print_results(std::ostream& s)
{
  // archive the single best point
  size_t num_best = 1, index = 0;
  archive_allocate_best(num_best);
  archive_best(index, bestVariablesArray.front(), bestResponseArray.front());

  // Print best design parameters.  Could just print all of best variables 
  // (as in ParamStudy::print_results), but restrict to just design 
  // parameters for the LeastSq branch.
  const Variables& best_vars = bestVariablesArray.front();
  s << "<<<<< Best parameters          =\n" << best_vars;
  //s << "<<<<< Best design parameters   =\n";
  //best_vars.continuous_variables().write(s);
  //best_vars.discrete_variables().write(s);

  const RealVector& fn_vals_star = bestResponseArray.front().function_values();
  const RealVector& lsq_weights
	= iteratedModel.subordinate_model().primary_response_fn_weights();
  Real t = 0.;
  for(size_t i=0; i<numLeastSqTerms; i++) {
    const Real& t1 = fn_vals_star[i];
    if (lsq_weights.empty())
      t += t1*t1;
    else  
      t += t1*t1*lsq_weights[i];
  }
  s << "<<<<< Best residual norm = " << std::setw(write_precision+7)
    << std::sqrt(t) << "; 0.5 * norm^2 = " << std::setw(write_precision+7)
    << 0.5*t << '\n';

  // Print best response functions
  if (numLeastSqTerms > 1) s << "<<<<< Best residual terms      =\n";
  else                     s << "<<<<< Best residual term       =\n";
  write_data_partial(s, (size_t)0, numLeastSqTerms, fn_vals_star);

  size_t num_cons = numFunctions - numLeastSqTerms;
  if (num_cons) {
    s << "<<<<< Best constraint values   =\n";
    write_data_partial(s, numLeastSqTerms, num_cons, fn_vals_star);
  }

  // Print fn. eval. number where best occurred.  This cannot be catalogued 
  // directly because the optimizers track the best iterate internally and 
  // return the best results after iteration completion.  Therfore, perform a
  // search in the data_pairs cache to extract the evalId for the best fn. eval.
  activeSet.request_values(1);
  PRPCacheHIter cache_it = lookup_by_val(data_pairs,
    iteratedModel.interface_id(), best_vars, activeSet);
  if (cache_it == data_pairs.get<hashed>().end())
    s << "<<<<< Best data not found in evaluation cache\n\n";
  else {
    int eval_id = cache_it->eval_id();
    if (eval_id > 0)
      s << "<<<<< Best data captured at function evaluation " << eval_id
	<< "\n\n";
    else // should not occur
      s << "<<<<< Best data not found in evaluations from current execution,"
	<< "\n      but retrieved from restart archive with evaluation id "
	<< -eval_id << "\n\n";
  }
 
  // Print confidence intervals for each estimated parameter. 
  // These CIs are based on a linear approximation of the underlying 
  // nonlinear model, and are individual CIs, not joint CIs.  
  // Currently there is no weighting matrix, but that can be added. 

  if (!confBoundsLower.empty() && !confBoundsUpper.empty()) {
    StringMultiArrayConstView cv_labels
      = iteratedModel.continuous_variable_labels();
    for (size_t i = 0; i < numContinuousVars; i++)
      s << "Confidence Interval for " << cv_labels[i] << " is [ "
	<< setw(write_precision+7) << confBoundsLower[i] << ", "
	<< setw(write_precision+7) << confBoundsUpper[i] << " ]\n";
  }
}


/** Apply weights to least squares residuals */
void LeastSq::
primary_resp_weighter(const Variables& unweighted_vars,
		      const Variables& weighted_vars,
		      const Response& unweighted_response,
		      Response& weighted_response)
{
  if (leastSqInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n--------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Weighting Residuals";
    Cout << "\n--------------------------------------------------------" 
	 << std::endl;
  }

  // TODO: is the DVV management necessary here?

  // apply any weights as necessary (consider code reuse with multi-objective?)
  // for now, recast models don't have the appropriate data for weights
  // TO DO: consider adding to the update from subordinate

  // weighting is the last transformation, so weights are available in
  // the first sub-model
  const RealVector& lsq_weights = leastSqInstance->
    iteratedModel.subordinate_model().primary_response_fn_weights();

  size_t i,j,k;
  SizetMultiArray var_ids;

  const ShortArray& asv = weighted_response.active_set_request_vector();
  const SizetArray& dvv = weighted_response.active_set_derivative_vector();
  const size_t num_deriv_vars = dvv.size(); 

  if (dvv == unweighted_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[unweighted_vars.cv()]);
    var_ids = unweighted_vars.continuous_variable_ids();
  }
  else if (dvv == unweighted_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[unweighted_vars.icv()]);
    var_ids = unweighted_vars.inactive_continuous_variable_ids();
  }
  else { // general derivatives
    var_ids.resize(boost::extents[unweighted_vars.acv()]);
    var_ids = unweighted_vars.all_continuous_variable_ids();
  }

  // TODO: review for efficiency (needless copies); can we just update
  // from the unweighted response, then modify?
  const RealVector& fn_vals = unweighted_response.function_values();
  RealVector wt_fn_vals = weighted_response.function_values_view();
  for (size_t i=0; i<leastSqInstance->numLeastSqTerms; i++) {
    // \Sum_i w_i T^2_i => residual scaling as \Sum_i [sqrt(w_i) T_i]^2
    Real wt_i = std::sqrt(lsq_weights[i]);
    // functions
    if (asv[i] & 1)
      wt_fn_vals[i] = wt_i * fn_vals[i];
    // gradients
    if (asv[i] & 2) {
      const RealMatrix& fn_grads = unweighted_response.function_gradients();
      RealMatrix wt_fn_grads = weighted_response.function_gradients_view();
      for (j=0; j<num_deriv_vars; ++j) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	if (xj_index != _NPOS)
	  wt_fn_grads(xj_index,i) = wt_i * fn_grads(xj_index,i);
      }
    }
    // hessians
    if (asv[i] & 4) {
      const RealSymMatrix& fn_hess = unweighted_response.function_hessian(i);
      RealSymMatrix wt_fn_hess = weighted_response.function_hessian_view(i);
      for (j=0; j<num_deriv_vars; ++j) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	if (xj_index != _NPOS) {
	  for (k=0; k<=j; ++k) {
	    size_t xk_index = find_index(var_ids, dvv[k]);
	    if (xk_index != _NPOS)
	      wt_fn_hess(xj_index,xk_index) = wt_i * fn_hess(xj_index,xk_index);
	  }
	}
      }
    }
  } // loop over least squares terms

  if (leastSqInstance->outputLevel > NORMAL_OUTPUT)
    Cout << "Least squares weight-transformed response:\n" << weighted_response 
	 << std::endl;

}


/** This function should be invoked (or reimplemented) by any derived
    implementations of initialize_run() (which would otherwise hide
    it). */
void LeastSq::initialize_run()
{
  Minimizer::initialize_run();

  // pull any late updates into the RecastModel
  if (scaleFlag || calibrationDataFlag)
    iteratedModel.update_from_subordinate_model(false); // recursion not reqd

  // Track any previous object instance in case of recursion.  Note that
  // leastSqInstance and minimizerInstance must be tracked separately since
  // the previous least squares method and previous minimizer could be
  // different instances (e.g., for UQ of calibrated solutions with NPSOL
  // for MPP search and NL2SOL for NLS, the previous minimizerInstance is
  // NPSOL and the previous leastSqInstance is NULL).  
  prevLSqInstance = leastSqInstance;
  leastSqInstance = this;
}


/** Implements portions of post_run specific to LeastSq for scaling
    back to native variables and functions.  This function should be
    invoked (or reimplemented) by any derived implementations of
    post_run() (which would otherwise hide it). */
void LeastSq::post_run(std::ostream& s)
{
  size_t num_points = bestVariablesArray.size();
  if (num_points != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
	 << std::endl;
    abort_handler(-1);
  }

  for (size_t point_index = 0; point_index < num_points; ++point_index) {
    
    Variables& best_vars = bestVariablesArray[point_index];
    Response&  best_resp = bestResponseArray[point_index];

    // Reverse transformations on each point in best data: unweight,
    // unscale, but leave differenced with data (for LeastSq problems,
    // always report the residuals)

    if (weightFlag) {
      const RealVector& lsq_weights
	= iteratedModel.subordinate_model().primary_response_fn_weights();
      const RealVector& fn_vals = best_resp.function_values();
      for (size_t i=0; i<numLeastSqTerms; i++)
	best_resp.function_value(fn_vals[i]/std::sqrt(lsq_weights[i]),i);
    }

    if (varsScaleFlag)
      best_vars.continuous_variables(
        modify_s2n(best_vars.continuous_variables(), cvScaleTypes,
		   cvScaleMultipliers, cvScaleOffsets));
  
    if (primaryRespScaleFlag || secondaryRespScaleFlag) {
      Response tmp_response = best_resp.copy();
      if (primaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
				 numLeastSqTerms)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, 0,
			    numLeastSqTerms);
	best_resp.update_partial(0, numLeastSqTerms, tmp_response, 0);
      }
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(),
				 numLeastSqTerms, numNonlinearConstraints)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, numLeastSqTerms,
			    numNonlinearConstraints);
	best_resp.update_partial(numLeastSqTerms, numNonlinearConstraints,
				 tmp_response, numLeastSqTerms);
      }
    }

  }

  Minimizer::post_run(s);
}


/** Calculate individual confidence intervals for each parameter. 
     These bounds are based on a linear approximation of the nonlinear model. */
void LeastSq::get_confidence_intervals()
{
  if (scaleFlag) {
    Cerr << "\nWarning: Confidence Interval calculations are not available"
         << "\n         when scaling is enabled.\n\n";
    return;
  }
  if (vendorNumericalGradFlag) {
    Cerr << "\nWarning: Confidence Interval calculations are not available"
         << "\n         for vendor numerical gradients.\n\n";
    return;
  }
  if (numLeastSqTerms < numContinuousVars) {
    Cerr << "\nWarning: Confidence Interval calculations are not available"
         << "\n         when number of residuals is less than number of"
	 << "\n         variables.\n\n";
    return;
  }

  const RealVector& best_c_vars
    = bestVariablesArray.front().continuous_variables();

  Real sigma_sq_hat; 
  Real sse_total = 0.;
  Real dof = std::max(double(numLeastSqTerms-numContinuousVars), 1.); 
  int i, j;
 
  Teuchos::LAPACK<int, Real> la;

  //first calculate the estimate of sigma-squared.  
  const RealVector& fn_vals_star = bestResponseArray.front().function_values();
  for (i=0; i<numLeastSqTerms; i++)
    sse_total += (fn_vals_star[i]*fn_vals_star[i]);
  sigma_sq_hat = sse_total/dof;
 
  int info;
  int M = numLeastSqTerms;
  int N = numContinuousVars;
  int LDA = numLeastSqTerms;
  double *tau, *work;
  double* Jmatrix = new double[numLeastSqTerms*numContinuousVars];
  
  short asv_request = 2;
  iteratedModel.continuous_variables(best_c_vars);
  activeSet.request_values(asv_request);
  iteratedModel.compute_response(activeSet);
  const RealMatrix& fn_grads
    = iteratedModel.current_response().function_gradients();

  // We are using a formulation where the standard error of the 
  // parameter vector is calculated as the square root of the diagonal 
  // elements of sigma_sq_hat*inverse(J'J), where J is the matrix 
  // of derivatives of the model with respect to the parameters, 
  // and J' is the transpose of J.  Insteaad of calculating J'J and 
  // explicitly taking the inverse, we are using a QR decomposition, 
  // where J=QR, and inv(J'J)= inv((QR)'QR)=inv(R'Q'QR)=inv(R'R)=
  // inv(R)*inv(R'). 
  // J must be in column order for the Fortran call
  for (i=0; i<numLeastSqTerms; i++)
    for (j=0; j<numContinuousVars; j++)
      Jmatrix[(j*numLeastSqTerms)+i]=fn_grads(j,i);

  // This is the QR decomposition, the results are returned in J
  work = new double[N + std::min(M,N)];
  tau = work + N;

  la.GEQRF(M,N,Jmatrix,LDA,tau,work,N,&info);
  bool error_flag = info;
  delete[] work;

  // if you add these three lines right after DGEQRF, then the upper triangular
  // part of Jmatrix will then contain Rinverse

  char uplo = 'U'; // upper triangular
  char unitdiag = 'N'; // non-unit trangular
  la.TRTRI(uplo, unitdiag, N, Jmatrix, LDA, &info); 
  error_flag &= info;

  if (error_flag) {
    Cerr << "\nWarning: LAPACK error computing confidence intervals.\n\n";
    return;
  }

  RealVector standard_error(numContinuousVars);
  RealVector diag(numContinuousVars, true);
  for (i=0; i<numContinuousVars; i++) {
    for (j=i; j<numContinuousVars; j++)
      diag(i) += Jmatrix[j*numLeastSqTerms+i]*Jmatrix[j*numLeastSqTerms+i];
    standard_error[i] = std::sqrt(diag(i)*sigma_sq_hat);
  }
  delete[] Jmatrix;

  confBoundsLower.sizeUninitialized(numContinuousVars);
  confBoundsUpper.sizeUninitialized(numContinuousVars); 
//#ifdef HAVE_BOOST
  Pecos::students_t_dist t_dist(dof);
  Real tdist =  bmth::quantile(t_dist,0.975);
/*
#elif HAVE_GSL
  Real tdist = gsl_cdf_tdist_Pinv(0.975,dof);
#else
  Real tdist = 0.;
  Cerr << "\nWarning: Confidence Interval calculations are not available"
       << "\n         (DAKOTA configured without GSL or BOOST).\n\n";
#endif // HAVE_GSL or HAVE_BOOST
*/
  for (j=0; j<numContinuousVars; j++) {
    confBoundsLower[j] = best_c_vars[j] - tdist * standard_error[j];
    confBoundsUpper[j] = best_c_vars[j] + tdist * standard_error[j];
  }
}

} // namespace Dakota
