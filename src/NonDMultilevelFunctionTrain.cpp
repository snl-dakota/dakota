/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDMultilevelFunctionTrain
//- Description: Implementation code for NonDMultilevelFunctionTrain class
//- Owner:       Mike Eldred, Sandia National Laboratories

#include "NonDMultilevelFunctionTrain.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "SharedC3ApproxData.hpp"
#include "C3Approximation.hpp"
#include "NonDQuadrature.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDMultilevelFunctionTrain::
NonDMultilevelFunctionTrain(ProblemDescDB& problem_db, Model& model):
  NonDC3FunctionTrain(DEFAULT_METHOD, problem_db, model),
  startRankSeqSpec(
    problem_db.get_sza("method.nond.c3function_train.start_rank_sequence")),
  startOrderSeqSpec(
    problem_db.get_usa("method.nond.c3function_train.start_order_sequence")),
  sequenceIndex(0) //resizedFlag(false), callResize(false)
{
  randomSeedSeqSpec = problem_db.get_sza("method.random_seed_sequence");

  assign_modes();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  // See SharedC3ApproxData::construct_basis().  C3 won't support STD_{BETA,
  // GAMMA,EXPONENTIAL} so use PARTIAL_ASKEY_U to map to STD_{NORMAL,UNIFORM}.
  short u_space_type = PARTIAL_ASKEY_U;//probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>
		       (iteratedModel, u_space_type)); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler; // evaluates truth model
  if (!config_regression(collocation_points(), regression_size(sequenceIndex),
			 random_seed(), u_space_sampler, g_u_model)) {
    Cerr << "Error: incomplete configuration in NonDMultilevelFunctionTrain "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  UShortArray start_orders;
  configure_expansion_orders(start_order(), dimPrefSpec, start_orders);
  short corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import
  String approx_type = "global_function_train";
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel consumes QoI aggregations; supports surrogate grad evals at most
  ShortArray asv(g_u_model.qoi(), 3); // for stand alone mode
  ActiveSet mlft_set(asv, recast_set.derivative_vector());
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, mlft_set, approx_type, start_orders, corr_type, corr_order,
    data_order, outputLevel, pt_reuse, importBuildPointsFile,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")));
  initialize_u_space_model();

  // Configure settings for ML allocation (requires uSpaceModel)
  assign_allocation_control();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(problem_db.get_ushort("method.sample_type"),
    problem_db.get_string("method.random_number_generator"),
    problem_db.get_ushort("method.nond.integration_refinement"),
    problem_db.get_iv("method.nond.refinement_samples"),
    probDescDB.get_string("method.import_approx_points_file"),
    probDescDB.get_ushort("method.import_approx_format"),
    probDescDB.get_bool("method.import_approx_active_only"));

  if (parallelLib.command_line_check())
    Cout << "\nFunction train construction completed: initial grid size of "
	 << numSamplesOnModel << " evaluations to be performed." << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ regression.
NonDMultilevelFunctionTrain::
NonDMultilevelFunctionTrain(unsigned short method_name, Model& model,
			    const SizetArray& colloc_pts_seq,
			    const RealVector& dim_pref,
			    Real colloc_ratio, const SizetArray& seed_seq,
			    short u_space_type, short refine_type,
			    short refine_control, short covar_control,
			    short ml_alloc_control, short ml_discrep,
			    //short rule_nest, short rule_growth,
			    bool piecewise_basis, bool use_derivs,
			    bool cv_flag, const String& import_build_pts_file,
			    unsigned short import_build_format,
			    bool import_build_active_only):
  NonDC3FunctionTrain(method_name, model, exp_coeffs_approach, dim_pref,
		      u_space_type, refine_type, refine_control, covar_control,
		      colloc_pts_seq, colloc_ratio, ml_alloc_control,
		      ml_discrep, //rule_nest, rule_growth,
		      piecewise_basis, use_derivs, 0, cv_flag),
  expOrderSeqSpec(exp_order_seq), sequenceIndex(0)
{
  randomSeedSeqSpec = seed_seq;

  assign_modes();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>
		       (iteratedModel, uSpaceType)); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler; // evaluates truth model
  if (!config_regression(collocation_points(), regression_size(sequenceIndex),
                         random_seed(), u_space_sampler, g_u_model)){
    Cerr << "Error: incomplete configuration in NonDMultilevelFunctionTrain "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  UShortArray start_orders;
  configure_expansion_orders(start_order(), dimPrefSpec, start_orders);
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (!import_build_pts_file.empty()) pt_reuse = "all";
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model.qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, approx_type, start_orders, corr_type, corr_order,
    data_order, outputLevel, pt_reuse, import_build_pts_file,
    import_build_format, import_build_active_only));
  initialize_u_space_model();

  // Configure settings for ML allocation (requires uSpaceModel)
  assign_allocation_control();

  // no expansionSampler, no numSamplesOnExpansion
}
*/


