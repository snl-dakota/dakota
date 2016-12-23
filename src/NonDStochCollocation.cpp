/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDStochCollocation
//- Description: Implementation code for NonDStochCollocation class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDSparseGrid.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "PecosApproximation.hpp"
#include "SharedInterpPolyApproxData.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION
//#define DEBUG


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDStochCollocation::
NonDStochCollocation(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model)
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order,
    u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  const UShortArray& quad_order_spec
    = probDescDB.get_usa("method.nond.quadrature_order");
  const UShortArray& ssg_level_spec
    = probDescDB.get_usa("method.nond.sparse_grid_level");
  const RealVector& dim_pref
    = probDescDB.get_rv("method.nond.dimension_preference");
  check_dimension_preference(dim_pref);
  if (!quad_order_spec.empty()) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_quadrature(u_space_sampler, g_u_model, quad_order_spec, dim_pref);
  }
  else if (!ssg_level_spec.empty()) {
    switch (expansionBasisType) {
    case Pecos::HIERARCHICAL_INTERPOLANT:
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;          break;
    case Pecos::NODAL_INTERPOLANT:
      expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;              break;
    case Pecos::DEFAULT_BASIS:
      if ( u_space_type == STD_UNIFORM_U && nestedRules &&// TO DO:retire nested
	   ( refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED ||
	     refineControl == Pecos::LOCAL_ADAPTIVE_CONTROL ) ) {
	expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
	expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
      }
      else {
	expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;
	expansionBasisType = Pecos::NODAL_INTERPOLANT;
      }
      break;
    }
    /*
    if (refineControl == Pecos::LOCAL_ADAPTIVE_CONTROL) {
      if (!piecewiseBasis ||
          expansionBasisType != Pecos::HIERARCHICAL_INTERPOLANT) {
	// TO DO: promote this error check to resolve_inputs()
	PCerr << "Warning: overriding basis type to local hierarchical\n.";
	piecewiseBasis = true;
	expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
      }
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
    }
    */
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec, dim_pref);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type;
  if (piecewiseBasis)
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ? 
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ?
      "global_hierarchical_interpolation_polynomial" :
      "global_nodal_interpolation_polynomial";
  UShortArray approx_order; // empty
  //const Variables& g_u_vars = g_u_model.current_variables();
  ActiveSet sc_set = g_u_model.current_response().active_set(); // copy
  sc_set.request_values(3); // stand-alone mode: surrogate grad evals at most
  String empty_str; // build data import not supported for structured grids
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    sc_set, approx_type, approx_order, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, empty_str, TABULAR_ANNOTATED, false,
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);
  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler(
    probDescDB.get_string("method.import_approx_points_file"),
    probDescDB.get_ushort("method.import_approx_format"),
    probDescDB.get_bool("method.import_approx_active_only"));

  if (parallelLib.command_line_check())
    Cout << "\nStochastic collocation construction completed: initial grid "
	 << "size of " << numSamplesOnModel << " evaluations to be performed."
	 << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDStochCollocation::
NonDStochCollocation(Model& model, short exp_coeffs_approach,
		     const UShortArray& num_int_seq, const RealVector& dim_pref,
		     short u_space_type, bool piecewise_basis, bool use_derivs):
  NonDExpansion(STOCH_COLLOCATION, model, exp_coeffs_approach, u_space_type,
		piecewise_basis, use_derivs)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dim_pref);

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_quadrature(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  case Pecos::COMBINED_SPARSE_GRID:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  case Pecos::HIERARCHICAL_SPARSE_GRID:
    expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type;
  if (piecewiseBasis)
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ?
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ?
      "global_hierarchical_interpolation_polynomial" :
      "global_nodal_interpolation_polynomial";
  UShortArray approx_order; // empty
  ActiveSet sc_set = g_u_model.current_response().active_set(); // copy
  sc_set.request_values(3); // TO DO: support surr Hessian evals in helper mode
                            // TO DO: consider passing in data_mode
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    sc_set, approx_type, approx_order, corr_type, corr_order, data_order,
    outputLevel, pt_reuse), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDStochCollocation::~NonDStochCollocation()
{ }


