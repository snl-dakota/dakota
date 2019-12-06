/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
  NonDC3FunctionTrain(BaseConstructor(), problem_db, model),
  mlmfAllocControl(
    probDescDB.get_short("method.nond.multilevel_allocation_control")),
  collocPtsSeqSpec(probDescDB.get_sza("method.nond.collocation_points")),
  sequenceIndex(0),
  kappaEstimatorRate(
    probDescDB.get_real("method.nond.multilevel_estimator_rate")),
  gammaEstimatorScale(1.),
  pilotSamples(probDescDB.get_sza("method.nond.pilot_samples")),
  importBuildPointsFile(
    probDescDB.get_string("method.import_build_points_file"))
  //resizedFlag(false), callResize(false)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

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
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    u_space_type), false); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // extract sequences and invoke shared helper fn with a scalar...
  size_t colloc_pts = std::numeric_limits<size_t>::max();
  if (!collocPtsSeqSpec.empty())
    colloc_pts = (sequenceIndex < collocPtsSeqSpec.size()) ?
      collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();
  Iterator u_space_sampler;
  if (!config_regression(colloc_pts, u_space_sampler, g_u_model)) {
    Cerr << "Error: incomplete configuration in NonDMultilevelFunctionTrain "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // mlmfAllocControl config and specification checks:
  //if (mlmfAllocControl == RANK_SAMPLING)
  //  crossValidation = crossValidNoiseOnly = true;

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  UShortArray approx_order; // unused by C3
  short corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import
  String approx_type = "global_function_train";
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel consumes QoI aggregations; supports surrogate grad evals at most
  ShortArray asv(g_u_model.qoi(), 3); // for stand alone mode
  ActiveSet mlft_set(asv, recast_set.derivative_vector());
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    mlft_set, approx_type, approx_order, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, importBuildPointsFile,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(
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
			      short exp_coeffs_approach,
			      const UShortArray& exp_order_seq,
			      const RealVector& dim_pref,
			      const SizetArray& colloc_pts_seq,
			      Real colloc_ratio, const SizetArray& pilot,
			      int seed, short u_space_type,
			      short refine_type, short refine_control,
			      short covar_control, short ml_alloc_control,
			      short ml_discrep,
			      //short rule_nest, short rule_growth,
			      bool piecewise_basis, bool use_derivs,
			      bool cv_flag, const String& import_build_pts_file,
			      unsigned short import_build_format,
			      bool import_build_active_only):
  NonDC3FunctionTrain(method_name, model, exp_coeffs_approach, dim_pref,
		      u_space_type, refine_type, refine_control, covar_control,
		      ml_discrep, //rule_nest, rule_growth,
		      piecewise_basis, use_derivs, colloc_ratio, seed, cv_flag),
  mlmfAllocControl(ml_alloc_control), expOrderSeqSpec(exp_order_seq),
  collocPtsSeqSpec(colloc_pts_seq), sequenceIndex(0), kappaEstimatorRate(2.),
  gammaEstimatorScale(1.), importBuildPointsFile(import_build_pts_file),
  pilotSamples(pilot)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  unsigned short exp_order_spec = USHRT_MAX;
  size_t colloc_pts = std::numeric_limits<size_t>::max();
  UShortArray exp_orders; // defined for expansion_samples/regression
  if (!expOrderSeqSpec.empty()) {
    exp_order_spec = (sequenceIndex  < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[sequenceIndex] : expOrderSeqSpec.back();
    config_expansion_orders(exp_order_spec, dimPrefSpec, exp_orders);
  }
  if (!collocPtsSeqSpec.empty())
    colloc_pts =      (sequenceIndex  < collocPtsSeqSpec.size()) ?
      collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();

  Iterator u_space_sampler;
  UShortArray tensor_grid_order; // for OLI + tensorRegression (not supported)
  String approx_type, rng("mt19937"), pt_reuse;
  config_regression(exp_orders, colloc_pts, 1, exp_coeffs_approach,
		    Pecos::DEFAULT_LEAST_SQ_REGRESSION, tensor_grid_order,
		    SUBMETHOD_LHS, rng, pt_reuse, u_space_sampler,
		    g_u_model, approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (!importBuildPointsFile.empty()) pt_reuse = "all";
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model.qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, importBuildPointsFile, import_build_format,
    import_build_active_only), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}
*/


NonDMultilevelFunctionTrain::~NonDMultilevelFunctionTrain()
{ }


void NonDMultilevelFunctionTrain::assign_hierarchical_response_mode()
{
  // override default SurrogateModel::responseMode for purposes of setting
  // comms for the ordered Models within HierarchSurrModel::set_communicators(),
  // which precedes mode updates in {multifidelity,multilevel}_expansion().

  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: multilevel/multifidelity expansions require a "
	 << "hierarchical model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // ML-MF FT is based on model discrepancies, but multi-index FT may evolve
  // towards BYPASS_SURROGATE as sparse grids in model space will manage QoI
  // differences.  (See also hierarchical multilevel SC.)
  if (multilevDiscrepEmulation == RECURSIVE_EMULATION)
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);
  else
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);//MODEL_DISCREPANCY
  // AGGREGATED_MODELS avoids decimation of data and can simplify algorithms,
  // but requires repurposing origSurrData + modSurrData for high-low QoI pairs
}


