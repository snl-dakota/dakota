/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "EmbedHybridMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "IRStore.hpp"
#include "LibraryRuntimeSupport.hpp"
#include "StudyRuntime.hpp"
#include <stdexcept>

static const char rcsId[]="@(#) $Id: EmbedHybridMetaIterator.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

EmbedHybridMetaIterator::EmbedHybridMetaIterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib):
  MetaIterator(problem_db, parallel_lib), singlePassedModel(false),
  localSearchProb(problem_db.get<const Real>("method.hybrid.local_search_probability"))
{ maxIteratorConcurrency = 1; }


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  MetaIterator(problem_db, parallel_lib, model), singlePassedModel(true),
  localSearchProb(problem_db.get<const Real>("method.hybrid.local_search_probability"))
{
  // Ensure consistency between iteratedModel and any method/model pointers
  check_model(problem_db.get<const String>("method.hybrid.global_method_pointer"),
	      problem_db.get<const String>("method.hybrid.global_model_pointer"));
  check_model(problem_db.get<const String>("method.hybrid.local_method_pointer"),
	      problem_db.get<const String>("method.hybrid.local_model_pointer"));

  maxIteratorConcurrency = 1;
}


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(const IRStore& method_store,
                        std::shared_ptr<Iterator> global_iterator,
                        std::shared_ptr<Iterator> local_iterator,
                        std::shared_ptr<StudyServices> services):
  MetaIterator(std::move(services), method_store,
               global_iterator ? global_iterator->iterated_model() : nullptr),
  globalIterator(std::move(global_iterator)),
  localIterator(std::move(local_iterator)), singlePassedModel(false),
  localSearchProb(method_store.get<Real>("hybrid.local_search_probability"))
{
  if (!globalIterator || !localIterator)
    throw std::runtime_error(
      "EmbedHybridMetaIterator requires non-null global and local iterators in "
      "DI construction.");
  if (!globalIterator->iterated_model() || !localIterator->iterated_model())
    throw std::runtime_error(
      "EmbedHybridMetaIterator requires sub_iterator->iterated_model() in DI "
      "construction.");

  detail::validate_services(
    "EmbedHybridMetaIterator", study_services(),
    {detail::runtime_dependency("Iterator", globalIterator),
     detail::runtime_dependency("Iterator", localIterator),
     detail::runtime_dependency("Model", globalIterator->iterated_model()),
     detail::runtime_dependency("Model", localIterator->iterated_model())});

  globalModel = globalIterator->iterated_model();
  localModel  =  localIterator->iterated_model();

  maxIteratorConcurrency = 1;
}


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(const IRStore& method_store,
                        std::shared_ptr<Model> global_model,
                        std::shared_ptr<Model> local_model,
                        std::shared_ptr<StudyServices> services):
  MetaIterator(std::move(services), method_store,
               global_model ? global_model : nullptr),
  singlePassedModel(false),
  localSearchProb(method_store.get<Real>("hybrid.local_search_probability"))
{
  if (!global_model || !local_model)
    throw std::runtime_error(
      "EmbedHybridMetaIterator (name form) requires non-null global and local models.");

  detail::validate_services(
    "EmbedHybridMetaIterator", study_services(),
    {detail::runtime_dependency("Model", global_model),
     detail::runtime_dependency("Model", local_model)});

  const String& global_method_name = method_store.get<String>("hybrid.global_method_name");
  const String& local_method_name  = method_store.get<String>("hybrid.local_method_name");

  if (global_method_name.empty())
    throw std::runtime_error(
      "EmbedHybridMetaIterator (name form) requires hybrid.global_method_name in the IR store.");
  if (local_method_name.empty())
    throw std::runtime_error(
      "EmbedHybridMetaIterator (name form) requires hybrid.local_method_name in the IR store.");

  globalModel    = std::move(global_model);
  localModel     = std::move(local_model);
  globalIterator = Iterator::get_iterator(global_method_name, globalModel);
  localIterator  = Iterator::get_iterator(local_method_name,  localModel);

  maxIteratorConcurrency = 1;
}


EmbedHybridMetaIterator::~EmbedHybridMetaIterator()
{ }