NonDMultilevelFunctionTrain::~NonDMultilevelFunctionTrain()
{ }


/** This implementation differs from those in C3Approximation and
    SharedC3ApproxData in that they are used for sample initialization
    from specification sequences, prior to any adaptation.  They pass 
    current/max values to the general SharedC3ApproxData helper. */
size_t NonDMultilevelFunctionTrain::regression_size(size_t index)
{
  // compute initial regression size using a static helper
  // (uSpaceModel.shared_approximation() is not yet available)

  bool max_r, max_o;
  switch (c3AdvancementType) {
  case MAX_RANK_ADVANCEMENT:       max_r = true;  max_o = false;  break;
  case MAX_ORDER_ADVANCEMENT:      max_o = true;  max_r = false;  break;
  case MAX_RANK_ORDER_ADVANCEMENT: max_r = max_o = true;          break;
  default:                         max_r = max_o = false;         break;
  }
  UShortArray regress_o;
  if (max_o) // order anisotropy not supported by adapt_order search
    //configure_expansion_orders(maxOrderSpec, dimPrefSpec, orders);
    regress_o.assign(numContinuousVars, maxOrderSpec);
  else
    configure_expansion_orders(start_order(index), dimPrefSpec, regress_o);
  size_t regress_r = (max_r) ? maxRankSpec : start_rank(index);
  return SharedC3ApproxData::regression_size(numContinuousVars,
    regress_r, maxRankSpec, regress_o, maxOrderSpec);
}


void NonDMultilevelFunctionTrain::initialize_u_space_model()
{
  // For greedy ML, activate combined stats now for propagation to Pecos
  // > don't call statistics_type() as ExpansionConfigOptions not initialized
  //if (multilevAllocControl == GREEDY_REFINEMENT)
  //  statsMetricType = Pecos::COMBINED_EXPANSION_STATS;

  NonDExpansion::initialize_u_space_model();

  initialize_c3_db_options(); // scalars (maxRankSpec,maxOrderSpec,randomSeed)

  // start rank & random seed not used by C3 until C3Approx::build()
  // > MF: assign_specification_sequence(),update_u_space_sampler() are used
  // > ML:     increment_sample_sequence(),update_u_space_sampler() are used
  //initialize_c3_start_rank(start_rank());
  //initialize_c3_seed(random_seed()); // sequence spec
  //
  // init of start orders redundant w/ DataFitSurrModel/SharedC3ApproxData ctors
  //UShortArray orders;
  //configure_expansion_orders(start_order(), dimPrefSpec, orders);
  //initialize_c3_start_orders(orders);

  // SharedC3ApproxData invokes ope_opts_alloc() to construct basis and
  // requires {start,max} order
  const Pecos::MultivariateDistribution& u_dist
    = uSpaceModel.truth_model().multivariate_distribution();
  uSpaceModel.shared_approximation().construct_basis(u_dist);

  // emulation mode needed for ApproximationInterface::qoi_set_to_key_index()
  uSpaceModel.discrepancy_emulation_mode(multilevDiscrepEmulation);
}


