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
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
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


/** Setup Recast for weighting model.  The weighting transformation
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
                           const Response&, Response&) = NULL;

  size_t recast_secondary_offset = numNonlinearIneqConstraints;
  SizetArray recast_vars_comps_total;  // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  const Response& curr_resp = iteratedModel.current_response();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  iteratedModel.assign_rep(new
    RecastModel(iteratedModel, var_map_indices, recast_vars_comps_total, 
		all_relax_di, all_relax_dr, nonlinear_vars_map, vars_recast,
		set_recast, primary_resp_map_indices,
		secondary_resp_map_indices, recast_secondary_offset,
		recast_resp_order, nonlinear_resp_map, pri_resp_recast,
		sec_resp_recast), false);
  ++myModelLayers;

  // This transformation consumes weights, so the resulting wrapped
  // model doesn't need them any longer, however don't want to recurse
  // and wipe out in sub-models.  Be explicit in case later
  // update_from_model() is used instead.
  bool recurse_flag = false;
  iteratedModel.primary_response_fn_weights(RealVector(), recurse_flag);

  // Preserve sense through the weighting transformation.
  const BoolDeque& submodel_sense = 
    iteratedModel.subordinate_model().primary_response_fn_sense();
  iteratedModel.primary_response_fn_sense(submodel_sense);
}


/** Redefines default iterator results printing to include nonlinear
    least squares results (residual terms and constraints). */