bool NonDStochCollocation::resize()
{
  bool parent_reinit_comms = NonDExpansion::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDStochCollocation::
resolve_inputs(short& u_space_type, short& data_order)
{
  // perform first due to piecewiseBasis overrides
  NonDExpansion::resolve_inputs(u_space_type, data_order);

  // There are two derivative cases of interest: (1) derivatives used as
  // additional data for forming the approximation (derivatives w.r.t. the
  // expansion variables), and (2) derivatives that will be approximated 
  // separately (derivatives w.r.t. auxilliary variables).  The data_order
  // passed through the DataFitSurrModel defines Approximation::buildDataOrder,
  // which is restricted to managing the former case.  If we need to manage the
  // latter case in the future, we do not have a good way to detect this state
  // at construct time, as neither the finalStats ASV/DVV nor subIteratorFlag
  // have yet been defined.  One indicator that is defined is the presence of
  // inactive continuous vars, since the subModel inactive view is updated
  // within the NestedModel ctor prior to subIterator instantiation.
  data_order = 1;
  if (useDerivs) { // input specification
    if (iteratedModel.gradient_type()  != "none") data_order |= 2;
    //if (iteratedModel.hessian_type() != "none") data_order |= 4; // not yet
#ifdef ALLOW_GLOBAL_HERMITE_INTERPOLATION
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
#else
    if (piecewiseBasis) {
      if (data_order == 1)
	Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	     << "requires a response\n         gradient specification.  "
	     << "Option will be ignored.\n" << std::endl;
    }
    else {
      Cerr << "\nWarning: use of global gradient-enhanced interpolants is "
	   << "disallowed in production\n         executables.  To activate "
	   << "this research capability, define\n         ALLOW_GLOBAL_HERMITE_"
	   << "INTERPOLATION in Dakota::NonDStochCollocation and recompile.\n"
	   << std::endl;
      data_order = 1;
    }
#endif
  }
  useDerivs = (data_order > 1); // override input specification

  // override u_space_type to STD_UNIFORM_U for global Hermite interpolation
  if (useDerivs && !piecewiseBasis) {
    if (u_space_type == ASKEY_U) // non-default
      Cerr << "\nWarning: overriding ASKEY to STD_UNIFORM for Hermite "
	   << "interpolation.\n" << std::endl;
    else if (u_space_type == STD_NORMAL_U) // non-default
      Cerr << "\nWarning: overriding WIENER to STD_UNIFORM for Hermite "
	   << "interpolation.\n" << std::endl;
    u_space_type = STD_UNIFORM_U;
  }
}


void NonDStochCollocation::initialize_u_space_model()
{
  // Commonly used approx settings (e.g., order, outputLevel, useDerivs) are
  // passed through the DataFitSurrModel ctor chain.  Additional data needed
  // by InterpPolyApproximation are passed using Pecos::BasisConfigOptions.
  // Note: passing useDerivs again is redundant with the DataFitSurrModel ctor.
  Pecos::BasisConfigOptions bc_options(nestedRules, piecewiseBasis,
				       true, useDerivs);

  // build a polynomial basis for purposes of defining collocation pts/wts
  std::vector<Pecos::BasisPolynomial> driver_basis;
  Pecos::SharedInterpPolyApproxData::construct_basis(natafTransform.u_types(),
    iteratedModel.aleatory_distribution_parameters(), bc_options, driver_basis);

  // set the polynomial basis within the NonDIntegration instance
  NonDIntegration* u_space_sampler_rep
    = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
  u_space_sampler_rep->initialize_grid(driver_basis);

  // perform last due to numSamplesOnModel update
  NonDExpansion::initialize_u_space_model();
}


void NonDStochCollocation::update_expansion()
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
    // grid levels have been updated, now evaluate the new points
    NonDSparseGrid* nond_sparse = (NonDSparseGrid*)
      uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sparse->evaluate_grid_increment(); // like NonDSG::evaluate_set()
    // append the new data to the existing approximation and rebuild
    uSpaceModel.append_approximation(true); // rebuild
  }
  else
    NonDExpansion::update_expansion(); // default: build from scratch
}


Real NonDStochCollocation::compute_covariance_metric()
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
    size_t i, j;
    RealSymMatrix delta_resp_covar(numFunctions, false);
    bool warn_flag = false,
      all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (i=0; i<numFunctions; ++i) {
      PecosApproximation* pa_rep_i
	= (PecosApproximation*)poly_approxs[i].approx_rep();
      if (pa_rep_i->expansion_coefficient_flag())
	for (j=0; j<=i; ++j) {
	  PecosApproximation* pa_rep_j
	    = (PecosApproximation*)poly_approxs[j].approx_rep();
	  if (pa_rep_j->expansion_coefficient_flag())
	    delta_resp_covar(i,j) = (all_vars) ?
	      pa_rep_i->delta_covariance(initialPtU, pa_rep_j) :
	      pa_rep_i->delta_covariance(pa_rep_j);
	  else
	    { warn_flag = true; delta_resp_covar(i,j) = 0.; }
	}
      else {
	warn_flag = true;
	for (j=0; j<=i; ++j)
	  delta_resp_covar(i,j) = 0.;
      }
    }
    if (warn_flag)
      Cerr << "Warning: expansion coefficients unavailable in "
	   << "NonDStochCollocation::compute_covariance_metric().\n         "
	   << "Zeroing affected delta_covariance terms." << std::endl;
    // reference covariance gets restored in NonDExpansion::increment_sets()
    respCovariance += delta_resp_covar;
    return delta_resp_covar.normFrobenius();
  }
  else // use default implementation
    return NonDExpansion::compute_covariance_metric();
}