void NonDMultilevelFunctionTrain::initialize_u_space_model()
{
  // For greedy ML, activate combined stats now for propagation to Pecos
  // > don't call statistics_type() as ExpansionConfigOptions not initialized
  //if (mlmfAllocControl == GREEDY_REFINEMENT)
  //  statsType = Pecos::COMBINED_EXPANSION_STATS;

  // initializes ExpansionConfigOptions, among other things
  NonDC3FunctionTrain::initialize_u_space_model();

  // Bind more than one SurrogateData instance via DataFitSurrModel ->
  // PecosApproximation
  uSpaceModel.link_multilevel_approximation_data();
}


void NonDMultilevelFunctionTrain::core_run()
{
  initialize_expansion();
  sequenceIndex = 0;

  bool multifid_uq = false;
  switch (methodName) {
  case MULTIFIDELITY_FUNCTION_TRAIN:
    multifid_uq = true;
    // general-purpose algorithms inherited from NonDExpansion:
    switch (mlmfAllocControl) {
    case GREEDY_REFINEMENT:  greedy_multifidelity_expansion();     break;
    default:                 multifidelity_expansion(refineType);  break;
    }
    break;
  case MULTILEVEL_FUNCTION_TRAIN:
    // TO DO: assign a default ML alloc_control = RANK_SAMPLING
    multilevel_regression();
    break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelFunctionTrain::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
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


void NonDMultilevelFunctionTrain::assign_specification_sequence()
{
  // regression
  if (sequenceIndex < collocPtsSeqSpec.size())
    numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];

  update_from_specification();
}


void NonDMultilevelFunctionTrain::increment_specification_sequence()
{
  // regression
  // advance expansionOrder and/or collocationPoints, as admissible
  size_t next_i = sequenceIndex + 1;
  if (next_i < collocPtsSeqSpec.size())
    { numSamplesOnModel = collocPtsSeqSpec[next_i]; ++sequenceIndex; }
  //else leave at previous value

  update_from_specification();
}


void NonDMultilevelFunctionTrain::update_from_specification()
{
  // udpate sampler settings (NonDQuadrature or NonDSampling)
  if (tensorRegression) {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_quad->samples(numSamplesOnModel);
    if (nond_quad->mode() == RANDOM_TENSOR) { // sub-sampling i/o filtering
      SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
	uSpaceModel.shared_approximation().data_rep();
      size_t ft_start_order = shared_data_rep->polynomial_order();
      UShortArray dim_quad_order(numContinuousVars);
      for (size_t i=0; i<numContinuousVars; ++i)
	//dim_quad_order[i] = exp_order[i] + 1;
	dim_quad_order[i] = ft_start_order + 1;// or tensor order user spec ?
      nond_quad->quadrature_order(dim_quad_order);
    }
    nond_quad->update(); // sanity check on sizes, likely a no-op
  }
  else { // enforce increment through sampling_reset()
    // no lower bound on samples in the subiterator
    uSpaceModel.subordinate_iterator().sampling_reference(0);
    DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
    dfs_model->total_points(numSamplesOnModel);
  }
}


void NonDMultilevelFunctionTrain::increment_sample_sequence(size_t new_samp)
{
  numSamplesOnModel = new_samp;

  // update sampler settings (NonDQuadrature or NonDSampling)
  Iterator* sub_iter_rep = uSpaceModel.subordinate_iterator().iterator_rep();
  if (tensorRegression) {
    NonDQuadrature* nond_quad = (NonDQuadrature*)sub_iter_rep;
    nond_quad->samples(numSamplesOnModel);
    if (nond_quad->mode() == RANDOM_TENSOR) { // sub-sampling i/o filtering
      SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
	uSpaceModel.shared_approximation().data_rep();
      size_t ft_start_order = shared_data_rep->polynomial_order();
      UShortArray dim_quad_order(numContinuousVars);
      for (size_t i=0; i<numContinuousVars; ++i)
	//dim_quad_order[i] = exp_order[i] + 1;
	dim_quad_order[i] = ft_start_order + 1;// or tensor order user spec ?
      nond_quad->quadrature_order(dim_quad_order);
    }
    nond_quad->update(); // sanity check on sizes, likely a no-op
  }
  // test for valid sampler for case of build data import (unstructured grid)
  else if (sub_iter_rep != NULL) { // enforce increment using sampling_reset()
    sub_iter_rep->sampling_reference(0);// no lower bnd on samples in sub-iter
    DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
    // total including reuse from DB/file (does not include previous ML iter)
    dfs_model->total_points(numSamplesOnModel);
  }
}


void NonDMultilevelFunctionTrain::multilevel_regression()
{
  // remove default key (empty activeKey) since this interferes with
  // combine_approximation().  Also useful for ML/MF re-entrancy.
  uSpaceModel.clear_model_keys();
  // all stats are level stats
  statistics_type(Pecos::ACTIVE_EXPANSION_STATS);

  // Allow either model forms or discretization levels, but not both
  // (discretization levels take precedence)
  unsigned short lev, form;
  size_t num_lev, iter = 0, max_iter = (maxIterations < 0) ? 25 : maxIterations;
  Real eps_sq_div_2, sum_root_var_cost, estimator_var0 = 0.; 
  RealVector cost;
  bool multilev, optional_cost = (mlmfAllocControl == RANK_SAMPLING),
    recursive = (multilevDiscrepEmulation == RECURSIVE_EMULATION);
  configure_levels(num_lev, form, multilev, false);
  configure_cost(num_lev, multilev, cost);
  RealVector level_metric(num_lev);

  // Multilevel variance aggregation requires independent sample sets
  Iterator* u_sub_iter = uSpaceModel.subordinate_iterator().iterator_rep();
  if (u_sub_iter != NULL)
    ((Analyzer*)u_sub_iter)->vary_pattern(true);

  // Build point import is active only for the pilot sample and we overlay an
  // additional pilot_sample spec, but we do not augment with samples from a
  // collocation pts/ratio enforcement (pts/ratio controls take over on
  // subsequent iterations).
  bool import_pilot = (!importBuildPointsFile.empty());// && recursive);
  if (import_pilot) {
    if (recursive)
      Cout << "\nPilot sample to include imported build points.\n";
    else {
      Cerr << "Error: build data import only supported for recursive emulation "
	   << "in NonDMultilevelFunctionTrain::multilevel_regression()."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  // Load the pilot sample from user specification
  SizetArray delta_N_l(num_lev);
  load_pilot_sample(pilotSamples, delta_N_l);

  // now converge on sample counts per level (NLev)
  NLev.assign(num_lev, 0);
  while ( iter <= max_iter &&
	  ( Pecos::l1_norm(delta_N_l) || (iter == 0 && import_pilot) ) ) {

    sum_root_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      configure_indices(lev, form, multilev);

      if (iter == 0) { // initial expansion build
	// Update solution control variable in uSpaceModel to support
	// DataFitSurrModel::consistent() logic
	if (import_pilot)
	  uSpaceModel.update_from_subordinate_model(); // max depth

	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev]);
	if (lev == 0 || import_pilot)
	  compute_expansion(); // init + import + build; not recursive
	else
	  update_expansion();  // just build; not recursive

	if (import_pilot) { // update counts to include imported data
	  NLev[lev] = delta_N_l[lev]
	    = uSpaceModel.approximation_data(0).points();
	  Cout << "Pilot count including import = " << delta_N_l[lev] << "\n\n";
	  // Trap zero samples as it will cause FPE downstream
	  if (NLev[lev] == 0) { // no pilot spec, no import match
	    Cerr << "Error: insufficient sample recovery for level " << lev
		 << " in multilevel_regression()." << std::endl;
	    abort_handler(METHOD_ERROR);
	  }
	}
      }
      else if (delta_N_l[lev]) {
	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev]);
	// Note: import build data is not re-processed by append_approximation()
	append_approximation();
      }

      switch (mlmfAllocControl) {
      case RANK_SAMPLING: // use RMS of rank across QoI
	if (delta_N_l[lev] > 0)
	  rank_metrics(level_metric[lev], 2.);
	break;
      default: { //case ESTIMATOR_VARIANCE:
	Real& agg_var_l = level_metric[lev];
	if (delta_N_l[lev] > 0) aggregate_variance(agg_var_l);
	sum_root_var_cost += std::pow(agg_var_l *
	  std::pow(level_cost(lev, cost), kappaEstimatorRate),
	  1./(kappaEstimatorRate+1.));
        // MSE reference is ML MC aggregation for pilot(+import) sample:
	if (iter == 0) estimator_var0 += agg_var_l / NLev[lev];
	break;
      }
      }
    }

    switch (mlmfAllocControl) {
    case RANK_SAMPLING:
      compute_sample_increment(2., level_metric, NLev, delta_N_l);
      break;
    default: //case ESTIMATOR_VARIANCE:
      if (iter == 0) { // eps^2 / 2 = var * relative factor
	eps_sq_div_2 = estimator_var0 * convergenceTol;
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Epsilon squared target = " << eps_sq_div_2 << '\n';
      }
      compute_sample_increment(level_metric, cost, sum_root_var_cost,
			       eps_sq_div_2, NLev, delta_N_l);
      break;
    }
    ++iter;
    Cout << "\nML FT iteration " << iter << " sample increments:\n"
	 << delta_N_l << std::endl;
  }
  compute_equivalent_cost(NLev, cost); // compute equivalent # of HF evals

  combined_to_active(); // combine FT terms and promote to active expansion
  // Final annotated results are computed / printed in core_run()
}


