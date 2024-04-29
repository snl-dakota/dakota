/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaResponse.hpp"
#include "DakotaNonD.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_tabular_io.hpp"
#include "NormalRandomVariable.hpp"
#include "ParallelLibrary.hpp"
#include "dakota_stat_util.hpp"

//#define DEBUG

static const char rcsId[]="@(#) $Id: DakotaNonD.cpp 7024 2010-10-16 01:24:42Z mseldre $";

namespace Dakota {

// initialization of statics
NonD* NonD::nondInstance(NULL);


NonD::NonD(ProblemDescDB& problem_db, Model& model):
  Analyzer(problem_db, model),
  respLevelTarget(problem_db.get_short("method.nond.response_level_target")),
  respLevelTargetReduce(
    problem_db.get_short("method.nond.response_level_target_reduce")),
  requestedRespLevels(problem_db.get_rva("method.nond.response_levels")),
  requestedProbLevels(problem_db.get_rva("method.nond.probability_levels")),
  requestedRelLevels(problem_db.get_rva("method.nond.reliability_levels")),
  requestedGenRelLevels(
    problem_db.get_rva("method.nond.gen_reliability_levels")),
  totalLevelRequests(0),
  cdfFlag(problem_db.get_short("method.nond.distribution") != COMPLEMENTARY),
  pdfOutput(false),
  finalMomentsType(problem_db.get_short("method.nond.final_moments"))
{
  initialize_counts();

  // When specifying z/p/beta/beta* levels, a spec with an index key (number of
  // levels = list of ints) will result in multiple vectors of levels, one for
  // each response fn.  A spec without an index key will result in a single
  // vector of levels.  This is as much logic as the parser can support since it
  // cannot access the number of response fns.  Here, support the shorthand spec
  // where there are multiple response fns but only one vector of levels (no
  // index key provided), which are to be evenly distributed among the response
  // fns.  This provides some measure of backwards compatibility.
  distribute_levels(requestedRespLevels);             // always ascending
  distribute_levels(requestedProbLevels,    cdfFlag); // ascending if cumulative
  distribute_levels(requestedRelLevels,    !cdfFlag);// descending if cumulative
  distribute_levels(requestedGenRelLevels, !cdfFlag);// descending if cumulative

  for (size_t i=0; i<numFunctions; i++)
    totalLevelRequests += requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();

  // simple PDF output control for now (suppressed if quiet/silent output level)
  if (totalLevelRequests && outputLevel >= NORMAL_OUTPUT)
    pdfOutput = true;
}


NonD::NonD(unsigned short method_name, Model& model):
  Analyzer(method_name, model), totalLevelRequests(0),
  cdfFlag(true), pdfOutput(false), finalMomentsType(Pecos::STANDARD_MOMENTS)
{
  // NonDEvidence and NonDAdaptImpSampling use this ctor

  initialize_counts();

  // current set of statistics is mean, standard deviation, and 
  // probability of failure for each response function 
  //ShortArray asv(3*numFunctions, 1); 
  //finalStatistics = Response(numUncertainVars, asv);
}


NonD::NonD(unsigned short method_name, Model& model,
	   const ShortShortPair& approx_view):
  Analyzer(method_name, model, approx_view), totalLevelRequests(0),
  cdfFlag(true), pdfOutput(false), finalMomentsType(Pecos::STANDARD_MOMENTS)
{
  initialize_counts();
}


NonD::NonD(unsigned short method_name, const RealVector& lower_bnds,
	   const RealVector& upper_bnds):
  Analyzer(method_name), epistemicStats(false), totalLevelRequests(0),
  cdfFlag(true), pdfOutput(false), finalMomentsType(Pecos::STANDARD_MOMENTS)
{
  // ConcurrentStrategy uses this ctor for design opt, either for multi-start
  // initial points or multibjective weight sets.

  startCAUV = 0;
  numCAUV = numContinuousVars
    = std::min(lower_bnds.length(), upper_bnds.length());
  numDiscreteIntVars = numDiscreteStringVars = numDiscreteRealVars = 0;
}


void NonD::initialize_counts()
{
  const Variables& vars = iteratedModel.current_variables();
  //short active_view = vars.view().first;
  const SizetArray& ac_totals = vars.shared_data().active_components_totals();
  // convenience looping bounds
  startCAUV = ac_totals[TOTAL_CDV];
  numCAUV   = ac_totals[TOTAL_CAUV];
  // stats type
  epistemicStats = (ac_totals[TOTAL_CEUV]  || ac_totals[TOTAL_DEUIV] ||
		    ac_totals[TOTAL_DEUSV] || ac_totals[TOTAL_DEURV]);
}


bool NonD::resize()
{
  bool parent_reinit_comms = Analyzer::resize();

  initialize_counts(); // sufficient at this time

  return parent_reinit_comms;
}


void NonD::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);
}


void NonD::
requested_levels(const RealVectorArray& req_resp_levels,
		 const RealVectorArray& req_prob_levels,
		 const RealVectorArray& req_rel_levels,
		 const RealVectorArray& req_gen_rel_levels,
		 short resp_lev_tgt, short resp_lev_tgt_reduce,
		 bool cdf_flag, bool pdf_output)
{
  respLevelTarget = resp_lev_tgt;
  respLevelTargetReduce = resp_lev_tgt_reduce;
  cdfFlag = cdf_flag;

  size_t i;
  totalLevelRequests = 0;
  if (req_resp_levels.empty())
    requestedRespLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedRespLevels = req_resp_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedRespLevels);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedRespLevels[i].length();
  }
  if (req_prob_levels.empty())
    requestedProbLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedProbLevels = req_prob_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedProbLevels, cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedProbLevels[i].length();
  }
  if (req_rel_levels.empty())
    requestedRelLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedRelLevels = req_rel_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedRelLevels, !cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedRelLevels[i].length();
  }
  if (req_gen_rel_levels.empty())
    requestedGenRelLevels.resize(numFunctions); // array of empty vectors
  else {
    requestedGenRelLevels = req_gen_rel_levels;
    // In current usage, incoming levels are already distributed.
    //distribute_levels(requestedGenRelLevels, !cdfFlag);
    for (i=0; i<numFunctions; ++i)
      totalLevelRequests += requestedGenRelLevels[i].length();
  }

  if (totalLevelRequests && pdf_output)
    pdfOutput = true;

  initialize_final_statistics();
  initialize_response_covariance();
}


void NonD::distribute_levels(RealVectorArray& levels, bool ascending)
{
  size_t i, j, num_level_arrays = levels.size();
  // when not already performed by the parser (i.e., a num_levels key
  // was not provided), distribute levels among an array of vectors
  if (num_level_arrays != numFunctions) {
    if (num_level_arrays == 0) // create array of empty vectors
      levels.resize(numFunctions);
      // NOTE: old response_thresholds default was a single 0 per resp fn.  New
      // default is no output level calculations if no requested input levels.
    else if (num_level_arrays == 1) { // evenly distribute among all resp fns
      RealVector level_array0 = levels[0];
      size_t total_len = level_array0.length();
      // check for divisibility
      if (total_len%numFunctions) {
        Cerr << "\nError: number of levels not evenly divisible by the number "
             << "of response functions." << std::endl;
        abort_handler(-1);
      }
      size_t new_len = total_len/numFunctions;
      levels.resize(numFunctions);
      for (i=0; i<numFunctions; ++i) {
	RealVector& vec_i = levels[i];
	vec_i.sizeUninitialized(new_len);
        for (j=0; j<new_len; ++j)
          vec_i[j] = level_array0[i*new_len+j];
      }
    }
    else {
      Cerr << "\nError: num_levels specification differs from the number of "
           << "response functions." << std::endl;
      abort_handler(-1);
    }
  }
  // now ensure that each vector is sorted (ascending order is default)
  for (i=0; i<numFunctions; i++) {
    size_t len_i = levels[i].length();
    if (len_i > 1) {
      Real* vec_i = levels[i].values();
      if (ascending)
	std::sort(vec_i, vec_i + len_i); // ascending sort by default
      else
	std::sort(vec_i, vec_i + len_i, std::greater<Real>());// descending sort
    }
  }
}


