/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDHierarchSampling
//- Description: Implementation code for NonDHierarchSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDHierarchSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDHierarchSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDHierarchSampling::
NonDHierarchSampling(ProblemDescDB& problem_db, Model& model):
  NonDEnsembleSampling(problem_db, model)
{
  // ensure iteratedModel is a hierarchical surrogate model and set initial
  // response mode (for set_communicators() which precedes core_run()).
  // Note: even though the hierarchy may be multilevel | multifidelity | both,
  // we require a hierarchical model to manage aggregations, reductions, etc.
  // (i.e. a SimulationModel with resolution hyper-parameters is insufficient).
  if (iteratedModel.surrogate_type() == "hierarchical")
    aggregated_models_mode();
  else {
    Cerr << "Error: Hierarch Monte Carlo requires a hierarchical "
	 << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& ordered_models = iteratedModel.subordinate_models(false);
  size_t i, num_mf = ordered_models.size(), num_lev, prev_lev = SZ_MAX,
    md_index, num_md;
  ModelLRevIter ml_rit;
  bool err_flag = false, mlmf = (methodName==MULTILEVEL_MULTIFIDELITY_SAMPLING);
  NLev.resize(num_mf);
  costMetadataIndices.resize(num_mf);
  for (i=num_mf-1, ml_rit=ordered_models.rbegin();
       ml_rit!=ordered_models.rend(); --i, ++ml_rit) { // high fid to low fid
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev  = ml_rit->solution_levels(); // lower bound is 1 soln level
    // Note: for ML and MLCV, metadata indices only vary per model form
    md_index = ml_rit->cost_metadata_index();
    num_md   = ml_rit->current_response().metadata().size();

    if (mlmf && num_lev > prev_lev) {
      Cerr << "\nWarning: unused solution levels in multilevel-multifidelity "
	   << "sampling for model " << ml_rit->model_id() << ".\n         "
	   << "Ignoring " << num_lev - prev_lev << " of " << num_lev
	   << " levels." << std::endl;
      num_lev = prev_lev;
    }

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    if (md_index == SZ_MAX && num_lev > ml_rit->solution_levels(false)) {
      Cerr << "Error: insufficient cost data provided for multilevel sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_rit->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer to pre_run()
    costMetadataIndices[i] = SizetSizetPair(md_index, num_md);
    prev_lev = num_lev;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  pilotSamples = problem_db.get_sza("method.nond.pilot_samples");
  if ( !std::all_of( std::begin(pilotSamples), std::end(pilotSamples),
		     [](int i){ return i > 0; }) ) {
    Cerr << "\nError: Some levels have pilot samples of size 0 in "
       << method_enum_to_string(methodName) << '.' << std::endl;
    abort_handler(METHOD_ERROR);
  }
  switch (pilotSamples.size()) {
    case 0:  maxEvalConcurrency *= 100;  break;
    default: {
      size_t max_ps = find_max(pilotSamples);
      if (max_ps) maxEvalConcurrency *= max_ps;
      break;
    }
  }
}


NonDHierarchSampling::~NonDHierarchSampling()
{ }


void NonDHierarchSampling::
accumulate_paired_online_cost(RealVector& accum_cost, SizetArray& num_cost,
			      size_t step)
{
  // This implementation is for singleton or paired responses, not for
  // aggregation of a full Model ensemble

  // for ML and MLCV, accumulation can span two calls --> init outside

  const Pecos::ActiveKey& key = iteratedModel.active_model_key();
  unsigned short hf_form = key.retrieve_model_form(0),
    lf_form = (key.data_size() > 1) ? key.retrieve_model_form(1) : USHRT_MAX;
  size_t hf_form_index = (hf_form == USHRT_MAX) ? 0 : (size_t)hf_form;
  // costMetadataIndices follows ordered models
  const SizetSizetPair& hf_cost_mdi = costMetadataIndices[hf_form_index];
  size_t hf_md_index = hf_cost_mdi.first, hf_md_len = hf_cost_mdi.second,
    prev_step = (step) ? step - 1 : SZ_MAX, lf_md_index;
  if (prev_step != SZ_MAX) {
    size_t lf_form_index = (lf_form == USHRT_MAX) ? 0 : (size_t)lf_form;
    lf_md_index = costMetadataIndices[lf_form_index].first;
  }

  using std::isfinite;
  Real cost;
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel
  IntRespMCIter r_cit;
  for (r_cit=allResponses.begin(); r_cit!=allResponses.end(); ++r_cit) {
    const std::vector<RespMetadataT>& md = r_cit->second.metadata();//aggregated

    // response metadata is paired {HF,LF} as for fn data
    cost = md[hf_md_index]; // offset by metadata index
    if (isfinite(cost)) {
      accum_cost[step] += cost;
      ++num_cost[step];
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Metadata:\n" << md << "HF cost: accum_cost = "
	     << accum_cost[step] << " num_cost = " << num_cost[step]<<std::endl;
    }

    if (prev_step != SZ_MAX) {
      cost = md[hf_md_len + lf_md_index]; // offset by metadata index
      if (isfinite(cost)) {
	accum_cost[prev_step] += cost;
	++num_cost[prev_step];
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "LF cost: accum_cost = " << accum_cost[prev_step]
	       << " num_cost = " << num_cost[prev_step] << std::endl;
      }
    }
  }
  // averaging is deferred until average_online_cost()
}


/*  ... Some early notes when there was one composite core_run() ...
void NonDHierarchSampling::core_run()
{
  // Future: include peer alternatives (1D list --> matrix)
  //         For MLMC, could seek adaptive selection of most correlated
  //         alternative (or a convex combination of alternatives).

  // TO DO: hierarchy incl peers (not peers each optionally incl hierarchy)
  //   num_mf     = iteratedModel.model_hierarchy_depth();
  //   num_peer_i = iteratedModel.model_peer_breadth(i);

  // TO DO: this initial logic is limiting:
  // > allow MLMC and CVMC for either model forms or discretization levels
  // > separate method specs that both map to NonDMultifidelitySampling ???

  // TO DO: following pilot sample across levels and fidelities in mixed case,
  // could pair models for CVMC based on estimation of rho2_LH.
}
*/

} // namespace Dakota
