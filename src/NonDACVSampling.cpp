/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Implementation code for NonDACVSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#elif HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDMLMFSampling(problem_db, model),
  pilotSamples(problem_db.get_sza("method.nond.pilot_samples")),
  randomSeedSeqSpec(problem_db.get_sza("method.random_seed_sequence")),
  mlmfIter(0),
  allocationTarget(problem_db.get_short("method.nond.allocation_target")),
  qoiAggregation(problem_db.get_short("method.nond.qoi_aggregation")),
  useTargetVarianceOptimizationFlag(
    problem_db.get_bool("method.nond.allocation_target.variance.optimization")),
  finalCVRefinement(true),
  exportSampleSets(problem_db.get_bool("method.nond.export_sample_sequence")),
  exportSamplesFormat(
    problem_db.get_ushort("method.nond.export_samples_format"))
{
  // initialize scalars from sequence
  seedSpec = randomSeed = random_seed(0);

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

  // check iteratedModel for model form hierarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel.surrogate_type() == "non_hierarchical")
    aggregated_models_mode();
  else {
    Cerr << "Error: Approximate Control Variate sampling requires a non-"
         << "hierarchical surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  size_t i, j, num_mf = model_ensemble.size(), num_lev,
    prev_lev = std::numeric_limits<size_t>::max(),
    pilot_size = pilotSamples.size();
  ModelLRevIter ml_it; bool err_flag = false;
  NLev.resize(num_mf);
  for (i=0, ml_it=model_ensemble.begin(); ml_it!=model_ensemble.end();
       ++i, ++ml_it) {
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_rit->solution_levels(); // lower bound is 1 soln level

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    if (num_lev > ml_rit->solution_levels(false)) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for multilevel sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_rit->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer to pre_run()

    prev_lev = num_lev;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  switch (pilot_size) {
  case 0: maxEvalConcurrency *= 100;             break;
  case 1: maxEvalConcurrency *= pilotSamples[0]; break;
  default: {
    size_t max_ps = 0;
    for (i=0; i<pilot_size; ++i)
      if (pilotSamples[i] > max_ps)
	max_ps = pilotSamples[i];
    if (max_ps)
      maxEvalConcurrency *= max_ps;
    break;
  }
  }

  // For testing multilevel_mc_Qsum():
  //subIteratorFlag = true;
}


NonDACVSampling::~NonDACVSampling()
{ }


bool NonDACVSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDACVSampling::pre_run()
{
  NonDSampling::pre_run();

  // reset sample counters to 0
  size_t i, j, num_mf = NLev.size(), num_lev;
  for (i=0; i<num_mf; ++i) {
    Sizet2DArray& Nl_i = NLev[i];
    num_lev = Nl_i.size();
    for (j=0; j<num_lev; ++j)
      Nl_i[j].assign(numFunctions, 0);
  }
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDACVSampling::core_run()
{
  size_t num_mf = NLev.size();
  switch (acvAlgorithmOption) {
  case ACV_MF:
  case ACV_IS:
  case ACV_KL:
  }
}


void NonDACVSampling::
configure_indices(size_t group, size_t form, size_t lev, short seq_type)
{
  // Notes:
  // > could consolidate with NonDExpansion::configure_indices() with a passed
  //   model and virtual *_mode() assignments.  Leaving separate for now...
  // > group index is assigned based on step in model form/resolution sequence
  // > CVMC does not use this helper; it requires uncorrected_surrogate_mode()

  // preserve special values across type conversions
  size_t SZ_MAX = std::numeric_limits<size_t>::max();
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form  == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  Pecos::ActiveKey hf_key;  hf_key.form_key(grp, frm, lev);

  if ( (seq_type == Pecos::MODEL_FORM_SEQUENCE       && form == 0) ||
       (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE && lev  == 0)) {
    // step 0 in the sequence
    bypass_surrogate_mode();
    iteratedModel.active_model_key(hf_key);      // one active fidelity
  }
  else {
    aggregated_models_mode();

    /* TO DO
    Pecos::ActiveKey lf_key(hf_key.copy()), agg_key;
    lf_key.decrement_key(seq_type); // seq_index defaults to 0
    // For MLMC/MFMC/MLMFMC, we aggregate levels but don't reduce them
    agg_key.aggregate_keys(hf_key, lf_key, Pecos::RAW_DATA);
    iteratedModel.active_model_key(agg_key); // two active fidelities
    */
  }
}


void NonDACVSampling::assign_specification_sequence(size_t index)
{
  // Note: seedSpec/randomSeed initialized from randomSeedSeqSpec in ctor

  // advance any sequence specifications, as admissible
  // Note: no colloc pts sequence as load_pilot_sample() handles this separately
  int seed_i = random_seed(index);
  if (seed_i) randomSeed = seed_i;// propagate to NonDSampling::initialize_lhs()
  // else previous value will allow existing RNG to continue for varyPattern
}


void NonDACVSampling::
export_all_samples(String root_prepend, const Model& model, size_t iter,
		   size_t lev)
{
  String tabular_filename(root_prepend);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  if (iface_id.empty()) tabular_filename += "NO_ID_i";
  else                  tabular_filename += iface_id + "_i";
  tabular_filename += std::to_string(iter)     +  "_l"
                   +  std::to_string(lev)      +  '_'
                   +  std::to_string(num_samp) + ".dat";
  Variables vars(model.current_variables().copy());

  String context_message("NonDACVSampling::export_all_samples");
  StringArray no_resp_labels;
  String cntr_label("sample_id"), interf_label("interface");

  // Rather than hard override, rely on output_precision user spec
  //int save_wp = write_precision;
  //write_precision = 16; // override
  std::ofstream tabular_stream;
  TabularIO::open_file(tabular_stream, tabular_filename, context_message);
  TabularIO::write_header_tabular(tabular_stream, vars, no_resp_labels,
				  cntr_label, interf_label,exportSamplesFormat);
  for (i=0; i<num_samp; ++i) {
    sample_to_variables(allSamples[i], vars); // NonDSampling version
    TabularIO::write_data_tabular(tabular_stream, vars, iface_id, i+1,
				  exportSamplesFormat);
  }

  TabularIO::close_file(tabular_stream, tabular_filename, context_message);
  //write_precision = save_wp; // restore
}


void NonDACVSampling::post_run(std::ostream& s)
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


void NonDACVSampling::print_results(std::ostream& s, short results_state)
{
  if (statsFlag) {
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
    archive_moments();
    archive_equiv_hf_evals(equivHFEvals); 
  }
}

} // namespace Dakota
