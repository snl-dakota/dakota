/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    abort_handler(-1);
  }

  initialize_random_variable_transformation();
  initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below
  // Note: initialize_random_variable_parameters() is performed at run time
  initialize_random_variable_correlations();
  verify_correlation_support();
  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);
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
  const RealVector& u = recast_vars.continuous_variables();
  size_t i, num_vars = u.length();
  short asv_val = recast_asv[0];
  if (asv_val & 1) {
    Real f = 0.;
    for (i=0; i<num_vars; i++)
      f += std::pow(u[i], 2); // f = u'u
    recast_response.function_value(f, 0);
  }
  if (asv_val & 2) {
    RealVector grad_f = recast_response.function_gradient_view(0);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f = recast_response.function_hessian_view(0);
    hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
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
  int fn_count = nondRelInstance->respFnCount;
  if (asv_val & 1) {
    const Real& sub_model_fn = sub_model_response.function_value(fn_count);
    recast_response.function_value(
      sub_model_fn - nondRelInstance->requestedRespLevel, 1);
  }
  if (asv_val & 2) // dG/du: no additional transformation needed
    recast_response.function_gradient(
      sub_model_response.function_gradient_view(fn_count), 1);
  if (asv_val & 4) // d^2G/du^2: no additional transformation needed
    recast_response.function_hessian(
      sub_model_response.function_hessian(fn_count), 1);
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
      = sub_model_response.function_value(nondRelInstance->respFnCount);
    if (positive)
      recast_response.function_value( sub_model_fn, 0);
    else
      recast_response.function_value(-sub_model_fn, 0);
  }
  if (asv_val & 2) { // dG/du: no additional transformation needed
    RealVector sub_model_grad
      = sub_model_response.function_gradient_view(nondRelInstance->respFnCount);
    if (positive)
      recast_response.function_gradient(sub_model_grad, 0);
    else {
      RealVector neg_sub_model_grad = recast_response.function_gradient_view(0);
      size_t i, num_vars = sub_model_response.function_gradients().numRows();
      for (i=0; i<num_vars; ++i)
	neg_sub_model_grad[i] = -sub_model_grad[i];
    }
  }
  if (asv_val & 4) { // d^2G/du^2: no additional transformation needed
    const RealSymMatrix& sub_model_hessian
      = sub_model_response.function_hessian(nondRelInstance->respFnCount);
    if (positive)
      recast_response.function_hessian(sub_model_hessian, 0);
    else {
      RealSymMatrix neg_sub_model_hessian
	= recast_response.function_hessian_view(0);
      size_t i, j, num_vars = sub_model_hessian.numRows();
      for (i=0; i<num_vars; ++i)
	for (j=0; j<=i; ++j)
	  neg_sub_model_hessian(i,j) = -sub_model_hessian(i,j);
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
    RealVector grad_f = recast_response.function_gradient_view(1);
    for (i=0; i<num_vars; ++i)
      grad_f[i] = 2.*u[i]; // grad f = 2u
  }
  if (asv_val & 4) {
    RealSymMatrix hess_f = recast_response.function_hessian_view(1);
    hess_f = 0.;
    for (i=0; i<num_vars; i++)
      hess_f(i,i) = 2.; // hess f = 2's on diagonal
  }
}


void NonDReliability::
PMA2_set_mapping(const Variables& recast_vars, const ActiveSet& recast_set,
		 ActiveSet& sub_model_set)
{
  // if the constraint value is requested for second-order PMA, then the
  // sub-model response gradient and Hessian are required to update beta-bar.
  // Note: tests for second-order PMA are performed prior to passing this
  // function into the PMA RecastModel instance.
  const ShortArray& recast_asv = recast_set.request_vector();
  if (recast_asv[1] & 1) {
    int sm_index = nondRelInstance->respFnCount;
    short sub_model_request = sub_model_set.request_value(sm_index);
    sub_model_request |= 6;
    sub_model_set.request_value(sub_model_request, sm_index);
  }
}

} // namespace Dakota