void LeastSq::print_results(std::ostream& s, short results_state)
{
  // archive the single best point
  size_t num_best = 1, best_ind = 0;
  archive_allocate_best(num_best);
  archive_best(best_ind, bestVariablesArray.front(), bestResponseArray.front());

  // Print best calibration parameters.  Include any inactive
  // variables unless they are used as experiment configuration
  // variables since there's no "best" configuration.
  const Variables& best_vars = bestVariablesArray.front();
  if (expData.config_vars().size() == 0)
    s << "<<<<< Best parameters          =\n" << best_vars;
  else {
    s << "<<<<< Best parameters (experiment config variables omitted) =\n";
    bool active_only = true;
    best_vars.write(s, ACTIVE_VARS);
  }

  // after post-run, responses should be back in user model space (no
  // data, scaling, or weighting)
  const RealVector& best_fns = bestResponseArray.front().function_values();

  // BMA TODO: The following is printing weight(data_trans(model)),
  // omitting scaling!
  if (calibrationDataFlag) {
    // TODO: approximate models with interpolation of field data may
    // not have recovered the correct best residuals
    DataTransformModel* dt_model_rep =
      static_cast<DataTransformModel*>(dataTransformModel.model_rep());
    dt_model_rep->print_best_responses(s, best_vars, bestResponseArray.front(),
                                       num_best, best_ind);
  }
  else {
    // the original model had least squares terms and numLeastSqTerms
    // == numTotalCalibTerms
    const RealVector& lsq_weights
      = iteratedModel.subordinate_model().primary_response_fn_weights();
    print_residuals(numUserPrimaryFns, best_fns, lsq_weights, 
		    num_best, best_ind, s);
  }

  // TODO: this may be per-experiment configuration, but there is no
  // management of it in the problem formulation.  Need to explicitly
  // disallow.
  if (numNonlinearConstraints) {
    s << "<<<<< Best constraint values   =\n";
    write_data_partial(s, numUserPrimaryFns, numNonlinearConstraints, best_fns);
  }

  // Print fn. eval. number where best occurred.  This cannot be catalogued 
  // directly because the optimizers track the best iterate internally and 
  // return the best results after iteration completion.  Therfore, perform a
  // search in the data_pairs cache to extract the evalId for the best fn. eval.

  // TODO: for multi-config, there won't be a single best would have
  // to check whether there are distinct configs

  // must search in the inbound Model's space (and even that may not
  // suffice if there are additional recastings underlying this
  // solver's Model) to find the function evaluation ID number
  Model orig_model = original_model();
  const String& interface_id = orig_model.interface_id(); 
  // use asv = 1's
  ActiveSet search_set(orig_model.num_functions(), numContinuousVars);

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
    if (expData.num_experiments() > 1) {
      s << "Warning: Confidence intervals may be inaccurate when "
        << "num_experiments > 1\n";
    }

    s << "Confidence Intervals on Calibrated Parameters:\n";

    StringMultiArrayConstView cv_labels
      = iteratedModel.continuous_variable_labels();
    for (size_t i = 0; i < numContinuousVars; i++)
      s << std::setw(14) << cv_labels[i] << ": [ "
	<< setw(write_precision+6) << confBoundsLower[i] << ", "
	<< setw(write_precision+6) << confBoundsUpper[i] << " ]\n";
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

  // pull any late updates into the RecastModel; may need to update
  // from the underlying user model in case of hybrid methods, so
  // should recurse through any local transformations
  if (myModelLayers > 0)
    iteratedModel.update_from_subordinate_model(myModelLayers-1);

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

  // TODO: no LSQ methods return multiple points, so consider removing this
  for (size_t point_index = 0; point_index < num_points; ++point_index) {
    
    // best_vars / best_resp will now contain residuals and parameters as the
    // LSQ solver (iterator) saw them
    Variables& best_vars = bestVariablesArray[point_index];
    Response&  best_resp = bestResponseArray[point_index];

    // Cache the iterator space vars before storing the native in best
    const RealVector iter_cv
      (Teuchos::Copy, best_vars.continuous_variables().values(),
       best_vars.continuous_variables().length());

    /// Transform variables back to inbound model, before any potential lookup
    if (scaleFlag) {
      ScalingModel* scale_model_rep = 
        static_cast<ScalingModel*>(scalingModel.model_rep());
      best_vars.continuous_variables
        (scale_model_rep->cv_scaled2native(best_vars.continuous_variables()));
    }

    // Only calculate confidence intervals for the first point, before
    // mucking with best_resp.  Here, best_vars is native space
    if (point_index == 0)
      get_confidence_intervals(iter_cv, best_vars);

    if (calibrationDataFlag && expData.interpolate_flag()) {
      // When interpolation is active, best we can do is a lookup to
      // get the original native Model response function.  This will
      // fail for surrogate models; need approx eval DB
      local_recast_retrieve(best_vars, best_resp);
    }
    else {
      // Derived classes populated best response with first
      // experiment's block of residuals as seen by the
      // solver. Reverse transformations on each point in best data:
      // unweight, unscale, restore data

      // BMA TODO: This requires fixing; when there is a data
      // transformation, size(fn_vals) != size(lsq_weights).  Why does
      // best_fns only contain the original user space functions at
      // this call point? The previous convention of storing
      // user-space best residuals in bestResponseArray, e.g., in
      // NL2SOLLeastSq, may not be sound any longer.
      if (weightFlag) {
        // the weighting transformation consumes weights; get some sub-model
        const RealVector& lsq_weights
          = iteratedModel.subordinate_model().primary_response_fn_weights();
        const RealVector& fn_vals = best_resp.function_values();
        for (size_t i=0; i<numLeastSqTerms; i++)
          best_resp.function_value(fn_vals[i]/std::sqrt(lsq_weights[i]),i);
      }
  
      // unscaling should be based on correct size in data difference case
      // scaling does not work in conjunction with data diff...
      if (scaleFlag) {
        // ScalingModel manages which transformations are needed
        ScalingModel* scale_model_rep = 
          static_cast<ScalingModel*>(scalingModel.model_rep());
        scale_model_rep->resp_scaled2native(best_vars, best_resp);
      }

      if (calibrationDataFlag) {
        // restore residuals to original model's primary responses
        // (leaving any constraints)
        // BMA TODO: verify that numUserPrimaryFns is correct
        RealVector resid_fns = best_resp.function_values_view();
        expData.recover_model(numUserPrimaryFns, resid_fns);
      }
    }
    
  }

  Minimizer::post_run(s);
}


/** Calculate individual confidence intervals for each parameter,
    based on a linear approximation of the nonlinear model. iter_cv
    are needed to eval iteratedModel gradients, while native_cv are
    needed for final reporting.  best_resp must contain the final
    differenced, scaled, weighted residuals. */
