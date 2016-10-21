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
  MetaIterator(problem_db), singlePassedModel(false),
  localSearchProb(problem_db.get_real("method.hybrid.local_search_probability"))
{ maxIteratorConcurrency = 1; }


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model), singlePassedModel(true),
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

  Model& global_model = (singlePassedModel) ? iteratedModel : globalModel;
  Model& local_model  = (singlePassedModel) ? iteratedModel :  localModel;

  iterSched.update(methodPCIter);

  IntIntPair g_ppi_pr = (!global_method_ptr.empty()) ?
    estimate_by_pointer(global_method_ptr, globalIterator, global_model) :
    estimate_by_name(global_method_name, global_model_ptr,
		     globalIterator, global_model);
  IntIntPair l_ppi_pr = (!local_method_ptr.empty()) ?
    estimate_by_pointer(local_method_ptr, localIterator, local_model) :
    estimate_by_name(local_method_name, local_model_ptr,
		     localIterator, local_model);
  IntIntPair ppi_pr(std::min(g_ppi_pr.first,  l_ppi_pr.first),
		    std::max(g_ppi_pr.second, l_ppi_pr.second));

  iterSched.partition(maxIteratorConcurrency, ppi_pr);
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
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.set_iterator(globalIterator, si_pl_iter);
    iterSched.set_iterator(localIterator,  si_pl_iter);
  }
}


void EmbedHybridMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.free_iterator(globalIterator, si_pl_iter);
    iterSched.free_iterator(localIterator,  si_pl_iter);
  }

  // deallocate the mi_pl parallelism level
  iterSched.free_iterator_parallelism();
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
