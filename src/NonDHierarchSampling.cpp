/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
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
NonDHierarchSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDEnsembleSampling(problem_db, model)
{
  bool err_flag = false;
  /*
  // ensure iteratedModel is an ensemble surrogate model and set initial
  // response mode (for set_communicators() which precedes core_run()).
  // Note: even though hierarchical sampling might involve a single model form,
  // we require an ensemble model to manage aggregations, reductions, etc.
  // (i.e. a SimulationModel with resolution hyper-parameters is insufficient).
  if (iteratedModel.surrogate_type() == "ensemble")
    iteratedModel.surrogate_response_mode(AGGREGATED_MODEL_PAIR);
  else {
    Cerr << "Error: Hierarchical sampling requires an ensemble surrogate "
	 << "model specification." << std::endl;
    err_flag = true;
  }
  */

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
ensemble_sample_batch(const String& prepend, int batch_id, bool new_samples)
{
  if (new_samples) {
    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);

    // export separate output files for each data set:
    // for hierarchical, can rely on active truth,surr keys
    if (exportSampleSets) {
      if (iteratedModel->active_truth_key())
	export_all_samples(prepend, *iteratedModel->active_truth_model(),
			   mlmfIter, batch_id);
      size_t i, num_active_surr = iteratedModel->active_surrogate_keys();
      for (i=0; i<num_active_surr; ++i)
	export_all_samples(prepend, *iteratedModel->active_surrogate_model(i),
			   mlmfIter, batch_id);
    }
  }

  // evaluate all{Samples,Variables} using model ensemble and migrate
  // all{Samples,Variables} to batch{Samples,Variables}Map
  evaluate_batch(*iteratedModel, batch_id); // excludes synchronize
}


void NonDHierarchSampling::
export_all_samples(String root_prepend, const Model& model, size_t iter,
		   int batch_id)
{
  String tabular_filename(root_prepend);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  if (iface_id.empty()) tabular_filename += "NO_ID_i";
  else                  tabular_filename += iface_id + "_i";
  tabular_filename += std::to_string(iter) +  "_b" + std::to_string(batch_id)
                   +  '_' + std::to_string(num_samp) + ".dat";

  NonDEnsembleSampling::export_all_samples(model, tabular_filename);
}

} // namespace Dakota
