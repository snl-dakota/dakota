/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDMultilevelStochCollocation
//- Description: Implementation code for NonDMultilevelStochCollocation class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "NonDMultilevelStochCollocation.hpp"
#include "NonDQuadrature.hpp"
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
NonDMultilevelStochCollocation::
NonDMultilevelStochCollocation(ProblemDescDB& problem_db, Model& model):
  NonDStochCollocation(BaseConstructor(), problem_db, model),
  mlmfAllocControl(
    probDescDB.get_short("method.nond.multilevel_allocation_control")),
  quadOrderSeqSpec(probDescDB.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(probDescDB.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order,
    u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize_random(u_space_type);

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
  unsigned short quad_order = USHRT_MAX, ssg_level = USHRT_MAX;
  if (!quadOrderSeqSpec.empty())
    quad_order = (sequenceIndex < quadOrderSeqSpec.size()) ?
      quadOrderSeqSpec[sequenceIndex] : quadOrderSeqSpec.back();
  if (!ssgLevelSeqSpec.empty())
    ssg_level = (sequenceIndex < ssgLevelSeqSpec.size()) ?
      ssgLevelSeqSpec[sequenceIndex] : ssgLevelSeqSpec.back();
  config_integration(quad_order, ssg_level,
		     probDescDB.get_rv("method.nond.dimension_preference"),
		     u_space_type, u_space_sampler, g_u_model);
  String pt_reuse, approx_type;
  config_approximation_type(approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
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
NonDMultilevelStochCollocation::
NonDMultilevelStochCollocation(Model& model, short exp_coeffs_approach,
			       const UShortArray& num_int_seq,
			       const RealVector& dim_pref, short u_space_type,
			       bool piecewise_basis, bool use_derivs):
  NonDStochCollocation(MULTIFIDELITY_STOCH_COLLOCATION, model,
		       exp_coeffs_approach, piecewise_basis, use_derivs),
  mlmfAllocControl(DEFAULT_MLMF_CONTROL), sequenceIndex(0)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: quadOrderSeqSpec = num_int_seq; break;
  default:                 ssgLevelSeqSpec = num_int_seq; break;
  }

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(u_space_type, data_order);
  initialize_random(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  unsigned short num_int = (sequenceIndex < num_int_seq.size()) ?
    num_int_seq[sequenceIndex] : num_int_seq.back();
  Iterator u_space_sampler;
  config_integration(expansionCoeffsApproach, num_int, dim_pref,
		     u_space_sampler, g_u_model);
  String pt_reuse, approx_type;
  config_approximation_type(approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
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


NonDMultilevelStochCollocation::~NonDMultilevelStochCollocation()
{ }


void NonDMultilevelStochCollocation::assign_discrepancy_mode()
{
  switch (multilevDiscrepEmulation) {
  /*
  case DISTINCT_EMULATION:
    if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
      Cerr << "Error: DISTINCT_EMULATION not currently supported for "
	   << "Multilevel SC with hierarchical interpolants." << std::endl;
      abort_handler(-1);
    }
    break;
  case RECURSIVE_EMULATION:
    if (expansionBasisType == Pecos::NODAL_INTERPOLANT) {
      Cerr << "Error: RECURSIVE_EMULATION not currently supported for "
	   << "Multilevel SC with nodal interpolants." << std::endl;
      abort_handler(-1);
    }
    break;
  */
  case DEFAULT_EMULATION: // assign method-specific default
    multilevDiscrepEmulation =
      //(expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) ?
      //RECURSIVE_EMULATION :
      DISTINCT_EMULATION;
    break;
  }
}


void NonDMultilevelStochCollocation::assign_hierarchical_response_mode()
{
  // override default SurrogateModel::responseMode for purposes of setting
  // comms for the ordered Models within HierarchSurrModel::set_communicators(),
  // which precedes mode updates in {multifidelity,multilevel}_expansion().

  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: multilevel/multifidelity expansions require a "
	 << "hierarchical model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Hierarchical SC is already based on surpluses, so avoid complexity of
  // using model discrepancies
  if (multilevDiscrepEmulation == RECURSIVE_EMULATION)
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);
  else
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);//MODEL_DISCREPANCY
  // AGGREGATED_MODELS avoids decimation of data and can simplify algorithms,
  // but requires repurposing origSurrData + modSurrData for high-low QoI pairs

  // Bind more than one SurrogateData instance via DataFitSurrModel ->
  // PecosApproximation
  uSpaceModel.link_multilevel_approximation_data();
}


bool NonDMultilevelStochCollocation::resize()
{
  bool parent_reinit_comms = NonDExpansion::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDMultilevelStochCollocation::core_run()
{
  initialize_expansion();
  sequenceIndex = 0;

  bool multifid_uq = true;
  switch (methodName) {
  case MULTIFIDELITY_STOCH_COLLOCATION:
    // algorithms inherited from NonDExpansion:
    switch (mlmfAllocControl) {
    case GREEDY_REFINEMENT:    greedy_multifidelity_expansion();    break;
    default:                   multifidelity_expansion(refineType); break;
    }
    break;
  //case MULTILEVEL_STOCH_COLLOCATION:
  //  multifid_uq = false;
  //  switch (mlmfAllocControl) {
  //  case GREEDY_REFINEMENT:    greedy_multifidelity_expansion();    break;
  //  case DEFAULT_MLMF_CONTROL: multifidelity_expansion(refineType); break;
  //  default:                   multilevel_sparse_grid();            break;
  //  }
  //  break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelStochCollocation::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }

  Cout << "\n----------------------------------------------------\n";
  if (multifid_uq) Cout << "Multifidelity UQ: ";
  else             Cout <<    "Multilevel UQ: ";
  Cout << "approximated high fidelity results"
       << "\n----------------------------------------------------\n\n";
  annotated_results(); // full set of statistics and debug traces (default)
  if (!summaryOutputFlag) // post_run() output is suppressed, leading to
    print_results(Cout);  // intermediate output wth no final output

  // clean up for re-entrancy of ML SC
  uSpaceModel.clear_inactive();

  ++numUncertainQuant;
}


void NonDMultilevelStochCollocation::assign_specification_sequence()
{
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex < quadOrderSeqSpec.size())
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    else if (refineControl)
      nond_quad->reset();   // reset driver to pre-refinement state
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex < ssgLevelSeqSpec.size())
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    else if (refineControl)
      nond_sparse->reset(); // reset driver to pre-refinement state
    break;
  }
  default:
    Cerr << "Error: unsupported expansion coefficient estimation approach in "
	 << "NonDMultilevelStochCollocation::assign_specification_sequence()"
	 << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}

void NonDMultilevelStochCollocation::increment_specification_sequence()
{
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex+1 < quadOrderSeqSpec.size()) {
      ++sequenceIndex;      // advance order sequence if sufficient entries
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    }
    else if (refineControl)
      nond_quad->reset();   // reset driver to pre-refinement state
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex+1 < ssgLevelSeqSpec.size()) {
      ++sequenceIndex;      // advance level sequence if sufficient entries
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    }
    else if (refineControl)
      nond_sparse->reset(); // reset driver to pre-refinement state
    break;
  }
  default:
    Cerr << "Error: unsupported expansion coefficient estimation approach in "
	 << "NonDMultilevelStochCollocation::increment_specification_sequence()"
	 << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}



