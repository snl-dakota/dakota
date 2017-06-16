/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDC3FunctionTrain
//- Description: Implementation code for NonDC3FunctionTrain class
//- Owner:       Alex Gorodetsky

#include "dakota_system_defs.hpp"
#include "NonDC3FunctionTrain.hpp"
#include "C3Approximation.hpp"
#include "SharedC3ApproxData.hpp"
#include "DataFitSurrModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
//#define DEBUG


namespace Dakota {

NonDC3FunctionTrain* NonDC3FunctionTrain::c3Instance(NULL);


/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDC3FunctionTrain::
NonDC3FunctionTrain(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  // numSamplesOnEmulator(probDescDB.get_int("method.nond.samples_on_emulator")),
  importApproxPointsFile(probDescDB.get_string("method.import_approx_points_file")),
  importApproxFormat(probDescDB.get_ushort("method.import_approx_format")),
  importApproxActiveOnly( probDescDB.get_bool("method.import_approx_active_only")),
  importBuildPointsFile(
    probDescDB.get_string("method.import_build_points_file")),
  importBuildFormat(probDescDB.get_ushort("method.import_build_format")),
  importBuildActiveOnly(probDescDB.get_bool("method.import_build_active_only"))

  // exportPointsFile(
  //     problem_db.get_string("model.export_approx_points_file"))
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  short u_space_type = ASKEY_U;//probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------

  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // Evaluates true model
  Iterator u_space_sampler;
  if (numSamplesOnModel) { // generate new data
	  // default pattern is fixed for consistency in any outer loop,
	  // but gets overridden in cases of unstructured grid refinement.
	  bool vary_pattern = false;
	  construct_lhs(u_space_sampler, g_u_model,
	    probDescDB.get_ushort("method.sample_type"), numSamplesOnModel,
	    randomSeed, probDescDB.get_string("method.random_number_generator"),
	    vary_pattern, ACTIVE);
	}

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse;
  String approx_type = "function_train"; // Modify DakotaApproximation get_approx()
  UShortArray approx_order; // empty
  ActiveSet sc_set = g_u_model.current_response().active_set(); // copy
  sc_set.request_values(3); // stand-alone mode: surrogate grad evals at most
  String empty_str; // build data import not supported for structured grids
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
                                              sc_set, approx_type, approx_order, corr_type, corr_order, data_order,
                                              outputLevel, pt_reuse, importBuildPointsFile, importBuildFormat,
                                              importBuildActiveOnly,
                                              probDescDB.get_string("method.export_approx_points_file"),
                                              probDescDB.get_ushort("method.export_approx_format")), false);

  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  // ...
  // follow construct_expansion_sampler from NonDExpansion.cpp which is called in NonDPolynomialChaos.cpp
  construct_expansion_sampler(importApproxPointsFile, importApproxFormat, 
                              importApproxActiveOnly);
}


NonDC3FunctionTrain::~NonDC3FunctionTrain()
{

}

    
void NonDC3FunctionTrain::
resolve_inputs(short& u_space_type, short& data_order)
{
  // May want this eventually to manage different transformation options...
    
  data_order = 1; // no deriv enhancement for now...
}

void NonDC3FunctionTrain::initialize(short u_space_type)
{
  // use Wiener/Askey/extended/piecewise u-space defn in Nataf transformation
  initialize_random_variable_transformation();
  initialize_random_variable_types(u_space_type); // need x/u_types below
  initialize_random_variable_correlations();
  // for lightweight ctor, defer until call to requested_levels()
  //initialize_final_statistics();
  verify_correlation_support(u_space_type); // correlation warping factors
}


