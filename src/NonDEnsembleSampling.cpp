/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDEnsembleSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"
#include "NormalRandomVariable.hpp"

static const char rcsId[]="@(#) $Id: NonDEnsembleSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDEnsembleSampling::
NonDEnsembleSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDSampling(problem_db, model),
  //pilotSamples(problem_db.get_sza("method.nond.pilot_samples")),
  pilotMgmtMode(
    problem_db.get_short("method.nond.ensemble_pilot_solution_mode")),
  optSubProblemForm(NO_OPTIMAL_ALLOCATION), optSubProblemSolver(SUBMETHOD_NONE),
  randomSeedSeqSpec(problem_db.get_sza("method.random_seed_sequence")),
  backfillFailures(false), // inactive option for now
  mlmfIter(0), equivHFEvals(0.), // also reset in pre_run()
  //allocationTarget(problem_db.get_short("method.nond.allocation_target")),
  //qoiAggregation(problem_db.get_short("method.nond.qoi_aggregation")),
  estVarMetricType(
    problem_db.get_short("method.nond.estimator_variance_metric")),
  estVarMetricNormOrder(
    problem_db.get_real("method.nond.estimator_variance_metric_norm_order")),
  finalStatsType(problem_db.get_short("method.nond.final_statistics")),
  exportSampleSets(problem_db.get_bool("method.nond.export_sample_sequence")),
  exportSamplesFormat(
    problem_db.get_ushort("method.nond.export_samples_format")),
  relaxFactor(1.), relaxIndex(0),
  relaxFactorSequence(
    problem_db.get_rv("method.nond.relaxation.factor_sequence")),
  relaxRecursiveFactor(
    problem_db.get_real("method.nond.relaxation.recursive_factor")),
  seedIndex(SZ_MAX)
{
  // check iteratedModel for model form hierarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel->surrogate_type() == "ensemble")
    iteratedModel->surrogate_response_mode(AGGREGATED_MODELS);
  else {
    Cerr << "Error: ensemble sampling for multifidelity analysis requires an "
	 << "ensemble surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& model_ensemble = iteratedModel->subordinate_models(false);
  size_t num_mf = model_ensemble.size(), num_lev, prev_lev = SZ_MAX,
    md_index, num_md;
  int m;  ModelLRevIter ml_rit; // reverse iteration for prev_lev tracking
  NLevActual.resize(num_mf);  NLevAlloc.resize(num_mf);
  costMetadataIndices.resize(num_mf);
  bool mlmf = (methodName == MULTILEVEL_MULTIFIDELITY_SAMPLING);
  for (m=num_mf-1, ml_rit=model_ensemble.rbegin();
       ml_rit!=model_ensemble.rend(); --m, ++ml_rit) { // high fid to low fid
    // Only SimulationModel supports solution_{levels,costs} and cost metadata,
    // and metadata indices only vary per response specification (model form).
    // Note: definition of the number of solution levels only requires user
    // specification of the solution level control string, which binds with the
    // number of admissible values for that identified variable.  In the event
    // that solution costs are not specified (and have to be recovered from
    // response metadata), SimulationModel::initialize_solution_control() still
    // sizes solnCntlCostMap such that the correct number of levels is returned.
    num_lev  = (*ml_rit)->solution_levels(); // lower bound is 1 soln level
    md_index = (*ml_rit)->cost_metadata_index();
    num_md   = (*ml_rit)->current_response().metadata().size();

    if (mlmf && num_lev > prev_lev) {
      Cerr << "\nWarning: unused solution levels in multilevel-multifidelity "
	   << "sampling for model " << (*ml_rit)->model_id() << ".\n         "
	   << "Ignoring " << num_lev - prev_lev << " of " << num_lev
	   << " levels." << std::endl;
      num_lev = prev_lev;
    }

    // Must manage N_actual vs. N_actual_proj in final roll ups:
    // > "Actual" means succeeded --> no projection-based updates to actual
    //    --> retains strict linkage with accumulated sums/stats.
    // > Projections are allocations --> include in NLevAlloc
    // > BUT, projected variance reduction calcs reuse best available varH in
    //   combination with projected NLevActual, so:
    //   >> Use NLevActual in multilevel_eval_summary();
    //      use NLevActual + deltaNLevActual in print_variance_reduction(),
    //      (for stats like varH, actual + proj is preferred to projected alloc)
    //   >> use similar approach with equivHFEvals (tracks actual) + delta
    //      (separated projection)
    //Sizet2DArray& Nl_m = NLevActual[m];
    NLevActual[m].resize(num_lev); //Nl_m.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_m[j].resize(numFunctions); // defer to pre_run()
    NLevAlloc[m].resize(num_lev);

    // Note: md_index is subject to updates downstream (precedence of user spec)
    costMetadataIndices[m] = SizetSizetPair(md_index, num_md);
    prev_lev = num_lev;
  }

  // Support multilevel LHS as a specification override.  The estimator variance
  // is known/correct for MC and an assumption/approximation for LHS.  To get an
  // accurate LHS estimator variance, one would need:
  // (a) assumptions about separability -> analytic variance reduction by a
  //     constant factor
  // (b) similarly, assumptions about the form relative to MC (e.g., a constant
  //     factor largely cancels out within the relative sample allocation.)
  // (c) numerically-generated estimator variance (from, e.g., replicated LHS)
  if (!sampleType) // SUBMETHOD_DEFAULT
    sampleType = SUBMETHOD_RANDOM;

  Real relax_fixed = problem_db.get_real("method.nond.relaxation.fixed_factor");
  if (relax_fixed > 0.) relaxFactor = relax_fixed; // else initialized to 1.

  switch (pilotMgmtMode) {
  case ONLINE_PILOT_PROJECTION:
    maxIterations = 0; // no iteration
    finalStatsType = ESTIMATOR_PERFORMANCE; // no mlmf_final_stats in spec
    break;
  case OFFLINE_PILOT_PROJECTION:
    maxIterations = 0; // no iteration
    finalStatsType = ESTIMATOR_PERFORMANCE; // no mlmf_final_stats in spec
    // relative accuracy control with convergenceTol is problematic since
    // the reference EstVar comes from offline eval with Oracle/overkill N.
    // Could support an absolute tolerance, but error for now.
    if (maxFunctionEvals == SZ_MAX) {
      Cerr << "Error: evaluation budget required for offline projection mode."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
    break;
  case ONLINE_PILOT:
    // MLMF-specific default: don't let allocator get stuck in fine-tuning
    if (maxIterations == SZ_MAX) maxIterations = 25;
    if (!finalStatsType) finalStatsType = QOI_STATISTICS; // mode default
    break;
  case OFFLINE_PILOT:
    maxIterations = 1;
    // convergenceTol option is problematic since the reference EstVar
    // comes from offline eval with Oracle/overkill N.  Could support an
    // absolute rather than relative tolerance, but error for now.
    if (maxFunctionEvals == SZ_MAX) {
      Cerr << "Error: evaluation budget required for offline pilot mode."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (!finalStatsType) finalStatsType = QOI_STATISTICS; // mode default
    break;
  default:
    Cerr << "Error: unrecognized pilot solution mode in ensemble sampling."
	 << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }

  initialize_final_statistics();
}


NonDEnsembleSampling::~NonDEnsembleSampling()
{ }


bool NonDEnsembleSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDEnsembleSampling::assign_specification_sequence(size_t index)
{
  // Note: seedSpec/randomSeed initialized from randomSeedSeqSpec in ctor

  // Advance any sequence specifications, as admissible.  Refer to
  // NonDSampling::initialize_sample_driver() for logic.
  // Note: no colloc pts sequence as load_pilot_sample() handles this separately
  int seed_i = seed_sequence(index);
  if (seed_i) randomSeed = seed_i;
  // else previous value will allow existing RNG to continue for varyPattern
}


void NonDEnsembleSampling::pre_run()
{
  NonDSampling::pre_run();

  // remove default key (empty activeKey) since this interferes with approx
  // combination in MF surrogates.  Also useful for ML/MF re-entrancy.
  iteratedModel->clear_model_keys();

  // reset shared accumulators
  // Note: numLHSRuns is interpreted differently here (accumulation of LHS runs
  //       for each execution of ensemble sampler) than for base NonDSampling
  //       (total accumulation of LHS runs)
  mlmfIter = numLHSRuns = 0;
  equivHFEvals = deltaEquivHF = 0.;
  seedSpec = randomSeed = seed_sequence(0); // (re)set seeds to sequence

  reset_relaxation();
}


void NonDEnsembleSampling::post_run(std::ostream& s)
{
  // Final moments are generated within core_run() by convert_moments().
  // No addtional stats are currently supported.
  //if (statsFlag) // calculate statistics on allResponses
  //  compute_statistics(allSamples, allResponses);

  // NonD::update_aleatory_final_statistics() pushes momentStats into
  // finalStatistics
  update_final_statistics();

  Analyzer::post_run(s);
}


void NonDEnsembleSampling::
accumulate_online_cost(const IntResponseMap& resp_map, RealVector& accum_cost,
		       SizetArray& num_cost)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  size_t m, cntr, start, end, md_index, md_index_m;
  unsigned short mf;  Real cost;  IntRespMCIter r_it;
  const Pecos::ActiveKey& active_key = iteratedModel->active_model_key();

  for (m=0, cntr=0, start=0; m<=numApprox; ++m) {
    end = start + numFunctions;

    // Locate cost meta-data for ensemble member m through its model form
    // > see SurrogateModel::insert_metadata(): aggregated metadata is padded
    //   for any inactive models within the AGGREGATED_MODELS set
    mf = active_key.retrieve_model_form(m);
    const SizetSizetPair& cost_mdi = costMetadataIndices[mf];
    md_index_m = cost_mdi.first;
    if (md_index_m != SZ_MAX) { // alternatively, if solnCntlCostMap key is 0.
      md_index = cntr + md_index_m; // index into aggregated metadata
      for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
	const Response& resp = r_it->second;
	if (non_zero(resp.active_set_request_vector(), start, end)) {
	  cost = resp.metadata(md_index);
	  if (isfinite(cost))
	    { accum_cost[m] += cost; ++num_cost[m]; }
	}
      }
    }

    start = end;
    cntr += cost_mdi.second; // offset by size of metadata for model
  } 
}


void NonDEnsembleSampling::initialize_final_statistics()
{
  switch (finalStatsType) {
  case ESTIMATOR_PERFORMANCE: { // MSE in stat goal(s) used for method selection
    size_t num_final = 2;
    ActiveSet set(num_final);//, num_active_vars); // default RV = 1
    set.derivative_vector(ModelUtils::inactive_continuous_variable_ids(*iteratedModel));
    finalStatistics = Response(SIMULATION_RESPONSE, set);

    StringArray stats_labels(num_final);
    if (maxFunctionEvals == SZ_MAX) // accuracy spec: equiv cost is objective
      { stats_labels[0] = "equiv_HF_cost"; stats_labels[1] = "est_var_metric"; }
    else                            // budget spec: equiv cost is constraint
      { stats_labels[0] = "est_var_metric"; stats_labels[1] = "equiv_HF_cost"; }

    finalStatistics.function_labels(stats_labels);
    break;
  }
  case QOI_STATISTICS: // final stats: moments + level mappings
    NonD::initialize_final_statistics();  break;
  }
}


void NonDEnsembleSampling::update_final_statistics()
{
  if (finalStatistics.is_null()) // some ctor chains do not track final stats
    return;

  /*
  if (epistemicStats) {
    size_t i, cntr = 0;
    for (i=0; i<numFunctions; ++i) {
      finalStatistics.function_value(extremeValues[i].first,  cntr++);
      finalStatistics.function_value(extremeValues[i].second, cntr++);
    }
  }
  */
  switch (finalStatsType) {
  case ESTIMATOR_PERFORMANCE:
    if (maxFunctionEvals == SZ_MAX) { // accuracy spec: equiv cost is objective
      finalStatistics.function_value(estimator_cost_metric(),     0);
      finalStatistics.function_value(estimator_accuracy_metric(), 1);
    }
    else { // budget spec: equiv cost returned as constraint
      finalStatistics.function_value(estimator_accuracy_metric(), 0);
      finalStatistics.function_value(estimator_cost_metric(),     1);
    }
    break;
  case QOI_STATISTICS: // final stats: moments + level mappings
    NonD::update_final_statistics(); break;
  }
}


void NonDEnsembleSampling::active_set_mapping()
{
  switch (finalStatsType) {
  case ESTIMATOR_PERFORMANCE:
    // transfer ASV requests from finalStatistics to activeSet
    // (we won't support derivatives of estimator performance metrics,
    // see also initialize_final_statistics() above)
    activeSet.request_values(1);        break;
  case QOI_STATISTICS: // final stats: moments + level mappings
    NonDSampling::active_set_mapping(); break;
  }
}


void NonDEnsembleSampling::
compute_mean_confidence_intervals(const RealMatrix& moment_stats,
				  const RealVector& mean_estvar,
				  RealMatrix& mean_conf_ints)
{
  size_t q, num_qoi = moment_stats.numCols();
  if (mean_conf_ints.empty())
    mean_conf_ints.shapeUninitialized(2, num_qoi);

  // 95% confidence interval for mean estimator given estimator variance
  // Normal: p(mu - 2sigma <= X <= mu + 2sigma) ~ .9545
  // Std normal: Phi(2) ~ .9772, Phi(z) = .975
  Real mean, sqrt_estvar, z
    = Pecos::NormalRandomVariable::inverse_std_cdf(.975);// ~ 2 stdev on 1 side
  for (q=0; q<num_qoi; ++q) {
    mean = moment_stats(0,q);
    sqrt_estvar = std::sqrt(mean_estvar[q]);
    Real* mean_ci_q = mean_conf_ints[q];
    mean_ci_q[0] = mean - z * sqrt_estvar; // interval lower bound
    mean_ci_q[1] = mean + z * sqrt_estvar; // interval upper bound
  }
}


void NonDEnsembleSampling::print_results(std::ostream& s, short results_state)
{
  if (!statsFlag)
    return;

  bool pilot_projection = (pilotMgmtMode ==  ONLINE_PILOT_PROJECTION ||
			   pilotMgmtMode == OFFLINE_PILOT_PROJECTION),
       cv_projection    = (finalStatsType == ESTIMATOR_PERFORMANCE),
       projections      = (pilot_projection || cv_projection);
  String summary_type   = (pilot_projection) ? "Projected " : "Online ";

  // model-based allocation methods, e.g. ML, MF, MLMF, ACV, GenACV
  print_multimodel_summary(s, summary_type, projections);
  // group-based allocation methods, e.g. ML BLUE
  print_multigroup_summary(s, summary_type, projections);
  // output performance of ensemble estimator
  print_variance_reduction(s);

  if (!projections) {
    s << "\nStatistics based on multilevel sample set:\n";
    //print_statistics(s);
    print_moments(s, momentStats, meanCIs, "response function",finalMomentsType,
		  ModelUtils::response_labels(*iteratedModel->truth_model()), true);
    archive_moments();
  }
}


void NonDEnsembleSampling::
print_multimodel_summary(std::ostream& s, const String& summary_type,
			 bool projections)
{
  bool discrep_flag = discrepancy_sample_counts();

  // Always report allocated, then optionally report actual.
  // Any offline pilot samples (N_pilot in *_offline()) are excluded.
  print_multilevel_model_summary(s, NLevAlloc, summary_type + "allocation of",
				 sequenceType, discrep_flag);
  Real proj_equiv_hf = equivHFEvals + deltaEquivHF;
  s << "<<<<< " << summary_type
    << "number of equivalent high fidelity evaluations: " << std::scientific
    << std::setprecision(write_precision) << proj_equiv_hf <<'\n';
  archive_equiv_hf_evals(proj_equiv_hf);

  if (projections || differ(NLevAlloc, NLevActual)) {
    // NLevActual includes successful sample accumulations used for stats
    // equivHFEvals includes incurred cost for evaluations, successful or not
    print_multilevel_model_summary(s, NLevActual, "Online accumulated",
                                   sequenceType, discrep_flag);
    s << "<<<<< Incurred cost in equivalent high fidelity evaluations: "
      << std::scientific << std::setprecision(write_precision) << equivHFEvals
      << '\n';
    //archive_incurred_equiv_hf_evals(equivHFEvals);
  }
}


void NonDEnsembleSampling::
export_all_samples(const Model& model, const String& tabular_filename)
{
  Variables vars(model.current_variables().copy());

  String context_message("NonDEnsembleSampling::export_all_samples");
  StringArray no_resp_labels;
  String cntr_label("sample_id"), interf_label("interface");

  // Rather than hard override, rely on output_precision user spec
  //int save_wp = write_precision;
  //write_precision = 16; // override
  std::ofstream tabular_stream;
  TabularIO::open_file(tabular_stream, tabular_filename, context_message);
  TabularIO::write_header_tabular(tabular_stream, vars, no_resp_labels,
				  cntr_label, interf_label,exportSamplesFormat);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  for (i=0; i<num_samp; ++i) {
    sample_to_variables(allSamples[i], vars); // NonDSampling version
    TabularIO::write_data_tabular(tabular_stream, vars, iface_id, i+1,
				  exportSamplesFormat);
  }

  TabularIO::close_file(tabular_stream, tabular_filename, context_message);
  //write_precision = save_wp; // restore
}


void NonDEnsembleSampling::
convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom)
{
  // Convert uncentered raw moment estimates to central moments
  if (finalMomentsType == Pecos::CENTRAL_MOMENTS)
    uncentered_to_centered(raw_mom, final_mom);
  // Convert uncentered raw moment estimates to standardized moments
  else { //if (finalMomentsType == Pecos::STANDARD_MOMENTS) {
    RealMatrix cent_mom;
    uncentered_to_centered(raw_mom, cent_mom);
    centered_to_standard(cent_mom, final_mom);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "raw moments   =\n" << raw_mom
	 << "final moments =\n" << final_mom << "\n\n";
}

} // namespace Dakota
