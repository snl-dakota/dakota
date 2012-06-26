/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       LeastSq
//- Description: Implementation code for the LeastSq class
//- Owner:       Mike Eldred
//- Checked by:

#include "system_defs.h"
#include "data_io.h"
#include "tabular_io.h"
#include "DakotaModel.H"
#include "DakotaLeastSq.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "ProblemDescDB.H"
#include "RecastModel.H"
#include "Teuchos_LAPACK.hpp"
#include "pecos_stat_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaLeastSq.C 7031 2010-10-22 16:23:52Z mseldre $";


using namespace std;

namespace Dakota {
  extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
LeastSq* LeastSq::leastSqInstance(NULL);

/** This constructor extracts the inherited data for the least squares
    branch and performs sanity checking on gradient and constraint
    settings. */
LeastSq::LeastSq(Model& model): Minimizer(model),
  numLeastSqTerms(probDescDB.get_sizet("responses.num_least_squares_terms")),
  weightFlag(!model.primary_response_fn_weights().empty()),
  obsDataFilename(probDescDB.get_string("responses.exp_data_filename")),
  obsDataFlag(!obsDataFilename.empty())
{
  // Check for proper function definition
  if (numLeastSqTerms <= 0) {
    Cerr << "\nError: number of least squares terms must be greater than zero "
         << "for least squares methods." << std::endl;
    abort_handler(-1);
  }

  // read observation data to compute least squares residuals if specified
  if (obsDataFlag) {

    // These may be promoted to members once we use state vars / sigma
    size_t num_experiments = probDescDB.get_sizet("responses.num_experiments");
    size_t num_config_vars_read = 
      probDescDB.get_sizet("responses.num_config_vars");
    size_t num_sigma_read = 
      probDescDB.get_sizet("responses.num_std_deviations");

    if (num_experiments > 1 && outputLevel >= QUIET_OUTPUT)
      Cout << "\nWarning (least squares): num_experiments > 1 unsupported; " 
	   << "only first will be used." << std::endl;
    if (num_config_vars_read > 0 && outputLevel >= QUIET_OUTPUT)
      Cout << "\nWarning (least squares): experimental_config_variables " 
	   << "will be read from file, but ignored." << std::endl;

    // a matrix with numExperiments rows and cols
    // numExpConfigVars X, numFunctions Y, [1 or numFunctions Sigma]
    RealMatrix experimental_data;
    
    size_t num_cols = num_config_vars_read + numLeastSqTerms + num_sigma_read;

    bool annotated = probDescDB.get_bool("responses.exp_data_file_annotated");

    TabularIO::read_data_tabular(obsDataFilename, "Least Squares", 
				 experimental_data, num_experiments, num_cols, 
				 annotated);

    // copy the y portion of the data to obsData
    obsData.resize(numLeastSqTerms);
    for (int y_ind = 0; y_ind < numLeastSqTerms; ++y_ind) {
      obsData[y_ind] = experimental_data(0, num_config_vars_read + y_ind);
      Cout << obsData[y_ind] << std::endl;
    }

    // weight the terms with sigma from the file if active
    if (num_sigma_read > 0) {
      if (weightFlag) {
	Cerr << "\nError: both weights and experimental standard deviations "
	     << "specified in Dakota::LeastSq." << std::endl;
	abort_handler(-1);
      }
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "\nLeast squares: weighting least squares terms with 1 / square"
	     << " of standard deviations " << "read from file." << std::endl;
      RealVector lsq_weights(numLeastSqTerms);
      if (num_sigma_read == 1) {
	double sigma = 
	  experimental_data(0, num_config_vars_read + numLeastSqTerms);
	lsq_weights = std::pow(sigma, -2.);
      }
      else if (num_sigma_read == numLeastSqTerms) {
	for (size_t i=0; i<numLeastSqTerms; ++i) {
	  double sigma = 
	    experimental_data(0, num_config_vars_read + numLeastSqTerms + i);
	  lsq_weights[i] = std::pow(sigma, -2.);
	}
      }
      else {
	Cerr << "\nError: std_deviations read needs to be length 1 or number "
	     << "of calibration_terms in Dakota::LeastSq." << std::endl;
	abort_handler(-1);
      }
      model.primary_response_fn_weights(lsq_weights);
      weightFlag = true;
    }
  }

  // set minimizer data for number of functions or least squares terms
  // and then instantiate RecastModel as necessary
  numIterPrimaryFns = numUserPrimaryFns = numLeastSqTerms;
  optimizationFlag  = false;
  if (weightFlag || scaleFlag || obsDataFlag){

    // user-space model becomes the sub-model of a RecastModel:
    SizetArray recast_vars_comps_total; // default: empty; no change in size
    iteratedModel.assign_rep(new
      RecastModel(model, recast_vars_comps_total, numLeastSqTerms,
		  numNonlinearConstraints, numNonlinearIneqConstraints), false);

    if (scaleFlag)
      initialize_scaling();

    // setup recast model mappings and flags
    // recast map is all one to one for least squares
    size_t i;
    Sizet2DArray var_map_indices(numContinuousVars), 
                 primary_resp_map_indices(numLeastSqTerms), 
                 secondary_resp_map_indices(numNonlinearConstraints);
    bool nonlinear_vars_map = false;
    BoolDequeArray nonlinear_resp_map(numFunctions);

    for (i=0; i<numContinuousVars; ++i) {
      var_map_indices[i].resize(1);
      var_map_indices[i][0] = i;
      if (varsScaleFlag && (cvScaleTypes[i] & SCALE_LOG))
	nonlinear_vars_map = true;
    }
    for (i=0; i<numLeastSqTerms; ++i) {
      primary_resp_map_indices[i].resize(1);
      primary_resp_map_indices[i][0] = i;
      nonlinear_resp_map[i].resize(1);
      nonlinear_resp_map[i][0] = primaryRespScaleFlag && 
	(responseScaleTypes[i] & SCALE_LOG);
    }
    for (i=0; i<numNonlinearConstraints; i++) {
      secondary_resp_map_indices[i].resize(1);
      secondary_resp_map_indices[i][0] = numLeastSqTerms + i;
      nonlinear_resp_map[numLeastSqTerms + i].resize(1);
      nonlinear_resp_map[numLeastSqTerms + i][0] = secondaryRespScaleFlag &&
	(responseScaleTypes[numLeastSqTerms + i] & SCALE_LOG);
    }

    // complete initialization of the RecastModel after alternate construction
    // may need response recast when variables are scaled (for grad, hess)
    void (*vars_recast) (const Variables&, Variables&)
      = (varsScaleFlag) ? variables_recast : NULL;
    void (*pri_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (weightFlag || obsDataFlag || primaryRespScaleFlag || varsScaleFlag) ? 
      primary_resp_recast : NULL;
    void (*sec_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (secondaryRespScaleFlag || varsScaleFlag) ? 
      secondary_resp_recast : NULL;
    RecastModel* recast_model_rep = (RecastModel*)iteratedModel.model_rep();
    recast_model_rep->initialize(var_map_indices, nonlinear_vars_map,
				 vars_recast, NULL, primary_resp_map_indices,
				 secondary_resp_map_indices, nonlinear_resp_map,
				 pri_resp_recast, sec_resp_recast);
    // since all LeastSq iterators are currently gradient-based, maxConcurrency
    // has already been defined in the Iterator initializer list, so init here:
    iteratedModel.init_communicators(maxConcurrency);
  }
  else
    iteratedModel = model;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


LeastSq::LeastSq(NoDBBaseConstructor, Model& model):
  Minimizer(NoDBBaseConstructor(), model),
  numLeastSqTerms(numFunctions - numNonlinearConstraints),
  weightFlag(false), //(!model.primary_response_fn_weights().empty()), // TO DO
  obsDataFlag(false)
{
  // Check for proper function definition
  if (numLeastSqTerms <= 0) {
    Cerr << "\nError: number of least squares terms must be greater than zero "
         << "for least squares methods." << std::endl;
    abort_handler(-1);
  }

  if (!model.primary_response_fn_weights().empty()) { // TO DO: support this
    Cerr << "Error: on-the-fly LeastSq instantiations do not currently support "
	 << "residual weightings." << std::endl;
    abort_handler(-1);
  }

  // set minimizer data for number of functions or least squares terms
  // and then instantiate RecastModel as necessary
  numIterPrimaryFns = numUserPrimaryFns = numLeastSqTerms;
  optimizationFlag  = false;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


/** Redefines default iterator results printing to include nonlinear
    least squares results (residual terms and constraints). */
void LeastSq::print_results(std::ostream& s)
{
  // Print best design parameters.  Could just print all of best variables 
  // (as in ParamStudy::print_results), but restrict to just design 
  // parameters for the LeastSq branch.
  const Variables& best_vars = bestVariablesArray.front();
  s << "<<<<< Best parameters          =\n" << best_vars;
  //s << "<<<<< Best design parameters   =\n";
  //best_vars.continuous_variables().write(s);
  //best_vars.discrete_variables().write(s);

  const RealVector& fn_vals_star = bestResponseArray.front().function_values();
  Real t = 0.;
  for(size_t i=0; i<numLeastSqTerms; i++) {
    const Real& t1 = fn_vals_star[i];
    t += t1*t1;
  }
  s << "<<<<< Best residual norm = " << std::setw(write_precision+7)
    << std::sqrt(t) << "; 0.5 * norm^2 = " << std::setw(write_precision+7)
    << 0.5*t << '\n';

  // Print best response functions
  s << "<<<<< Best residual terms      =\n";
  write_data_partial(s, 0, numLeastSqTerms, fn_vals_star);
  size_t num_cons = numFunctions - numLeastSqTerms;
  if (num_cons) {
    s << "<<<<< Best constraint values   =\n";
    write_data_partial(s, numLeastSqTerms, num_cons, fn_vals_star);
  }

  // Print fn. eval. number where best occurred.  This cannot be catalogued 
  // directly because the optimizers track the best iterate internally and 
  // return the best results after iteration completion.  Therfore, perform a
  // search in the data_pairs cache to extract the evalId for the best fn. eval.
  int eval_id;
  activeSet.request_values(1);
  if (lookup_by_val(data_pairs, iteratedModel.interface_id(), best_vars,
		    activeSet, eval_id))
    s << "<<<<< Best data captured at function evaluation " << eval_id
      << std::endl;
  else
    s << "<<<<< Best data not found in evaluation cache" << std::endl;
 
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


/** Least squares function map from user/native space to iterator/scaled space
    using a RecastModel. If no scaling also copies constraints. */
void LeastSq::
primary_resp_recast(const Variables& native_vars,
		    const Variables& scaled_vars,
		    const Response& native_response,
		    Response& iterator_response)
{
  if (leastSqInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n-----------------------------------";
    Cout << "\nPost-processing Function Evaluation";
    Cout << "\n-----------------------------------" << std::endl;
  }

  // need to scale if primary responses are scaled or (variables are
  // scaled and grad or hess requested)
  bool scale_transform_needed = leastSqInstance->primaryRespScaleFlag ||
    leastSqInstance->need_resp_trans_byvars(
      native_response.active_set_request_vector(), 0,
      leastSqInstance->numLeastSqTerms);

  const ShortArray& asv = iterator_response.active_set_request_vector();

  // if necessary, compute residuals by subtracting observations, then scale
  if (leastSqInstance->obsDataFlag) {
    bool functions_req = false; // toggle output on function transformation
    const RealVector& fn_vals = native_response.function_values();
    // obs data plus scaling
    if (scale_transform_needed) {
      Response tmp_response = native_response.copy();
      for (size_t i=0; i<leastSqInstance->numLeastSqTerms; i++) {
	if (asv[i] & 1) {
	  tmp_response.function_value(fn_vals[i]-leastSqInstance->obsData[i],i);
	  functions_req = true;
	}
      }
      if (leastSqInstance->outputLevel > NORMAL_OUTPUT && functions_req) {
	Cout << "Least squares data transformation:\n";
	write_data(Cout, tmp_response.function_values(),
                   tmp_response.function_labels());
	Cout << std::endl;
      }
      leastSqInstance->response_modify_n2s(native_vars, tmp_response,
        iterator_response, 0, 0, leastSqInstance->numLeastSqTerms);
    }
    // obs data only
    else {
      iterator_response.update(native_response);
      for (size_t i=0; i<leastSqInstance->numLeastSqTerms; i++) {
	if (asv[i] & 1) {
	  iterator_response.function_value(fn_vals[i] - 
					   leastSqInstance->obsData[i],i);
	  functions_req = true;
	}
      }
      if (leastSqInstance->outputLevel > NORMAL_OUTPUT && functions_req) {
	Cout << "Least squares data transformation:\n";
	write_data(Cout, iterator_response.function_values(),
                   iterator_response.function_labels());
	Cout << std::endl;
      }
    }
  }
  // scaling only
  else if (scale_transform_needed)
    leastSqInstance->response_modify_n2s(native_vars, native_response,
      iterator_response, 0, 0, leastSqInstance->numLeastSqTerms);
  // copy response
  else
    iterator_response.update_partial(0, leastSqInstance->numLeastSqTerms, 
				     native_response, 0);

  // apply any weights as necessary (consider code reuse with multi-objective?)
  // for now, recast models don't have the appropriate data for weights
  // TO DO: consider adding to the update from subordinate
  if (leastSqInstance->weightFlag) {

    const RealVector& lsq_weights = leastSqInstance->
      iteratedModel.subordinate_model().primary_response_fn_weights();

    size_t i,j,k;
    SizetMultiArray var_ids;

    const SizetArray& dvv = iterator_response.active_set_derivative_vector();
    const size_t num_deriv_vars = dvv.size(); 

    if (dvv == native_vars.continuous_variable_ids()) {
      var_ids.resize(boost::extents[native_vars.cv()]);
      var_ids = native_vars.continuous_variable_ids();
    }
    else if (dvv == native_vars.inactive_continuous_variable_ids()) {
      var_ids.resize(boost::extents[native_vars.icv()]);
      var_ids = native_vars.inactive_continuous_variable_ids();
    }
    else { // general derivatives
      var_ids.resize(boost::extents[native_vars.acv()]);
      var_ids = native_vars.all_continuous_variable_ids();
    }

    RealVector fn_vals = iterator_response.function_values_view();
    for (size_t i=0; i<leastSqInstance->numLeastSqTerms; i++) {
      // \Sum_i w_i T^2_i => residual scaling as \Sum_i [sqrt(w_i) T_i]^2
      Real wt_i = std::sqrt(lsq_weights[i]);
      // functions
      if (asv[i] & 1)
	fn_vals[i] *= wt_i;
      // gradients
      if (asv[i] & 2) {
	RealVector fn_grad = iterator_response.function_gradient_view(i);
	for (j=0; j<num_deriv_vars; ++j) {
	  size_t xj_index = find_index(var_ids, dvv[j]);
	  if (xj_index != _NPOS)
	    fn_grad[xj_index] *= wt_i;
	}
      }
      // hessians
      if (asv[i] & 4) {
	RealSymMatrix fn_hess = iterator_response.function_hessian_view(i);
	for (j=0; j<num_deriv_vars; ++j) {
	  size_t xj_index = find_index(var_ids, dvv[j]);
	  if (xj_index != _NPOS) {
	    for (k=0; k<=j; ++k) {
	      size_t xk_index = find_index(var_ids, dvv[k]);
	      if (xk_index != _NPOS)
		fn_hess(xj_index,xk_index) *= wt_i;
	    }
	  }
	}
      }
    } // loop over least squares terms

    if (leastSqInstance->outputLevel > NORMAL_OUTPUT)
      Cout << "Least squares weighting transformation:\n" << iterator_response 
	   << std::endl;
  }
}


/** This function should be invoked (or reimplemented) by any derived
    implementations of initialize_run() (which would otherwise hide
    it). */
void LeastSq::initialize_run()
{
  Minimizer::initialize_run();

  // pull any late updates into the RecastModel
  if (scaleFlag || obsDataFlag)
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
  // scaling transformation needs to be performed on each best point
  size_t num_points = bestVariablesArray.size();
  if (num_points != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
	 << std::endl;
    abort_handler(-1);
  }

  for (size_t point_index = 0; point_index < num_points; ++point_index) {
    
    Variables& best_vars = bestVariablesArray[point_index];
    Response&  best_resp = bestResponseArray[point_index];

    if (varsScaleFlag)
      best_vars.continuous_variables(
        modify_s2n(best_vars.continuous_variables(), cvScaleTypes,
		   cvScaleMultipliers, cvScaleOffsets));

    // Unweight and unscale residuals. If this class applied an observed
    // data transformation to a user model, we leave that in place,
    // reporting residuals.
    // TO DO: add support for bestResponseArray derivative scaling
    if (weightFlag) {
      const RealVector& lsq_weights
	= iteratedModel.subordinate_model().primary_response_fn_weights();
      const RealVector& fn_vals = best_resp.function_values();
      for (size_t i=0; i<numLeastSqTerms; i++)
	best_resp.function_value(fn_vals[i]/std::sqrt(lsq_weights[i]),i);
    }
  
    // TODO: need to transform back if gradients and CDV scaled
    if (primaryRespScaleFlag || secondaryRespScaleFlag) {

      Response tmp_response = best_resp.copy();
      if (primaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
				 numLeastSqTerms)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, 0, 0,
			    numLeastSqTerms);
	best_resp.update_partial(0, numLeastSqTerms, tmp_response, 0);
      }
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(),
				 numLeastSqTerms, numNonlinearConstraints)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, numLeastSqTerms,
			    numLeastSqTerms, numNonlinearConstraints);
	best_resp.update_partial(numLeastSqTerms, numNonlinearConstraints,
				 tmp_response, numLeastSqTerms);
      }
    }

  }

  Iterator::post_run(s);
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