void NonDMultilevelFunctionTrain::aggregate_variance(Real& agg_var_l)
{
  // case ESTIMATOR_VARIANCE:
  // statsType remains as Pecos::ACTIVE_EXPANSION_STATS

  // control ML using aggregated variance across the vector of QoI
  // (alternate approach: target QoI with largest variance)
  agg_var_l = 0.;
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    C3Approximation* poly_approx_q
      = (C3Approximation*)poly_approxs[qoi].approx_rep();
    Real var_l = poly_approx_q->variance(); // for active level
    agg_var_l += var_l;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Variance(" /*"lev " << lev << ", "*/ << "qoi " << qoi
	/* << ", iter " << iter */ << ") = " << var_l << '\n';
  }
}


/* Compute power mean of rank (common power values: 1 = average, 2 = RMS). */
void NonDMultilevelFunctionTrain::rank_metrics(Real& rank_metric_l, Real power)
{
  // case RANK_SAMPLING:

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  Real sum = 0.; bool pow1 = (power == 1.); // simple average
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    C3Approximation* poly_approx_q
      = (C3Approximation*)poly_approxs[qoi].approx_rep();
    Real rank_l = poly_approx_q->average_rank(); // power 1 over dims (average)
    sum += (pow1) ? rank_l : std::pow(rank_l, power);
    //if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Rank(" /*lev " << lev << ", "*/ << "qoi " << qoi
	/* << ", iter " << iter */ << ") = " << rank_l << '\n';
  }
  sum /= numFunctions;
  rank_metric_l = (pow1) ? sum : std::pow(sum, 1. / power);
}


