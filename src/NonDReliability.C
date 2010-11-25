/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDReliability
//- Description: Implementation code for NonDReliability class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "system_defs.h"
#include "ProblemDescDB.H"
#include "NonDReliability.H"
#include "DakotaGraphics.H"


static const char rcsId[] = "@(#) $Id: NonDReliability.C 6793 2010-05-21 00:07:25Z mseldre $";

namespace Dakota {

// initialization of statics
NonDReliability* NonDReliability::nondRelInstance(NULL);


NonDReliability::NonDReliability(Model& model): NonD(model),
  integrationRefinement(NO_INT_REFINE), numRelAnalyses(0)
  //refinementSamples(probDescDB.get_int("method.samples")),
  //refinementSeed(probDescDB.get_int("method.random_seed"))
{
  bool err_flag = false;

  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    err_flag = true;
  }

  natafTransform = Pecos::ProbabilityTransformation("nataf");
  const RealSymMatrix& uncertain_corr
    = model.distribution_parameters().uncertain_correlations();
  if (!uncertain_corr.empty()) {
    natafTransform.initialize_random_variable_correlations(uncertain_corr);
    if (numContDesVars || numContEpistUncVars || numContStateVars)
      // expand ProbabilityTransformation::corrMatrixX to include design + state
      // + epistemic uncertain vars.  TO DO: propagate through model recursion?
      natafTransform.reshape_correlation_matrix(numContDesVars,
	numContAleatUncVars, numContEpistUncVars+numContStateVars);
  }

  initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below

  // Check for correlations among variable types (bounded normal, bounded
  // lognormal, loguniform, triangular, beta, and histogram) that are not
  // supported by Der Kiureghian & Liu for correlation warping estimation when
  // transforming to std normals.  See also related logic in NonDExpansion.
  const Pecos::ShortArray& x_types = natafTransform.x_types();
  if (natafTransform.x_correlation()) {
    const Pecos::RealSymMatrix& x_corr = natafTransform.x_correlation_matrix();
    size_t i, j;
    bool distribution_error = false;
    for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i)
      if ( x_types[i] == Pecos::BOUNDED_NORMAL    ||
	   x_types[i] == Pecos::BOUNDED_LOGNORMAL ||
	   x_types[i] == Pecos::LOGUNIFORM || x_types[i] == Pecos::TRIANGULAR ||
	   x_types[i] == Pecos::BETA || x_types[i] == Pecos::HISTOGRAM_BIN )
	for (j=numContDesVars; j<numContDesVars+numContAleatUncVars; ++j)
	  if (i != j && std::fabs(x_corr(i, j)) > 1.e-25)
	    { distribution_error = true; break; }
    if (distribution_error) {
      Cerr << "Error: correlation warping for Nataf variable transformation of "
	   << "bounded normal,\n       bounded lognormal, loguniform, "
	   << "triangular, beta, and histogram bin\n       distributions is "
	   << "not currently supported in NonDExpansion." << std::endl;
      err_flag = true;
    }
  }
  if (err_flag)
    abort_handler(-1);

  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);

  // initialize finalStatistics using the default statistics set
  initialize_final_statistics();
}


NonDReliability::~NonDReliability()
{ }