void NonDMultilevelStochCollocation::metric_roll_up()
{
  // greedy_multifidelity_expansion() assesses level candidates using combined
  // stats --> roll up approx for combined stats
  if (mlmfAllocControl == GREEDY_REFINEMENT && refineControl)
    uSpaceModel.combine_approximation();
}


void NonDMultilevelStochCollocation::compute_covariance()
{
  // greedy_multifidelity_expansion() (multifidelity_expansion() on inner loop):
  // > roll up effect of level candidate on combined multilevel covariance,
  //   avoiding combined_to_active() promotion until end
  // > limited stats support for combinedExpCoeffs: only compute_covariance()
  if (mlmfAllocControl == GREEDY_REFINEMENT && refineControl)
    compute_combined_covariance();
  // multifidelity_expansion() is outer loop:
  // > use of refine_expansion(): refine individually based on level covariance
  // > after combine_approx(), combined_to_active() enables use of active covar
  else
    NonDExpansion::compute_covariance();
}


Real NonDMultilevelStochCollocation::
compute_covariance_metric(bool restore_ref, bool print_metric,
			  bool relative_metric)
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {

    // perform any roll-ups of expansion contributions, prior to metric compute
    metric_roll_up();

    size_t i, j;
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    bool warn_flag = false,
      all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
      //compute_ref = (relative_metric || !restore_ref);
    for (i=0; i<numFunctions; ++i) {
      PecosApproximation* pa_rep_i
	= (PecosApproximation*)poly_approxs[i].approx_rep();
      if (!pa_rep_i->expansion_coefficient_flag())
	{ warn_flag = true; break; }
    }
    if (warn_flag)
      Cerr << "Warning: expansion coefficients unavailable in NonDMultilevel"
	   << "StochCollocation::compute_covariance_metric().\n         "
	   << "Zeroing affected delta_covariance terms." << std::endl;

    Real scale, delta_norm;
    switch (covarianceControl) {
    case DIAGONAL_COVARIANCE: {
      RealVector delta_resp_var(numFunctions, false);
      for (i=0; i<numFunctions; ++i) {
	PecosApproximation* pa_rep_i
	  = (PecosApproximation*)poly_approxs[i].approx_rep();
	if (pa_rep_i->expansion_coefficient_flag()) {
	  if (mlmfAllocControl == GREEDY_REFINEMENT && refineControl)
	    // refinement assessed for impact on combined expansion from roll up
	    delta_resp_var[i] = (all_vars) ?
	      pa_rep_i->delta_combined_covariance(initialPtU, pa_rep_i) :
	      pa_rep_i->delta_combined_covariance(pa_rep_i);
	  else // refinement assessed for impact on the current expansion
	    delta_resp_var[i] = (all_vars) ?
	      pa_rep_i->delta_covariance(initialPtU, pa_rep_i) :
	      pa_rep_i->delta_covariance(pa_rep_i);
	}
	else delta_resp_var[i] = 0.;
      }

      delta_norm = delta_resp_var.normFrobenius();
      if (relative_metric) // reference covariance, bounded from zero
	scale = std::max(Pecos::SMALL_NUMBER, respVariance.normFrobenius());
      // reference covariance gets restored in NonDExpansion::increment_sets()
      if (!restore_ref) respVariance += delta_resp_var;
      if (print_metric) print_variance(Cout, delta_resp_var, "Change in");
      break;
    }
    case FULL_COVARIANCE: {
      RealSymMatrix delta_resp_covar(numFunctions, false);
      for (i=0; i<numFunctions; ++i) {
	PecosApproximation* pa_rep_i
	  = (PecosApproximation*)poly_approxs[i].approx_rep();
	if (pa_rep_i->expansion_coefficient_flag())
	  for (j=0; j<=i; ++j) {
	    PecosApproximation* pa_rep_j
	      = (PecosApproximation*)poly_approxs[j].approx_rep();
	    if (pa_rep_j->expansion_coefficient_flag()) {
	      if (mlmfAllocControl == GREEDY_REFINEMENT && refineControl)
		// refinement assessed for impact on combined exp from roll up
		delta_resp_covar(i,j) = (all_vars) ?
		  pa_rep_i->delta_combined_covariance(initialPtU, pa_rep_j) :
		  pa_rep_i->delta_combined_covariance(pa_rep_j);
	      else // refinement assessed for impact on the current expansion
		delta_resp_covar(i,j) = (all_vars) ?
		  pa_rep_i->delta_covariance(initialPtU, pa_rep_j) :
		  pa_rep_i->delta_covariance(pa_rep_j);
	    }
	    else delta_resp_covar(i,j) = 0.;
	  }
	else
	  for (j=0; j<=i; ++j)
	    delta_resp_covar(i,j) = 0.;
      }

      // Metric scale is determined from reference covariance.  While defining
      // the scale from an updated covariance would eliminate problems with
      // zero covariance for adaptations from level 0, different refinement
      // candidates would score equally at 1 (induced 100% of change in
      // updated covariance) in this initial set of candidates.  Therefore,
      // use reference covariance as the scale and trap covariance underflows.
      delta_norm = delta_resp_covar.normFrobenius();
      if (relative_metric) // reference covariance, bounded from zero
	scale = std::max(Pecos::SMALL_NUMBER, respCovariance.normFrobenius());
      // reference covariance gets restored in NonDExpansion::increment_sets()
      if (!restore_ref) respCovariance += delta_resp_covar;
      if (print_metric) print_covariance(Cout, delta_resp_covar, "Change in");
      break;
    }
    }

    return (relative_metric) ? delta_norm / scale : delta_norm;
  }
  else // use default implementation
    return NonDExpansion::
      compute_covariance_metric(restore_ref, print_metric, relative_metric);
}


Real NonDMultilevelStochCollocation::
compute_final_statistics_metric(bool restore_ref, bool print_metric,
				bool relative_metric)
{
  if (expansionBasisType == Pecos::HIERARCHICAL_INTERPOLANT) {
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

      // *** TO DO: update NonDStochCollocation::compute_final_stats_metric()
      // ***        to compute delta's relative to combined stats

      Cerr << "Error: NonDMultilevelStochCollocation::compute_final_statistics"
	   << "_metric() not yet implemented." << std::endl;
      abort_handler(METHOD_ERROR);      
    }
    else // use default implementation if no beta-mapping increments
      return NonDExpansion::
	compute_final_statistics_metric(restore_ref, print_metric,
					relative_metric);
  }
  else // use default implementation for Nodal
    return NonDExpansion::
      compute_final_statistics_metric(restore_ref, print_metric,
				      relative_metric);
}

} // namespace Dakota