void NonDMultilevelFunctionTrain::
compute_sample_increment(const RealVector& agg_var, const RealVector& cost,
			 Real sum_root_var_cost, Real eps_sq_div_2,
			 const SizetArray& N_l, SizetArray& delta_N_l)
{
  // case ESTIMATOR_VARIANCE:

  // eps^2 / 2 target computed based on relative tolerance: total MSE = eps^2
  // which is equally apportioned (eps^2 / 2) among discretization MSE and
  // estimator variance (\Sum var_Y_l / NLev).  Since we do not know the
  // discretization error, we compute an initial estimator variance and then
  // seek to reduce it by a relative_factor <= 1.

  // We assume a functional dependence of estimator variance on NLev
  // for minimizing aggregate cost subject to an MSE error balance:
  //   Var(Q-hat) = sigma_Q^2 / (gamma NLev^kappa)
  // where Monte Carlo has gamma = kappa = 1.  To fit these parameters,
  // one approach is to numerically estimate the variance in the mean
  // estimator (alpha_0) from two sources:
  // > from variation across k folds for the selected CV settings
  // > from var decrease as NLev increases across iters

  // compute and accumulate variance of mean estimator from the set of
  // k-fold results within the selected settings from cross-validation:
  //Real cv_var_i = poly_approx_rep->
  //  cross_validation_solver().cv_metrics(MEAN_ESTIMATOR_VARIANCE);
  //  (need to make MultipleSolutionLinearModelCrossValidationIterator
  //   cv_iterator class scope)
  // To validate this approach, the actual estimator variance can be
  // computed and compared with the CV variance approximation (as for
  // traditional CV error plots, but predicting estimator variance
  // instead of L2 fit error).

  // update targets based on variance estimates
  Real new_N_l; size_t lev, num_lev = N_l.size();
  Real fact = std::pow(sum_root_var_cost / eps_sq_div_2 / gammaEstimatorScale,
		       1. / kappaEstimatorRate);
  for (lev=0; lev<num_lev; ++lev) {
    new_N_l = std::pow(agg_var[lev] / level_cost(lev, cost),
		       1. / (kappaEstimatorRate+1.)) * fact;
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l);
  }
}