void LeastSq::get_confidence_intervals(const RealVector& iter_cv,
				       const Variables& native_vars)
{
  // TODO: Fix CIs for interpolation and multi-experiment cases.  For
  // simple multi-experiment cases, can just use the model derivatives
  // without replication.  Concern is with singular aggregate Jacobian
  // matrix J.

  // Confidence intervals should be based on weighted/scaled iterator
  // residuals (since that was the nonlinear regression problem
  // formulation), but original user parameters, so must use
  // d(scaled_residual) / d(native_vars).
  if (vendorNumericalGradFlag) {
    Cout << "\nWarning: Confidence Interval calculations are not available"
         << "\n         for vendor numerical gradients.\n\n";
    return;
  }
  if (numLeastSqTerms < numContinuousVars) {
    Cout << "\nWarning: Confidence Interval calculations are not available"
         << "\n         when number of residuals is less than number of"
	 << "\n         variables.\n\n";
    return;
  }
  if (bestVariablesArray.size() > 1) {
    Cout << "\nWarning: Multiple solutions found, but confidence intervals are "
	 << "only calculated\n         for the single best point." << std::endl;
  }

  // Due to all the complicated use cases for residual recovery, we
  // opt to just re-evaluate the necessary models here, performing fn
  // and grad evals separately, in case they are cached in different
  // evals

  // recover the iterator space residuals, hopefully via duplicate detection
  short asv_request = 1;
  iteratedModel.continuous_variables(iter_cv);
  activeSet.request_values(asv_request);
  iteratedModel.evaluate(activeSet);
  const RealVector& resid_star =
    iteratedModel.current_response().function_values();

  // first calculate the estimate of sigma-squared.
  Real dof = std::max(double(numLeastSqTerms-numContinuousVars), 1.);
  Real sse_total = 0.;
  for (int i=0; i<numLeastSqTerms; i++)
    sse_total += (resid_star[i]*resid_star[i]);
  Real sigma_sq_hat = sse_total/dof;
 
  // We are using a formulation where the standard error of the
  // parameter vector is calculated as the square root of the diagonal
  // elements of sigma_sq_hat*inverse(J'J), where J is the matrix
  // of derivatives of the model with respect to the parameters,
  // and J' is the transpose of J.  Insteaad of calculating J'J and
  // explicitly taking the inverse, we are using a QR decomposition,
  // where J=QR, and inv(J'J)= inv((QR)'QR)=inv(R'Q'QR)=inv(R'R)=
  // inv(R)*inv(R').
  // J must be in column order for the Fortran call
  Teuchos::LAPACK<int, Real> la;
  int info;
  int M = numLeastSqTerms;
  int N = numContinuousVars;
  int LDA = numLeastSqTerms;
  double *tau, *work;
  double* Jmatrix = new double[numLeastSqTerms*numContinuousVars];
  
  // Unfortunately, best_resp may not contain gradients, so we
  // evaluate the iteratedModel at the iterator variables, aiming to
  // catch a duplicate eval .  This evaluation will potentially yield
  // d(scaled_resp) / d(scaled_params).
  asv_request = 2;
  iteratedModel.continuous_variables(iter_cv);
  activeSet.request_values(asv_request);
  iteratedModel.evaluate(activeSet);

  // When parameters are scaled, have to apply the variable
  // transformation Jacobian to get to
  // d(scaled_resp) / d(native_params) =
  //   d(scaled_resp) / d(scaled_params) * d(scaled_params) / d(native_params)

  // envelope to hold the either unscaled or iterator response
  Response ultimate_resp;
  if (scaleFlag) {
    const Response& scaled_resp = iteratedModel.current_response();
    ultimate_resp = scaled_resp.copy();
    ScalingModel* scale_model_rep =
      static_cast<ScalingModel*>(scalingModel.model_rep());
    bool unscale_resp = false;
    scale_model_rep->response_modify_s2n(native_vars, scaled_resp,
					 ultimate_resp, 0, numLeastSqTerms,
					 unscale_resp);
  }
  else
    ultimate_resp = iteratedModel.current_response();
  const RealMatrix& fn_grads = ultimate_resp.function_gradients_view();

  // BMA: TODO we don't need to transpose this matrix...
  for (int i=0; i<numLeastSqTerms; i++)
    for (int j=0; j<numContinuousVars; j++)
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
  for (int i=0; i<numContinuousVars; i++) {
    for (int j=i; j<numContinuousVars; j++)
      diag(i) += Jmatrix[j*numLeastSqTerms+i]*Jmatrix[j*numLeastSqTerms+i];
    standard_error[i] = std::sqrt(diag(i)*sigma_sq_hat);
  }
  delete[] Jmatrix;

  confBoundsLower.sizeUninitialized(numContinuousVars);
  confBoundsUpper.sizeUninitialized(numContinuousVars); 

  Pecos::students_t_dist t_dist(dof);
  Real tdist =  bmth::quantile(t_dist,0.975);

  const RealVector& native_cv = native_vars.continuous_variables();
  for (int j=0; j<numContinuousVars; j++) {
    confBoundsLower[j] = native_cv[j] - tdist * standard_error[j];
    confBoundsUpper[j] = native_cv[j] + tdist * standard_error[j];
  }
}

} // namespace Dakota
