/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
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
  // ensure iteratedModel is an ensemble surrogate model and set initial
  // response mode (for set_communicators() which precedes core_run()).
  // Note: even though hierarchical sampling might involve a single model form,
  // we require an ensemble model to manage aggregations, reductions, etc.
  // (i.e. a SimulationModel with resolution hyper-parameters is insufficient).
  bool err_flag = false;
  if (iteratedModel.surrogate_type() == "ensemble")
    iteratedModel.surrogate_response_mode(AGGREGATED_MODEL_PAIR);
  else {
    Cerr << "Error: Hierarchical sampling requires an ensemble surrogate "
	 << "model specification." << std::endl;
    err_flag = true;
  }

  pilotSamples = problem_db.get_sza("method.nond.pilot_samples");
  if ( !std::all_of( std::begin(pilotSamples), std::end(pilotSamples),
		     [](int i){ return i > 0; }) ) {
    Cerr << "\nError: Some levels have pilot samples of size 0 in "
       << method_enum_to_string(methodName) << '.' << std::endl;
    err_flag = true;
  }
  switch (pilotSamples.size()) {
    case 0:  maxEvalConcurrency *= 100;  break;
    default: {
      size_t max_ps = find_max(pilotSamples);
      if (max_ps) maxEvalConcurrency *= max_ps;
      break;
    }
  }

  if (err_flag)
    abort_handler(METHOD_ERROR);
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