void NonD::
construct_lhs(Iterator& u_space_sampler, Model& u_model,
	      unsigned short sample_type, int num_samples, int seed,
	      const String& rng, bool vary_pattern, short sampling_vars_mode)
{
  // sanity checks
  if (num_samples <= 0) {
    Cerr << "Error: bad samples specification (" << num_samples << ") in "
	 << "NonD::construct_lhs()." << std::endl;
    abort_handler(-1);
  }

  // construct NonDLHSSampling with default sampling_vars_mode (ACTIVE)
  u_space_sampler.assign_rep(std::make_shared<NonDLHSSampling>(u_model,
    sample_type, num_samples, seed, rng, vary_pattern, sampling_vars_mode));
}


void NonD::initialize_response_covariance()
{ } // default is no-op


/** Default definition of virtual function (used by sampling, reliability,
    and stochastic expansion methods) defines the set of statistical results
    to include the first two moments and level mappings for each QoI. */
void NonD::initialize_final_statistics()
{
  size_t i, j, num_levels, cntr = 0, rl_len = 0, num_final_stats,
    num_active_vars = iteratedModel.cv();
  if (epistemicStats)
    num_final_stats = 2*numFunctions;
  else { // aleatory UQ
    num_final_stats  = (finalMomentsType) ? 2*numFunctions : 0;
    num_final_stats += totalLevelRequests;
    if (respLevelTargetReduce) {
      rl_len = requestedRespLevels[0].length();
      for (i=1; i<numFunctions; ++i)
	if (requestedRespLevels[i].length() != rl_len) {
	  Cerr << "Error: system metric aggregation from component metrics "
	       << "requires\n       consistency in length of response_levels "
	       << "among response function set." << std::endl;
	  abort_handler(-1);
	}
      num_final_stats += rl_len; // 1 aggregated system metric per resp level
    }
  }
  // Instantiate finalStatistics:
  // > default response ASV/DVV may be overridden by NestedModel update
  //   in subIterator.response_results_active_set(sub_iterator_set)
  // > inactive views are not set until NestedModel ctor defines them, and
  //   subIterator construction follows in NestedModel::derived_init_comms()
  //   --> invocation of this fn from NonD ctors should have inactive view
  ActiveSet stats_set(num_final_stats);//, num_active_vars); // default RV = 1
  stats_set.derivative_vector(iteratedModel.inactive_continuous_variable_ids());
  finalStatistics = Response(SIMULATION_RESPONSE, stats_set);

  // Assign meaningful labels to finalStatistics (appear in NestedModel output)
  char resp_tag[10];
  StringArray stats_labels(num_final_stats);
  if (epistemicStats) { // epistemic & mixed aleatory/epistemic
    for (i=0; i<numFunctions; ++i) {
      std::sprintf(resp_tag, "_r%zu", i+1);
      stats_labels[cntr++] = String("z_lo") + String(resp_tag);
      stats_labels[cntr++] = String("z_up") + String(resp_tag);
    }
  }
  else {                     // aleatory
    char lev_tag[10];
    String dist_tag = (cdfFlag) ? String("cdf") : String("ccdf");
    for (i=0; i<numFunctions; ++i) {
      std::sprintf(resp_tag, "_r%zu", i+1);
      if (finalMomentsType) {
	stats_labels[cntr++] = String("mean") + String(resp_tag);
	stats_labels[cntr++] = (finalMomentsType == Pecos::CENTRAL_MOMENTS) ?
	  String("variance") + String(resp_tag) :
	  String("std_dev")  + String(resp_tag);
      }
      num_levels = requestedRespLevels[i].length();
      for (j=0; j<num_levels; ++j, ++cntr) {
	switch (respLevelTarget) {
	case PROBABILITIES:     std::sprintf(lev_tag, "_plev%zu",  j+1); break;
	case RELIABILITIES:     std::sprintf(lev_tag, "_blev%zu",  j+1); break;
	case GEN_RELIABILITIES: std::sprintf(lev_tag, "_b*lev%zu", j+1); break;
	}
	stats_labels[cntr] = dist_tag + String(lev_tag) + String(resp_tag);
      }
      num_levels = requestedProbLevels[i].length() +
	requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
      for (j=0; j<num_levels; ++j, ++cntr) {
	std::sprintf(lev_tag, "_zlev%zu", j+1);
	stats_labels[cntr] = dist_tag + String(lev_tag) + String(resp_tag);
      }
    }
    if (respLevelTargetReduce) {
      String sys_tag("_sys");
      for (j=0; j<rl_len; ++j, ++cntr) {
	switch (respLevelTarget) {
	case PROBABILITIES:     std::sprintf(lev_tag, "_plev%zu",  j+1); break;
	case RELIABILITIES:     std::sprintf(lev_tag, "_blev%zu",  j+1); break;
	case GEN_RELIABILITIES: std::sprintf(lev_tag, "_b*lev%zu", j+1); break;
	}
	stats_labels[cntr] = dist_tag + String(lev_tag) + sys_tag;
      }
    }
  }

  finalStatistics.function_labels(stats_labels);
}


void NonD::pull_level_mappings(RealVector& level_maps, size_t offset)
{
  if (level_maps.length() < offset + totalLevelRequests)
    level_maps.resize(totalLevelRequests);

  size_t i, j, num_lev, cntr = offset;
  for (i=0; i<numFunctions; ++i) {
    num_lev = requestedRespLevels[i].length();
    switch (respLevelTarget) {
    case PROBABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	level_maps[cntr] = computedProbLevels[i][j];
      break;
    case RELIABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	level_maps[cntr] = computedRelLevels[i][j];
      break;
    case GEN_RELIABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	level_maps[cntr] = computedGenRelLevels[i][j];
      break;
    }
    num_lev = requestedProbLevels[i].length() + requestedRelLevels[i].length()
            + requestedGenRelLevels[i].length();
    for (j=0; j<num_lev; ++j, ++cntr)
      level_maps[cntr] = computedRespLevels[i][j];
  }
}


