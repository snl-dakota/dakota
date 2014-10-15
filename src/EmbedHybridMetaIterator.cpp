/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EmbedHybridMetaIterator
//- Description: Implementation code for the EmbedHybridMetaIterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "EmbedHybridMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: EmbedHybridMetaIterator.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

EmbedHybridMetaIterator::EmbedHybridMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db),
  localSearchProb(problem_db.get_real("method.hybrid.local_search_probability"))
{ maxIteratorConcurrency = 1; }


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model),
  localSearchProb(problem_db.get_real("method.hybrid.local_search_probability"))
{
  // Ensure consistency between iteratedModel and any method/model pointers
  check_model(problem_db.get_string("method.hybrid.global_method_pointer"),
	      problem_db.get_string("method.hybrid.global_model_pointer"));
  check_model(problem_db.get_string("method.hybrid.local_method_pointer"),
	      problem_db.get_string("method.hybrid.local_model_pointer"));

  maxIteratorConcurrency = 1;
}


EmbedHybridMetaIterator::~EmbedHybridMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.
}


void EmbedHybridMetaIterator::derived_init_communicators(ParLevLIter pl_iter)
{
  // See notes in ConcurrentMetaIterator::derived_init_communicators()

  const String& global_method_ptr
    = probDescDB.get_string("method.hybrid.global_method_pointer");
  const String& global_method_name
    = probDescDB.get_string("method.hybrid.global_method_name");
  const String& global_model_ptr
    = probDescDB.get_string("method.hybrid.global_model_pointer");

  const String& local_method_ptr
    = probDescDB.get_string("method.hybrid.local_method_pointer");
  const String& local_method_name
    = probDescDB.get_string("method.hybrid.local_method_name");
  const String& local_model_ptr
    = probDescDB.get_string("method.hybrid.local_model_pointer");

  Model& global_model = (new_model(global_method_ptr, global_model_ptr)) ?
    globalModel : iteratedModel;
  Model& local_model  = (new_model(local_method_ptr, local_model_ptr)) ?
    localModel  : iteratedModel;

  std::pair<int, int> g_ppi = (!global_method_ptr.empty()) ?
    estimate_by_pointer(global_method_ptr, globalIterator, global_model) :
    estimate_by_name(global_method_name, global_model_ptr,
		     globalIterator, global_model);
  std::pair<int, int> l_ppi = (!local_method_ptr.empty()) ?
    estimate_by_pointer(local_method_ptr, localIterator, local_model) :
    estimate_by_name(local_method_name, local_model_ptr,
		     localIterator, local_model);

  iterSched.init_iterator_parallelism(maxIteratorConcurrency,
				      std::min(g_ppi.first,  l_ppi.first),
				      std::max(g_ppi.second, l_ppi.second));
  size_t pl_index = parallelLib.parallel_level_index(pl_iter);
  miPLIndexMap[pl_index] = iterSched.miPLIndex;
  summaryOutputFlag = iterSched.lead_rank();
  if (iterSched.iteratorServerId > iterSched.numIteratorServers)
    return;

  if (!global_method_ptr.empty())
    allocate_by_pointer(global_method_ptr, globalIterator, global_model);
  else
    allocate_by_name(global_method_name, global_model_ptr,
		     globalIterator, global_model);
  if (!local_method_ptr.empty())
    allocate_by_pointer(local_method_ptr, localIterator, local_model);
  else
    allocate_by_name(local_method_name, local_model_ptr,
		     localIterator, local_model);
}


void EmbedHybridMetaIterator::derived_set_communicators(ParLevLIter pl_iter)
{
  // free the communicators for selectedIterators
  size_t pl_index = parallelLib.parallel_level_index(pl_iter),
      mi_pl_index = miPLIndexMap[pl_index]; // same or one beyond pl_iter
  iterSched.update(mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.set_iterator(globalIterator, si_pl_iter);
    iterSched.set_iterator(localIterator,  si_pl_iter);
  }

  // See notes in NestedModel::derived_set_communicators() for reasons why
  // a streamlined implementation (no miPLIndexMap) is insufficient.
}


void EmbedHybridMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  // free the communicators for globalIterator and localIterator
  size_t pl_index = parallelLib.parallel_level_index(pl_iter),
      mi_pl_index = miPLIndexMap[pl_index]; // same or one beyond pl_iter
  iterSched.update(mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.free_iterator(globalIterator, si_pl_iter);
    iterSched.free_iterator(localIterator,  si_pl_iter);
  }
  // See notes in NestedModel::derived_set_communicators() for reasons why
  // a streamlined implementation (no miPLIndexMap) is insufficient.

  // deallocate the mi_pl parallelism level
  iterSched.free_iterator_parallelism();

  miPLIndexMap.erase(pl_index);
}


void EmbedHybridMetaIterator::core_run()
{
  if (iterSched.lead_rank()) {
    Cout << "\n>>>>> Running Embedded Hybrid Minimizer with global method = "
         << globalIterator.method_string() << " and local method = "
	 <<  localIterator.method_string() << std::endl;
    //globalIterator.set_local_search(localIterator);
    //globalIterator.set_local_search_probability(localSearchProb);
  }

  // For graphics data, limit to iterator server comm leaders; this is
  // further segregated within initialize_graphics(): all iterator masters
  // stream tabular data, but only server 1 generates a graphics window.
  int server_id = iterSched.iteratorServerId;
  if (iterSched.iteratorCommRank == 0 && server_id > 0 &&
      server_id <= iterSched.numIteratorServers)
    globalIterator.initialize_graphics(server_id);

  iterSched.schedule_iterators(*this, globalIterator);
}

} // namespace Dakota