void NonDMultilevelFunctionTrain::core_run()
{
  initialize_expansion();
  sequenceIndex = 0;

  bool multifid_uq = false;
  switch (methodName) {
  case MULTIFIDELITY_FUNCTION_TRAIN:
    multifid_uq = true;
    multifidelity_expansion();    break;
  case MULTILEVEL_FUNCTION_TRAIN:
    multilevel_regression();      break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelFunctionTrain::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);  break;
  }

  // generate final results
  Cout << "\n----------------------------------------------------\n";
  if (multifid_uq) Cout << "Multifidelity UQ: ";
  else             Cout <<    "Multilevel UQ: ";
  Cout << "approximated high fidelity results"
       << "\n----------------------------------------------------\n\n";
  compute_statistics(FINAL_RESULTS);
  // Override summaryOutputFlag control (see Analyzer::post_run()) for ML case
  // to avoid intermediate output with no final output
  if (!summaryOutputFlag) print_results(Cout, FINAL_RESULTS);

  // clean up for re-entrancy of ML FT
  uSpaceModel.clear_inactive();

  finalize_expansion();
}


void NonDMultilevelFunctionTrain::assign_allocation_control()
{
  // multilevAllocControl config and specification checks:
  switch (methodName) {
  case MULTILEVEL_FUNCTION_TRAIN:
    // multilevAllocControl config and specification checks:
    switch (multilevAllocControl) {
    case DEFAULT_MLMF_CONTROL: // define MLFT-specific default
      multilevAllocControl = RANK_SAMPLING; break;
    case RANK_SAMPLING: {
      // ensure adaptRank is on since RANK_SAMPLING seeks to discover
      // the underlying low-rank structure of the QoI
      std::shared_ptr<SharedC3ApproxData> shared_data_rep =
	std::static_pointer_cast<SharedC3ApproxData>(
	uSpaceModel.shared_approximation().data_rep());
      shared_data_rep->set_parameter("adapt_rank", true);
      // Note: C3Approximation::build() defines an appropriate range in the
      //       case of unspecified max rank: [start_r, start_r+max_cv*kick_r]
      break;
    }
    case ESTIMATOR_VARIANCE:
      // estimator variance is not dependent on rank: adapt rank is optional
      break;
    default:
      Cerr << "Error: unsupported multilevAllocControl in "
	   << "NonDMultilevelFunctionTrain constructor." << std::endl;
      abort_handler(METHOD_ERROR);           break;
    }
    break;
  case MULTIFIDELITY_FUNCTION_TRAIN:
    break; // GREEDY remains off by default
  }
}