void NonD::push_level_mappings(const RealVector& level_maps, size_t offset)
{
  if (level_maps.length() < offset + totalLevelRequests) {
    Cerr << "Error: insufficient vector length in NonD::push_level_mappings()"
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  size_t i, j, num_lev, cntr = offset;
  for (i=0; i<numFunctions; ++i) {
    num_lev = requestedRespLevels[i].length();
    switch (respLevelTarget) {
    case PROBABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	computedProbLevels[i][j] = level_maps[cntr];
      break;
    case RELIABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	computedRelLevels[i][j] = level_maps[cntr];
      break;
    case GEN_RELIABILITIES:
      for (j=0; j<num_lev; ++j, ++cntr)
	computedGenRelLevels[i][j] = level_maps[cntr];
      break;
    }
    num_lev = requestedProbLevels[i].length() + requestedRelLevels[i].length()
            + requestedGenRelLevels[i].length();
    for (j=0; j<num_lev; ++j, ++cntr)
      computedRespLevels[i][j] = level_maps[cntr];
  }
}


/** A one-dimensional sequence is assumed in this case. */
void NonD::
configure_sequence(//unsigned short hierarch_dim,
		   size_t& num_steps, size_t& secondary_index, short& seq_type)
{
  // Allow either model forms or discretization levels, but not both
  // (precedence determined by ML/MF calling algorithm)
  ModelList& sub_models = iteratedModel.subordinate_models(false);
  ModelLIter m_iter = --sub_models.end(); // HF model
  size_t num_mf = sub_models.size(), num_hf_lev = m_iter->solution_levels();
  bool ml = iteratedModel.multilevel();

  //switch (hierarch_dim) {
  //case 1:
    if (ml || iteratedModel.multilevel_multifidelity()) {
      // only loop (1D) or outer loop (2D)
      seq_type  = Pecos::RESOLUTION_LEVEL_SEQUENCE;
      num_steps = num_hf_lev;  secondary_index = num_mf - 1;
      if (ml && num_mf > 1)
	Cerr << "Warning: multiple model forms will be ignored by "
	     << "NonD::configure_sequence() for ML precedence.\n";
    }
    else if (iteratedModel.multifidelity()) {
      seq_type  = Pecos::MODEL_FORM_SEQUENCE;
      num_steps = num_mf;
      // retain each model's active solution control index:
      secondary_index = SZ_MAX;
      if (num_hf_lev > 1)
	Cerr << "Warning: solution control levels will be ignored by "
	     << "NonD::configure_sequence() for MF precedence.\n";
    }
    else {
      Cerr << "Error: no model hierarchy evident in NonD::configure_sequence()."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
  /*
    break;
  case 2:
    if (iteratedModel.multilevel_multifidelity()) {
      seq_type  = Pecos::FORM_RESOLUTION_ENUMERATION;
      num_steps = num_mf;
      secondary_index = num_hf_lev; // use this field as secondary step count?
    }
    else {
      Cerr << "Error: no compatible model hierarchy evident in NonDExpansion::"
	   << "configure_sequence()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    break;
  }
  */
}


/** Both model forms and resolutions are enumerated, from head to tail with
    resolutions as inner loop. */
void NonD::
configure_enumeration(size_t& num_combinations)//, short& seq_type)
{
  // Enumerate both model forms and discretization levels
  num_combinations = 0;
  ModelList& sub_models = iteratedModel.subordinate_models(false);// includes HF
  for (ModelLIter m_iter=sub_models.begin(); m_iter!=sub_models.end(); ++m_iter)
    num_combinations += m_iter->solution_levels();
}


bool NonD::
query_cost(unsigned short num_costs, short seq_type, RealVector& cost)
{
  ModelList& sub_models = iteratedModel.subordinate_models(false);
  bool cost_defined;
  switch (seq_type) {
  case Pecos::RESOLUTION_LEVEL_SEQUENCE: // 1D resolution hierarchy for HF model
    cost_defined = query_cost(num_costs, sub_models.back(), cost);  break;
  case Pecos::MODEL_FORM_SEQUENCE:
    if (sub_models.size() == num_costs) {
      cost.sizeUninitialized(num_costs);
      ModelLIter m_iter = sub_models.begin();
      for (unsigned short i=0; i<num_costs; ++i, ++m_iter)
	cost[i] = m_iter->solution_level_cost();//active soln index; 0 if !found
      cost_defined = valid_cost_values(cost);  // must be > 0.
    }
    else cost_defined = false;
    if (!cost_defined) cost.sizeUninitialized(0);//for compute_equivalent_cost()
    break;
  case Pecos::FORM_RESOLUTION_ENUMERATION: {
    cost.sizeUninitialized(num_costs); // includes lower bound of 1 per model
    ModelLIter m_iter;  cost_defined = true;  size_t cntr = 0;
    // assemble resolution level costs head to tail across models
    // (model costs are presumed to be more separated than resolution costs)
    for (m_iter=sub_models.begin(); m_iter!=sub_models.end(); ++m_iter) {
      RealVector m_costs = m_iter->solution_level_costs();
      size_t num_cost_m = m_costs.length(); // may be zero
      if (num_cost_m == m_iter->solution_levels() &&
	  valid_cost_values(m_costs)) {
	copy_data_partial(m_costs, cost, cntr);
	cntr += num_cost_m;
      }
      else
	{ cost_defined = false;  break; }
    }
    if (!cost_defined) cost.sizeUninitialized(0);//for compute_equivalent_cost()
    break;
  }
  }
  return cost_defined;
}


bool NonD::query_cost(unsigned short num_costs, Model& model, RealVector& cost)
{
  cost = model.solution_level_costs(); // can be empty
  if (cost.length() != num_costs || !valid_cost_values(cost))
    { cost.sizeUninitialized(0); return false; }
  return true;
}


bool NonD::valid_cost_values(const RealVector& cost)
{
  size_t i, len = cost.length();
  for (i=0; i<len; ++i)
    if (cost[i] <= 0.)
      return false;
  return true;
}


void NonD::
load_pilot_sample(const SizetArray& pilot_spec, size_t num_groups,
		  SizetArray& delta_N_l)
{
  size_t pilot_size = pilot_spec.size();
  if (num_groups == pilot_size)
    delta_N_l = pilot_spec;
  else if (pilot_size <= 1) {
    size_t num_samp = (pilot_size) ? pilot_spec[0] : 100;
    delta_N_l.assign(num_groups, num_samp);
  }
  else {
    Cerr << "Error: inconsistent pilot sample size (" << pilot_size
	 << ") in NonD::load_pilot_sample(SizetArray).  " << num_groups
	 << " expected." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  Cout << "\nPilot sample:\n" << delta_N_l << std::endl;
}


void NonD::
load_pilot_sample(const SizetArray& pilot_spec, short seq_type,
		  const Sizet3DArray& N_l, Sizet2DArray& delta_N_l)
{
  size_t i, num_samp, pilot_size = pilot_spec.size(), num_mf = N_l.size();
  delta_N_l.resize(num_mf);

  // allow several different pilot sample specifications
  // This approach assumes MLMF: includes all model forms and resolution levels
  if (pilot_size <= 1) {
    num_samp = (pilot_size) ? pilot_spec[0] : 100;
    for (i=0; i<num_mf; ++i)
      delta_N_l[i].assign(N_l[i].size(), num_samp);
  }
  else {
    size_t j, num_lev, num_prev_lev, num_total_lev = 0;
    bool same_lev = true;

    for (i=0; i<num_mf; ++i) {
      num_lev = N_l[i].size();
      delta_N_l[i].resize(num_lev);
      if (i && num_lev != num_prev_lev) same_lev = false;
      num_total_lev += num_lev; num_prev_lev = num_lev;
    }

    if (same_lev && pilot_size == num_lev)
      for (j=0; j<num_lev; ++j) {
	num_samp = pilot_spec[j];
	for (i=0; i<num_mf; ++i)
	  delta_N_l[i][j] = num_samp;
      }
    else if (pilot_size == num_total_lev) {
      size_t cntr = 0;
      for (i=0; i<num_mf; ++i) {
	SizetArray& delta_N_li = delta_N_l[i]; num_lev = delta_N_li.size();
	for (j=0; j<num_lev; ++j, ++cntr)
	  delta_N_li[j] = pilot_spec[cntr];
      }
    }
    else {
      Cerr << "Error: inconsistent pilot sample size (" << pilot_size
	   << ") in NonD::load_pilot_sample(Sizet2DArray)." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  Cout << "\nMultilevel-multifidelity pilot sample:\n";
  print_multilevel_model_summary(Cout, delta_N_l, "Pilot", seq_type,
				 discrepancy_sample_counts());
}


void NonD::resize_final_statistics_gradients()
{
  if (finalStatistics.is_null()) // not all ctor chains track final stats
    return;

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, num_final_stats = final_asv.size();
  bool final_grad_flag = false;
  for (i=0; i<num_final_stats; i++)
    if (final_asv[i] & 2) // no need to distinguish moment/level mapping grads
      { final_grad_flag = true; break; }
  finalStatistics.reshape(num_final_stats, final_dvv.size(),
			  final_grad_flag, false); // no final Hessians
}


void NonD::update_final_statistics()
{
  if (finalStatistics.is_null()) // some ctor chains do not track final stats
    return;  

  // this default implementation gets overridden/augmented in derived classes
  update_aleatory_final_statistics();

  if (respLevelTargetReduce) {
    update_system_final_statistics();
    update_system_final_statistics_gradients();
  }
}


void NonD::update_aleatory_final_statistics()
{
  // update finalStatistics from computed{Resp,Prob,Rel,GenRel}Levels
  size_t i, j, cntr = 0, rl_len, pl_bl_gl_len;
  for (i=0; i<numFunctions; ++i) {
    // final stats from compute_moments()
    if (finalMomentsType) {
      if (momentStats.empty())
	cntr += 2;
      else {
	const Real* mom_i = momentStats[i];
	finalStatistics.function_value(mom_i[0], cntr++); // mean
	finalStatistics.function_value(mom_i[1], cntr++); // stdev or var
      }
    }
    // final stats from compute_level_mappings()
    rl_len = requestedRespLevels[i].length();
    switch (respLevelTarget) { // individual component p/beta/beta*
    case PROBABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedProbLevels[i][j], cntr);
      break;
    case RELIABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedRelLevels[i][j], cntr);
      break;
    case GEN_RELIABILITIES:
      for (j=0; j<rl_len; ++j, ++cntr)
	finalStatistics.function_value(computedGenRelLevels[i][j], cntr);
      break;
    }
    pl_bl_gl_len = requestedProbLevels[i].length()
      + requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
    for (j=0; j<pl_bl_gl_len; ++j, ++cntr)
      finalStatistics.function_value(computedRespLevels[i][j], cntr);
  }
}


void NonD::update_system_final_statistics()
{
  // same rl_len enforced for all resp fns in initialize_final_statistics()
  size_t i, j, rl_len = requestedRespLevels[0].length(),
    cntr = totalLevelRequests;
  if (finalMomentsType) cntr += 2*numFunctions;
  for (j=0; j<rl_len; ++j, ++cntr) {
    // compute system probability
    Real system_p = 1.;
    switch (respLevelTargetReduce) {
    case SYSTEM_SERIES: // system p_success = product of component p_success
      for (i=0; i<numFunctions; ++i)
	system_p *= (1.-computedProbLevels[i][j]);
      system_p = 1. - system_p; // convert back to p_fail
      break;
    case SYSTEM_PARALLEL: // system p_fail = product of component p_fail
      for (i=0; i<numFunctions; ++i)
	system_p *= computedProbLevels[i][j];
      break;
    }
#ifdef DEBUG
    Cout << "\nSystem p = " << system_p << " from component p:\n";
    for (i=0; i<numFunctions; ++i)
      Cout << "  " << computedProbLevels[i][j] << '\n';
    Cout << '\n';
#endif // DEBUG
    // convert system probability to desired system metric
    switch (respLevelTarget) {
    case PROBABILITIES:
      finalStatistics.function_value(system_p, cntr); break;
    case RELIABILITIES: case GEN_RELIABILITIES:
      finalStatistics.function_value(
	-Pecos::NormalRandomVariable::inverse_std_cdf(system_p), cntr);
      break;
    }
  }
}


void NonD::update_system_final_statistics_gradients()
{
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  // same rl_len enforced for all resp fns in initialize_final_statistics()
  size_t l, v, s, p, rl_len = requestedRespLevels[0].length(),
    num_deriv_vars = final_dvv.size(), cntr = totalLevelRequests,
    moment_offset = (finalMomentsType) ? 2 : 0;
  if (finalMomentsType) cntr += 2*numFunctions;
  RealVector final_stat_grad(num_deriv_vars, false);
  RealVectorArray component_grad(numFunctions);
  Real prod;
  for (l=0; l<rl_len; ++l, ++cntr) {
    if (final_asv[cntr] & 2) {
      // Retrieve component probability gradients from finalStatistics
      size_t index = 0;
      for (s=0; s<numFunctions; ++s) {
	index += moment_offset;
	if (respLevelTarget == PROBABILITIES)
	  component_grad[s] = finalStatistics.function_gradient_view(index+l);
	else {
	  component_grad[s] = finalStatistics.function_gradient_copy(index+l);
	  Real component_beta = finalStatistics.function_value(index+l);
	  component_grad[s].scale(
	    -Pecos::NormalRandomVariable::std_pdf(-component_beta));
	}
	index += rl_len + requestedProbLevels[s].length() +
	  requestedRelLevels[s].length() + requestedGenRelLevels[s].length();
#ifdef DEBUG
	Cout << "component gradient " << s+1 << ":\n"
	     << component_grad[s] << '\n';
#endif // DEBUG
      }
      // Compute system probability
      for (v=0; v<num_deriv_vars; ++v) {
	// apply product rule over n factors
	Real& sum = final_stat_grad[v]; sum = 0.;
	for (s=0; s<numFunctions; ++s) {
	  prod = 1.;
	  switch (respLevelTargetReduce) {
	  case SYSTEM_SERIES:// system p_success = prod of component p_success
	    for (p=0; p<numFunctions; ++p)
	      prod *= (p == s) ? -component_grad[p][v] :
		1.-computedProbLevels[p][l];
	    break;
	  case SYSTEM_PARALLEL: // system p_fail = product of component p_fail
	    for (p=0; p<numFunctions; ++p)
	      prod *= (p == s) ? component_grad[p][v] :
		computedProbLevels[p][l];
	    break;
	  }
	  sum += prod;
	}
      }
      Real factor = 1.; bool scale = false;
      // negate gradient if converting system p_success to system p_fail
      if (respLevelTargetReduce == SYSTEM_SERIES)
	{ factor *= -1.; scale = true; }
      // define any scaling for system metric type
      if (respLevelTarget != PROBABILITIES) {
	Real sys_beta = finalStatistics.function_value(cntr);
	factor /= -Pecos::NormalRandomVariable::std_pdf(-sys_beta);
	scale = true;
      }
      if (scale) final_stat_grad.scale(factor);
      finalStatistics.function_gradient(final_stat_grad, cntr);
    }
  }
}


void NonD::initialize_level_mappings()
{
  // Default sizing assumes no distinction between requested and achieved
  // levels for the same measure (the request is always achieved) and assumes
  // probability (e.g., computed by binning) and reliability (e.g., computed
  // by moment projection) are not collapsible.
  if (computedRespLevels.empty() || computedProbLevels.empty() ||
      computedRelLevels.empty()  || computedGenRelLevels.empty()) {
    computedRespLevels.resize(numFunctions);
    computedProbLevels.resize(numFunctions);
    computedRelLevels.resize(numFunctions);
    computedGenRelLevels.resize(numFunctions);
    for (size_t i=0; i<numFunctions; ++i) {
      switch (respLevelTarget) {
      case PROBABILITIES:
	computedProbLevels[i].resize(requestedRespLevels[i].length());   break;
      case RELIABILITIES:
	computedRelLevels[i].resize(requestedRespLevels[i].length());    break;
      case GEN_RELIABILITIES:
	computedGenRelLevels[i].resize(requestedRespLevels[i].length()); break;
      }
      computedRespLevels[i].resize(requestedProbLevels[i].length() +
				   requestedRelLevels[i].length()  +
				   requestedGenRelLevels[i].length());
    }
  }
}


/** This function infers PDFs from the CDF/CCDF level mappings, in order
    to enable PDF computation after CDF/CCDF probability level refinement
    (e.g., from importance sampling).  

    prob_refinement alerts the routine to exclude inverse mappings from
    the PDF, since refinement only applies to z->p forward mappings and
    mixing refined and unrefined probability mappings results in an
    inconsistency (potentially manifesting as negative density values).

    all_levels_computed is an option used by reliability methods where 
    computed*Levels are defined across the union of all requested levels. */
void NonD::
compute_densities(const RealRealPairArray& min_max_fns,
		  bool prob_refinement, bool all_levels_computed)
{
  if (!pdfOutput)
    return;

  computedPDFAbscissas.resize(numFunctions);
  computedPDFOrdinates.resize(numFunctions);
  archive_allocate_pdf();

  size_t i, j, cntr, core_pdf_bins, pdf_size, offset;
  Real z, min, max, prev_r, prev_p, new_r, new_p, last_r;
  std::map<Real, Real> cdf_map;
  std::map<Real, Real>::iterator it, it_last;
  pdfComputed.resize(numFunctions, false);
  for (i=0; i<numFunctions; ++i) {

    // CDF/CCDF mappings: z -> p/beta/beta* and p/beta/beta* -> z
    const RealVector&  req_rlev_i = requestedRespLevels[i];
    const RealVector&  req_plev_i = requestedProbLevels[i];
    const RealVector&  req_glev_i = requestedGenRelLevels[i];
    const RealVector& comp_rlev_i = computedRespLevels[i];
    const RealVector& comp_plev_i = computedProbLevels[i];
    const RealVector& comp_glev_i = computedGenRelLevels[i];
    size_t rl_len = req_rlev_i.length(), pl_len = req_plev_i.length(),
      bl_len = requestedRelLevels[i].length(), gl_len = req_glev_i.length();

    // Define a unique sorted map of response levels -> cdf probabilities.
    // (refer to NonD::initialize_level_mappings() for default indexing;
    cdf_map.clear(); min = min_max_fns[i].first; max = min_max_fns[i].second;
    if (all_levels_computed) {
      for (j=0; j<rl_len; ++j) {
        z = comp_rlev_i[j]; // request may be outside extreme values
        cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
      }
      if (!prob_refinement || !rl_len) { // don't combine refined/unrefined
        size_t total_len = rl_len + pl_len + bl_len + gl_len;
        for (j=rl_len; j<total_len; ++j) {
          z = comp_rlev_i[j];
          //if (z >= min && z <= max) // exclude any extrapolations outside bnds
          cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
        }
      }
    }
    else {
      switch (respLevelTarget) {
        case PROBABILITIES:
        for (j=0; j<rl_len; ++j) {
          z = req_rlev_i[j]; // request may be outside extreme values
          cdf_map[z] = (cdfFlag) ? comp_plev_i[j] : 1.-comp_plev_i[j];
        }
        break;
        //case RELIABILITIES: // exclude reliability level mappings from cdf_map
        case GEN_RELIABILITIES:
        for (j=0; j<rl_len; ++j) {
          z = req_rlev_i[j]; // request may be outside extreme values
          Real g_cdf = (cdfFlag) ? comp_glev_i[j] : -comp_glev_i[j];
          cdf_map[z] = Pecos::NormalRandomVariable::std_cdf(-g_cdf);
        }
        break;
      }
      if (!prob_refinement || !rl_len) { // don't combine refined/unrefined
        for (j=0; j<pl_len; ++j) {
          z = comp_rlev_i[j];
          //if (z >= min && z <= max) // exclude any extrapolations outside bnds
          cdf_map[z] = (cdfFlag) ? req_plev_i[j] : 1.-req_plev_i[j];
        }
        // exclude reliability level mappings from cdf_map
        for (j=0, cntr=pl_len+bl_len; j<gl_len; ++j, ++cntr) {
          z = comp_rlev_i[cntr];
          //if (z >= min && z <= max) {//exclude any extrapolations outside bnds
          Real g_cdf = (cdfFlag) ? req_glev_i[j] : -req_glev_i[j];
          cdf_map[z] = Pecos::NormalRandomVariable::std_cdf(-g_cdf);
            //}
        }
      }
    }

    if (!cdf_map.empty()) {
      it      =   cdf_map.begin(); prev_r = it->first;
      it_last = --cdf_map.end();   last_r = it_last->first;
      // compute computedPDF{Abscissas,Ordinates} from bin counts and widths
      core_pdf_bins = cdf_map.size()-1; pdf_size = core_pdf_bins;
      if (min < prev_r) ++pdf_size;
      if (max > last_r) ++pdf_size;
      RealVector& abs_i = computedPDFAbscissas[i]; abs_i.resize(pdf_size+1);
      RealVector& ord_i = computedPDFOrdinates[i]; ord_i.resize(pdf_size);
      if (min < prev_r) { // first bin accumulates p0 over [min,lev0]
        offset   = 1;   prev_p   = it->second;
        abs_i[0] = min;	ord_i[0] = prev_p/(prev_r - min);
      }
      else { // first bin accumulates p0+p1 over [lev0,lev1]
        offset   = 0;   prev_p   = 0.;
      }
      for (j=0; j<core_pdf_bins; ++j) {
        ++it; new_r = it->first; new_p = it->second;
        abs_i[j+offset] = prev_r;
        ord_i[j+offset] = (new_p - prev_p) / (new_r - prev_r);
        prev_r = new_r; prev_p = new_p;
      }
      if (max > last_r) {
        abs_i[pdf_size-1] = last_r;
        ord_i[pdf_size-1] = (1. - it_last->second)/(max - last_r);
        abs_i[pdf_size]   = max;  // no ordinate for final abscissa
      }
      else
        abs_i[pdf_size] = last_r; // no ordinate for final abscissa
    pdfComputed[i] = true;
    }
  }
}


/** Print distribution mappings, including to file per response. */
void NonD::
print_level_mappings(std::ostream& s, String qoi_type,
		     const StringArray& qoi_labels) const
{
  print_densities(s, qoi_type, qoi_labels);

  // output CDF/CCDF probabilities resulting from binning or CDF/CCDF
  // reliabilities resulting from number of std devs separating mean & target
  s << std::scientific << std::setprecision(write_precision)
    << "\nLevel mappings for each " << qoi_type << ":\n";
  for (size_t i=0; i<numFunctions; ++i)
    if (!requestedRespLevels[i].empty() || !requestedProbLevels[i].empty() ||
	!requestedRelLevels[i].empty()  || !requestedGenRelLevels[i].empty()) {
      print_level_map(s, i, qoi_labels[i]);
      // optionally write the distribution mapping to a .dist file
      if (outputLevel >= VERBOSE_OUTPUT)
	level_mappings_file(i, qoi_labels[i]);
    }
}


/** This version differs in its use of a concatenated vector of level
    mappings, rathen than computed{Resp,Prob,Real,GenRel}Levels. */
void NonD::
print_level_mappings(std::ostream& s, const RealVector& level_maps,
		     bool moment_offset, const String& prepend)
{
  if (level_maps.empty()) return;

  if (prepend.empty())   s << "\nLevel mappings for each response function:\n";
  else s << '\n' << prepend << " level mappings for each response function:\n";

  size_t i, j, cntr,
    width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;
  const StringArray& qoi_labels = iteratedModel.response_labels();
  for (i=0, cntr=0; i<numFunctions; ++i) {
    if (moment_offset) cntr += 2; // skip over moments, if present
    if (cdfFlag) s << "Cumulative Distribution Function (CDF) for ";
    else s << "Complementary Cumulative Distribution Function (CCDF) for ";
    s << qoi_labels[i] << ":\n     Response Level  Probability Level  "
      << "Reliability Index  General Rel Index\n     --------------  "
      << "-----------------  -----------------  -----------------\n";
    const RealVector& req_z_levs = requestedRespLevels[i];
    size_t num_z_levs = req_z_levs.length();
    for (j=0; j<num_z_levs; ++j, ++cntr) {
      s << "  " << std::setw(width) << req_z_levs[j] << "  ";
      switch (respLevelTarget) {
      case PROBABILITIES:
	s << std::setw(width) << level_maps[cntr] << '\n'; break;
      case RELIABILITIES:
	s << std::setw(w2p2)  << level_maps[cntr] << '\n'; break;
      case GEN_RELIABILITIES:
	s << std::setw(w3p4)  << level_maps[cntr] << '\n'; break;
      }
    }
    const RealVector& req_p_levs = requestedProbLevels[i];
    size_t num_p_levs = req_p_levs.length();
    for (j=0; j<num_p_levs; ++j, ++cntr)
      s << "  " << std::setw(width) << level_maps[cntr]
	<< "  "	<< std::setw(width) << req_p_levs[j] << '\n';
    const RealVector& req_b_levs = requestedRelLevels[i];
    size_t num_b_levs = req_b_levs.length();
    for (j=0; j<num_b_levs; ++j, ++cntr)
      s << "  " << std::setw(width) << level_maps[cntr]
	<< "  "	<< std::setw(w2p2)  << req_b_levs[j] << '\n';
    const RealVector& req_g_levs = requestedGenRelLevels[i];
    size_t num_g_levs = req_g_levs.length();
    for (j=0; j<num_g_levs; ++j, ++cntr)
      s << "  " << std::setw(width) << level_maps[cntr]
	<< "  "	<< std::setw(w3p4)  << req_g_levs[j] << '\n';
  }
}


void NonD::
print_densities(std::ostream& s, String qoi_type,
		const StringArray& pdf_labels) const
{
  if (!pdfOutput)
    return;

  size_t i, j, wpp7 = write_precision+7, num_qoi = computedPDFOrdinates.size();
  if (num_qoi)
    s << std::scientific << std::setprecision(write_precision)
      << "\nProbability Density Function (PDF) histograms for each "
      << qoi_type << ":\n";
  for (i=0; i<num_qoi; ++i) {
    const RealVector& ord_i = computedPDFOrdinates[i];
    const RealVector& abs_i = computedPDFAbscissas[i];
    size_t pdf_len = ord_i.length();
    if (pdf_len) {
      s << "PDF for " << pdf_labels[i] << ":\n"
	<< "          Bin Lower          Bin Upper      Density Value\n"
	<< "          ---------          ---------      -------------\n";
      for (j=0; j<pdf_len; ++j)
	s << "  " << std::setw(wpp7) << abs_i[j]
	  << "  " << std::setw(wpp7) << abs_i[j+1]
	  << "  " << std::setw(wpp7) << ord_i[j] << '\n';
    }
  }
}


/// Write distribution mappings to a file for a single response
void NonD::level_mappings_file(size_t fn_index, const String& qoi_label) const
{
  std::string dist_filename = qoi_label + ".dist";
  std::ofstream dist_file;
  TabularIO::open_file(dist_file, dist_filename, "Distribution Map Output"); 
  dist_file << std::scientific << std::setprecision(write_precision);
  print_level_map(dist_file, fn_index, qoi_label);
}


/** Print the distribution mapping for a single response function to
    the passed output stream.  This base class version maps from one
    requested level type to one computed level type; some derived
    class implementations (e.g., local and global reliability) output
    multiple computed level types. */
void NonD::
print_level_map(std::ostream& s, size_t fn_index, const String& qoi_label) const
{
  size_t j, width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;

  if (cdfFlag)
    s << "Cumulative Distribution Function (CDF) for ";
  else
    s << "Complementary Cumulative Distribution Function (CCDF) for ";
  s << qoi_label << ":\n     Response Level  Probability Level  "
    << "Reliability Index  General Rel Index\n     --------------  "
    << "-----------------  -----------------  -----------------\n";
  size_t num_resp_levels = requestedRespLevels[fn_index].length();
  for (j=0; j<num_resp_levels; j++) { // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << requestedRespLevels[fn_index][j] << "  ";
    switch (respLevelTarget) {
    case PROBABILITIES:
      s << std::setw(width) << computedProbLevels[fn_index][j]   << '\n'; break;
    case RELIABILITIES:
      s << std::setw(w2p2)  << computedRelLevels[fn_index][j]    << '\n'; break;
    case GEN_RELIABILITIES:
      s << std::setw(w3p4)  << computedGenRelLevels[fn_index][j] << '\n'; break;
    }
  }
  size_t num_prob_levels = requestedProbLevels[fn_index].length();
  for (j=0; j<num_prob_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j]
      << "  " << std::setw(width) << requestedProbLevels[fn_index][j] << '\n';
  size_t num_rel_levels = requestedRelLevels[fn_index].length(),
    offset = num_prob_levels;
  for (j=0; j<num_rel_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j+offset]
      << "  " << std::setw(w2p2)  << requestedRelLevels[fn_index][j] << '\n';
  size_t num_gen_rel_levels = requestedGenRelLevels[fn_index].length();
  offset += num_rel_levels;
  for (j=0; j<num_gen_rel_levels; j++) // map from 1 requested to 1 computed
    s << "  " << std::setw(width) << computedRespLevels[fn_index][j+offset]
      << "  " << std::setw(w3p4)  << requestedGenRelLevels[fn_index][j] << '\n';
}


void NonD::print_system_mappings(std::ostream& s) const
{
  size_t rl_len = requestedRespLevels[0].length();
  if (respLevelTargetReduce && rl_len) {
    size_t i, width = write_precision+7, g_width = 2*width+2,
      cntr = totalLevelRequests;
    if (finalMomentsType) cntr += 2*numFunctions;
    const RealVector& final_stats = finalStatistics.function_values();
    s << std::scientific << std::setprecision(write_precision)
      << "\nSystem response level mappings:\n";
    if (cdfFlag) s << "Cumulative distribution metrics ";
    else         s << "Complementary cumulative distribution metrics ";
    if (respLevelTargetReduce == SYSTEM_SERIES)        s << "for series ";
    else if (respLevelTargetReduce == SYSTEM_PARALLEL) s << "for parallel ";
    s << "system:\n     Resp Level Set  Probability Level  Reliability Index  "
      << "General Rel Index\n     --------------  -----------------  "
      << "-----------------  -----------------\n";
    Real prob, gen_rel;
    for (i=0; i<rl_len; ++i, ++cntr) {
      switch (respLevelTarget) {
      case PROBABILITIES:
	prob    =  final_stats[cntr];
	gen_rel = -Pecos::NormalRandomVariable::inverse_std_cdf(prob); break;
      default:
	gen_rel =  final_stats[cntr];
	prob    =  Pecos::NormalRandomVariable::std_cdf(-gen_rel);     break;
      }
      s << "  " << std::setw(width) << i+1 << "  " << std::setw(width) << prob
	<< "  " << std::setw(g_width) << gen_rel << '\n';
    }
  }
}


void NonD::
print_multilevel_evaluation_summary(std::ostream& s, const SizetArray& N_m)
{
  size_t j, width = write_precision+7, num_lev = N_m.size();
  for (j=0; j<num_lev; ++j)
    s << "                     " << std::setw(width) << N_m[j]
      << "  QoI_lev" << j << '\n'; // QoI counts
}


void NonD::
print_multilevel_evaluation_summary(std::ostream& s, const Sizet2DArray& N_m)
{
  size_t j, width = write_precision+7, num_lev = N_m.size();
  for (j=0; j<num_lev; ++j) {
    const SizetArray& N_j = N_m[j];
    if (!N_j.empty()) {
      s << "                     " << std::setw(width) << N_j[0];
      if (!homogeneous(N_j)) { // print all counts in this 1D array
	size_t q, num_q = N_j.size();
	for (size_t q=1; q<num_q; ++q)
	  s << ' ' << N_j[q];
      }
      s << "  QoI_lev" << j << '\n';
    }
  }
}


void NonD::
print_multilevel_discrepancy_summary(std::ostream& s, const SizetArray& N_m)
{
  size_t j, N_unroll, width = write_precision+7, num_lev = N_m.size(),
    num_discrep = num_lev - 1;
  for (j=0; j<num_lev; ++j) {
    s << std::setw(width) << N_m[j] << "  DeltaQoI_lev" << j;
    N_unroll = (j<num_discrep) ?  N_m[j] + N_m[j+1] : N_m[j];
    s << std::setw(width) << N_unroll  << "  QoI_lev"      << j << '\n';
  }
}


void NonD::
print_multilevel_discrepancy_summary(std::ostream& s, const SizetArray& N_m,
				     const SizetArray& N_mp1)
{
  size_t j, width = write_precision+7,
    num_lev = std::min(N_m.size(), N_mp1.size());
  for (j=0; j<num_lev; ++j) { // should be 1 for current use cases
    s << std::setw(width) << N_m[j] << "  DeltaQoI_lev" << j;
    s << std::setw(width) << N_m[j] + N_mp1[j]  << "  QoI_lev"   << j << '\n';
  }
}


void NonD::
print_multilevel_discrepancy_summary(std::ostream& s, const Sizet2DArray& N_m)
{
  size_t j, N_unroll, width = write_precision+7, num_lev = N_m.size(),
    num_discrep = num_lev - 1;
  for (j=0; j<num_lev; ++j) {
    const SizetArray& N_mj = N_m[j];
    if (N_mj.empty()) continue;

    print_multilevel_row(s, N_mj);  s << "  DeltaQoI_lev" << j;
    if (j<num_discrep) print_multilevel_row(s, N_mj, N_m[j+1]);
    else               print_multilevel_row(s, N_mj);
    s << "  QoI_lev" << j << '\n';
  }
}


void NonD::
print_multilevel_discrepancy_summary(std::ostream& s, const Sizet2DArray& N_m,
				     const Sizet2DArray& N_mp1)
{
  size_t j, N_unroll, width = write_precision+7,
    num_lev = std::min(N_m.size(), N_mp1.size());
  for (j=0; j<num_lev; ++j) { // should be 1 for current use cases
    const SizetArray& N_mj = N_m[j];
    if (N_mj.empty()) continue;

    print_multilevel_row(s, N_mj);            s << "  DeltaQoI_lev" << j;
    print_multilevel_row(s, N_mj, N_mp1[j]);  s << "  QoI_lev" << j << '\n';
  }
}


void NonD::
print_multilevel_row(std::ostream& s, const SizetArray& N_j)
{
  s << std::setw(write_precision+7) << N_j[0];
  if (!homogeneous(N_j)) { // print all QoI counts in this 1D array
    size_t q, num_q = N_j.size();
    for (size_t q=1; q<num_q; ++q)
      s << ' ' << N_j[q];
  }
}


void NonD::
print_multilevel_row(std::ostream& s, const SizetArray& N_j,
		     const SizetArray& N_jp1)
{
  s << std::setw(write_precision+7) << N_j[0] + N_jp1[0];
  if (!homogeneous(N_j) || !homogeneous(N_jp1)) {
    size_t q, num_q = N_j.size();
    for (size_t q=1; q<num_q; ++q)
      s << ' ' << N_j[q] + N_jp1[q];
  }
}


unsigned short NonD::
sub_optimizer_select(unsigned short requested_sub_method,
		     unsigned short   default_sub_method)
{
  bool have_npsol = false, have_optpp = false, have_ncsu = false;
#ifdef HAVE_NPSOL
  have_npsol = true;
#endif
#ifdef HAVE_OPTPP
  have_optpp = true;
#endif
#ifdef HAVE_NCSU
  have_ncsu = true;
#endif

  unsigned short assigned_sub_method = SUBMETHOD_NONE;
  switch (requested_sub_method) {
  case SUBMETHOD_NPSOL:
    if (have_npsol) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with NPSOL SQP.\n       "
	   << "Please select alternate sub-method solver." << std::endl;
    break;

  case SUBMETHOD_OPTPP:
    if (have_optpp) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with OPT++ NIP.\n       "
	   << "Please select alternate sub-method solver." << std::endl;
    break;

  case SUBMETHOD_NPSOL_OPTPP: // not currently a spec option
    if (have_npsol && have_optpp) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with both OPT++ and "
	   << "NPSOL.\n       Please select alternate sub-method solver."
	   << std::endl;
    break;

  case SUBMETHOD_DIRECT:
    if (have_ncsu) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with NCSU DIRECT.\n"
	   << "Please select alternate sub-method solver." << std::endl;
    break;

  case SUBMETHOD_DIRECT_NPSOL:
    if (have_ncsu && have_npsol) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with both NCSU DIRECT "
	   << "and NPSOL.\n       Please select alternate sub-method solver."
	   << std::endl;
    break;

  case SUBMETHOD_DIRECT_OPTPP:
    if (have_ncsu && have_optpp) assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with both NCSU DIRECT "
	   << "and OPT++.\n       Please select alternate sub-method solver."
	   << std::endl;
    break;

  case SUBMETHOD_DIRECT_NPSOL_OPTPP:
    if (have_ncsu && have_npsol && have_optpp)
      assigned_sub_method = requested_sub_method;
    else
      Cerr << "\nError: this executable not configured with NCSU DIRECT, NPSOL"
	   << ", and OPT++.\n       Please select alternate sub-method solver."
	   << std::endl;
    break;

  case SUBMETHOD_DEFAULT:
    switch (default_sub_method) {
    case SUBMETHOD_NPSOL: // use SUBMETHOD_NPSOL if available
      if      (have_npsol) assigned_sub_method = default_sub_method;
      else if (have_optpp) assigned_sub_method = SUBMETHOD_OPTPP;
      break;
    case SUBMETHOD_OPTPP: // use SUBMETHOD_OPTPP if available
      if      (have_optpp) assigned_sub_method = default_sub_method;
      else if (have_npsol) assigned_sub_method = SUBMETHOD_NPSOL;
      break;
    case SUBMETHOD_NPSOL_OPTPP: // use both OPTPP and NPSOL if available
      if (have_npsol && have_optpp) assigned_sub_method = default_sub_method;
      else if (have_npsol) assigned_sub_method = SUBMETHOD_NPSOL;
      else if (have_optpp) assigned_sub_method = SUBMETHOD_OPTPP;
      break;
    case SUBMETHOD_DIRECT:
      if (have_ncsu) assigned_sub_method = default_sub_method;
      break;
    case SUBMETHOD_DIRECT_NPSOL:
      if (have_ncsu && have_npsol) assigned_sub_method = default_sub_method;
      else if (have_ncsu)  assigned_sub_method = SUBMETHOD_DIRECT;
      else if (have_npsol) assigned_sub_method = SUBMETHOD_NPSOL;
      break;
    case SUBMETHOD_DIRECT_OPTPP:
      if (have_ncsu && have_optpp) assigned_sub_method = default_sub_method;
      else if (have_ncsu)  assigned_sub_method = SUBMETHOD_DIRECT;
      else if (have_optpp) assigned_sub_method = SUBMETHOD_OPTPP;
      break;
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:
      if (have_ncsu) {
	if (have_npsol && have_optpp) assigned_sub_method = default_sub_method;
	else if (have_npsol) assigned_sub_method = SUBMETHOD_DIRECT_NPSOL;
	else if (have_optpp) assigned_sub_method = SUBMETHOD_DIRECT_OPTPP;
      }
      else if (have_npsol && have_optpp)
	assigned_sub_method = SUBMETHOD_NPSOL_OPTPP;
      else if (have_npsol) assigned_sub_method = SUBMETHOD_NPSOL;
      else if (have_optpp) assigned_sub_method = SUBMETHOD_OPTPP;
      break;
    }
    if (assigned_sub_method == SUBMETHOD_NONE)
      Cerr << "\nError: this executable not configured with an available "
	   << "sub-method solver." << std::endl;
    break;

  case SUBMETHOD_NONE:
    // assigned = requested = SUBMETHOD_NONE is valid for the case where a
    // sub-method solve is to be suppressed; clients are free to treat this
    // return value as an error
    break;

  default:
    Cerr << "\nError: sub-method " << requested_sub_method
	 << " not recognized in NonD::sub_optimizer_select()." << std::endl;
    break;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nSub-method " << assigned_sub_method
	 << " assigned in NonD::sub_optimizer_select()." << std::endl;

  return assigned_sub_method;
}


void NonD::archive_allocate_mappings()
{
  if (!resultsDB.active())  return;

  bool req_resp = false, req_prob = false, req_rel = false, req_gen = false;
  for (size_t i=0; i<numFunctions; ++i) {
    if (requestedRespLevels[i].length() > 0)   req_resp = true;
    if (requestedProbLevels[i].length() > 0)   req_prob = true;
    if (requestedRelLevels[i].length() > 0)    req_rel  = true;
    if (requestedGenRelLevels[i].length() > 0) req_gen  = true;
  }

  if (req_resp) {
    std::string resp_target, data_name;
    switch (respLevelTarget) { 
    case PROBABILITIES:
      resp_target = "Probability";
      data_name = resultsNames.map_resp_prob;
      break;
    case RELIABILITIES:
      resp_target = "Reliability";
      data_name = resultsNames.map_resp_rel;
      break;
    case GEN_RELIABILITIES: 
      resp_target = "Generalized Reliability";
      data_name = resultsNames.map_resp_genrel;
      break;
    }

    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Response Level", resp_target + " Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), data_name, numFunctions, md);
  }
  if (req_prob) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Probability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_prob_resp, numFunctions, md);
  }
  if (req_rel) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Reliability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_rel_resp, numFunctions, md);
  }
  if (req_gen) {
    // mapping per function, possibly empty
    MetaDataType md;
    md["Array Spans"] = make_metadatavalue("Response Functions");
    md["Column Labels"] = 
      make_metadatavalue("Generalized Reliability Level", "Response Level");
    resultsDB.array_allocate<RealMatrix>
      (run_identifier(), resultsNames.map_genrel_resp, numFunctions, md);
  }
}


