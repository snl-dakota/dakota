/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
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
  unsigned short form1 = key.retrieve_model_form(0);
  size_t form1_index = (form1 == USHRT_MAX) ? 0 : (size_t)form1;
  // costMetadataIndices follows ordered models
  const SizetSizetPair& cost1_mdi = costMetadataIndices[form1_index];
  size_t md1_index = cost1_mdi.first, md1_len = cost1_mdi.second,
    md2_index, step1, step2;
  if (step) {
    unsigned short form2 = key.retrieve_model_form(1);
    size_t form2_index = (form2 == USHRT_MAX) ? 0 : (size_t)form2;
    md2_index = costMetadataIndices[form2_index].first;
  }

  using std::isfinite;  Real cost1, cost2;  IntRespMCIter r_cit;
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel
  for (r_cit=allResponses.begin(); r_cit!=allResponses.end(); ++r_cit) {
    const std::vector<RespMetadataT>& md = r_cit->second.metadata();//aggregated

    if (step) { step1 = step - 1; step2 = step; }
    else        step1 = step;

    cost1 = md[md1_index]; // offset by metadata index
    if (isfinite(cost1)) {
      accum_cost[step1] += cost1;
      ++num_cost[step1];
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Metadata:\n" << md << "Model key1 cost: accum_cost = "
	     << accum_cost[step1] << " num_cost = "<<num_cost[step1]<<std::endl;
    }

    if (step) {
      cost2 = md[md1_len + md2_index]; // offset by metadata index
      if (isfinite(cost2)) {
	accum_cost[step2] += cost2;
	++num_cost[step2];
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "Model key2 cost: accum_cost = " << accum_cost[step2]
	       << " num_cost = " << num_cost[step2] << std::endl;
      }
    }
  }
  // averaging is deferred until average_online_cost()
}


/** The version in NonDNonHierarchSampling would be sufficiently general here
    as well, given AGGREGATED_MODELS controlled by the ASV. However, MLMC and
    MLCV MC employ AGGREGATED_MODEL_PAIR without ASV subsetting, so we
    specialize for that case. */
void NonDHierarchSampling::
step_increments(SizetArray& delta_N_l, String prepend)
{
  if (mlmfIter == 0) Cout << "\nPerforming pilot sample for model groups.\n";
  else Cout << "\nSampling iteration " << mlmfIter << ": sample increment =\n"
	    << delta_N_l << '\n';

  // *** TO DO: sufficient for spanning ML batches but not ML + CV ***

  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  size_t num_steps = delta_N_l.size(), form, lev,
    &step = (multilev) ? lev : form;
  if (multilev) form = secondaryIndex;
  else          lev  = secondaryIndex;

  for (step=0; step<num_steps; ++step) {
    numSamples = delta_N_l[step];
    if (numSamples) {
      configure_indices(step, form, lev, sequenceType);
      assign_specification_sequence(step);  // indexed so can skip if no batch
      ensemble_sample_batch(prepend, step); // index is group_id; non-blocking
    }
  }

  if (iteratedModel.asynch_flag())
    synchronize_batches(iteratedModel); // schedule all groups (return ignored)
}

} // namespace Dakota
