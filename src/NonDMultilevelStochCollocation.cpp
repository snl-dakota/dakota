/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDMultilevelStochCollocation.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSparseGrid.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "PecosApproximation.hpp"
#include "SharedInterpPolyApproxData.hpp"
#include "dakota_system_defs.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION
//#define DEBUG


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDMultilevelStochCollocation::
NonDMultilevelStochCollocation(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  NonDStochCollocation(DEFAULT_METHOD, problem_db, parallel_lib, model), // bypass SC ctor
  quadOrderSeqSpec(problem_db.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(problem_db.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0)
{
  assign_modes();
  configure_1d_sequence(numSteps, secondaryIndex, sequenceType);
  costSource
    = initialize_costs(sequenceCost, modelCostSpec, costMetadataIndices);

  // ----------------
  // Resolve settings
  // ----------------
  short data_order,
    u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  auto g_u_model = std::make_shared<ProbabilityTransformModel>(
    iteratedModel, u_space_type); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  std::shared_ptr<Iterator> u_space_sampler;
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
  short corr_order = -1, corr_type = NO_CORRECTION;
  UShortArray approx_order; // empty
  const ActiveSet& recast_set = g_u_model->current_response().active_set();
  // DFSModel: consume any QoI aggregation; support surrogate gradient evals
  ShortArray sc_asv(g_u_model->qoi(), 3); // for stand alone mode
  ActiveSet  sc_set(sc_asv, recast_set.derivative_vector());
  const ShortShortPair& sc_view = g_u_model->current_variables().view();
  String empty_str; // build data import not supported for structured grids
  uSpaceModel = std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, sc_set, sc_view, approx_type, approx_order, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, empty_str, TABULAR_ANNOTATED,
    false, probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format"));
  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler(problem_db.get_ushort("method.sample_type"),
    problem_db.get_string("method.random_number_generator"),
    problem_db.get_ushort("method.nond.integration_refinement"),
    problem_db.get_iv("method.nond.refinement_samples"),
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
NonDMultilevelStochCollocation(std::shared_ptr<Model> model, short exp_coeffs_approach,
			       const UShortArray& num_int_seq,
			       const RealVector& dim_pref, short u_space_type,
			       short refine_type, short refine_control,
			       short covar_control, short ml_alloc_cntl,
			       short ml_discrep, short rule_nest,
			       short rule_growth, bool piecewise_basis,
			       bool use_derivs):
  NonDStochCollocation(MULTIFIDELITY_STOCH_COLLOCATION, model,
		       exp_coeffs_approach, dim_pref, refine_type,
		       refine_control, covar_control, ml_alloc_cntl, ml_discrep,
		       rule_nest, rule_growth, piecewise_basis, use_derivs),
  sequenceIndex(0)
{
  assign_modes();
  configure_1d_sequence(numSteps, secondaryIndex, sequenceType);
  costSource
    = initialize_costs(sequenceCost, modelCostSpec, costMetadataIndices);

  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: quadOrderSeqSpec = num_int_seq; break;
  default:                 ssgLevelSeqSpec = num_int_seq; break;
  }

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  auto g_u_model = std::make_shared<ProbabilityTransformModel>(
    iteratedModel, u_space_type); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  unsigned short num_int = (sequenceIndex < num_int_seq.size()) ?
    num_int_seq[sequenceIndex] : num_int_seq.back();
  std::shared_ptr<Iterator> u_space_sampler;
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
  short corr_order = -1, corr_type = NO_CORRECTION;
  UShortArray approx_order; // empty
  const ActiveSet& recast_set = g_u_model->current_response().active_set();
  // DFSModel: consume any QoI aggregation.
  // TO DO: support surrogate Hessians in helper mode.
  ShortArray sc_asv(g_u_model->qoi(), 3); // TO DO: consider passing in data_mode
  ActiveSet  sc_set(sc_asv, recast_set.derivative_vector());
  const ShortShortPair& sc_view = g_u_model->current_variables().view();
  uSpaceModel = std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, sc_set, sc_view, approx_type, approx_order, corr_type,
    corr_order, data_order, outputLevel, pt_reuse);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDMultilevelStochCollocation::~NonDMultilevelStochCollocation()
{ }


void NonDMultilevelStochCollocation::initialize_u_space_model()
{
  // For greedy ML, activate combined stats now for propagation to Pecos
  // > don't call statistics_type() as ExpansionConfigOptions not initialized
  //if (multilevAllocControl == GREEDY_REFINEMENT)
  //  statsMetricType = Pecos::COMBINED_EXPANSION_STATS;

  // initializes ExpansionConfigOptions, among other things
  NonDStochCollocation::initialize_u_space_model();

  // emulation mode needed for ApproximationInterface::qoi_set_to_key_index()
  uSpaceModel->discrepancy_emulation_mode(multilevDiscrepEmulation);

  // Bind more than one SurrogateData instance via DataFitSurrModel ->
  // PecosApproximation
  //uSpaceModel->link_multilevel_approximation_data();
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
    multifidelity_expansion();    break;
  //case MULTILEVEL_STOCH_COLLOCATION:
  //  multifid_uq = false;
  //  multilevel_sparse_grid();   break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelStochCollocation::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);  break;
  }

  Cout << "\n----------------------------------------------------\n";
  if (multifid_uq) Cout << "Multifidelity UQ: ";
  else             Cout <<    "Multilevel UQ: ";
  Cout << "approximated high fidelity results"
       << "\n----------------------------------------------------\n\n";
  compute_statistics(FINAL_RESULTS);
  // Override summaryOutputFlag control (see Analyzer::post_run()) for ML case
  // to avoid intermediate output with no final output
  if (!summaryOutputFlag) print_results(Cout, FINAL_RESULTS);

  // clean up for re-entrancy of ML SC
  uSpaceModel->clear_inactive();

  finalize_expansion();
}


void NonDMultilevelStochCollocation::assign_specification_sequence()
{
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    std::shared_ptr<NonDQuadrature> nond_quad =
      std::static_pointer_cast<NonDQuadrature>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex < quadOrderSeqSpec.size())
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    else //if (refineControl)
      nond_quad->reset();   // reset refinement, capture dist param updates
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex < ssgLevelSeqSpec.size())
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    else //if (refineControl)
      nond_sparse->reset(); // reset refinement, capture dist param updates
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
    std::shared_ptr<NonDQuadrature> nond_quad =
      std::static_pointer_cast<NonDQuadrature>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex+1 < quadOrderSeqSpec.size()) {
      ++sequenceIndex;      // advance order sequence if sufficient entries
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    }
    else //if (refineControl)
      nond_quad->reset();   // reset refinement, capture dist param updates
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex+1 < ssgLevelSeqSpec.size()) {
      ++sequenceIndex;      // advance level sequence if sufficient entries
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    }
    else //if (refineControl)
      nond_sparse->reset(); // reset refinement, capture dist param updates
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


