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
  quadOrderSeqSpec(probDescDB.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(probDescDB.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0)
{
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
  const UShortArray& quad_order_seq
    = probDescDB.get_usa("method.nond.quadrature_order");
  const UShortArray& ssg_level_seq
    = probDescDB.get_usa("method.nond.sparse_grid_level");
  const RealVector& dim_pref
    = probDescDB.get_rv("method.nond.dimension_preference");
  check_dimension_preference(dim_pref);
  if (!quad_order_seq.empty()) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    unsigned short quad_order = (sequenceIndex < quad_order_seq.size()) ?
      quad_order_seq[sequenceIndex] : quad_order_seq.back();
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref);
  }
  else if (!ssg_level_seq.empty()) {
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
    unsigned short ssg_level = (sequenceIndex < ssg_level_seq.size()) ?
      ssg_level_seq[sequenceIndex] : ssg_level_seq.back();
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level, dim_pref);
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
NonDMultilevelStochCollocation::
NonDMultilevelStochCollocation(Model& model, short exp_coeffs_approach,
			       const UShortArray& num_int_seq,
			       const RealVector& dim_pref, short u_space_type,
			       bool piecewise_basis, bool use_derivs):
  NonDStochCollocation(MULTIFIDELITY_STOCH_COLLOCATION, model,
		       exp_coeffs_approach, piecewise_basis, use_derivs)
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
  unsigned short num_int = (sequenceIndex < num_int_seq.size()) ?
    num_int_seq[sequenceIndex] : num_int_seq.back();
  Iterator u_space_sampler;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_quadrature(u_space_sampler, g_u_model, num_int, dim_pref);
    break;
  case Pecos::COMBINED_SPARSE_GRID:
    expansionBasisType = Pecos::NODAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int, dim_pref);
    break;
  case Pecos::HIERARCHICAL_SPARSE_GRID:
    expansionBasisType = Pecos::HIERARCHICAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int, dim_pref);
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


NonDMultilevelStochCollocation::~NonDMultilevelStochCollocation()
{ }


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

  /* *** TO DO: *** sanity checks;  Allow model forms?
  size_t num_mf = iteratedModel.subordinate_models(false).size(),
     num_hf_lev = iteratedModel.truth_model().solution_levels();
     // for now, only SimulationModel supports solution_levels()
  if (num_mf > 1 && num_hf_lev == 1)                     // multifidelity PCE
    NonDExpansion::multifidelity_expansion();
  else if (num_mf == 1 && num_hf_lev > 1 &&              // multilevel LLS/CS
	   expansionCoeffsApproach >= Pecos::DEFAULT_REGRESSION) {

  }
  else {
    Cerr << "Error: unsupported combination of fidelities and levels within "
	 << "NonDMultilevelStochCollocation::core_run()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  */

  switch (methodName) {
  case MULTIFIDELITY_STOCH_COLLOCATION:
    multifidelity_expansion(); // from NonDExpansion
    break;
  // case MULTILEVEL_STOCH_COLLOCATION:
  //   if (multilevDiscrepEmulation == RECURSIVE_EMULATION)
  //     recursive_regression(0);
  //   else
  //     multilevel_regression(0);
  //   break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelStochCollocation::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
  
  // generate final results
  compute_print_converged_results();
  update_final_statistics(); // virtual fn redefined below
  ++numUncertainQuant;
}

} // namespace Dakota