void NonDMultilevelFunctionTrain::assign_specification_sequence()
{
  // This fn assigns the user specification from the relevant model sequences,
  // prior to any refinement/adaptation (use local attributes, not the state
  // of SharedC3ApproxData,C3Approximation

  UShortArray start_orders;
  configure_expansion_orders(start_order(), dimPrefSpec, start_orders);
  push_c3_active(start_orders); // push active {start,max}{order,rank}, seed

  size_t colloc_pts = collocation_points();
  if (colloc_pts == SZ_MAX) { // seq not defined
    if (collocRatio > 0.)
      numSamplesOnModel
	= terms_ratio_to_samples(regression_size(sequenceIndex), collocRatio);
    else {
      Cerr << "Error: incomplete specification in NonDMultilevelFunctionTrain"
	   << "::assign_specification_sequence()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  else
    numSamplesOnModel = colloc_pts;

  update_u_space_sampler(sequenceIndex, start_orders);
}


void NonDMultilevelFunctionTrain::increment_specification_sequence()
{
  // regression

  // advance sequence specs (Note: sequenceIndex admissibility is adequately
  // handled in active value accessors)
  //size_t next_i = sequenceIndex + 1;
  //if (next_i <  collocPtsSeqSpec.size() || next_i < startRankSeqSpec.size() ||
  //    next_i < startOrderSeqSpec.size() || next_i < randomSeedSeqSpec.size())
    ++sequenceIndex;

  assign_specification_sequence();
}


void NonDMultilevelFunctionTrain::
initialize_ml_regression(size_t num_lev, bool& import_pilot)
{
  NonDExpansion::initialize_ml_regression(num_lev, import_pilot);

  // Build point import is active only for the pilot sample and we overlay an
  // additional pilot_sample spec, but we do not augment with samples from a
  // collocation pts/ratio enforcement (pts/ratio controls take over on
  // subsequent iterations).
  if (!importBuildPointsFile.empty()) {
    if (multilevDiscrepEmulation == RECURSIVE_EMULATION) {
      Cout << "\nPilot sample to include imported build points.\n";
      import_pilot = true;
    }
    else
      Cerr << "Warning: build data import only supported for recursive "
	   << "emulation in multilevel_regression()." << std::endl;
  }
}


void NonDMultilevelFunctionTrain::
infer_pilot_sample(/*Real ratio, */size_t num_steps, SizetArray& pilot)
{
  pilot.resize(num_steps);
  for (size_t i=0; i<num_steps; ++i)
    pilot[i] = (size_t)std::floor(collocRatio * (Real)regression_size(i) + .5);
}


void NonDMultilevelFunctionTrain::
increment_sample_sequence(size_t new_samp, size_t total_samp, size_t step)
{
  numSamplesOnModel = new_samp; // total_samp,lev not used by this derived class

  // ML FT: This virtual fn is called from multilevel_regression() for the
  //   current resolution level.  No explicit rank/order refinement occurs
  //   (only implicit via C3 adapt_rank), but sequence spec (along with any
  //   future advancements) must be propagated.
  // MF FT: rank/order advancements occur in {assign,increment}_specification_
  //   sequence() above
  sequenceIndex = step;
  UShortArray orders;
  configure_expansion_orders(start_order(), dimPrefSpec, orders);
  push_c3_active(orders);

  update_u_space_sampler(step, orders);
}


void NonDMultilevelFunctionTrain::
compute_sample_increment(const RealVector& regress_metrics,
			 const SizetArray& N_l, SizetArray& delta_N_l)
{
  // case RANK_SAMPLING in NonDExpansion::multilevel_regression():

  // update targets based on rank estimates: sample requirements scale as
  // O(p r^2 d), which shapes the profile
  //size_t lev, num_lev = N_l.size();
  //RealVector new_N_l(num_lev, false);
  //Real r, fact_var = collocRatio * numContinuousVars;
  //for (lev=0; lev<num_lev; ++lev)
  //  { r = rank[lev];  new_N_l[lev] = fact_var * r * r; }

  // NOTE: feedback of new samples inducing increased rank can happen but
  //       can be controlled with the rounding tolerance.
  // > Alex checks 2 things before advancing the rank: (1) if CV error decreases
  //   and if ranks decrease when rounding, then done
  // > May need to tune this user spec (and its default)

  // update targets based on regression size
  RealVector new_N_l  = regress_metrics; // number of unknowns (RMS across QoI)
  if (collocRatio > 0.) new_N_l.scale(collocRatio);
  else                  new_N_l.scale(2.); // default: over-sample 2x

  // Retain the shape of the profile but enforce an upper bound
  //scale_profile(..., new_N_l);

  size_t lev, num_lev = N_l.size();
  delta_N_l.resize(num_lev);
  for (lev=0; lev<num_lev; ++lev)
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l[lev]);
}


void NonDMultilevelFunctionTrain::
print_results(std::ostream& s, short results_state)
{
  switch (results_state) {
  case REFINEMENT_RESULTS:  case INTERMEDIATE_RESULTS:
    //if (outputLevel == DEBUG_OUTPUT)   print_coefficients(s);
    break;
  case FINAL_RESULTS:
    //if (outputLevel >= NORMAL_OUTPUT)  print_coefficients(s);
    //if (!expansionExportFile.empty())  export_coefficients();
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

  // skip over NonDC3FunctionTrain::print_results()
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