void NonDC3FunctionTrain::initialize_u_space_model()
{
  // // Commonly used approx settings (e.g., order, outputLevel, useDerivs) are
  // // passed through the DataFitSurrModel ctor chain.  Additional data needed
  // // by OrthogPolyApproximation are passed using Pecos::BasisConfigOptions.
  // // Note: passing useDerivs again is redundant with the DataFitSurrModel ctor.
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();

  // // For PCE, the approximation and integration bases are the same.  We (always)
  // // construct it for the former and (conditionally) pass it in to the latter.
  shared_data_rep->construct_basis(natafTransform.u_types(),
                                   iteratedModel.aleatory_distribution_parameters());

  size_t model_index    = probDescDB.get_db_model_node(); // for restoration
  String model_ptr_name  = probDescDB.get_string("method.c3function_train.model_param_spec");
  // String model_ptr_name  = "FT";
  probDescDB.set_db_model_nodes(model_ptr_name);

  size_t maxnum       = probDescDB.get_sizet("model.c3function_train.max_num");
  size_t maximum_rank = probDescDB.get_sizet("model.c3function_train.max_rank");
  size_t start_order  = probDescDB.get_sizet("model.c3function_train.start_order");
  size_t initial_rank = probDescDB.get_sizet("model.c3function_train.start_rank");
  size_t rank_adapt   = probDescDB.get_sizet("model.c3function_train.rank_adapt");
  size_t crossMaxIter = probDescDB.get_sizet("model.c3function_train.cross_maxiter");
  double convergence_tol = probDescDB.get_real("model.c3function_train.solver_tolerance");
  double rounding_tol    = probDescDB.get_real("model.c3function_train.rounding_tolerance");
  // int    maximum_iters   = probDescDB.get_int("model.max_iterations");
  size_t kickrank        = probDescDB.get_sizet("model.c3function_train.kick_rank");
  size_t verbose         = probDescDB.get_sizet("model.c3function_train.verbosity");
  
  probDescDB.set_db_model_nodes(model_index); // restore

  shared_data_rep->set_parameter("maximum_num_param",&maxnum);
  shared_data_rep->set_parameter("maximum_rank"     ,&maximum_rank);
  shared_data_rep->set_parameter("start_poly_order" ,&start_order);
  shared_data_rep->set_parameter("initial_ranks",  &initial_rank);
  shared_data_rep->set_parameter("rank_adapt",     &rank_adapt);
  shared_data_rep->set_parameter("maximum_cross_approximation_iters", &crossMaxIter);
  shared_data_rep->set_parameter("convergence_tol",&convergence_tol);
  shared_data_rep->set_parameter("rounding_tol",   &rounding_tol);
  // shared_data_rep->set_parameter("maximum_iters",   &maximum_iters);
  shared_data_rep->set_parameter("kickrank",       &kickrank);
  shared_data_rep->set_parameter("verbose",        &verbose);

  std::cout << "convergence_tol = " << convergence_tol << "\n";
  std::cout << "rank_adapt = " << rank_adapt << "\n";
  
  // // if all variables mode, initialize key to random variable subset
  // NOT SURE WHAT TO DO BELOW --AG
  // if (numContDesVars || numContEpistUncVars || numContStateVars) {
  //   Pecos::BitArray random_vars_key(numContinuousVars);
  //   size_t i, num_cdv_cauv = numContDesVars + numContAleatUncVars;
  //   for (i=0; i<numContinuousVars; ++i)
  //     random_vars_key[i] = (i >= numContDesVars && i < num_cdv_cauv);
  //   shared_data_rep->random_variables_key(random_vars_key);
  // }

  // share natafTransform instance with u-space sampler
  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  if (!u_space_sampler.is_null())
    ((NonD*)u_space_sampler.iterator_rep())->
      initialize_random_variables(natafTransform); // shared rep

}
    
    
/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDC3FunctionTrain::core_run()
{
    c3Instance = this;

    initialize_expansion(); // from parent class

    // for (size_t ii = 0; ii < numFunctions; ii++){

    //         // specify_approximation_options()
    //     ft[ii].set_variable_types(?,?);
    //     ft[ii].set_random_vars(?);
                
    // }


        
    compute_expansion(); //below
    compute_print_converged_results();
    update_final_statistics(); 
    ++numUncertainQuant;
}