// archive the mappings from response levels
void NonD::archive_from_resp(size_t i, size_t inc_id)
{
  // only insert if active and response levels specified
  size_t num_resp_levels = requestedRespLevels[i].length(); 
  if (!resultsDB.active() || num_resp_levels == 0)  return;

  size_t j;
  std::string data_name;

  RealMatrix mapping(num_resp_levels, 2);
 
  DimScaleMap scale;
  scale.emplace(0, RealScale("response_levels", requestedRespLevels[i]));
  const StringArray &labels = iteratedModel.response_labels();
  RealVector *result;

  // TODO: could use SetCol?
  switch (respLevelTarget) { 
  case PROBABILITIES:
    data_name = resultsNames.map_resp_prob;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedProbLevels[i][j];
    }
    result = &computedProbLevels[i]; 
    break;
  case RELIABILITIES:
    data_name = resultsNames.map_resp_rel;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedRelLevels[i][j];
    }
    result = &computedRelLevels[i];
    break;
  case GEN_RELIABILITIES: 
    data_name = resultsNames.map_resp_genrel;
    for (j=0; j<num_resp_levels; ++j) {
      mapping(j, 0) = requestedRespLevels[i][j];
      mapping(j, 1) = computedGenRelLevels[i][j];
    }
    result = &computedGenRelLevels[i];
    break;
  }

  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back(String("response_levels"));
  location.push_back(labels[i]);

  resultsDB.insert(run_identifier(), location, *result, scale); 

  resultsDB.array_insert<RealMatrix>(run_identifier(), data_name, i, mapping);
}