void EmbedHybridMetaIterator::derived_init_communicators(ParLevLIter pl_iter)
{
  // See notes in ConcurrentMetaIterator::derived_init_communicators()

  if (&probDescDB == &dummy_db) { // DI: iterators and models already exist
    iterSched.update(methodPCIter);
    IntIntPair g_ppi_pr = iterSched.configure(globalIterator),
               l_ppi_pr = iterSched.configure(localIterator);
    IntIntPair ppi_pr(std::min(g_ppi_pr.first,  l_ppi_pr.first),
		      std::max(g_ppi_pr.second, l_ppi_pr.second));

    iterSched.partition(maxIteratorConcurrency, ppi_pr);
    summaryOutputFlag = iterSched.lead_rank();
    if (iterSched.idle_partition())
      return;

    iterSched.initialize_iterator(globalIterator->method_string(),
				  globalIterator, globalModel);
    iterSched.initialize_iterator(localIterator->method_string(),
				  localIterator, localModel);
    return;
  }

  const String& global_method_ptr
    = probDescDB.get<const String>("method.hybrid.global_method_pointer");
  const String& global_method_name
    = probDescDB.get<const String>("method.hybrid.global_method_name");
  const String& global_model_ptr
    = probDescDB.get<const String>("method.hybrid.global_model_pointer");

  const String& local_method_ptr
    = probDescDB.get<const String>("method.hybrid.local_method_pointer");
  const String& local_method_name
    = probDescDB.get<const String>("method.hybrid.local_method_name");
  const String& local_model_ptr
    = probDescDB.get<const String>("method.hybrid.local_model_pointer");

  auto& global_model = (singlePassedModel) ? iteratedModel : globalModel;
  auto& local_model  = (singlePassedModel) ? iteratedModel :  localModel;

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
  if (iterSched.idle_partition())
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
  iterSched.set_child_iterators({globalIterator.get(), localIterator.get()},
                                methodPCIter, pl_iter);
}


void EmbedHybridMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  iterSched.free_child_iterators({globalIterator.get(), localIterator.get()},
                                 methodPCIter, pl_iter);
}

IntIntPair EmbedHybridMetaIterator::estimate_partition_bounds()
{
  // Note: EmbedHybridMetaIterator::derived_init_communicators() calls
  // IteratorExecutor::configure() to estimate_partition_bounds() on the
  // subIterator, not the MetaIterator.  When EmbedHybridMetaIterator is a
  // sub-iterator, we augment the subIterator concurrency with the MetaIterator
  // concurrency.  [Thus, this is not redundant with configure().]

  // DI construction injects the sub-iterators, so only the DB path
  // constructs them here
  if (&probDescDB != &dummy_db) {
    const String& global_method_ptr
      = probDescDB.get<const String>("method.hybrid.global_method_pointer");
    const String& global_model_ptr
      = probDescDB.get<const String>("method.hybrid.global_model_pointer");
    const String& local_method_ptr
      = probDescDB.get<const String>("method.hybrid.local_method_pointer");
    const String& local_model_ptr
      = probDescDB.get<const String>("method.hybrid.local_model_pointer");

    auto global_model = (singlePassedModel) ? iteratedModel : globalModel;
    auto local_model  = (singlePassedModel) ? iteratedModel :  localModel;

    iterSched.construct_sub_iterator(probDescDB, parallelLib, globalIterator, global_model,
      global_method_ptr,probDescDB.get<const String>("method.hybrid.global_method_name"),
      global_model_ptr);
    iterSched.construct_sub_iterator(probDescDB, parallelLib, localIterator, local_model,
      local_method_ptr, probDescDB.get<const String>("method.hybrid.local_method_name"),
      local_model_ptr);
  }

  IntIntPair global_min_max = globalIterator->estimate_partition_bounds(),
    local_min_max = localIterator->estimate_partition_bounds(), min_max;
  int min_procs = std::min(global_min_max.first,  local_min_max.first),
      max_procs = std::max(global_min_max.second, local_min_max.second);

  // now apply scheduling data for this level (recursion is complete)
  min_max.first  = ProblemDescDB::min_procs_per_level(min_procs,
    iterSched.procsPerIterator(),	iterSched.numIteratorServers());
  min_max.second = ProblemDescDB::max_procs_per_level(max_procs,
    iterSched.procsPerIterator(), iterSched.numIteratorServers(),
    iterSched.iteratorScheduling(), 1, false, maxIteratorConcurrency);
  return min_max;
}


void EmbedHybridMetaIterator::core_run()
{
  if (iterSched.lead_rank()) {
    Cout << "\n>>>>> Running Embedded Hybrid Minimizer with global method = "
         << globalIterator->method_string() << " and local method = "
	 <<  localIterator->method_string() << std::endl;
    //globalIterator->set_local_search(localIterator);
    //globalIterator->set_local_search_probability(localSearchProb);
  }

  // For graphics data, limit to iterator server comm leaders; this is
  // further segregated within initialize_graphics(): all iterator leaders
  // stream tabular data, but only server 1 generates a graphics window.
  if (iterSched.graphics_server())
    globalIterator->initialize_graphics(iterSched.iteratorServerId());

  iterSched.schedule_iterators(*this, *globalIterator);
}

} // namespace Dakota