Real NonDStochCollocation::compute_final_statistics_metric()
{
  // combine delta_beta() and delta_z() from HierarchInterpPolyApproximation
  // with default definition of delta-{p,beta*}

  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
    // Note: from a generality perspective, it would be desirable to map through
    // support for delta_mean() and delta_std_deviation().  But how to manage
    // user requests (w/o nested response mappings)?  Since delta in response
    // covariance (the default metric) is preferred to either delta-sigma or
    // delta-mu, we will omit mean/std deviation for now.  With a finer-grained
    // adaptivity specification, they could be readily added to the logic below.
    bool beta_map = false, numerical_map = false; size_t i, j, cntr;
    for (i=0; i<numFunctions; ++i) {
      if ( !requestedRelLevels[i].empty() || ( !requestedRespLevels[i].empty()
	   && respLevelTarget == RELIABILITIES ) )
	beta_map = true;
      if ( !requestedProbLevels[i].empty() || !requestedGenRelLevels[i].empty()
	   || ( !requestedRespLevels[i].empty() &&
		respLevelTarget != RELIABILITIES ) )
	numerical_map = true;
    }
    if (beta_map) { // hierarchical increments in beta-bar->z and z-bar->beta
      RealVector delta_final_stats;
      if (numerical_map) { // merge in z-bar->p,beta* & p-bar,beta*-bar->z
        delta_final_stats  = finalStatistics.function_values(); // deep copy
	compute_statistics();                                   // update
	delta_final_stats -= finalStatistics.function_values(); // compute delta
      }
#ifdef DEBUG
      else
        delta_final_stats.size(finalStatistics.num_functions());
#endif // DEBUG
      bool warn_flag = false,
	all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
      std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
      Real delta, sum_sq = 0.;
      for (i=0, cntr=0; i<numFunctions; ++i) {
	size_t rl_len = requestedRespLevels[i].length(),
	       pl_len = requestedProbLevels[i].length(),
	       bl_len = requestedRelLevels[i].length(),
	       gl_len = requestedGenRelLevels[i].length();
	PecosApproximation* pa_rep_i
	  = (PecosApproximation*)poly_approxs[i].approx_rep();
	cntr += 2; // skip moments
	if (pa_rep_i->expansion_coefficient_flag()) {
	  if (respLevelTarget == RELIABILITIES)
	    for (j=0; j<rl_len; ++j, ++cntr) {
#ifdef DEBUG
	      delta = delta_final_stats[cntr] = (all_vars) ?
#else
	      delta = (all_vars) ? 
#endif // DEBUG
		pa_rep_i->delta_beta(initialPtU, cdfFlag,
				     requestedRespLevels[i][j]) :
		pa_rep_i->delta_beta(cdfFlag, requestedRespLevels[i][j]);
	      sum_sq += delta * delta;
	    }
	  else
	    for (j=0; j<rl_len; ++j, ++cntr)
	      sum_sq += delta_final_stats[cntr] * delta_final_stats[cntr];
	  for (j=0; j<pl_len; ++j, ++cntr)
	    sum_sq += delta_final_stats[cntr] * delta_final_stats[cntr];
	  for (j=0; j<bl_len; ++j, ++cntr) {
#ifdef DEBUG
	    delta = delta_final_stats[cntr] = (all_vars) ?
#else
	    delta = (all_vars) ?
#endif // DEBUG
	      pa_rep_i->delta_z(initialPtU, cdfFlag, requestedRelLevels[i][j]) :
	      pa_rep_i->delta_z(cdfFlag, requestedRelLevels[i][j]);
	    sum_sq += delta * delta;
	  }
	  for (j=0; j<gl_len; ++j, ++cntr)
	    sum_sq += delta_final_stats[cntr] * delta_final_stats[cntr];
	}
	else {
	  warn_flag = true;
	  cntr += rl_len + pl_len + bl_len + gl_len;
	}
      }
      if (warn_flag)
	Cerr << "Warning: expansion coefficients unavailable in "
	     << "NonDStochCollocation::compute_final_statistics_metric().\n"
	     << "         Omitting affected final statistics." << std::endl;
#ifdef DEBUG
      Cout << "In compute_final_statistics_metric(), delta_final_stats =\n";
      write_data(Cout, delta_final_stats);
#endif // DEBUG
      return std::sqrt(sum_sq); // neglect moment deltas
      //return delta_final_stats.normFrobenius();
    }
    else // use default implementation if no beta-mapping increments
      return NonDExpansion::compute_final_statistics_metric();
  }
  else // use default implementation for Nodal
    return NonDExpansion::compute_final_statistics_metric();
}

} // namespace Dakota