// archive the mappings to response levels
void NonD::archive_to_resp(size_t i, size_t inc_id)
{
  if (!resultsDB.active())  return;

  DimScaleMap scale;
  const StringArray &labels = iteratedModel.response_labels();
  StringArray location;
  size_t r_index = 0;
  if(inc_id) {
    location.push_back(String("increment:") + std::to_string(inc_id));
    r_index = 1;
  }
  location.push_back(String(""));
  location.push_back(labels[i]);
  size_t j;
  size_t num_prob_levels = requestedProbLevels[i].length();
  if (num_prob_levels > 0) {
    RealMatrix mapping(num_prob_levels, 2);
    for (j=0; j<num_prob_levels; j++) {
      mapping(j, 0) = requestedProbLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_prob_resp, i, mapping);
    location[r_index] = String("probability_levels");
     scale.emplace(0, RealScale("probability_levels", requestedProbLevels[i]));
    RealVector result(Teuchos::View, &computedRespLevels[i][0], num_prob_levels);
    resultsDB.insert(run_identifier(), location, result, scale);
  } 
  size_t num_rel_levels = requestedRelLevels[i].length();
  size_t offset = num_prob_levels; 
  if (num_rel_levels > 0) {
    RealMatrix mapping(num_rel_levels, 2);
    for (j=0; j<num_rel_levels; j++) {
      mapping(j, 0) = requestedRelLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j+offset];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_rel_resp, i, mapping);

    scale.emplace(0, RealScale("reliability_levels", requestedRelLevels[i]));
    RealVector result(Teuchos::View, &computedRespLevels[i][0] + offset, num_rel_levels);
    location[r_index] = String("reliability_levels");
    resultsDB.insert(run_identifier(), location, result, scale);
  } 
  size_t num_gen_rel_levels = requestedGenRelLevels[i].length();
  offset += num_rel_levels; 
  if (num_gen_rel_levels > 0) {
    RealMatrix mapping(num_gen_rel_levels, 2);
    for (j=0; j<num_gen_rel_levels; j++) {
      mapping(j, 0) = requestedGenRelLevels[i][j];
      mapping(j, 1) = computedRespLevels[i][j+offset];
    }
    resultsDB.
      array_insert<RealMatrix>(run_identifier(), 
			       resultsNames.map_genrel_resp, i, mapping);

    scale.emplace(0, RealScale("gen_reliability_levels", requestedGenRelLevels[i]));
    RealVector result(Teuchos::View, &computedRespLevels[i][0] + offset, num_gen_rel_levels);
    location[r_index] = String("gen_reliability_levels");
    resultsDB.insert(run_identifier(), location, result, scale);

  } 
}


