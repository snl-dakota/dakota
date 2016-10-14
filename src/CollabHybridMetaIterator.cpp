/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CollabHybridMetaIterator
//- Description: Implementation code for the CollabHybridMetaIterator class
//- Owner:       Patty Hough/John Siirola
//- Checked by:

#include "CollabHybridMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: CollabHybridMetaIterator.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

CollabHybridMetaIterator::CollabHybridMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db), singlePassedModel(false)
  //hybridCollabType(
  //  problem_db.get_string("method.hybrid.collaborative_type"))
{
  const StringArray& method_ptrs
    = problem_db.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = problem_db.get_sa("method.hybrid.method_names");

  if (!method_ptrs.empty())
    { lightwtMethodCtor = false; methodStrings = method_ptrs;  }
  else if (!method_names.empty()) {
    lightwtMethodCtor = true;    methodStrings = method_names;
    modelStrings = problem_db.get_sa("method.hybrid.model_pointers");
    // define an array of null strings to use for set_db_model_nodes()
    if (modelStrings.empty()) modelStrings.resize(method_names.size());
    // allow input of single string
    else      Pecos::inflate_scalar(modelStrings, method_names.size());
  }
  else {
    Cerr << "Error: incomplete hybrid meta-iterator specification."<< std::endl;
    abort_handler(METHOD_ERROR);
  }

  maxIteratorConcurrency = methodStrings.size();
  if (!maxIteratorConcurrency) { // verify at least one method in list
    if (parallelLib.world_rank() == 0) // prior to lead_rank()
      Cerr << "Error: hybrid method list must have a least one entry."
	   << std::endl;
    abort_handler(-1);
  }
}


CollabHybridMetaIterator::
CollabHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model), singlePassedModel(true)
  //hybridCollabType(
  //  problem_db.get_string("method.hybrid.collaborative_type"))
{
  const StringArray& method_ptrs
    = problem_db.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = problem_db.get_sa("method.hybrid.method_names");
  const StringArray& model_ptrs
    = problem_db.get_sa("method.hybrid.model_pointers");

  // process and validate method and model strings
  size_t i, num_iterators; String empty_str;
  if (!method_ptrs.empty()) {
    lightwtMethodCtor = false;
    num_iterators = method_ptrs.size();
    for (i=0; i<num_iterators; ++i)
      check_model(method_ptrs[i], empty_str);
    methodStrings = method_ptrs;
  }
  else if (!method_names.empty()) {
    lightwtMethodCtor = true;
    methodStrings = method_names;
    num_iterators = method_names.size();
    // define an array of strings to use for set_db_model_nodes()
    if (model_ptrs.empty()) // assign array using id from iteratedModel
      modelStrings.assign(num_iterators, iteratedModel.model_id());
    else {
      size_t num_models = model_ptrs.size();
      for (i=0; i<num_models; ++i)
	check_model(empty_str, model_ptrs[i]);
      modelStrings = model_ptrs;
      Pecos::inflate_scalar(modelStrings, num_iterators); // allow single input
    }
  }
  else {
    Cerr << "Error: incomplete hybrid meta-iterator specification."<< std::endl;
    abort_handler(METHOD_ERROR);
  }

  if (!num_iterators) { // verify at least one method in list
    if (parallelLib.world_rank() == 0) // prior to lead_rank()
      Cerr << "Error: hybrid method list must have a least one entry."
	   << std::endl;
    abort_handler(-1);
  }

  maxIteratorConcurrency = num_iterators;
}


CollabHybridMetaIterator::~CollabHybridMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.
}


void CollabHybridMetaIterator::derived_init_communicators(ParLevLIter pl_iter)
{
  size_t i, num_iterators = methodStrings.size();
  selectedIterators.resize(num_iterators); // slaves also need for run_iterator
  if (!singlePassedModel)
    selectedModels.resize(num_iterators);

  iterSched.update(methodPCIter);

  IntIntPair ppi_pr_i, ppi_pr(INT_MAX, 0);
  String empty_str;
  for (i=0; i<num_iterators; ++i) {
    // compute min/max processors per iterator for each method
    Iterator& the_iterator = selectedIterators[i];
    Model& the_model = (singlePassedModel) ? iteratedModel : selectedModels[i];
    ppi_pr_i = (lightwtMethodCtor) ?
      estimate_by_name(methodStrings[i], modelStrings[i], the_iterator,
		       the_model) :
      estimate_by_pointer(methodStrings[i], the_iterator, the_model);
    if (ppi_pr_i.first  < ppi_pr.first)  ppi_pr.first  = ppi_pr_i.first;
    if (ppi_pr_i.second > ppi_pr.second) ppi_pr.second = ppi_pr_i.second;
  }

  iterSched.partition(maxIteratorConcurrency, ppi_pr);
  summaryOutputFlag = iterSched.lead_rank();
  // from this point on, we can specialize logic in terms of iterator servers.
  // An idle partition need not instantiate iterators/models (empty Iterator
  // envelopes are adequate for serve_iterators()), so return now.  A dedicated
  // master processor is managed in IteratorScheduler::init_iterator().
  if (iterSched.iteratorServerId > iterSched.numIteratorServers)
    return;

  // Instantiate all Models and Iterators
  for (i=0; i<num_iterators; ++i) {
    Model& the_model = (singlePassedModel) ? iteratedModel : selectedModels[i];
    if (lightwtMethodCtor)
      allocate_by_name(methodStrings[i], modelStrings[i],
		       selectedIterators[i], the_model);
    else
      allocate_by_pointer(methodStrings[i], selectedIterators[i], the_model);
  }
  // Note: rather than the standard allocate_methods() approach, this is where
  // logic would be placed to process the model list and create
  // CollaborativeModel recursions, one for each unique user-defined model.
  // These CollaborativeModels would then be passed into the selectedIterators
  // instantiations and would support a shared processing queue.
}


void CollabHybridMetaIterator::derived_set_communicators(ParLevLIter pl_iter)
{
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    size_t i, num_iterators = methodStrings.size();
    for (i=0; i<num_iterators; ++i)
      iterSched.set_iterator(selectedIterators[i], si_pl_iter);
  }
}


void CollabHybridMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    size_t i, num_iterators = methodStrings.size();
    for (i=0; i<num_iterators; ++i)
      iterSched.free_iterator(selectedIterators[i], si_pl_iter);
  }

  // deallocate the mi_pl parallelism level
  iterSched.free_iterator_parallelism();
}


void CollabHybridMetaIterator::core_run()
{
  // THIS CODE IS JUST A PLACEHOLDER

  bool lead_rank = iterSched.lead_rank();
  size_t i, num_iterators = methodStrings.size();
  int server_id =  iterSched.iteratorServerId;
  bool    rank0 = (iterSched.iteratorCommRank == 0);
  for (i=0; i<num_iterators; i++) {

    if (lead_rank)
      Cout << "\n>>>>> Running Collaborative Hybrid with iterator "
	   << methodStrings[i] << ".\n";

    Iterator& curr_iterator = selectedIterators[i];

    // For graphics data, limit to iterator server comm leaders; this is
    // further segregated within initialize_graphics(): all iterator masters
    // stream tabular data, but only server 1 generates a graphics window.
    if (rank0 && server_id > 0 && server_id <= iterSched.numIteratorServers)
      curr_iterator.initialize_graphics(server_id);

    iterSched.schedule_iterators(*this, curr_iterator);
  }
}

} // namespace Dakota
