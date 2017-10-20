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
  quadOrderSeqSpec(probDescDB.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(probDescDB.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0)
{
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
  sequenceIndex(0)
{
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
  //     recursive_sparse_grid(0);
  //   else
  //     multilevel_sparse_grid(0);
  //   break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelStochCollocation::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }

  // generate final results
  Cout << "\n----------------------------------------------------"
       << "\nMultifidelity UQ: approximated high fidelity results"
       << "\n----------------------------------------------------\n\n";
  annotated_results(); // full set of statistics and debug traces (default)
  if (!summaryOutputFlag) // post_run() output is suppressed, leading to
    print_results(Cout);  // intermediate output wth no final output

  // clean up for re-entrancy of ML SC
  uSpaceModel.clear_stored();

  ++numUncertainQuant;
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
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::HIERARCHICAL_SPARSE_GRID: {
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

} // namespace Dakota