void NonDC3FunctionTrain::compute_expansion()
{
#ifdef DERIV_DEBUG
  // numerical verification of analytic Jacobian/Hessian routines
  RealVector rdv_u;
  natafTransform.trans_X_to_U(iteratedModel.continuous_variables(), rdv_u);
  natafTransform.verify_trans_jacobian_hessian(rdv_u);//(rdv_x);
  natafTransform.verify_design_jacobian(rdv_u);
#endif // DERIV_DEBUG

  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  NonD* u_space_sampler_rep = (NonD*)u_space_sampler.iterator_rep();

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, j, rl_len, pl_len, bl_len, gl_len, cntr = 0,
    num_final_stats = final_asv.size(), num_final_grad_vars = final_dvv.size();
  bool final_stat_value_flag = false, final_stat_grad_flag = false;
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 1)
      { final_stat_value_flag = true; break; }
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 2)
      { final_stat_grad_flag  = true; break; }

  // define ASV for u_space_sampler and expansion coefficient/gradient
  // data flags for PecosApproximation
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  ShortArray sampler_asv(numFunctions, 0);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (i=0; i<numFunctions; ++i) {
    C3Approximation* poly_approx_rep
      = (C3Approximation*)poly_approxs[i].approx_rep();
    bool expansion_coeff_flag = false, expansion_grad_flag = false,
         mean_grad_flag       = false, std_dev_grad_flag   = false;
    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    }
    else
      rl_len = pl_len = bl_len = gl_len = 0;

    if (final_stat_value_flag)
      // map final_asv value bits into expansion_coeff_flag requirements
      for (j=0; j<2+rl_len+pl_len+bl_len+gl_len; ++j)
	if (final_asv[cntr+j] & 1)
	  { expansion_coeff_flag = true; break; }

    if (final_stat_grad_flag) {
      // map final_asv gradient bits into moment grad requirements
      if (final_asv[cntr++] & 2)
	mean_grad_flag    = true;
      if (final_asv[cntr++] & 2)
	std_dev_grad_flag = true;
      if (respLevelTarget == RELIABILITIES)
	for (j=0; j<rl_len; ++j)
	  if (final_asv[cntr+j] & 2)
	    { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += rl_len + pl_len;
      for (j=0; j<bl_len; ++j)
	if (final_asv[cntr+j] & 2)
	  { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += bl_len + gl_len;
      // map moment grad requirements into expansion_{coeff,grad}_flag reqmts
      // (refer to *PolyApproximation::get_*_gradient() implementations)
      if (all_vars) { // aleatory + design/epistemic
	if (std_dev_grad_flag)
	  expansion_coeff_flag = true;
	size_t deriv_index, num_deriv_vars = final_dvv.size();
	for (j=0; j<num_deriv_vars; ++j) {
	  deriv_index = final_dvv[j] - 1; // OK since we are in an "All" view
	  if (deriv_index >= numContDesVars &&
	      deriv_index <  numContDesVars + numContAleatUncVars) { // ran var
	    if (mean_grad_flag || std_dev_grad_flag)
	      expansion_grad_flag = true;
	  }
	  else if (mean_grad_flag)
	    expansion_coeff_flag = true;
	}
      }
      else { // aleatory expansion variables
	if (mean_grad_flag)
	  expansion_grad_flag = true;
	if (std_dev_grad_flag)
	  expansion_coeff_flag = expansion_grad_flag = true;
      }
    }
    else
      cntr += 2 + rl_len + pl_len + bl_len + gl_len;

    // map expansion_{coeff,grad}_flag requirements into ASV and
    // C3Approximation settings
    if (expansion_coeff_flag)
      sampler_asv[i] |= 1;
    if (expansion_grad_flag || useDerivs)
      sampler_asv[i] |= 2;
    poly_approx_rep->eval_flag(expansion_coeff_flag);
    poly_approx_rep->gradient_flag(expansion_grad_flag);
  }

  // If OUU/SOP (multiple calls to core_run()), an expansion constructed over
  // the full range of all variables does not need to be reconstructed on
  // subsequent calls.  However, an all_vars construction over a trust region
  // needs rebuilding when the trust region is updated.  In the checks below,
  // all_approx detects any variable insertions or ASV omissions and
  // force_rebuild() manages variable augmentations.
  bool all_approx = false;
  if (all_vars && numUncertainQuant && secondaryACVarMapTargets.empty()) {
    all_approx = true;
    // does sampler_asv contain content not evaluated previously
    const ShortArray& prev_asv = u_space_sampler.active_set().request_vector();
    for (i=0; i<numFunctions; ++i)
      // bit-wise AND checks if each sampler_asv bit is present in prev_asv
      if ( (prev_asv[i] & sampler_asv[i]) != sampler_asv[i] )
	{ all_approx = false; break; }
  }
  if (!all_approx || uSpaceModel.force_rebuild()) {

    if (u_space_sampler_rep) {

      // Set the sampler ASV (defined from previous loop over numFunctions)
      ActiveSet sampler_set;
      sampler_set.request_vector(sampler_asv);

      // if required statistical sensitivities are not covered by All variables
      // mode for augmented design variables, then the simulations must evaluate
      // response sensitivities.
      bool sampler_grad = false, dist_param_deriv = false;
      if (final_stat_grad_flag) {
	size_t i, num_outer_cv = secondaryACVarMapTargets.size();
	for (i=0; i<num_outer_cv; ++i)
	  if (secondaryACVarMapTargets[i] != Pecos::NO_TARGET) // insertion
	    { dist_param_deriv = true; break; }
	sampler_grad = (all_vars) ? dist_param_deriv : true;
      }
      u_space_sampler_rep->distribution_parameter_derivatives(dist_param_deriv);
      if (dist_param_deriv)
	u_space_sampler.active_variable_mappings(primaryACVarMapIndices,
	  primaryADIVarMapIndices, primaryADSVarMapIndices,
	  primaryADRVarMapIndices, secondaryACVarMapTargets,
	  secondaryADIVarMapTargets, secondaryADSVarMapTargets,
	  secondaryADRVarMapTargets);

      // Set the u_space_sampler DVV, managing different gradient modes & their
      // combinations.  The u_space_sampler's DVV may then be augmented for
      // correlations in NonD::set_u_to_x_mapping().  Sources for DVV content
      // include the model's continuous var ids and the final_dvv set by a
      // NestedModel.  In the latter case, NestedModel::derived_compute_response
      // maps top-level optimizer derivative vars to sub-iterator derivative
      // vars in NestedModel::set_mapping() and then sets this DVV within
      // finalStats using subIterator.response_results_active_set().
      if (useDerivs) {
	SizetMultiArrayConstView cv_ids
	  = iteratedModel.continuous_variable_ids();
	if (sampler_grad) { // merge cv_ids with final_dvv
	  SizetSet merged_set; SizetArray merged_dvv;
	  merged_set.insert(cv_ids.begin(), cv_ids.end());
	  merged_set.insert(final_dvv.begin(), final_dvv.end());
	  std::copy(merged_set.begin(), merged_set.end(), merged_dvv.begin());
	  sampler_set.derivative_vector(merged_dvv);
	}
	else // assign cv_ids
	  sampler_set.derivative_vector(cv_ids);
      }
      else if (all_vars && sampler_grad) { // filter: retain only insertion tgts
	SizetArray filtered_final_dvv;
	size_t num_cdv_cauv = numContDesVars+numContAleatUncVars;
	for (i=0; i<num_final_grad_vars; ++i) {
	  size_t dvv_i = final_dvv[i];
	  if (dvv_i > numContDesVars && dvv_i <= num_cdv_cauv)
	    filtered_final_dvv.push_back(dvv_i);
	}
	sampler_set.derivative_vector(filtered_final_dvv);
      }
      else if (sampler_grad)
	sampler_set.derivative_vector(final_dvv);
      else // derivs not needed, but correct DVV len needed for MPI buffers
	sampler_set.derivative_vector(iteratedModel.continuous_variable_ids());

      // Build the orthogonal/interpolation polynomial approximations:
      u_space_sampler.active_set(sampler_set);
    }

    uSpaceModel.build_approximation();
  }
}

    