void NonDMultilevelFunctionTrain::
compute_sample_increment(Real factor, const RealVector& rank,
			 const SizetArray& N_l, SizetArray& delta_N_l)
{
  // case RANK_SAMPLING:
  // > sample requirements scale as O(r^2 d), which shapes the profile
  // > *** TO DO: adapt profile factor in coordination with adapt_rank ?
  // > there is a dependence on the polynomial order, but this should not
  //   grow very high, could just fix this factor at ~10.
  // > The function function_train_get_nparams(const struct FunctionTrain *);
  //   is more direct in returning the number of unknowns in the regression
  //   (per QoI, per level) which is directly (p r^2 d) without over-sampling
  //   excluding the need to average r over d.  I then need to add any factor
  //   on top of this, maybe 2 instead of 10.
  // > TO DO: repurpose collocation_ratio spec to allow user tuning.
  
  // update targets based on rank estimates
  size_t lev, num_lev = N_l.size();
  RealVector new_N_l(num_lev, false);
  Real r, fact_var = factor * numContinuousVars;
  for (lev=0; lev<num_lev; ++lev)
    { r = rank[lev];  new_N_l[lev] = fact_var * r * r; }

  // Retain the shape of the profile but enforce an upper bound
  //scale_profile(..., new_N_l);

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
      s << "<<<<< Equivalent number of high fidelity evaluations: "
	<< equivHFEvals << std::endl;
    }
    break;
  }

  // skip over NonDC3FunctionTrain::print_results()
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
