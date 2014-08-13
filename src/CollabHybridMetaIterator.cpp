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

CollabHybridMetaIterator::
CollabHybridMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db)
  //hybridCollabType(
  //  problem_db.get_string("method.hybrid.collaborative_type"))
{
  const StringArray& method_ptrs
    = problem_db.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = problem_db.get_sa("method.hybrid.method_names");
  if (!method_ptrs.empty())
    { methodList = method_ptrs;  lightwtCtor = false; }
  else if (!method_names.empty())
    { methodList = method_names; lightwtCtor = true;  }

  size_t i, num_iterators = methodList.size();
  if (!num_iterators) { // verify at least one method in list
    if (problem_db.parallel_library().world_rank() == 0)
      Cerr << "Error: hybrid method list must have a least one entry."
	   << std::endl;
    abort_handler(-1);
  }

  maxIteratorConcurrency = num_iterators;

  const String& model_ptr = probDescDB.get_string("method.sub_model_pointer");
  int min_procs_per_iter = INT_MAX, max_procs_per_iter = 0;
  std::pair<int, int> ppi_pr;
  for (i=0; i<num_iterators; ++i) {
    if (lightwtCtor)
      ppi_pr = estimate_by_name(methodList[i], model_ptr,
				selectedIterators[i], iteratedModel);
    else
      ppi_pr = estimate_by_pointer(methodList[i], selectedIterators[i],
				   selectedModels[i]);
    if (ppi_pr.first  < min_procs_per_iter) min_procs_per_iter = ppi_pr.first;
    if (ppi_pr.second > max_procs_per_iter) max_procs_per_iter = ppi_pr.second;
  }

  iterSched.init_iterator_parallelism(maxIteratorConcurrency,
				      min_procs_per_iter, max_procs_per_iter);
  summaryOutputFlag = iterSched.lead_rank();

  // Instantiate all Models and Iterators
  selectedIterators.resize(num_iterators); // slaves also need for run_iterator
  if (lightwtCtor)
    for (i=0; i<num_iterators; ++i)
      allocate_by_name(methodList[i], model_ptr, selectedIterators[i],
		       iteratedModel);
  else {
    selectedModels.resize(num_iterators);
    for (i=0; i<num_iterators; ++i)
      allocate_by_pointer(methodList[i], selectedIterators[i],
			  selectedModels[i]);
  }
  // Note: rather than the standard allocate_methods() approach, this is where
  // logic would be placed to process the model list and create
  // CollaborativeModel recursions, one for each unique user-defined model.
  // These CollaborativeModels would then be passed into the selectedIterators
  // instantiations and would support a shared processing queue.
}


CollabHybridMetaIterator::
CollabHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model), lightwtCtor(true)
  //hybridCollabType(
  //  problem_db.get_string("method.hybrid.collaborative_type"))
{
  // Hard-wired to lightweight methodList instantiation for now.  To support
  // a more general case indicated by the sequential hybrid spec, will need
  // to validate iteratedModel against any model pointers (--> warnings, see
  // SurrBasedLocalMinimizer for example).

  methodList = problem_db.get_sa("method.hybrid.method_names");
  size_t i, num_iterators = methodList.size();
  if (!num_iterators) { // verify at least one method in list
    //if (problem_db.parallel_library().world_rank() == 0)
    Cerr << "Error: hybrid method list must have a least one entry."
	 << std::endl;
    abort_handler(-1);
  }
  selectedIterators.resize(num_iterators); // slaves also need for run_iterator
  String empty_model_ptr; // no need to reassign DB model nodes

  maxIteratorConcurrency = num_iterators;

  int min_procs_per_iter = INT_MAX, max_procs_per_iter = 0;
  std::pair<int, int> ppi_pr;
  for (i=0; i<num_iterators; ++i) {
    ppi_pr = estimate_by_name(methodList[i], empty_model_ptr,
			      selectedIterators[i], iteratedModel);
    if (ppi_pr.first  < min_procs_per_iter) min_procs_per_iter = ppi_pr.first;
    if (ppi_pr.second > max_procs_per_iter) max_procs_per_iter = ppi_pr.second;
  }

  iterSched.init_iterator_parallelism(maxIteratorConcurrency,
				      min_procs_per_iter, max_procs_per_iter);
  summaryOutputFlag = iterSched.lead_rank();

  // Instantiate all Models and Iterators
  for (i=0; i<num_iterators; ++i)
    allocate_by_name(methodList[i], empty_model_ptr,
		     selectedIterators[i], iteratedModel);

  // Note: rather than the standard allocate_methods() approach, this is where
  // logic would be placed to process the model list and create
  // CollaborativeModel recursions, one for each unique user-defined model.
  // These CollaborativeModels would then be passed into the selectedIterators
  // instantiations and would support a shared processing queue.
}


CollabHybridMetaIterator::~CollabHybridMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.

  size_t i, num_iterators = methodList.size();
  if (lightwtCtor)
    for (i=0; i<num_iterators; ++i)
      deallocate(selectedIterators[i], iteratedModel);
  else
    for (i=0; i<num_iterators; ++i)
      deallocate(selectedIterators[i], selectedModels[i]);

  // mi_pl parallelism level is deallocated in ~MetaIterator
}


void CollabHybridMetaIterator::core_run()
{
  // THIS CODE IS JUST A PLACEHOLDER

  bool lead_rank = iterSched.lead_rank();
  size_t i, num_iterators = methodList.size();
  int server_id =  iterSched.iteratorServerId;
  bool    rank0 = (iterSched.iteratorCommRank == 0);
  for (i=0; i<num_iterators; i++) {

    if (lead_rank)
      Cout << "\n>>>>> Running Collaborative Hybrid with iterator "
	   << methodList[i] << ".\n";

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