int NonDC3FunctionTrain::
qoi_eval(size_t num_samp, const double* var_sets, double* qoi_sets, void* args)
{
  size_t num_cv  = c3Instance->numContinuousVars,
         num_fns = c3Instance->numFunctions;

  bool asynch_flag = c3Instance->iteratedModel.asynch_flag();

  RealVector cv_i(num_cv, false);
  for (size_t i=0; i<num_samp; ++i) {
    copy_data(var_sets+num_cv*i, num_cv, cv_i);
    c3Instance->iteratedModel.continuous_variables(cv_i);
    if (asynch_flag)
      c3Instance->iteratedModel.evaluate_nowait();
    else {
      c3Instance->iteratedModel.evaluate();
      // pack Dakota resp data into qoi_sets...
      const RealVector& fns_i = c3Instance->iteratedModel.current_response().function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  if (asynch_flag) {
    const IntResponseMap& resp_map = c3Instance->iteratedModel.synchronize();
    // pack Dakota resp data into qoi_sets...
    IntRespMCIter r_cit; size_t i;
    for (r_cit=resp_map.begin(), i=0; r_cit!=resp_map.end(); ++r_cit, ++i) {
      const RealVector& fns_i = r_cit->second.function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  return 0;
}


void NonDC3FunctionTrain::post_run(std::ostream& s)
{
  // Statistics are generated here and output in print_results() below
  // if (statsFlag) // calculate statistics on allResponses
  //     compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
}

void NonDC3FunctionTrain::compute_analytic_statistics()
{
    const ShortArray& final_asv = finalStatistics.active_set_request_vector();
    const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
    bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
    size_t i, j, k, rl_len, pl_len, bl_len, gl_len, cntr = 0,
        num_final_grad_vars = final_dvv.size();

    // loop over response fns and compute/store analytic stats/stat grads
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    Real mu, sigma, beta, z;
    RealVector mu_grad, sigma_grad, final_stat_grad;
    C3Approximation* poly_approx_rep;
    for (i=0; i<numFunctions; ++i) {
        if (totalLevelRequests) {
            rl_len = requestedRespLevels[i].length();
            pl_len = requestedProbLevels[i].length();
            bl_len = requestedRelLevels[i].length();
            gl_len = requestedGenRelLevels[i].length();
        }
        else{
            rl_len = pl_len = bl_len = gl_len = 0;
        }

        poly_approx_rep = (C3Approximation*)poly_approxs[i].approx_rep();

        if (all_vars) poly_approx_rep->compute_moments(initialPtU);
        else          poly_approx_rep->compute_moments();

        const RealVector& moments = poly_approx_rep->moments(); // virtual
        // Pecos provides central moments
        mu = moments[0]; const Real& var = moments[1];
        if (covarianceControl ==  DIAGONAL_COVARIANCE) respVariance[i]     = var;
        else if (covarianceControl == FULL_COVARIANCE) respCovariance(i,i) = var;
        if (var >= 0.)
            sigma = std::sqrt(var);
        else { // negative variance can happen with SC on sparse grids
            Cerr << "Warning: stochastic expansion variance is negative in "
                 << "computation of std deviation.\n         Setting std "
                 << "deviation to zero." << std::endl;
            sigma = 0.;
        }

        // compute moment gradients if needed for beta mappings
        bool moment_grad_mapping_flag = false;
        if (respLevelTarget == RELIABILITIES)
            for (j=0; j<rl_len; ++j) // dbeta/ds requires mu,sigma,dmu/ds,dsigma/ds
                if (final_asv[cntr+2+j] & 2)
                { moment_grad_mapping_flag = true; break; }
        if (!moment_grad_mapping_flag)
            for (j=0; j<bl_len; ++j)   // dz/ds requires dmu/ds,dsigma/ds
                if (final_asv[cntr+2+pl_len+j] & 2)
                { moment_grad_mapping_flag = true; break; }

        // *** mean
        if (final_asv[cntr] & 1)
            finalStatistics.function_value(mu, cntr);
        // *** mean gradient
        if (final_asv[cntr] & 2 || moment_grad_mapping_flag) {
            const RealVector& grad = (all_vars) ?
                poly_approx_rep->mean_gradient(initialPtU, final_dvv) :
                poly_approx_rep->mean_gradient();
            if (final_asv[cntr] & 2)
                finalStatistics.function_gradient(grad, cntr);
            if (moment_grad_mapping_flag)
                mu_grad = grad; // transfer to code below
        }
        ++cntr;

        // *** std deviation
        if (final_asv[cntr] & 1)
            finalStatistics.function_value(sigma, cntr);
        // *** std deviation gradient
        if (final_asv[cntr] & 2 || moment_grad_mapping_flag) {
            sigma_grad = (all_vars) ?
                poly_approx_rep->variance_gradient(initialPtU, final_dvv) :
                poly_approx_rep->variance_gradient();
            if (sigma > 0.)
                for (j=0; j<num_final_grad_vars; ++j)
                    sigma_grad[j] /= 2.*sigma;
            else {
                Cerr << "Warning: stochastic expansion std deviation is zero in "
                     << "computation of std deviation gradient.\n         Setting "
                     << "gradient to zero." << std::endl;
                sigma_grad = 0.;
            }
            if (final_asv[cntr] & 2)
                finalStatistics.function_gradient(sigma_grad, cntr);
        }
        ++cntr;

        if (respLevelTarget == RELIABILITIES) {
            for (j=0; j<rl_len; ++j, ++cntr) {
                // *** beta
                if (final_asv[cntr] & 1) {
                    const Real& z_bar = requestedRespLevels[i][j];
                    if (sigma > Pecos::SMALL_NUMBER) {
                        Real ratio = (mu - z_bar)/sigma;
                        computedRelLevels[i][j] = beta = (cdfFlag) ? ratio : -ratio;
                    }
                    else
                        computedRelLevels[i][j] = beta =
                            ( (cdfFlag && mu <= z_bar) || (!cdfFlag && mu > z_bar) ) ?
                            -Pecos::LARGE_NUMBER : Pecos::LARGE_NUMBER;
                    finalStatistics.function_value(beta, cntr);
                }
                // *** beta gradient
                if (final_asv[cntr] & 2) {
                    if (final_stat_grad.empty())
                        final_stat_grad.sizeUninitialized(num_final_grad_vars);
                    if (sigma > Pecos::SMALL_NUMBER) {
                        const Real& z_bar = requestedRespLevels[i][j];
                        for (k=0; k<num_final_grad_vars; ++k) {
                            Real dratio_dx = (sigma*mu_grad[k] - (mu - z_bar)*
                                              sigma_grad[k]) / std::pow(sigma, 2);
                            final_stat_grad[k] = (cdfFlag) ? dratio_dx : -dratio_dx;
                        }
                    }
                    else
                        final_stat_grad = 0.;
                    finalStatistics.function_gradient(final_stat_grad, cntr);
                }
            }
        }
        else
            cntr += rl_len;

        // no analytic mappings for probability levels
        cntr += pl_len;

        for (j=0; j<bl_len; ++j, ++cntr) {
            const Real& beta_bar = requestedRelLevels[i][j];
            if (final_asv[cntr] & 1) {
                // *** z
                computedRespLevels[i][j+pl_len] = z = (cdfFlag) ?
                    mu - beta_bar * sigma : mu + beta_bar * sigma;
                finalStatistics.function_value(z, cntr);
            }
            if (final_asv[cntr] & 2) {
                // *** z gradient
                if (final_stat_grad.empty())
                    final_stat_grad.sizeUninitialized(num_final_grad_vars);
                for (k=0; k<num_final_grad_vars; ++k)
                    final_stat_grad[k] = (cdfFlag) ?
                        mu_grad[k] - beta_bar * sigma_grad[k] :
                        mu_grad[k] + beta_bar * sigma_grad[k];
                finalStatistics.function_gradient(final_stat_grad, cntr);
            }
        }

        // no analytic mappings for generalized reliability levels
        cntr += gl_len;
 
    // *** local sensitivities
        if (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT){
            
            // expansion sensitivities are defined from the coefficients and basis
            // polynomial derivatives.  They are computed for the means of the
            // uncertain varables and provide a measure of local importance (but not
            // scaled by input covariance as in mean value importance factors).
            const RealVector& exp_grad_u
                = poly_approxs[i].gradient(uSpaceModel.current_variables());
            RealVector exp_grad_x;
            SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
            SizetArray x_dvv; copy_data(cv_ids, x_dvv);
            RealVector x_means(natafTransform.x_means());
            natafTransform.trans_grad_U_to_X(exp_grad_u, exp_grad_x, x_means,
                                             x_dvv, cv_ids);
            Teuchos::setCol(exp_grad_x, (int)i, expGradsMeanX);

#ifdef TEST_HESSIANS
            const RealSymMatrix& exp_hess_u
                = poly_approxs[i].hessian(uSpaceModel.current_variables());
            //RealSymMatrix exp_hess_x;
            //natafTransform.trans_hess_U_to_X(exp_hess_u, exp_hess_x, x_means,
            //                                 x_dvv, cv_ids);
            write_data(Cout, exp_hess_u); //exp_hess_x
#endif // TEST_HESSIANS
        }

        // *** global sensitivities:
        if (vbdFlag) {
            if ((vbdOrderLimit != 1) && (vbdOrderLimit != 0)){
                poly_approx_rep->compute_all_sobol_indices(vbdOrderLimit); 
            }
            else{
                poly_approx_rep->compute_all_sobol_indices(numContStateVars);

            }

        }
    }
    if (numFunctions > 1 && covarianceControl == FULL_COVARIANCE){
        compute_off_diagonal_covariance(); // diagonal entries were filled in above
    }    
}

void NonDC3FunctionTrain::compute_off_diagonal_covariance()
{
    size_t i, j;
    bool warn_flag = false;
    bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (i=0; i<numFunctions; ++i) {
        C3Approximation* poly_approx_rep_i = (C3Approximation*)poly_approxs[i].approx_rep();
        for (j=0; j<i; ++j) {
            C3Approximation*poly_approx_rep_j = (C3Approximation*)poly_approxs[j].approx_rep();
            respCovariance(i,j) = (all_vars) ?
                poly_approx_rep_i->covariance(initialPtU, poly_approx_rep_j) :
                poly_approx_rep_i->covariance(poly_approx_rep_j);
        }
    }
}
    
void NonDC3FunctionTrain::compute_diagonal_variance()
{
    bool warn_flag = false;
    bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (size_t i=0; i<numFunctions; ++i) {
        Real& var_i = (covarianceControl == DIAGONAL_COVARIANCE) ? respVariance[i] : respCovariance(i,i);
        C3Approximation* poly_approx_rep_i  = (C3Approximation*)poly_approxs[i].approx_rep();
        var_i = (all_vars) ? poly_approx_rep_i->variance(initialPtU) : poly_approx_rep_i->variance();
    }
}    
     
void NonDC3FunctionTrain::print_moments(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  // std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, width = write_precision+7;

  s << "\nMoment statistics for each response function:\n";
  // Handle cases of both expansion/numerical moments or only one or the other:
  //   both exp/num: SC and PCE with numerical integration
  //   exp only:     PCE with unstructured grids (regression, exp sampling)
  // Also handle numerical exception of negative variance in either exp or num
  bool exception = false, prev_exception = false;

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();  
  for (size_t i=0; i<numFunctions; ++i) {
      C3Approximation* poly_approx_rep_i = (C3Approximation*)poly_approxs[i].approx_rep();
       if (i==0 || !prev_exception){
           s << std::setw(width+15) << "Mean" << std::setw(width+1) << "Std Dev"
             << std::setw(width+1)  << "Skewness" << std::setw(width+2)
             << "Kurtosis\n";
       }
       RealVector moments = poly_approx_rep_i->moments();
       s <<  fn_labels[i] << '\n' << std::setw(14) << "analytical: ";
       s << ' ' << std::setw(width) << moments[0]
         << ' ' << std::setw(width) << sqrt(moments[1])
         << ' ' << std::setw(width) << poly_approx_rep_i->skewness()
         << ' ' << std::setw(width) << poly_approx_rep_i->kurtosis()-3;
      s << '\n';
  }
}

struct SPrintArgs
{
    std::ostream *s;
    StringMultiArrayConstView * cv_labels;
    double variance;
    
};
    
void print_c3_sobol_indices(double value, size_t ninteract, size_t * interactions, void * arg)
{
    if (ninteract > 1){
        struct SPrintArgs * pa = (struct SPrintArgs *)arg;

        StringMultiArrayConstView cv_label = *(pa->cv_labels);
    
        String label;
        for (size_t jj = 0; jj < ninteract; jj++){
            label += cv_label[interactions[jj]] + " ";
        }
        *(pa->s) << "                     " << std::setw(write_precision+7) << value/pa->variance;
        *(pa->s) << ' ' << label << '\n';
    }
    
}
    
void NonDC3FunctionTrain::print_sobol_indices(std::ostream& s)
{
    s << "\nGlobal sensitivity indices for each response function:\n";

    const StringArray& fn_labels = iteratedModel.response_labels();

    StringMultiArrayConstView cv_labels = iteratedModel.continuous_variable_labels();

    // print sobol indices per response function
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (size_t i=0; i<numFunctions; ++i) {
        C3Approximation* poly_approx_rep_i = (C3Approximation*)poly_approxs[i].approx_rep();
        
        // Print Main and Total effects
        s << fn_labels[i] << " Sobol' indices:\n" << std::setw(38) << "Main"
          << std::setw(19) << "Total\n";
        
        RealVector moments = poly_approx_rep_i->moments();
        Real var = moments(1);
        
        for (size_t j=0; j<numContinuousVars; ++j) {
            s << "                     "   <<        std::setw(write_precision+7)
              << poly_approx_rep_i->main_sobol_index(j)/var << ' ' << std::setw(write_precision+7)
              << poly_approx_rep_i->total_sobol_index(j) << ' ' << cv_labels[j] << '\n';
        }
          
        // Print Interaction effects
        if (vbdOrderLimit != 1) { 
            s << std::setw(39) << "Interaction\n";
            StringMultiArrayConstView cv_labels
                = iteratedModel.continuous_variable_labels();
            
            struct SPrintArgs pa;
            pa.s = &s;
            pa.cv_labels = &cv_labels;
            pa.variance = var;

            poly_approx_rep_i->sobol_iterate_apply(print_c3_sobol_indices,&pa);
        }
    }
}


void NonDC3FunctionTrain::print_local_sensitivity(std::ostream& s)
{
    const StringArray& fn_labels = iteratedModel.response_labels();
    s << "\nLocal sensitivities for each response function evaluated at "
      << "uncertain variable means:\n";
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (size_t i=0; i<numFunctions; ++i) {
        C3Approximation* poly_approx_rep = (C3Approximation*)poly_approxs[i].approx_rep();
        s << fn_labels[i] << ":\n";
        write_col_vector_trans(s, (int)i, expGradsMeanX);
    }
}

    
} // namespace Dakota