/*
void NonDMultilevelStochCollocation::combined_to_active()
{
  switch (expansionBasisType) {
  case Pecos::NODAL_INTERPOLANT:
    NonDExpansion::combined_to_active(); break;
  case Pecos::HIERARCHICAL_INTERPOLANT:
    uSpaceModel->combine_approximation();
    // copy combined to active, but retain combined for use in hybrid stats.
    // *** This is a short term solution; best solution may be to
    //     support complete set of stats using the combined data.
    uSpaceModel->combined_to_active(false);
    // don't force update to active statistics; allow hybrid approach where
    // combined can still be used when needed (integrate_response_moments())
    //statistics_type(Pecos::ACTIVE_EXPANSION_STATS, false); // don't restore
    statistics_type(Pecos::COMBINED_EXPANSION_STATS, false); // override
    break;
  }
}
*/


void NonDMultilevelStochCollocation::
print_results(std::ostream& s, short results_state)
{
  switch (results_state) {
  //case REFINEMENT_RESULTS:
  //case INTERMEDIATE_RESULTS:
  //  break;
  case FINAL_RESULTS:
    if (!NLev.empty()) {
      s << "<<<<< Samples per solution level:\n";
      print_multilevel_evaluation_summary(s, NLev);
      if (equivHFEvals > 0.) {
	s << "<<<<< Equivalent number of high fidelity evaluations: "
	  << equivHFEvals << std::endl;
	archive_equiv_hf_evals(equivHFEvals);
      }
    }
    break;
  }

  // nothing defined at NonDStochCollocation level
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