void NonDReliability::initialize_graphics(bool graph_2d, bool tabular_data,
					  const String& tabular_file)
{
  // Set up special graphics for CDF/CCDF visualization
  if (totalLevelRequests) {
    extern Graphics dakota_graphics; // defined in ParallelLibrary.C
    // Customizations must follow 2D plot initialization (setting axis labels
    // calls SciPlotUpdate) and must precede tabular data file initialization
    // (so that the file header includes any updates to tabularCntrLabel).
    if (graph_2d) {     // initialize the 2D plots
      dakota_graphics.create_plots_2d(iteratedModel.current_variables(),
				      iteratedModel.current_response());
      // Visualize mostProbPointX in the vars windows and CDF/CCDF
      // probability/reliability-response level pairs in the response windows.
      dakota_graphics.set_x_labels2d("Response Level");
      size_t i;
      for (i=0; i<numFunctions; i++)
	dakota_graphics.set_y_label2d(i, "Probability");
      for (i=0; i<numUncertainVars; i++)
	dakota_graphics.set_y_label2d(i+numFunctions, "Most Prob Point");
    }

    //if (tabular_data) { // initialize the tabular data file
    //  dakota_graphics.tabular_counter_label("z");
    //  dakota_graphics.create_tabular_datastream(
    //    iteratedModel.current_variables(), iteratedModel.current_response(),
    //    tabular_file);
    //}
  }
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an RIA objective function. */
void NonDReliability::
RIA_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // ----------------------------------------
  // The RIA objective function is (norm u)^2
  // ----------------------------------------

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  const RealVector&     u = recast_vars.continuous_variables();
  size_t i, num_vars = u.length();
  short asv_val = recast_asv[0];
  if (asv_val & 1) {
    Real f = 0.;
    for (i=0; i<num_vars; i++)
      f += std::pow(u[i], 2); // f = u'u
    recast_response.function_value(f, 0);
  }
  if (asv_val & 2) {
    RealVector grad_f(num_vars);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
    recast_response.function_gradient(grad_f, 0);
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f(num_vars); hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
    recast_response.function_hessian(hess_f, 0);
  }

  // Using f = norm u is a poor choice, since grad f is undefined at u = 0.
  //Real sqrt_sum_sq = std::sqrt(sum_sq);
  //if (sqrt_sum_sq > 0.)
  //  grad_f[i] = u[i]/sqrt_sum_sq;
  //else // gradient undefined at origin, use 0. to keep optimizer happy
  //  grad_f[i] = 0.;
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an RIA equality constraint. */
void NonDReliability::
RIA_constraint_eval(const Variables& sub_model_vars,
		    const Variables& recast_vars,
		    const Response& sub_model_response,
		    Response& recast_response)
{
  // --------------------------------------------------------
  // The RIA equality constraint is G(u) - response level = 0
  // --------------------------------------------------------

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  short asv_val = recast_asv[1]; //sub_model_asv[nondRelInstance->respFnCount];
  if (asv_val & 1) {
    const Real& sub_model_fn
      = sub_model_response.function_values()[nondRelInstance->respFnCount];
    Real c = sub_model_fn - nondRelInstance->requestedRespLevel;
    recast_response.function_value(c, 1);
  }
  if (asv_val & 2) { // dG/du: no additional transformation needed
    RealVector sub_model_grad
      = sub_model_response.function_gradient(nondRelInstance->respFnCount);
    recast_response.function_gradient(sub_model_grad, 1);
  }
  if (asv_val & 4) { // d^2G/du^2: no additional transformation needed
    const RealSymMatrix& sub_model_hessian
      = sub_model_response.function_hessians()[nondRelInstance->respFnCount];
    recast_response.function_hessian(sub_model_hessian, 1);
  }
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into an PMA objective function. */
void NonDReliability::
PMA_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // ----------------------------------
  // The PMA objective function is G(u)
  // ----------------------------------

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  short asv_val = recast_asv[0]; //sub_model_asv[nondRelInstance->respFnCount];

  // CDF probability < 0.5  -->  CDF beta > 0  -->  minimize g
  // CDF probability > 0.5  -->  CDF beta < 0  -->  maximize g
  // CDF probability = 0.5  -->  CDF beta = 0  -->  compute g
  // Note: "compute g" means that min/max is irrelevant since there should only
  // be one G(u) intersection when the circle of radius beta collapses to a pt.
  bool positive = (nondRelInstance->requestedCDFRelLevel >= 0.);

  if (asv_val & 1) {
    const Real& sub_model_fn
      = sub_model_response.function_values()[nondRelInstance->respFnCount];
    if (positive)
      recast_response.function_value( sub_model_fn, 0);
    else
      recast_response.function_value(-sub_model_fn, 0);
  }
  if (asv_val & 2) { // dG/du: no additional transformation needed
    RealVector sub_model_grad
      = sub_model_response.function_gradient(nondRelInstance->respFnCount);
    if (positive)
      recast_response.function_gradient(sub_model_grad, 0);
    else {
      size_t num_vars = sub_model_response.function_gradients().numRows();
      RealVector neg_sub_model_grad(num_vars);
      for (int i=0; i<num_vars; ++i)
	neg_sub_model_grad[i] = -sub_model_grad[i];
      recast_response.function_gradient(neg_sub_model_grad, 0);
    }
  }
  if (asv_val & 4) { // d^2G/du^2: no additional transformation needed
    const RealSymMatrix& sub_model_hessian
      = sub_model_response.function_hessians()[nondRelInstance->respFnCount];
    if (positive)
      recast_response.function_hessian(sub_model_hessian, 0);
    else {
      size_t num_vars = sub_model_hessian.numRows();
      RealSymMatrix neg_sub_model_hessian(num_vars);
      for (size_t i=0; i<num_vars; ++i)
	for (size_t j=0; j<num_vars; ++j)
	  neg_sub_model_hessian(i,j) = -sub_model_hessian(i,j);
      recast_response.function_hessian(neg_sub_model_hessian, 0);
    }
  }
}


/** This function recasts a G(u) response set (already transformed and
    approximated in other recursions) into a PMA equality constraint. */
void NonDReliability::
PMA_constraint_eval(const Variables& sub_model_vars,
		    const Variables& recast_vars,
		    const Response& sub_model_response,
		    Response& recast_response)
{
  // ------------------------------------------------------
  // The PMA equality constraint is (norm u)^2 - beta^2 = 0
  // ------------------------------------------------------

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  const RealVector&     u = recast_vars.continuous_variables();
  size_t i, num_vars = u.length();
  short asv_val = recast_asv[1];
  if (asv_val & 1) {
    // calculate the reliability index (beta)
    Real beta_sq = 0.;
    for (i=0; i<num_vars; ++i)
      beta_sq += std::pow(u[i], 2); //use beta^2 to avoid singular grad @ origin

    // Update requestedCDFRelLevel for PMA SORM with specified probability
    // levels or specified generalized reliability levels.
    nondRelInstance->update_pma_reliability_level();

    // calculate the equality constraint: u'u - beta_target^2
    Real c = beta_sq - std::pow(nondRelInstance->requestedCDFRelLevel, 2);
    recast_response.function_value(c, 1);
  }
  if (asv_val & 2) {
    // Note: for second-order integrations with a specified probability level
    // p-bar, beta-bar is a function of p-bar and kappa(u).  dbeta/du involves
    // dkappa/du which involves d^3G/du^3 which is impractical.  Therefore, we
    // live with an approximate constraint gradient in this case.  It is 
    // preferable to use a surrogate-based PMA to converge beta-bar(p-bar) in 
    // steps rather than PMA SORM which requires the full constraint gradient.
    RealVector grad_f(num_vars);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
    recast_response.function_gradient(grad_f, 1);
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f(num_vars); hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
    recast_response.function_hessian(hess_f, 1);
  }
}


void NonDReliability::
PMA2_set_mapping(const ActiveSet& recast_set, ActiveSet& sub_model_set)
{
  // if the constraint value is requested for second-order PMA, then the
  // sub-model response gradient and Hessian are required to update beta-bar.
  // Note: tests for second-order PMA are performed prior to passing this
  // function into the PMA RecastModel instance.
  const ShortArray& recast_asv = recast_set.request_vector();
  if (recast_asv[1] & 1) {
    ShortArray sub_model_asv = sub_model_set.request_vector();
    sub_model_asv[nondRelInstance->respFnCount] |= 6;
    sub_model_set.request_vector(sub_model_asv);
  }
}

} // namespace Dakota