void NonD::archive_allocate_pdf() // const
{
  if (!resultsDB.active())  return;

  // pdf per function, possibly empty
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Response Functions");
  md["Row Labels"] = 
    make_metadatavalue("Bin Lower", "Bin Upper", "Density Value");
  resultsDB.array_allocate<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, numFunctions, md);
}


void NonD::archive_pdf(size_t i, size_t inc_id) // const
{
  if (!resultsDB.active() || !pdfOutput || computedPDFOrdinates[i].length() == 0) return;

  size_t pdf_len = computedPDFOrdinates[i].length();
  RealMatrix pdf(3, pdf_len);
  for (size_t j=0; j<pdf_len; ++j) {
    pdf(0, j) = computedPDFAbscissas[i][j];
    pdf(1, j) = computedPDFAbscissas[i][j+1];
    pdf(2, j) = computedPDFOrdinates[i][j];
  }
  
  resultsDB.array_insert<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, i, pdf);

  const StringArray &labels = iteratedModel.response_labels();
  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back("probability_density");
  location.push_back(labels[i]);
  DimScaleMap scales;
  scales.emplace(0, RealScale("lower_bounds", &computedPDFAbscissas[i][0], pdf_len, ScaleScope::UNSHARED));
  scales.emplace(0, RealScale("upper_bounds", &computedPDFAbscissas[i][1], pdf_len, ScaleScope::UNSHARED));
  resultsDB.insert(run_identifier(),location, computedPDFOrdinates[i], scales);
}

void NonD::archive_equiv_hf_evals(const Real equiv_hf_evals) {
  if (!resultsDB.active()) return;
  resultsDB.add_metadata_to_execution(run_identifier(), 
      {ResultAttribute<Real>("equiv_hf_evals",equiv_hf_evals)});
}

} // namespace Dakota
